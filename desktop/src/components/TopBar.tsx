import { DeckConfig } from "../types";

interface Props {
  config: DeckConfig;
  onChange: (next: DeckConfig) => void;
  onSave: () => void;
  dirty: boolean;
  saving: boolean;
  configPath: string;
}

export default function TopBar({ config, onChange, onSave, dirty, saving, configPath }: Props) {
  return (
    <div className="top-bar">
      <div className="top-bar-title">
        <h1>Deck Configurator</h1>
        <span className="config-path" title={configPath}>
          {configPath}
        </span>
      </div>

      <div className="top-bar-fields">
        <label>
          Grid
          <span className="grid-dims">
            <input
              type="number"
              min={1}
              max={8}
              value={config.grid.cols}
              onChange={(e) =>
                onChange({ ...config, grid: { ...config.grid, cols: Number(e.target.value) } })
              }
            />
            ×
            <input
              type="number"
              min={1}
              max={8}
              value={config.grid.rows}
              onChange={(e) =>
                onChange({ ...config, grid: { ...config.grid, rows: Number(e.target.value) } })
              }
            />
          </span>
        </label>

        <label>
          HA base URL
          <input
            value={config.ha?.base_url ?? ""}
            onChange={(e) => onChange({ ...config, ha: { base_url: e.target.value, token: config.ha?.token ?? "" } })}
            placeholder="http://homeassistant.local:8123"
          />
        </label>

        <label>
          HA token
          <input
            type="password"
            value={config.ha?.token ?? ""}
            onChange={(e) => onChange({ ...config, ha: { base_url: config.ha?.base_url ?? "", token: e.target.value } })}
          />
        </label>

        <label>
          Agent pairing token
          <input
            value={config.agent?.token ?? ""}
            onChange={(e) => onChange({ ...config, agent: { token: e.target.value } })}
            placeholder="8-char-pairing-secret"
          />
        </label>
      </div>

      <div className="top-bar-actions">
        <span className={`save-status ${dirty ? "dirty" : "clean"}`}>
          {saving ? "Saving…" : dirty ? "Unsaved changes" : "Saved"}
        </span>
        <button type="button" onClick={onSave} disabled={saving}>
          Save
        </button>
      </div>
    </div>
  );
}
