import { listen } from "@tauri-apps/api/event";
import { useEffect, useRef, useState } from "react";
import {
  configPath as fetchConfigPath,
  DeviceInfo,
  forgetDevice,
  listDevices,
  loadConfig,
  saveConfig,
} from "./api";
import "./App.css";
import AppTabs from "./components/AppTabs";
import ButtonEditor from "./components/ButtonEditor";
import DevicesPage from "./components/DevicesPage";
import GridView from "./components/GridView";
import Onboarding from "./components/Onboarding";
import PageTabs from "./components/PageTabs";
import { PendingPair } from "./components/PairPanel";
import StatePage from "./components/StatePage";
import TopBar from "./components/TopBar";
import { DeckConfig, DeckPage } from "./types";

function nextPageId(pages: DeckPage[]): string {
  let n = pages.length + 1;
  while (pages.some((p) => p.id === `page${n}`)) n++;
  return `page${n}`;
}

export default function App() {
  const [config, setConfig] = useState<DeckConfig | null>(null);
  const [currentPageId, setCurrentPageId] = useState("home");
  const [selectedPos, setSelectedPos] = useState<number | null>(null);
  const [dirty, setDirty] = useState(false);
  const [saving, setSaving] = useState(false);
  const [path, setPath] = useState("");
  const [pending, setPending] = useState<PendingPair | null>(null);
  const [devices, setDevices] = useState<DeviceInfo[]>([]);
  const [view, setView] = useState<"editor" | "devices" | "state">("editor");
  const [onboarding, setOnboarding] = useState(false);
  const bootstrapped = useRef(false);

  useEffect(() => {
    loadConfig().then(setConfig);
    fetchConfigPath().then(setPath);
  }, []);

  useEffect(() => {
    const unReq = listen<PendingPair>("deck-pair-request", (e) => setPending(e.payload));
    const unCancel = listen<{ device_id: string }>("deck-pair-cancel", (e) =>
      setPending((p) => (p && p.device_id === e.payload.device_id ? null : p)),
    );
    return () => {
      unReq.then((f) => f());
      unCancel.then((f) => f());
    };
  }, []);

  useEffect(() => {
    let cancelled = false;
    const poll = () =>
      listDevices().then((list) => {
        if (cancelled) return;
        setDevices(list);
        if (!bootstrapped.current) {
          bootstrapped.current = true;
          if (list.length === 0) setOnboarding(true);
        }
      });
    poll();
    const id = setInterval(poll, 1500);
    return () => {
      cancelled = true;
      clearInterval(id);
    };
  }, []);

  useEffect(() => {
    if (onboarding && devices.length > 0) {
      setOnboarding(false);
      setView("editor");
    }
  }, [onboarding, devices.length]);

  async function handleForget(deviceId: string) {
    try {
      await forgetDevice(deviceId);
      setDevices((ds) => ds.filter((d) => d.device_id !== deviceId));
    } catch (e) {
      console.error("forget failed", e);
    }
  }

  if (!config) {
    return <div className="loading">Loading…</div>;
  }
  const cfg: DeckConfig = config;

  if (onboarding) {
    return (
      <Onboarding
        pending={pending}
        onResolved={() => setPending(null)}
        onSkip={() => setOnboarding(false)}
      />
    );
  }

  function update(next: DeckConfig) {
    setConfig(next);
    setDirty(true);
  }

  async function handleSave() {
    setSaving(true);
    try {
      await saveConfig(cfg);
      setDirty(false);
    } finally {
      setSaving(false);
    }
  }

  const currentPage = cfg.pages.find((p) => p.id === currentPageId) ?? cfg.pages[0];

  function updatePage(next: DeckPage) {
    update({
      ...cfg,
      pages: cfg.pages.map((p) => (p.id === next.id ? next : p)),
    });
  }

  function handleAddPage() {
    const id = nextPageId(cfg.pages);
    const next = { ...cfg, pages: [...cfg.pages, { id, title: "New Page", buttons: [] }] };
    update(next);
    setCurrentPageId(id);
  }

  function handleRenamePage(id: string, title: string) {
    update({
      ...cfg,
      pages: cfg.pages.map((p) => (p.id === id ? { ...p, title } : p)),
    });
  }

  function handleRemovePage(id: string) {
    if (id === "home") return;
    update({ ...cfg, pages: cfg.pages.filter((p) => p.id !== id) });
    if (currentPageId === id) setCurrentPageId("home");
    setSelectedPos(null);
  }

  const selectedButton = currentPage.buttons.find((b) => b.pos === selectedPos) ?? null;

  function handleSelectSlot(pos: number) {
    setSelectedPos(pos);
    if (!currentPage.buttons.some((b) => b.pos === pos)) {
      updatePage({
        ...currentPage,
        buttons: [...currentPage.buttons, { pos, label: "New Button" }],
      });
    }
  }

  function handleButtonChange(next: typeof selectedButton) {
    if (!next) return;
    updatePage({
      ...currentPage,
      buttons: currentPage.buttons.map((b) => (b.pos === next.pos ? next : b)),
    });
  }

  function handleClearSlot() {
    if (selectedPos === null) return;
    updatePage({
      ...currentPage,
      buttons: currentPage.buttons.filter((b) => b.pos !== selectedPos),
    });
    setSelectedPos(null);
  }

  return (
    <div className="app">
      <AppTabs
        view={view}
        onSelect={setView}
        pendingBadge={!!pending && view !== "devices"}
      />
      {view === "editor" ? (
        <>
          {pending && (
            <div className="editor-banner">
              A deck wants to pair —{" "}
              <button type="button" onClick={() => setView("devices")}>
                review in Devices
              </button>
            </div>
          )}
          <TopBar
            config={cfg}
            onChange={update}
            onSave={handleSave}
            dirty={dirty}
            saving={saving}
            configPath={path}
          />
          <PageTabs
            pages={cfg.pages}
            currentPageId={currentPage.id}
            onSelect={(id) => {
              setCurrentPageId(id);
              setSelectedPos(null);
            }}
            onAdd={handleAddPage}
            onRename={handleRenamePage}
            onRemove={handleRemovePage}
          />
          <div className="main-layout">
            <GridView
              page={currentPage}
              cols={cfg.grid.cols}
              rows={cfg.grid.rows}
              selectedPos={selectedPos}
              onSelect={handleSelectSlot}
            />
            <div className="side-panel">
              {selectedButton ? (
                <ButtonEditor
                  button={selectedButton}
                  pages={cfg.pages}
                  currentPageId={currentPage.id}
                  onChange={handleButtonChange}
                  onClear={handleClearSlot}
                />
              ) : (
                <p className="hint">Select a tile to edit it.</p>
              )}
            </div>
          </div>
        </>
      ) : view === "devices" ? (
        <DevicesPage
          devices={devices}
          onForget={handleForget}
          pending={pending}
          onResolved={() => setPending(null)}
        />
      ) : (
        <StatePage />
      )}
    </div>
  );
}
