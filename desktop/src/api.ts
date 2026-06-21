import { invoke } from "@tauri-apps/api/core";
import { DeckConfig, emptyConfig } from "./types";

export interface DeviceInfo {
  device_id: string;
  name: string;
  fw: string;
  online: boolean;
}

export async function confirmPairing(deviceId: string, name: string): Promise<void> {
  await invoke("confirm_pairing", { deviceId, name });
}

export async function rejectPairing(deviceId: string): Promise<void> {
  await invoke("reject_pairing", { deviceId });
}

export async function forgetDevice(deviceId: string): Promise<void> {
  await invoke("forget_device", { deviceId });
}

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

export async function listDevices(): Promise<DeviceInfo[]> {
  return invoke<DeviceInfo[]>("list_devices");
}

export interface IconMeta {
  name: string;
  w: number;
  h: number;
  preview: string; // data: URL, ready to drop into an <img src>
}

// Resizes/converts the PNG at `path` to RGB565 and stores it under the app's
// icons dir; returns the stored filename (for DeckButton.icon) + a preview.
export async function importIcon(path: string): Promise<IconMeta> {
  return invoke<IconMeta>("import_icon", { path });
}

// Re-renders a stored icon back to a PNG data URL for display, or null if
// `name` isn't a known icon (e.g. referenced but never imported on this machine).
export async function iconPreview(name: string): Promise<string | null> {
  return invoke<string | null>("icon_preview", { name });
}

export async function setState(key: string, value: string): Promise<void> {
  await invoke("set_state", { key, value });
}

export async function listState(): Promise<Record<string, string>> {
  return invoke<Record<string, string>>("list_state");
}
