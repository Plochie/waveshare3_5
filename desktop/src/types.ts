// Mirrors docs/superpowers/specs/2026-06-19-wifi-streamdeck-protocol.md §1-2.
// This is the canonical config JSON shape pushed to the device.

export type StepType =
  | "http_request"
  | "ha_service"
  | "ha_webhook"
  | "mqtt_publish"
  | "hotkey"
  | "type_text"
  | "launch_app"
  | "run_command"
  | "media_key"
  | "delay";

export const STEP_TYPES: StepType[] = [
  "http_request",
  "ha_service",
  "ha_webhook",
  "mqtt_publish",
  "hotkey",
  "type_text",
  "launch_app",
  "run_command",
  "media_key",
  "delay",
];

export const HTTP_METHODS = ["GET", "POST", "PUT", "DELETE"] as const;
export type HttpMethod = (typeof HTTP_METHODS)[number];

export const MEDIA_KEYS = ["play_pause", "next", "prev", "vol_up", "vol_down", "mute"] as const;
export type MediaKey = (typeof MEDIA_KEYS)[number];

export interface Step {
  type: StepType;
  // http_request
  method?: HttpMethod;
  url?: string;
  headers?: Record<string, string>;
  body?: string;
  // ha_service
  domain?: string;
  service?: string;
  data?: Record<string, string>;
  // ha_webhook
  id?: string;
  // mqtt_publish
  topic?: string;
  payload?: string;
  // hotkey
  keys?: string[];
  // type_text
  text?: string;
  // launch_app / run_command
  target?: string;
  command?: string;
  // media_key
  key?: MediaKey;
  // delay
  ms?: number;
}

export interface DeckButton {
  pos: number;
  label: string;
  icon?: string;
  color?: string;
  open_page?: string;
  steps?: Step[];
}

export interface DeckPage {
  id: string;
  title: string;
  buttons: DeckButton[];
}

export interface DeckConfig {
  version: number;
  grid: { cols: number; rows: number };
  agent?: { token: string };
  ha?: { base_url: string; token: string };
  pages: DeckPage[];
}

export function defaultStepForType(type: StepType): Step {
  switch (type) {
    case "http_request":
      return { type, method: "GET", url: "" };
    case "ha_service":
      return { type, domain: "", service: "", data: {} };
    case "ha_webhook":
      return { type, id: "" };
    case "mqtt_publish":
      return { type, topic: "", payload: "" };
    case "hotkey":
      return { type, keys: [] };
    case "type_text":
      return { type, text: "" };
    case "launch_app":
      return { type, target: "" };
    case "run_command":
      return { type, command: "" };
    case "media_key":
      return { type, key: "play_pause" };
    case "delay":
      return { type, ms: 1000 };
  }
}

export function emptyConfig(): DeckConfig {
  return {
    version: 1,
    grid: { cols: 3, rows: 4 },
    agent: { token: "" },
    ha: { base_url: "", token: "" },
    pages: [{ id: "home", title: "Home", buttons: [] }],
  };
}
