# Deck Configurator

Tauri (Rust + React/TypeScript) desktop app for designing the button/page
layout used by the ESP32 WiFi Stream Deck firmware (`src/apps/deck/`).
Edits a config matching
`docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md` §1, persisted
locally to the app's data dir as `deck-config.json` (Phase 2a). Pushing the
config to a paired device over WebSocket is a later phase.

## Develop

```
npm install
npm run tauri dev
```

## Recommended IDE Setup

- [VS Code](https://code.visualstudio.com/) + [Tauri](https://marketplace.visualstudio.com/items?itemName=tauri-apps.tauri-vscode) + [rust-analyzer](https://marketplace.visualstudio.com/items?itemName=rust-lang.rust-analyzer)
