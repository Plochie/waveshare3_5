use axum::{
    extract::{
        ws::{Message, WebSocket, WebSocketUpgrade},
        State,
    },
    response::IntoResponse,
    routing::get,
    Router,
};
use futures_util::{SinkExt, StreamExt};
use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use std::collections::{HashMap, HashSet};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::Arc;
use std::time::Duration;
use tauri::{AppHandle, Emitter};
use tokio::net::TcpListener;
use tokio::sync::{mpsc, Mutex};

// Implements the device-facing side of
// docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md §3: this app
// is the WS server, the ESP32 deck is the client. Handshake/auth is gated on
// the config's agent.token (§3.1); config push/ack is §3.2. Icon transfer
// (§3.3) and host-step exec (§3.4) are not wired yet (Phase 2c/2d/3).

#[derive(Clone, Serialize)]
pub struct DeviceInfo {
    pub conn_id: u64,
    pub device_id: String,
    pub fw: String,
    pub authed: bool,
}

struct Client {
    tx: mpsc::UnboundedSender<Message>,
    device_id: String,
    fw: String,
    known_icons: HashSet<String>,
}

pub struct ServerState {
    pub config: Mutex<Option<Value>>,
    rev: AtomicU64,
    clients: Mutex<HashMap<u64, Client>>,
    next_id: AtomicU64,
    app: AppHandle,
}

impl ServerState {
    pub fn new(app: AppHandle) -> Self {
        Self {
            config: Mutex::new(None),
            rev: AtomicU64::new(0),
            clients: Mutex::new(HashMap::new()),
            next_id: AtomicU64::new(1),
            app,
        }
    }
}

pub type SharedState = Arc<ServerState>;

#[derive(Deserialize)]
#[serde(tag = "t")]
enum InMsg {
    #[serde(rename = "hello")]
    Hello {
        device_id: String,
        fw: String,
        token: String,
    },
    #[serde(rename = "config_ack")]
    ConfigAck { rev: u64 },
    #[serde(rename = "have_icons")]
    HaveIcons { names: Vec<String> },
    #[serde(rename = "result")]
    Result {
        id: u64,
        ok: bool,
        error: Option<String>,
    },
    #[serde(rename = "exec")]
    Exec { id: u64, step: Value },
    #[serde(other)]
    Unknown,
}

pub async fn run(app: AppHandle, state: SharedState) {
    let listener = match TcpListener::bind("0.0.0.0:0").await {
        Ok(l) => l,
        Err(e) => {
            eprintln!("ws_server: failed to bind: {e}");
            return;
        }
    };
    let port = listener.local_addr().map(|a| a.port()).unwrap_or(0);
    println!("ws_server: listening on port {port}");

    crate::mdns::advertise(port);

    let router = Router::new().route("/ws", get(ws_handler)).with_state(state.clone());
    let _ = app; // app handle is reachable via state.app for events
    if let Err(e) = axum::serve(listener, router).await {
        eprintln!("ws_server: crashed: {e}");
    }
}

async fn ws_handler(ws: WebSocketUpgrade, State(state): State<SharedState>) -> impl IntoResponse {
    ws.on_upgrade(move |socket| handle_socket(socket, state))
}

async fn handle_socket(socket: WebSocket, state: SharedState) {
    let (mut sender, mut receiver) = socket.split();
    let (tx, mut rx) = mpsc::unbounded_channel::<Message>();
    let conn_id = state.next_id.fetch_add(1, Ordering::SeqCst);

    let pump = tokio::spawn(async move {
        while let Some(msg) = rx.recv().await {
            if sender.send(msg).await.is_err() {
                break;
            }
        }
    });

    // §3.1: first message after connect must be `hello`; nothing else is
    // honored before auth_ok.
    let hello = tokio::time::timeout(Duration::from_secs(5), receiver.next()).await;
    let (device_id, fw) = match hello {
        Ok(Some(Ok(Message::Text(text)))) => match serde_json::from_str::<InMsg>(&text) {
            Ok(InMsg::Hello { device_id, fw, token }) => {
                let expected = expected_token(&state).await;
                if expected.is_empty() || token != expected {
                    let _ = tx.send(auth_fail("bad token"));
                    let _ = tx.send(Message::Close(None));
                    drop(tx);
                    let _ = pump.await;
                    return;
                }
                (device_id, fw)
            }
            _ => {
                let _ = tx.send(auth_fail("expected hello"));
                let _ = tx.send(Message::Close(None));
                drop(tx);
                let _ = pump.await;
                return;
            }
        },
        _ => {
            drop(tx);
            let _ = pump.await;
            return;
        }
    };

    state.clients.lock().await.insert(
        conn_id,
        Client {
            tx: tx.clone(),
            device_id: device_id.clone(),
            fw: fw.clone(),
            known_icons: HashSet::new(),
        },
    );
    let _ = tx.send(Message::Text(json!({"t": "auth_ok"}).to_string()));
    emit_devices(&state).await;
    push_config_to(&tx, &state).await;

    while let Some(Ok(msg)) = receiver.next().await {
        let Message::Text(text) = msg else { continue };
        match serde_json::from_str::<InMsg>(&text) {
            Ok(InMsg::ConfigAck { rev }) => {
                let _ = state
                    .app
                    .emit("deck-config-ack", json!({ "conn_id": conn_id, "rev": rev }));
            }
            Ok(InMsg::HaveIcons { names }) => {
                let _ = state
                    .app
                    .emit("deck-have-icons", json!({ "conn_id": conn_id, "names": names }));
                let known: HashSet<String> = names.into_iter().collect();
                if let Some(c) = state.clients.lock().await.get_mut(&conn_id) {
                    c.known_icons = known.clone();
                }
                let cfg = state.config.lock().await.clone();
                if let Some(cfg) = cfg {
                    push_missing_icons(&state.app, &tx, &known, &cfg);
                }
            }
            Ok(InMsg::Result { id, ok, error }) => {
                let _ = state.app.emit(
                    "deck-exec-result",
                    json!({ "conn_id": conn_id, "id": id, "ok": ok, "error": error }),
                );
            }
            Ok(InMsg::Exec { id, step }) => {
                // §3.4: device -> app. This connection already passed the
                // hello/token check above, so it's implicitly authenticated -
                // no extra gating needed before running the step. enigo +
                // process spawning can block briefly, so run it off the
                // async runtime's worker threads.
                let tx2 = tx.clone();
                tokio::task::spawn_blocking(move || {
                    let msg = match crate::host_actions::execute_step(&step) {
                        Ok(()) => json!({"t": "result", "id": id, "ok": true}).to_string(),
                        Err(e) => {
                            json!({"t": "result", "id": id, "ok": false, "error": e}).to_string()
                        }
                    };
                    let _ = tx2.send(Message::Text(msg));
                });
            }
            Ok(InMsg::Hello { .. }) | Ok(InMsg::Unknown) | Err(_) => {}
        }
    }

    state.clients.lock().await.remove(&conn_id);
    emit_devices(&state).await;
    drop(tx);
    let _ = pump.await;
}

fn auth_fail(reason: &str) -> Message {
    Message::Text(json!({"t": "auth_fail", "reason": reason}).to_string())
}

async fn expected_token(state: &SharedState) -> String {
    state
        .config
        .lock()
        .await
        .as_ref()
        .and_then(|c| c.get("agent"))
        .and_then(|a| a.get("token"))
        .and_then(|t| t.as_str())
        .unwrap_or("")
        .to_string()
}

// Sends icon_begin/binary/icon_end for every icon `config` references that
// isn't already in `known` (protocol §3.3). Best-effort: failures to read an
// icon file are silently skipped (the device just keeps showing label-only).
fn push_missing_icons(
    app: &AppHandle,
    tx: &mpsc::UnboundedSender<Message>,
    known: &HashSet<String>,
    config: &Value,
) {
    for name in crate::icons::icons_referenced(config) {
        if known.contains(&name) {
            continue;
        }
        if let Some((w, h, bytes)) = crate::icons::read_icon(app, &name) {
            let _ = tx.send(Message::Text(
                json!({"t": "icon_begin", "name": name, "bytes": bytes.len(), "w": w, "h": h}).to_string(),
            ));
            let _ = tx.send(Message::Binary(bytes));
            let _ = tx.send(Message::Text(json!({"t": "icon_end", "name": name}).to_string()));
        }
    }
}

async fn push_config_to(tx: &mpsc::UnboundedSender<Message>, state: &SharedState) {
    let cfg = state.config.lock().await.clone();
    if let Some(cfg) = cfg {
        let rev = state.rev.load(Ordering::SeqCst);
        let _ = tx.send(Message::Text(json!({"t": "config", "config": cfg, "rev": rev}).to_string()));
    }
}

async fn emit_devices(state: &SharedState) {
    let devices: Vec<DeviceInfo> = state
        .clients
        .lock()
        .await
        .iter()
        .map(|(id, c)| DeviceInfo {
            conn_id: *id,
            device_id: c.device_id.clone(),
            fw: c.fw.clone(),
            authed: true,
        })
        .collect();
    let _ = state.app.emit("deck-devices", devices);
}

// Called by the save_config command whenever the desktop app's config
// changes; bumps rev and pushes the new config to every connected device.
pub async fn broadcast_config(state: &SharedState, config: Value) {
    *state.config.lock().await = Some(config.clone());
    let rev = state.rev.fetch_add(1, Ordering::SeqCst) + 1;
    let msg = json!({"t": "config", "config": config, "rev": rev}).to_string();
    let clients = state.clients.lock().await;
    for c in clients.values() {
        let _ = c.tx.send(Message::Text(msg.clone()));
        push_missing_icons(&state.app, &c.tx, &c.known_icons, &config);
    }
}

#[tauri::command]
pub async fn list_devices(state: tauri::State<'_, SharedState>) -> Result<Vec<DeviceInfo>, String> {
    Ok(state
        .clients
        .lock()
        .await
        .iter()
        .map(|(id, c)| DeviceInfo {
            conn_id: *id,
            device_id: c.device_id.clone(),
            fw: c.fw.clone(),
            authed: true,
        })
        .collect())
}
