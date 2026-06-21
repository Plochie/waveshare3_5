import { DeviceInfo } from "../api";
import PairPanel, { PendingPair } from "./PairPanel";

interface Props {
  devices: DeviceInfo[];
  onForget: (deviceId: string) => void;
  pending: PendingPair | null;
  onResolved: () => void;
}

export default function DevicesPage({ devices, onForget, pending, onResolved }: Props) {
  return (
    <div className="devices-page">
      <div className="dp-left">
        <div className="dp-section">Paired decks</div>
        {devices.length === 0 ? (
          <p className="dp-empty">No decks paired yet.</p>
        ) : (
          devices.map((d) => (
            <div className="dp-row" key={d.device_id}>
              <span className={`dp-dot ${d.online ? "online" : "offline"}`} />
              <div>
                <div className="dp-name">{d.name || d.device_id}</div>
                <div className="dp-meta">
                  {d.device_id}
                  {d.online ? ` · fw ${d.fw}` : " · offline"}
                </div>
              </div>
              <button type="button" className="dp-forget" onClick={() => onForget(d.device_id)}>
                Forget
              </button>
            </div>
          ))
        )}
      </div>
      <div className="dp-right">
        <div className="dp-section dp-section-center">Add a device</div>
        <PairPanel pending={pending} context="devices" onResolved={onResolved} />
      </div>
    </div>
  );
}
