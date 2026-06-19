import { useEffect, useState } from "react";
import { configPath as fetchConfigPath, loadConfig, saveConfig } from "./api";
import "./App.css";
import ButtonEditor from "./components/ButtonEditor";
import DevicesPanel from "./components/DevicesPanel";
import GridView from "./components/GridView";
import PageTabs from "./components/PageTabs";
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

  useEffect(() => {
    loadConfig().then(setConfig);
    fetchConfigPath().then(setPath);
  }, []);

  if (!config) {
    return <div className="loading">Loading…</div>;
  }
  const cfg: DeckConfig = config;

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
      <TopBar
        config={cfg}
        onChange={update}
        onSave={handleSave}
        dirty={dirty}
        saving={saving}
        configPath={path}
      />

      <DevicesPanel />

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
    </div>
  );
}
