# Stream Deck Pairing UX Redesign Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the desktop configurator's popup pairing modal and thin device strip with a professional first-run onboarding gate, a shared inline pairing panel, and a permanent Devices page, navigated by top-level Editor | Devices tabs.

**Architecture:** Pure React/TypeScript + CSS change in `desktop/src/`. Build four leaf components (each compiles standalone, unused until wired), then rewire `App.tsx` into a shell that owns the device poll + view/onboarding state and renders the gate / editor / devices surfaces. No firmware, Rust, protocol, or `api.ts` changes — every command/event/type already exists.

**Tech Stack:** React 19, TypeScript, Vite, Tauri v2 (`@tauri-apps/api/event` `listen`).

## Global Constraints

- Desktop frontend only. No changes to `desktop/src/api.ts`, `desktop/src-tauri/`, firmware, or the protocol.
- Build check (every task): `cd desktop && npm run build` (runs `tsc && vite build`) must pass with no errors. There is no JS test harness — tasks are build-gated; visual/behavioral verification is manual in the final task.
- Existing API surface (do not change): `listDevices(): Promise<DeviceInfo[]>` where `DeviceInfo { device_id: string; name: string; fw: string; online: boolean }`; `confirmPairing(deviceId, name)`, `rejectPairing(deviceId)`, `forgetDevice(deviceId)`; Tauri events `deck-pair-request` (`{device_id, code}`) and `deck-pair-cancel` (`{device_id}`).
- `PendingPair` type is `{ device_id: string; code: string }` and is exported from `PairPanel.tsx` (Task 1) once it exists.
- Theme: surfaces `#16161a` (window) / `#1a1a20` (right panel) / `#1b1b21` (rows); borders `#2c2c34`; text `#e6e6ea`, muted `#9a9aa6`, faint `#7a7a86`; accent green `#3ECF8E`; reject red `#d66`. 6-digit code rendered large (~2.4rem), weight 700, letter-spacing 0.16em, digits space-separated.

---

### Task 1: PairPanel component (shared device area)

**Files:**
- Create: `desktop/src/components/PairPanel.tsx`
- Modify: `desktop/src/App.css` (append `.pp-*` styles)

**Interfaces:**
- Consumes: `confirmPairing`, `rejectPairing` from `../api`.
- Produces: `export interface PendingPair { device_id: string; code: string }`; `export default function PairPanel(props: { pending: PendingPair | null; context: "onboarding" | "devices"; onResolved: () => void })`. States: searching (`pending===null`), confirm (`pending!==null`), and a ~900ms success flash after a successful Pair (then calls `onResolved`). Reject calls `onResolved` immediately on success.

- [ ] **Step 1: Create the component**

Create `desktop/src/components/PairPanel.tsx`:

```tsx
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
```

- [ ] **Step 2: Append the styles**

Append to `desktop/src/App.css`:

```css
/* PairPanel (shared device area) */
.pp {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  text-align: center;
  height: 100%;
  padding: 24px;
}
.pp-pill {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  background: #1d1d23;
  border: 1px solid #2c2c34;
  border-radius: 999px;
  padding: 6px 14px;
  font-size: 0.85rem;
  color: #c4c4ce;
}
.pp-spinner {
  width: 12px;
  height: 12px;
  border: 2px solid #3ecf8e;
  border-top-color: transparent;
  border-radius: 50%;
  animation: pp-spin 0.9s linear infinite;
}
@keyframes pp-spin {
  to {
    transform: rotate(360deg);
  }
}
.pp-hint {
  font-size: 0.78rem;
  color: #7a7a86;
  margin-top: 12px;
  max-width: 230px;
}
.pp-card {
  display: flex;
  flex-direction: column;
  align-items: center;
}
.pp-title {
  margin: 0;
  font-size: 1rem;
  font-weight: 650;
}
.pp-id {
  font-family: ui-monospace, Menlo, monospace;
  font-size: 0.72rem;
  color: #9a9aa6;
  background: #23232b;
  border: 1px solid #2c2c34;
  border-radius: 999px;
  padding: 3px 10px;
  margin-top: 8px;
}
.pp-code {
  font-size: 2.4rem;
  font-weight: 700;
  letter-spacing: 0.16em;
  color: #fff;
  margin: 14px 0 0;
}
.pp-instr {
  font-size: 0.78rem;
  color: #9a9aa6;
  margin: 10px 0 0;
  max-width: 230px;
}
.pp-name {
  display: flex;
  flex-direction: column;
  gap: 4px;
  font-size: 0.75rem;
  color: #9a9aa6;
  margin-top: 14px;
  width: 200px;
  text-align: left;
}
.pp-name input {
  background: #15151a;
  border: 1px solid #2c2c34;
  border-radius: 7px;
  padding: 7px 10px;
  color: #e6e6ea;
  font-size: 0.8rem;
}
.pp-error {
  color: #f87171;
  font-size: 0.78rem;
  margin: 10px 0 0;
}
.pp-actions {
  display: flex;
  gap: 10px;
  margin-top: 16px;
}
.pp-actions button {
  border-radius: 7px;
  padding: 8px 18px;
  font-size: 0.8rem;
  font-weight: 600;
  border: 1px solid;
  cursor: pointer;
}
.pp-pair {
  background: #3ecf8e;
  color: #08130d;
  border-color: #3ecf8e;
}
.pp-reject {
  background: transparent;
  color: #d66;
  border-color: #d66;
}
.pp-success {
  font-size: 1.4rem;
  font-weight: 700;
  color: #3ecf8e;
}
```

- [ ] **Step 3: Build**

Run: `cd desktop && npm run build`
Expected: PASS (PairPanel compiles; it is not imported anywhere yet, which is fine).

- [ ] **Step 4: Commit**

```bash
git add desktop/src/components/PairPanel.tsx desktop/src/App.css
git commit -m "feat(desktop): shared PairPanel (searching/confirm/paired)

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 2: AppTabs component (top-level Editor | Devices tabs)

**Files:**
- Create: `desktop/src/components/AppTabs.tsx`
- Modify: `desktop/src/App.css` (append `.app-tab*` styles)

**Interfaces:**
- Produces: `export default function AppTabs(props: { view: "editor" | "devices"; onSelect: (v: "editor" | "devices") => void; pendingBadge: boolean })`.

- [ ] **Step 1: Create the component**

Create `desktop/src/components/AppTabs.tsx`:

```tsx
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
```

- [ ] **Step 2: Append the styles**

Append to `desktop/src/App.css`:

```css
/* Top-level Editor | Devices tabs */
.app-tabs {
  display: flex;
  gap: 4px;
  padding: 8px 12px 0;
  background: #1d1d23;
  border-bottom: 1px solid #2c2c34;
}
.app-tab {
  position: relative;
  font-size: 0.82rem;
  padding: 7px 16px;
  border-radius: 7px 7px 0 0;
  color: #9a9aa6;
  background: transparent;
  border: 1px solid transparent;
  border-bottom: none;
  cursor: pointer;
}
.app-tab.active {
  background: #16161a;
  color: #fff;
  border-color: #2c2c34;
}
.app-tab-badge {
  position: absolute;
  top: 5px;
  right: 6px;
  width: 7px;
  height: 7px;
  border-radius: 50%;
  background: #3ecf8e;
}
```

- [ ] **Step 3: Build**

Run: `cd desktop && npm run build`
Expected: PASS (AppTabs compiles; not imported yet).

- [ ] **Step 4: Commit**

```bash
git add desktop/src/components/AppTabs.tsx desktop/src/App.css
git commit -m "feat(desktop): top-level Editor/Devices tab bar

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 3: Onboarding component (first-run gate)

**Files:**
- Create: `desktop/src/components/Onboarding.tsx`
- Modify: `desktop/src/App.css` (append `.onboarding`/`.ob-*` styles)

**Interfaces:**
- Consumes: `PairPanel` + `PendingPair` from `./PairPanel` (Task 1).
- Produces: `export default function Onboarding(props: { pending: PendingPair | null; onResolved: () => void; onSkip: () => void })`.

- [ ] **Step 1: Create the component**

Create `desktop/src/components/Onboarding.tsx`:

```tsx
import PairPanel, { PendingPair } from "./PairPanel";

interface Props {
  pending: PendingPair | null;
  onResolved: () => void;
  onSkip: () => void;
}

export default function Onboarding({ pending, onResolved, onSkip }: Props) {
  return (
    <div className="onboarding">
      <div className="ob-left">
        <div className="ob-glyph" />
        <h1 className="ob-title">Connect your deck</h1>
        <p className="ob-subtitle">Let's get your Stream Deck paired.</p>
        <ol className="ob-steps">
          <li>
            <span className="ob-num">1</span>Power on your deck
          </li>
          <li>
            <span className="ob-num">2</span>Join the same Wi-Fi network
          </li>
        </ol>
        <button type="button" className="ob-skip" onClick={onSkip}>
          Skip for now
        </button>
      </div>
      <div className="ob-right">
        <PairPanel pending={pending} context="onboarding" onResolved={onResolved} />
      </div>
    </div>
  );
}
```

- [ ] **Step 2: Append the styles**

Append to `desktop/src/App.css`:

```css
/* Onboarding gate */
.onboarding {
  display: flex;
  height: 100vh;
  background: #16161a;
  color: #e6e6ea;
}
.ob-left {
  width: 46%;
  max-width: 380px;
  padding: 48px 40px;
  border-right: 1px solid #2c2c34;
  display: flex;
  flex-direction: column;
}
.ob-glyph {
  width: 54px;
  height: 40px;
  border: 2px solid #3ecf8e;
  border-radius: 7px;
}
.ob-title {
  font-size: 1.5rem;
  font-weight: 650;
  margin: 20px 0 0;
}
.ob-subtitle {
  font-size: 0.9rem;
  color: #9a9aa6;
  margin: 6px 0 0;
}
.ob-steps {
  list-style: none;
  padding: 0;
  margin: 28px 0 0;
}
.ob-steps li {
  display: flex;
  align-items: center;
  gap: 10px;
  font-size: 0.9rem;
  color: #b6b6c0;
  margin: 10px 0;
}
.ob-num {
  width: 20px;
  height: 20px;
  border-radius: 50%;
  background: #23232b;
  color: #3ecf8e;
  font-size: 0.75rem;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}
.ob-skip {
  margin-top: auto;
  align-self: flex-start;
  background: none;
  border: none;
  color: #7a7a86;
  font-size: 0.8rem;
  text-decoration: underline;
  cursor: pointer;
  padding: 0;
}
.ob-right {
  flex: 1;
  background: #1a1a20;
}
```

- [ ] **Step 3: Build**

Run: `cd desktop && npm run build`
Expected: PASS (Onboarding compiles; not imported yet).

- [ ] **Step 4: Commit**

```bash
git add desktop/src/components/Onboarding.tsx desktop/src/App.css
git commit -m "feat(desktop): first-run onboarding gate

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 4: DevicesPage component (paired list + add-a-device)

**Files:**
- Create: `desktop/src/components/DevicesPage.tsx`
- Modify: `desktop/src/App.css` (append `.devices-page`/`.dp-*` styles)

**Interfaces:**
- Consumes: `DeviceInfo` from `../api`; `PairPanel` + `PendingPair` from `./PairPanel` (Task 1).
- Produces: `export default function DevicesPage(props: { devices: DeviceInfo[]; onForget: (deviceId: string) => void; pending: PendingPair | null; onResolved: () => void })`.

- [ ] **Step 1: Create the component**

Create `desktop/src/components/DevicesPage.tsx`:

```tsx
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
```

- [ ] **Step 2: Append the styles**

Append to `desktop/src/App.css`:

```css
/* Devices page */
.devices-page {
  display: flex;
  flex: 1;
  min-height: 0;
}
.dp-left {
  width: 52%;
  padding: 20px;
  border-right: 1px solid #2c2c34;
  overflow-y: auto;
}
.dp-right {
  flex: 1;
  background: #1a1a20;
  display: flex;
  flex-direction: column;
}
.dp-section {
  font-size: 0.7rem;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: #7a7a86;
  margin-bottom: 12px;
}
.dp-section-center {
  padding: 18px 18px 0;
}
.dp-empty {
  font-size: 0.85rem;
  color: #7a7a86;
}
.dp-row {
  display: flex;
  align-items: center;
  gap: 10px;
  background: #1b1b21;
  border: 1px solid #2a2a32;
  border-radius: 8px;
  padding: 11px 12px;
  margin-bottom: 8px;
}
.dp-dot {
  width: 9px;
  height: 9px;
  border-radius: 50%;
  flex-shrink: 0;
}
.dp-dot.online {
  background: #3ecf8e;
  box-shadow: 0 0 6px rgba(62, 207, 142, 0.5);
}
.dp-dot.offline {
  background: transparent;
  border: 1px solid #6a6a72;
}
.dp-name {
  font-size: 0.85rem;
  font-weight: 600;
}
.dp-meta {
  font-size: 0.72rem;
  color: #8a8a94;
  font-family: ui-monospace, Menlo, monospace;
}
.dp-forget {
  margin-left: auto;
  font-size: 0.72rem;
  color: #d66;
  background: transparent;
  border: 1px solid #d66;
  border-radius: 5px;
  padding: 3px 9px;
  cursor: pointer;
}
```

- [ ] **Step 3: Build**

Run: `cd desktop && npm run build`
Expected: PASS (DevicesPage compiles; not imported yet).

- [ ] **Step 4: Commit**

```bash
git add desktop/src/components/DevicesPage.tsx desktop/src/App.css
git commit -m "feat(desktop): Devices page (paired list + add-a-device)

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 5: App shell integration

Rewire `App.tsx` into the shell: lift the device poll, add `view` + `onboarding` state, render the onboarding gate / editor / devices surfaces with the tab bar and the editor pairing banner. Remove the old `PairModal` and `DevicesPanel` (files + dead CSS).

**Files:**
- Modify (full rewrite): `desktop/src/App.tsx`
- Delete: `desktop/src/components/PairModal.tsx`, `desktop/src/components/DevicesPanel.tsx`
- Modify: `desktop/src/App.css` (add `.editor-banner`; remove dead modal/strip styles)

**Interfaces:**
- Consumes: `AppTabs` (Task 2), `Onboarding` (Task 3), `DevicesPage` (Task 4), `PairPanel`/`PendingPair` (Task 1), `listDevices`/`forgetDevice`/`DeviceInfo` (api). Keeps existing `TopBar`, `PageTabs`, `GridView`, `ButtonEditor`.

- [ ] **Step 1: Rewrite App.tsx**

Replace the entire contents of `desktop/src/App.tsx` with:

```tsx
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
  const [view, setView] = useState<"editor" | "devices">("editor");
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
      ) : (
        <DevicesPage
          devices={devices}
          onForget={handleForget}
          pending={pending}
          onResolved={() => setPending(null)}
        />
      )}
    </div>
  );
}
```

- [ ] **Step 2: Delete the replaced components**

```bash
git rm desktop/src/components/PairModal.tsx desktop/src/components/DevicesPanel.tsx
```

- [ ] **Step 3: Add the editor banner style and remove dead CSS**

Append to `desktop/src/App.css`:

```css
/* Editor "a deck wants to pair" banner */
.editor-banner {
  background: #23231a;
  border-bottom: 1px solid #3a3a2a;
  color: #e8e0b0;
  font-size: 0.82rem;
  padding: 8px 14px;
  display: flex;
  align-items: center;
  gap: 8px;
}
.editor-banner button {
  background: #3ecf8e;
  color: #08130d;
  border: none;
  border-radius: 6px;
  padding: 4px 12px;
  font-size: 0.78rem;
  font-weight: 600;
  cursor: pointer;
}
```

Then remove the now-unused rule blocks from `desktop/src/App.css` (their components are deleted). Delete every CSS rule whose selector starts with any of these — the old device strip and the old modal:

```
.devices-panel        .devices-label      .devices-empty      .device-chip
.device-dot           .device-meta        .forget-btn
.modal-overlay        .modal              .pair-modal
.pair-device-id       .pair-instructions  .pair-code
.pair-name            .pair-error         .pair-actions
```

(These were used only by `DevicesPanel.tsx` and `PairModal.tsx`. `PairPanel` uses `.pp-*` and `DevicesPage` uses `.dp-*`, so none of the above are referenced anymore. Leaving any behind is harmless to the build but the task is to remove them.)

- [ ] **Step 4: Build**

Run: `cd desktop && npm run build`
Expected: PASS — `tsc` clean (no references to the deleted `PairModal`/`DevicesPanel` remain) and `vite build` completes.

- [ ] **Step 5: Manual end-to-end verification**

Run `cd desktop && npm run tauri dev` and verify against the spec's flow. With a real (or freshly flashed) device on the same network:

1. **First run** (no devices paired): the window shows the split onboarding gate with "Searching for your deck…". Power on an unpaired deck → the right panel swaps to the confirm card with the 6-digit code matching the device screen → click **Pair** → "✓ Paired" → the app lands in the Editor. Relaunch (`r` in the tauri dev console, or restart) → opens straight to the Editor (no gate).
2. **Tabs**: Editor ⇄ Devices switch via the top tabs. The paired deck appears on the Devices page, online dot green; power-cycle it → dot goes hollow then back.
3. **Forget**: Devices page → Forget → the row disappears and the deck re-enters pairing on its next reconnect.
4. **Skip**: with no devices, click "Skip for now" → lands in the Editor; relaunch → gate returns.
5. **Pair while editing**: with one deck paired and on the Editor tab, trigger a new deck's pairing → the editor banner + the Devices-tab badge appear (no hijack); clicking "review in Devices" shows the confirm card there.

- [ ] **Step 6: Commit**

```bash
git add desktop/src/App.tsx desktop/src/App.css
git commit -m "feat(desktop): pairing onboarding gate + Devices page shell

Replaces the popup pairing modal and thin device strip with a first-run
onboarding gate, top-level Editor/Devices tabs, and a permanent Devices
page; removes PairModal and DevicesPanel.

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

## Notes on testing strategy

This project has no JS test harness, so each task is gated on `npm run build` (`tsc` type-check + `vite build`). Tasks 1–4 create components that compile standalone but aren't mounted until Task 5, so their visual/behavioral verification happens in Task 5 Step 5's manual end-to-end pass, which walks the full spec §Testing matrix (first-run, tabs, forget, skip, pair-while-editing).
