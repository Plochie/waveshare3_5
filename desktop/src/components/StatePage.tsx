import { listen } from "@tauri-apps/api/event";
import { useEffect, useState } from "react";
import { listState, setState } from "../api";

export default function StatePage() {
  const [values, setValues] = useState<Record<string, string>>({});
  const [newKey, setNewKey] = useState("");
  const [newVal, setNewVal] = useState("");

  useEffect(() => {
    listState().then(setValues);
    const un = listen<{ key: string; value: string }>("deck-state", (e) =>
      setValues((v) => ({ ...v, [e.payload.key]: e.payload.value })),
    );
    return () => {
      un.then((f) => f());
    };
  }, []);

  function edit(key: string, value: string) {
    setValues((v) => ({ ...v, [key]: value }));
  }

  async function addKey() {
    const k = newKey.trim();
    if (!k) return;
    await setState(k, newVal);
    setNewKey("");
    setNewVal("");
  }

  const keys = Object.keys(values).sort();
  return (
    <div className="state-page">
      <div className="sp-section">Live values</div>
      {keys.length === 0 ? (
        <p className="sp-empty">No values yet. Add one below, or push via the /kv endpoint.</p>
      ) : (
        keys.map((k) => (
          <div className="sp-row" key={k}>
            <span className="sp-key">{k}</span>
            <input className="sp-val" value={values[k]} onChange={(e) => edit(k, e.target.value)} />
            <button type="button" onClick={() => setState(k, values[k])}>
              Set
            </button>
          </div>
        ))
      )}
      <div className="sp-add">
        <input placeholder="key" value={newKey} onChange={(e) => setNewKey(e.target.value)} />
        <input placeholder="value" value={newVal} onChange={(e) => setNewVal(e.target.value)} />
        <button type="button" onClick={addKey}>
          Add
        </button>
      </div>
    </div>
  );
}
