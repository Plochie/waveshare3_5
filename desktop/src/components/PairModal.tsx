import { useState } from "react";
import { confirmPairing, rejectPairing } from "../api";

export interface PendingPair {
  device_id: string;
  code: string;
}

interface Props {
  pending: PendingPair;
  onDone: () => void;
}

export default function PairModal({ pending, onDone }: Props) {
  const [name, setName] = useState("");
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);

  async function pair() {
    setBusy(true);
    setError(null);
    try {
      await confirmPairing(pending.device_id, name.trim() || pending.device_id);
      onDone();
    } catch (e) {
      setError(String(e));
    } finally {
      setBusy(false);
    }
  }

  async function reject() {
    setBusy(true);
    setError(null);
    try {
      await rejectPairing(pending.device_id);
      onDone();
    } catch (e) {
      setError(String(e));
    } finally {
      setBusy(false);
    }
  }

  return (
    <div className="modal-overlay">
      <div className="modal pair-modal">
        <h2>New device wants to pair</h2>
        <p className="pair-device-id">{pending.device_id}</p>
        <p className="pair-instructions">
          Confirm this code matches the one shown on the device screen:
        </p>
        <div className="pair-code">{pending.code.split("").join(" ")}</div>
        <label className="pair-name">
          Name (optional)
          <input
            value={name}
            placeholder={pending.device_id}
            onChange={(e) => setName(e.target.value)}
          />
        </label>
        {error && <p className="pair-error">{error}</p>}
        <div className="pair-actions">
          <button type="button" className="reject" onClick={reject} disabled={busy}>
            Reject
          </button>
          <button type="button" className="confirm" onClick={pair} disabled={busy}>
            Pair
          </button>
        </div>
      </div>
    </div>
  );
}
