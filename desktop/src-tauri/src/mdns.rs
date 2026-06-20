use mdns_sd::{ServiceDaemon, ServiceInfo};
use std::collections::HashMap;

// Advertises the WS server per the protocol spec (§3): service
// `_deckhost._tcp`, TXT `ws_path` so the device can discover host:port
// without a fixed config.
pub fn advertise(port: u16) {
    let daemon = match ServiceDaemon::new() {
        Ok(d) => d,
        Err(e) => {
            eprintln!("mDNS daemon failed to start: {e}");
            return;
        }
    };

    let mut properties = HashMap::new();
    properties.insert("ws_path".to_string(), "/ws".to_string());

    let service_info = match ServiceInfo::new(
        "_deckhost._tcp.local.",
        "deck-configurator",
        "deck-configurator.local.",
        "", // no fixed IP: enable_addr_auto() below fills these from local interfaces
        port,
        Some(properties),
    ) {
        Ok(info) => info.enable_addr_auto(),
        Err(e) => {
            eprintln!("mDNS service info failed: {e}");
            return;
        }
    };

    if let Err(e) = daemon.register(service_info) {
        eprintln!("mDNS register failed: {e}");
    }

    // The daemon must outlive the app; it has no natural owner in our
    // setup() flow, so it's intentionally leaked for the process lifetime.
    std::mem::forget(daemon);
}
