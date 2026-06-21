import { useState } from "react";
import { confirmPairing, rejectPairing } from "../api";

export interface PendingPair {
  device_id: string;
  code: string;
}

interface Props {
  pending: PendingPair | null;
  context: "onboarding" | "devices";
  onResolved: () => void;
}

export default function PairPanel({ pending, context, onResolved }: Props) {
  const [name, setName] = useState("");
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [paired, setPaired] = useState(false);

  async function pair() {
    if (!pending) return;
    setBusy(true);
    setError(null);
    try {
      await confirmPairing(pending.device_id, name.trim() || pending.device_id);
      setPaired(true);
      setName("");
      setTimeout(() => {
        setPaired(false);
        onResolved();
      }, 900);
    } catch (e) {
      setError(String(e));
    } finally {
      setBusy(false);
    }
  }

  async function reject() {
    if (!pending) return;
    setBusy(true);
    setError(null);
    try {
      await rejectPairing(pending.device_id);
      onResolved();
    } catch (e) {
      setError(String(e));
    } finally {
      setBusy(false);
    }
  }

  if (paired) {
    return (
      <div className="pp">
        <div className="pp-success">✓ Paired</div>
      </div>
    );
  }

  if (!pending) {
    const msg = context === "onboarding" ? "Searching for your deck…" : "Waiting for a device…";
    return (
      <div className="pp">
        <span className="pp-pill">
          <span className="pp-spinner" />
          {msg}
        </span>
        <p className="pp-hint">Power on a deck on this network and its pairing code appears here.</p>
      </div>
    );
  }

  return (
    <div className="pp">
      <div className="pp-card">
        <h3 className="pp-title">New device wants to pair</h3>
        <span className="pp-id">{pending.device_id}</span>
        <div className="pp-code">{pending.code.split("").join(" ")}</div>
        <p className="pp-instr">Confirm this matches the code on your deck's screen</p>
        <label className="pp-name">
          Name (optional)
          <input
            value={name}
            placeholder={pending.device_id}
            onChange={(e) => setName(e.target.value)}
          />
        </label>
        {error && <p className="pp-error">{error}</p>}
        <div className="pp-actions">
          <button type="button" className="pp-reject" onClick={reject} disabled={busy}>
            Reject
          </button>
          <button type="button" className="pp-pair" onClick={pair} disabled={busy}>
            Pair
          </button>
        </div>
      </div>
    </div>
  );
}
