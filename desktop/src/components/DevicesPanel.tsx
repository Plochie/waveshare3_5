import { useEffect, useState } from "react";
import { DeviceInfo, listDevices } from "../api";

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

  return (
    <div className="devices-panel">
      <span className="devices-label">Devices</span>
      {devices.length === 0 ? (
        <span className="devices-empty">none connected</span>
      ) : (
        devices.map((d) => (
          <span className="device-chip" key={d.conn_id}>
            <span className="device-dot" /> {d.device_id} (fw {d.fw})
          </span>
        ))
      )}
    </div>
  );
}
