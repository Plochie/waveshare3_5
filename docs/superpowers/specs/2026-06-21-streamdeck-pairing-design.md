# WiFi Stream Deck — Per-Device Pairing UX

A redesign of the pairing flow for the WiFi Stream Deck, replacing the single
shared `agent.token` + hidden text field with **per-device tokens**, an
explicit **code-confirm** handshake, and dedicated pairing UI on both the
desktop app and the device touchscreen.

Companion to
[2026-06-19-wifi-streamdeck-design.md](2026-06-19-wifi-streamdeck-design.md)
(system design) and
[2026-06-19-wifi-streamdeck-protocol.md](2026-06-19-wifi-streamdeck-protocol.md)
(wire contract — this spec adds messages to its §3.1). Phases 1–3 (firmware
direct-network steps, desktop sync, host actions) are already shipped; this is
a UX/security refinement layered on top.

## Motivation

The shipped pairing flow is unintuitive and weak:

- The pairing secret is a raw text field buried in the desktop TopBar, with a
  "generate" dice button the user has to discover.
- There is **one shared token** for all devices — you cannot revoke a single
  device without locking out every device.
- First-time pairing originally required hand-editing `config.json` on the SD
  card; the interim fix (silently trusting the first device to connect while no
  token is set) removed the manual step but added no confirmation — any device
  on the LAN could grab the open slot.

Per the design doc's own security section, `run_command` / `launch_app` make an
unauthenticated LAN server a remote-code-execution risk, so pairing must be both
**deliberate** (a human confirms each device) and **per-device** (revocable
individually).

## 1. Data model: identity vs. synced config

The driving constraint: **secrets must never be broadcast**. Device A must not
receive device B's token. So device identity/secrets are split from the synced
config.

### Desktop canonical config (`deck-config.json`, app data dir)

The `agent` block becomes a paired-devices registry:

```jsonc
"agent": {
  "devices": [
    {
      "device_id": "esp32-aabbcc",   // unique key (device-reported, from MAC)
      "name": "Desk Deck",            // user-assigned label, optional
      "token": "<32-char secret>",    // long-lived per-device auth secret
      "paired_at": 1719000000         // unix seconds, for display/sorting
    }
  ]
}
```

The legacy `"agent": { "token": "..." }` shape is **removed**. There is no
migration path written for it because no real deployment depends on it yet (the
only existing pairing was a dev test); a desktop config carrying the old shape
simply starts with an empty `devices` list and re-pairs.

### Broadcast config (over the wire to devices)

`broadcast_config` / `push_config_to` send the full config **minus the `agent`
block**. The device never sees the registry or any token but its own. `ha`
(base_url + token) is still broadcast — devices need it for `ha_*` steps; only
`agent` is withheld.

### Device side

Each device stores **only its own** token, in a new file separate from the
synced config (which is overwritten on every push):

```jsonc
// /deck/pairing.json
{ "device_id": "esp32-aabbcc", "token": "<32-char secret>" }
```

`config.json` on the device no longer carries any `agent` field.
`deck_client::load_agent_token()` reads from `pairing.json` instead of parsing
the config. A device with no `pairing.json` (or an empty token) is "unpaired".

## 2. Protocol: pairing handshake

Extends protocol §3.1. The **device generates a 6-digit code** (proving the
operator has physical sight of the device's screen); the **desktop mints the
token** (the authority owns the long-lived secret). The code is a
human-verified value, not a secret — it is never used to derive the token.

```jsonc
// 1. device → app (unchanged shape; token from pairing.json, "" if unpaired)
{ "t": "hello", "device_id": "esp32-aabbcc", "fw": "1.0.0", "token": "" }

// 2a. app → device — token matched a registry entry: authenticated
{ "t": "auth_ok" }

// 2b. app → device — no registry match: device must pair
{ "t": "pair_required" }

// 3. device → app — device generates+shows a 6-digit code, sends it up
{ "t": "pair_request", "device_id": "esp32-aabbcc", "code": "482917" }

// 4a. app → device — operator confirmed: app mints + stores the token
{ "t": "paired", "token": "<32-char secret>" }
//    device writes pairing.json, shows success, becomes authed;
//    app then pushes config + icons as normal (§3.2/§3.3)

// 4b. app → device — operator rejected
{ "t": "pair_rejected" }
//    device shows "Rejected"; app closes the socket
```

Auth check on the desktop (`hello` handler): look up `device_id` in the
registry; authenticate only if found **and** the presented token matches that
entry's token. Any other case (unknown device, empty token, or token mismatch)
→ `pair_required`. The old "empty expected token ⇒ trust anyone" bootstrap is
removed; pairing always goes through explicit confirm now.

On `pair_request`, the desktop emits a Tauri event `deck-pair-request`
(`{ device_id, code }`) that drives the confirm modal (§3). The operator
verifies the modal's code equals the code on the device screen, then confirms —
**that visual match is the security boundary.** An attacker's device on the LAN
would display a code the operator never sees, so its request is rejected.

The protocol's forward-compat rule (§3.5) still holds: a device that receives an
unrecognized `t` ignores it.

## 3. Desktop UI

### A. Pair-request modal (`PairModal.tsx`, new)

Shown when `deck-pair-request` fires; overlays the app:

```
┌─────────────────────────────────────┐
│  New device wants to pair            │
│                                      │
│  esp32-aabbcc                        │
│                                      │
│  Confirm this code matches the one   │
│  shown on the device screen:         │
│                                      │
│        4 8 2 9 1 7                    │
│                                      │
│  Name (optional): [ Desk Deck      ] │
│                                      │
│      [ Reject ]        [ Pair ]      │
└─────────────────────────────────────┘
```

- **Pair** → `confirm_pairing(device_id, name)` command.
- **Reject** → `reject_pairing(device_id)` command.
- The modal auto-dismisses if the device disconnects mid-prompt (a
  `deck-devices` update no longer lists that `conn_id`/`device_id`).

### B. Paired Devices panel (rework of `DevicesPanel.tsx`)

Today it lists only live connections. The new panel lists the **registry**
(every paired device, online or not), joined with live connection state:

```
Paired Devices
● Desk Deck      esp32-aabbcc   fw 1.0.0   online    [ Forget ]
○ Couch Deck     esp32-ddeeff   —          offline   [ Forget ]
```

- Filled dot = currently connected; hollow = paired but offline.
- **Forget** → `forget_device(device_id)`: removes the registry entry and, if
  the device is connected, closes its socket so it immediately drops to
  unpaired (and re-pairs on its next reconnect).

### C. TopBar

The "Agent pairing token" text field and its dice/generate button are
**removed**. HA base_url/token fields stay. Pairing is driven entirely by the
modal + panel.

### New Rust commands

- `confirm_pairing(device_id, name)` — mint a 32-char token, insert/replace the
  registry entry, persist the desktop config, send `paired` to that device's
  socket, then push config + icons.
- `reject_pairing(device_id)` — send `pair_rejected`, close that socket.
- `forget_device(device_id)` — remove the registry entry, persist, close the
  socket if connected.
- `list_devices` (extended) — returns the registry merged with live connection
  state (`online` bool + `fw` for connected ones), so the panel can render
  offline-but-paired devices too.

## 4. Device UI (LVGL)

### A. Pairing screen (`src/apps/deck/deck_pairing.{h,cpp}`, new)

A full-screen screen built with `ui/styles.h` colors, following the `deck.cpp`
style. Shows the device-generated code large and centered:

```
        Pairing

   ┌───────────────────┐
   │   4 8 2 9 1 7     │
   └───────────────────┘

  Confirm this code on your
   computer to pair this deck.

      ⟳ waiting…
```

- On `paired` → swaps to a "✓ Paired" success state briefly.
- On `pair_rejected` → "✗ Rejected".
- Exposes `lv_obj_t *deck_pairing_create()` returning a fresh screen for
  `screen_manager::push()`, matching the app-screen convention.

### B. `deck_client` pairing state machine

`deck_client` gains a small state machine readable from the LVGL thread:

- On `pair_required`: generate a 6-digit code, send `pair_request`, transition
  to `pending`.
- On `paired`: write `/deck/pairing.json`, set state `success`, set
  `s_authed = true`; the normal config push follows.
- On `pair_rejected`: set state `rejected`.
- API:
  - `struct pairing_status { int state; char code[7]; }` with
    `pairing_status pairing_state()` (states: `idle`, `pending`, `success`,
    `rejected`).
  - `bool consume_pairing_changed()` — edge signal (true once per transition)
    so the UI thread knows when to push/pop the pairing screen.

The code is generated with the ESP32 hardware RNG (`esp_random()`), formatted as
6 zero-padded decimal digits.

### C. Auto-switch via `screen_manager`

`main.cpp`'s loop already polls `consume_config_pushed()`. It also polls the
pairing edge signal:

- transition into `pending` → `screen_manager::push(deck_pairing_create())` over
  whatever screen is open, so the code surfaces itself without the user knowing
  to open the Deck app.
- transition to `success` / `rejected` / `idle` → pop the pairing screen (after
  the brief success/reject display).

This keeps pairing-UI orchestration in the same place as the existing config
poll, rather than scattering it across app screens.

## 5. Error handling & edge cases

- **Concurrent pairings** — keyed by `device_id` (registry) and `conn_id`
  (live socket); multiple confirm modals can stack, each resolving its own
  device.
- **Device disconnects mid-prompt** — desktop auto-dismisses the modal; the
  device re-generates and re-shows a code on reconnect.
- **Forget a connected device** — registry entry removed + socket closed; the
  device drops to unpaired and re-pairs on its next reconnect (self-healing).
- **Stale token** (desktop forgot a device, but the device still holds an old
  `pairing.json`) — `hello` token won't match any registry entry →
  `pair_required` → re-pair. Self-healing; no manual cleanup needed.
- **Duplicate `name`** — allowed; `device_id` is the unique key, `name` is
  purely a display label.
- **Code collision** between two simultaneously-pairing devices — harmless: the
  code is verification-only (eyeballed by the operator), never a secret, and
  each request carries its own `device_id`.
- **`pairing.json` write fails** (no SD / IO error) — device logs the failure
  and stays unpaired; it re-pairs on the next connect rather than entering a
  half-paired state.

## 6. Testing

- **First pair:** fresh device (no `pairing.json`) connects → device shows code,
  desktop modal shows the same code → Pair → device persists token, shows
  paired, grid loads. Power-cycle → reconnects with stored token → `auth_ok`,
  no prompt.
- **Reject:** connect an unpaired device → Reject on desktop → device shows
  "Rejected", does not auth.
- **Forget + re-pair:** Forget a paired device → its tile leaves the panel (or
  goes offline); reconnect → it re-enters the pairing flow.
- **Two devices:** pair a second device with a distinct `device_id` → both
  appear in the panel with independent Forget buttons; forgetting one leaves the
  other authenticated.
- **Stale token:** manually clear the desktop registry while a device holds a
  token → device reconnect → `pair_required` → re-pair succeeds.
- **Auto-switch:** with a non-Deck app open on the device, trigger pairing →
  the pairing screen pushes itself to the foreground; on resolve it pops back.

## Out of scope

- Command allowlist for `run_command` / `launch_app` (a separate later
  hardening item).
- TLS / encryption of tokens at rest — still Phase 4 (the device is a
  local-only, plaintext-config appliance per the original design).
- Renaming a paired device after the fact (only set-at-pair-time for now; YAGNI
  until asked).
