import {
  HTTP_METHODS,
  MEDIA_KEYS,
  STEP_TYPES,
  Step,
  StepType,
  defaultStepForType,
} from "../types";
import KeyValueEditor from "./KeyValueEditor";

interface Props {
  steps: Step[];
  onChange: (steps: Step[]) => void;
}

function StepFields({ step, onChange }: { step: Step; onChange: (s: Step) => void }) {
  switch (step.type) {
    case "http_request":
      return (
        <>
          <label>
            Method
            <select
              value={step.method ?? "GET"}
              onChange={(e) => onChange({ ...step, method: e.target.value as Step["method"] })}
            >
              {HTTP_METHODS.map((m) => (
                <option key={m} value={m}>
                  {m}
                </option>
              ))}
            </select>
          </label>
          <label>
            URL
            <input
              value={step.url ?? ""}
              onChange={(e) => onChange({ ...step, url: e.target.value })}
              placeholder="http://homeserver.local/api/x"
            />
          </label>
          <label>
            Headers
            <KeyValueEditor
              value={step.headers ?? {}}
              onChange={(headers) => onChange({ ...step, headers })}
              keyPlaceholder="Header"
              valuePlaceholder="Value"
            />
          </label>
          <label>
            Body
            <textarea
              value={step.body ?? ""}
              onChange={(e) => onChange({ ...step, body: e.target.value })}
              placeholder='{"on":true}'
            />
          </label>
        </>
      );
    case "ha_service":
      return (
        <>
          <label>
            Domain
            <input
              value={step.domain ?? ""}
              onChange={(e) => onChange({ ...step, domain: e.target.value })}
              placeholder="light"
            />
          </label>
          <label>
            Service
            <input
              value={step.service ?? ""}
              onChange={(e) => onChange({ ...step, service: e.target.value })}
              placeholder="toggle"
            />
          </label>
          <label>
            Data
            <KeyValueEditor
              value={step.data ?? {}}
              onChange={(data) => onChange({ ...step, data })}
              keyPlaceholder="entity_id"
              valuePlaceholder="light.office"
            />
          </label>
        </>
      );
    case "ha_webhook":
      return (
        <label>
          Webhook id
          <input
            value={step.id ?? ""}
            onChange={(e) => onChange({ ...step, id: e.target.value })}
            placeholder="office_scene"
          />
        </label>
      );
    case "mqtt_publish":
      return (
        <>
          <label>
            Topic
            <input
              value={step.topic ?? ""}
              onChange={(e) => onChange({ ...step, topic: e.target.value })}
              placeholder="home/deck/btn1"
            />
          </label>
          <label>
            Payload
            <input
              value={step.payload ?? ""}
              onChange={(e) => onChange({ ...step, payload: e.target.value })}
              placeholder="ON"
            />
          </label>
          <p className="hint">Schema-only in v1 - not yet executed by the device.</p>
        </>
      );
    case "hotkey":
      return (
        <label>
          Keys (comma separated)
          <input
            value={(step.keys ?? []).join(",")}
            onChange={(e) =>
              onChange({
                ...step,
                keys: e.target.value
                  .split(",")
                  .map((k) => k.trim())
                  .filter(Boolean),
              })
            }
            placeholder="cmd,shift,a"
          />
        </label>
      );
    case "type_text":
      return (
        <label>
          Text
          <textarea
            value={step.text ?? ""}
            onChange={(e) => onChange({ ...step, text: e.target.value })}
            placeholder="hello world"
          />
        </label>
      );
    case "launch_app":
      return (
        <label>
          App (name / path / bundle id)
          <input
            value={step.target ?? ""}
            onChange={(e) => onChange({ ...step, target: e.target.value })}
            placeholder="zoom.us"
          />
        </label>
      );
    case "run_command":
      return (
        <label>
          Command
          <input
            value={step.command ?? ""}
            onChange={(e) => onChange({ ...step, command: e.target.value })}
            placeholder="/usr/local/bin/backup.sh"
          />
        </label>
      );
    case "media_key":
      return (
        <label>
          Key
          <select
            value={step.key ?? "play_pause"}
            onChange={(e) => onChange({ ...step, key: e.target.value as Step["key"] })}
          >
            {MEDIA_KEYS.map((k) => (
              <option key={k} value={k}>
                {k}
              </option>
            ))}
          </select>
        </label>
      );
    case "delay":
      return (
        <label>
          Milliseconds
          <input
            type="number"
            min={0}
            value={step.ms ?? 0}
            onChange={(e) => onChange({ ...step, ms: Number(e.target.value) })}
          />
        </label>
      );
  }
}

export default function StepEditor({ steps, onChange }: Props) {
  function updateStep(index: number, next: Step) {
    onChange(steps.map((s, i) => (i === index ? next : s)));
  }

  function removeStep(index: number) {
    onChange(steps.filter((_, i) => i !== index));
  }

  function moveStep(index: number, dir: -1 | 1) {
    const target = index + dir;
    if (target < 0 || target >= steps.length) return;
    const next = [...steps];
    [next[index], next[target]] = [next[target], next[index]];
    onChange(next);
  }

  function addStep() {
    onChange([...steps, defaultStepForType("http_request")]);
  }

  return (
    <div className="step-editor">
      {steps.map((step, i) => (
        <div className="step-card" key={i}>
          <div className="step-card-header">
            <span className="step-index">{i + 1}</span>
            <select
              value={step.type}
              onChange={(e) => updateStep(i, defaultStepForType(e.target.value as StepType))}
            >
              {STEP_TYPES.map((t) => (
                <option key={t} value={t}>
                  {t}
                </option>
              ))}
            </select>
            <div className="step-card-actions">
              <button type="button" className="icon-btn" onClick={() => moveStep(i, -1)} disabled={i === 0}>
                ↑
              </button>
              <button
                type="button"
                className="icon-btn"
                onClick={() => moveStep(i, 1)}
                disabled={i === steps.length - 1}
              >
                ↓
              </button>
              <button type="button" className="icon-btn" onClick={() => removeStep(i)}>
                ✕
              </button>
            </div>
          </div>
          <div className="step-card-fields">
            <StepFields step={step} onChange={(next) => updateStep(i, next)} />
          </div>
        </div>
      ))}
      <button type="button" className="link-btn" onClick={addStep}>
        + Add step
      </button>
    </div>
  );
}
