import { invoke } from "@tauri-apps/api/core";
import { DeckConfig, emptyConfig } from "./types";

// Wraps the Rust-side commands in src-tauri/src/lib.rs. The config lives at a
// fixed file (deck-config.json) in the app's data dir - no file picker yet,
// matching the "single config the app owns" model from the protocol spec.

export async function loadConfig(): Promise<DeckConfig> {
  const json = await invoke<string | null>("load_config");
  if (!json) return emptyConfig();
  return JSON.parse(json) as DeckConfig;
}

export async function saveConfig(cfg: DeckConfig): Promise<void> {
  await invoke("save_config", { json: JSON.stringify(cfg, null, 2) });
}

export async function configPath(): Promise<string> {
  return invoke<string>("config_path_string");
}
