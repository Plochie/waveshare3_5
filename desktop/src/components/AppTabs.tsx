interface Props {
  view: "editor" | "devices";
  onSelect: (v: "editor" | "devices") => void;
  pendingBadge: boolean;
}

export default function AppTabs({ view, onSelect, pendingBadge }: Props) {
  return (
    <div className="app-tabs">
      <button
        type="button"
        className={`app-tab ${view === "editor" ? "active" : ""}`}
        onClick={() => onSelect("editor")}
      >
        Editor
      </button>
      <button
        type="button"
        className={`app-tab ${view === "devices" ? "active" : ""}`}
        onClick={() => onSelect("devices")}
      >
        Devices
        {pendingBadge && <span className="app-tab-badge" />}
      </button>
    </div>
  );
}
