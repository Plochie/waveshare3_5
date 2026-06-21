import { open } from "@tauri-apps/plugin-dialog";
import { useEffect, useState } from "react";
import { iconPreview, importIcon } from "../api";
import { DeckButton, DeckPage } from "../types";
import StepEditor from "./StepEditor";

interface Props {
  button: DeckButton;
  pages: DeckPage[];
  currentPageId: string;
  onChange: (next: DeckButton) => void;
  onClear: () => void;
}

export default function ButtonEditor({ button, pages, currentPageId, onChange, onClear }: Props) {
  const isFolder = !!button.open_page;
  const [preview, setPreview] = useState<string | null>(null);
  const [importing, setImporting] = useState(false);

  useEffect(() => {
    setPreview(null);
    if (!button.icon) return;
    let cancelled = false;
    iconPreview(button.icon).then((p) => {
      if (!cancelled) setPreview(p);
    });
    return () => {
      cancelled = true;
    };
  }, [button.icon]);

  async function handlePickIcon() {
    const path = await open({
      multiple: false,
      filters: [{ name: "Image", extensions: ["png", "jpg", "jpeg", "bmp", "gif"] }],
    });
    if (!path || typeof path !== "string") return;
    setImporting(true);
    try {
      const meta = await importIcon(path);
      setPreview(meta.preview);
      onChange({ ...button, icon: meta.name });
    } finally {
      setImporting(false);
    }
  }

  function setMode(folder: boolean) {
    if (folder) {
      const target = pages.find((p) => p.id !== currentPageId)?.id ?? "";
      onChange({ ...button, open_page: target, steps: undefined });
    } else {
      onChange({ ...button, open_page: undefined, steps: button.steps ?? [] });
    }
  }

  return (
    <div className="button-editor">
      <div className="panel-header">
        <h3>Slot {button.pos}</h3>
        <button type="button" className="link-btn danger" onClick={onClear}>
          Clear slot
        </button>
      </div>

      <label>
        Label
        <input value={button.label} onChange={(e) => onChange({ ...button, label: e.target.value })} />
      </label>

      <label>
        Icon
        <span className="icon-row">
          {preview ? (
            <img className="icon-thumb" src={preview} alt={button.icon} />
          ) : (
            <span className="icon-thumb icon-thumb-empty" />
          )}
          <button type="button" onClick={handlePickIcon} disabled={importing}>
            {importing ? "Importing…" : button.icon ? "Replace…" : "Choose image…"}
          </button>
          {button.icon ? (
            <button type="button" className="link-btn danger" onClick={() => onChange({ ...button, icon: undefined })}>
              Remove
            </button>
          ) : null}
        </span>
      </label>

      <label>
        Color
        <div className="color-row">
          <input
            type="color"
            value={button.color && /^#[0-9a-fA-F]{6}$/.test(button.color) ? button.color : "#3ecf8e"}
            onChange={(e) => onChange({ ...button, color: e.target.value })}
          />
          <input
            value={button.color ?? ""}
            onChange={(e) => onChange({ ...button, color: e.target.value })}
            placeholder="#3ECF8E"
          />
        </div>
      </label>

      <div className="bind-editor">
        <label className="bind-enable">
          <input
            type="checkbox"
            checked={!!button.bind}
            onChange={(e) =>
              onChange({
                ...button,
                bind: e.target.checked ? { key: "", mode: "toggle" } : undefined,
              })
            }
          />
          Bind to a live value
        </label>
        {button.bind && (
          <div className="bind-fields">
            <label>
              Key
              <input
                value={button.bind.key}
                onChange={(e) => onChange({ ...button, bind: { ...button.bind!, key: e.target.value } })}
              />
            </label>
            <label>
              Mode
              <select
                value={button.bind.mode}
                onChange={(e) =>
                  onChange({ ...button, bind: { ...button.bind!, mode: e.target.value as "toggle" | "text" } })
                }
              >
                <option value="toggle">Toggle</option>
                <option value="text">Text</option>
              </select>
            </label>
            {button.bind.mode === "toggle" ? (
              <>
                <label>
                  On value
                  <input
                    value={button.bind.on_value ?? ""}
                    placeholder="1"
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, on_value: e.target.value } })}
                  />
                </label>
                <label>
                  Off value
                  <input
                    value={button.bind.off_value ?? ""}
                    placeholder="0"
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, off_value: e.target.value } })}
                  />
                </label>
                <label>
                  On label
                  <input
                    value={button.bind.on_label ?? ""}
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, on_label: e.target.value } })}
                  />
                </label>
                <label>
                  Off label
                  <input
                    value={button.bind.off_label ?? ""}
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, off_label: e.target.value } })}
                  />
                </label>
                <label>
                  On color
                  <input
                    value={button.bind.on_color ?? ""}
                    placeholder="#F87171"
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, on_color: e.target.value } })}
                  />
                </label>
                <label>
                  Off color
                  <input
                    value={button.bind.off_color ?? ""}
                    placeholder="#3A3A44"
                    onChange={(e) => onChange({ ...button, bind: { ...button.bind!, off_color: e.target.value } })}
                  />
                </label>
              </>
            ) : (
              <label>
                Format
                <input
                  value={button.bind.format ?? ""}
                  placeholder="{}"
                  onChange={(e) => onChange({ ...button, bind: { ...button.bind!, format: e.target.value } })}
                />
              </label>
            )}
          </div>
        )}
      </div>

      <div className="mode-toggle">
        <button type="button" className={!isFolder ? "active" : ""} onClick={() => setMode(false)}>
          Action steps
        </button>
        <button type="button" className={isFolder ? "active" : ""} onClick={() => setMode(true)}>
          Open page
        </button>
      </div>

      {isFolder ? (
        <label>
          Target page
          <select
            value={button.open_page ?? ""}
            onChange={(e) => onChange({ ...button, open_page: e.target.value })}
          >
            {pages
              .filter((p) => p.id !== currentPageId)
              .map((p) => (
                <option key={p.id} value={p.id}>
                  {p.title} ({p.id})
                </option>
              ))}
          </select>
        </label>
      ) : (
        <StepEditor steps={button.steps ?? []} onChange={(steps) => onChange({ ...button, steps })} />
      )}
    </div>
  );
}
