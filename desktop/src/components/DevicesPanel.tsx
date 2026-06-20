import { useEffect, useState } from "react";
import { DeviceInfo, forgetDevice, listDevices } from "../api";

export default function DevicesPanel() {
  const [devices, setDevices] = useState<DeviceInfo[]>([]);

  useEffect(() => {
    let cancelled = false;
    const poll = () => {
      listDevices().then((list) => {
        if (!cancelled) setDevices(list);
      });
    };
    poll();
    const id = setInterval(poll, 1500);
    return () => {
      cancelled = true;
      clearInterval(id);
    };
  }, []);

  async function forget(device_id: string) {
    await forgetDevice(device_id);
    setDevices((ds) => ds.filter((d) => d.device_id !== device_id));
  }

  return (
    <div className="devices-panel">
      <span className="devices-label">Paired Devices</span>
      {devices.length === 0 ? (
        <span className="devices-empty">none paired</span>
      ) : (
        devices.map((d) => (
          <span className="device-chip" key={d.device_id}>
            <span className={`device-dot ${d.online ? "online" : "offline"}`} />
            {d.name || d.device_id}
            <span className="device-meta">
              {d.device_id}
              {d.online ? ` · fw ${d.fw}` : " · offline"}
            </span>
            <button type="button" className="forget-btn" onClick={() => forget(d.device_id)}>
              Forget
            </button>
          </span>
        ))
      )}
    </div>
  );
}
