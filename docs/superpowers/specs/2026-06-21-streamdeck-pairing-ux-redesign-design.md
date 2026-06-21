# WiFi Stream Deck — Pairing UX Redesign (Desktop)

A frontend redesign of the desktop configurator's pairing experience: replace
the out-of-nowhere popup modal with a professional first-run onboarding flow and
a permanent Devices page, navigated by top-level tabs.

Builds on
[2026-06-21-streamdeck-pairing-design.md](2026-06-21-streamdeck-pairing-design.md)
(the per-device pairing feature, already shipped) and the wire protocol
[2026-06-19-wifi-streamdeck-protocol.md](2026-06-19-wifi-streamdeck-protocol.md)
§3.1. This spec changes only how pairing is *presented*; the protocol, the
firmware, and the Rust backend are unchanged.

## Motivation

Pairing currently works but feels unpolished: the desktop app opens straight to
the grid editor, the "Paired Devices" UI is a thin status strip, and a pairing
prompt appears only as a popup modal triggered by a `deck-pair-request` event —
with no sense of onboarding or place. A first-time user who just installed the
app and powered on a deck is given no guidance. The goal is a deliberate,
professional flow: a first-run "Connect your deck" screen, inline (not popup)
code confirmation, and a permanent place to manage devices.

## Scope

**Desktop frontend only (React/TypeScript + CSS).** No changes to:

- the wire protocol or message shapes,
- the firmware (the on-device pairing screen already works),
- the Rust backend — the commands `confirm_pairing` / `reject_pairing` /
  `forget_device`, the events `deck-pair-request` (`{device_id, code}`) /
  `deck-pair-cancel` (`{device_id}`), and `list_devices` (returning
  `DeviceInfo { device_id, name, fw, online }`) are all already implemented and
  sufficient.

There is **no active network scan**: the desktop is the WS server and devices
connect to it, so "searching" is the idle state of waiting for a device to
connect and send `pair_request`. The "device discovered" moment *is* the
`deck-pair-request` event.

## App shell & navigation

`App.tsx` becomes a shell that owns:

- `devices: DeviceInfo[]` — lifted from `DevicesPanel`'s local poll to the
  shell; a single `list_devices` poll every 1.5s, shared by tabs, gate, and
  badge. (`DevicesPanel` is removed; its rendering moves into the Devices page.)
- `pending: PendingPair | null` — the current pair request, from the existing
  `deck-pair-request` / `deck-pair-cancel` listeners (already in `App.tsx`).
- `view: "editor" | "devices"` — the active top-level tab.
- `onboarding: boolean` — whether the first-run gate is showing. This is
  **explicit state, not a value derived continuously from `devices`**. It is
  initialized `true` once the first `list_devices` poll returns an empty list,
  and set `false` on either a successful first pair or "Skip for now". It is
  **never re-asserted** if `devices` later drops back to zero (e.g. forgetting
  the last device mid-session) — see Edge cases. Initial value before the first
  poll resolves: `false` (don't flash the gate; flip it on once the empty poll
  confirms there are no devices).

**Top-level tabs** render above the existing per-deck `PageTabs`: `Editor` and
`Devices`. The Devices tab shows a small badge/dot when `pending` is set and the
user is not currently looking at it.

**Three top-level surfaces:**

1. **Onboarding gate** — shown when `onboarding === true`. Full window; tabs
   hidden.
2. **Editor** — the existing grid/config editor (TopBar + PageTabs + GridView +
   ButtonEditor), shown when `!onboarding && view === "editor"`.
3. **Devices page** — shown when `!onboarding && view === "devices"`.

## Onboarding gate (`Onboarding.tsx`, new)

Split two-panel layout (visual direction B), app dark theme + green accent
(`#3ECF8E`):

- **Left panel** — a deck glyph, "Connect your deck" headline, a short subtitle,
  numbered steps ("1. Power on your deck", "2. Join the same Wi-Fi network"),
  and a "Skip for now" link at the bottom. Skip sets `onboarding = false` (this
  session only — the flag isn't persisted, so relaunching with still-zero
  devices initializes `onboarding = true` again).
- **Right panel** — the shared **PairPanel** (below) in its searching/confirm/
  paired states.

When the first device finishes pairing (`devices.length` transitions to ≥1 while
`onboarding` is true), the gate shows the PairPanel's brief "✓ Paired" success
state, then sets `onboarding = false` and `view = "editor"`. Because `onboarding`
is one-way state once cleared, the gate never reappears for the rest of the
session.

## PairPanel (`PairPanel.tsx`, new) — the shared device area

A single component used by both the onboarding right panel and the Devices page
add-a-device area. Replaces the popup `PairModal` (which is removed). Props:
`pending: PendingPair | null`, plus an `onPaired?` callback for the success
transition. Three states:

1. **Searching** (`pending === null`) — a spinner pill ("Searching for your
   deck…" in onboarding / "Waiting for a device…" on the Devices page) and a
   one-line hint ("Power on a deck on this network and its pairing code appears
   here").
2. **Confirm** (`pending !== null`) — the **large-code** card (visual style 1):
   "New device wants to pair", the `device_id` chip, the 6-digit code shown
   large and letter-spaced (`4 8 2 9 1 7`), the instruction "Confirm this
   matches the code on your deck's screen", an optional **Name** input, and
   **Reject** / **Pair** buttons. Pair calls `confirmPairing(device_id, name ||
   device_id)`; Reject calls `rejectPairing(device_id)`. Errors from either IPC
   surface inline (no silent failure); the card stays open on error.
3. **Paired** (brief, after a successful Pair) — a green "✓ Paired" confirmation
   for ~900ms, then the panel returns to searching (and onboarding uses this as
   the cue to advance to the Editor).

The card markup/logic is migrated from the existing `PairModal.tsx` (same
`confirmPairing`/`rejectPairing` calls, busy guard, inline error, name fallback)
but rendered **inline in the panel** rather than inside a `.modal-overlay`.

## Pair request while in the Editor

If `pending` becomes set while `view === "editor"` (a device is already paired
and a *new* one wants in — so the gate is not active), the app does **not**
hijack the screen. Instead:

- a dismissible banner appears at the top of the editor ("A deck wants to pair —
  review in Devices") with a button that sets `view = "devices"`, and
- the Devices tab shows its badge.

On the Devices page (or the onboarding gate), the PairPanel shows the confirm
card directly — no banner needed there.

## Devices page (`DevicesPage.tsx`, new)

Reuses the split layout:

- **Left** — "Paired decks" section: one row per registry device (from
  `devices`), each with an online (filled green) / offline (hollow) status dot,
  `name || device_id`, a meta line (`device_id · fw X` when online, `device_id ·
  offline` otherwise), and a **Forget** button calling `forgetDevice(device_id)`
  (with the existing forget error handling). Empty state: "No decks paired yet."
- **Right** — "Add a device": the shared **PairPanel**, so a newly powered deck's
  confirm card appears here exactly as in onboarding.

## Files

- `desktop/src/App.tsx` — modify: become the shell (lifted `devices` poll, `view`
  + `skippedOnboarding` state, tab bar, gate/editor/devices switch, editor pairing
  banner). Keep the existing `deck-pair-request`/`deck-pair-cancel` listeners.
- `desktop/src/components/Onboarding.tsx` — new: the first-run gate.
- `desktop/src/components/PairPanel.tsx` — new: shared searching/confirm/paired
  device area (absorbs `PairModal`'s confirm logic).
- `desktop/src/components/DevicesPage.tsx` — new: paired list + add-a-device.
- `desktop/src/components/AppTabs.tsx` — new (or inline in App): the Editor |
  Devices tab bar with the pending badge.
- `desktop/src/components/PairModal.tsx` — remove (replaced by PairPanel).
- `desktop/src/components/DevicesPanel.tsx` — remove (replaced by DevicesPage).
- `desktop/src/App.css` — modify: tab bar, onboarding split, PairPanel states,
  devices page, editor pairing banner; remove the old `.devices-panel` strip and
  `.modal-overlay`/`.pair-modal` styles that are no longer used.
- `desktop/src/api.ts` — unchanged (all needed wrappers exist).

## Visual spec (from approved mockups)

- Dark surfaces: window `#16161a`, panels `#1a1a20`, rows `#1b1b21`, borders
  `#2c2c34`; text `#e6e6ea` / muted `#9a9aa6` / faint `#7a7a86`.
- Accent green `#3ECF8E` (online dot with subtle glow, Pair button, step
  numbers, spinner); reject red `#d66`.
- Split panels divided by a 1px `#2c2c34` border; right/add panel slightly
  lighter (`#1a1a20`) than the left.
- 6-digit code: large (~40px), weight 700, letter-spacing ~0.16em, spaced
  digits.
- Tabs: active tab uses the window background with a top border, inactive muted.

## Edge cases

- **Skip then relaunch with no devices** — gate returns: `onboarding`
  initializes `true` again from the empty poll because the skip was session-only
  and never persisted.
- **Pending device disconnects mid-confirm** — `deck-pair-cancel` clears
  `pending`; the PairPanel returns to searching and the editor banner (if shown)
  dismisses. (Existing event handling.)
- **Forget the last device while in Editor/Devices** — `devices.length` drops to
  0, but `onboarding` stays `false` (it is one-way state, only set `true` during
  the initial empty poll). The gate does **not** re-assert mid-session.
  Rationale: yanking a working session back to onboarding because you forgot a
  device is jarring. The Devices page already shows the empty state and the
  add-a-device panel, which is sufficient.
- **Pair request arrives while already on the Devices page** — the PairPanel
  shows the confirm card directly; no banner/badge needed.
- **Two devices want to pair at once** — `pending` holds one at a time (matches
  current single-`pending` behavior); a second `deck-pair-request` replaces it.
  Acceptable for v1 (pairing is a deliberate one-at-a-time act); not worth a
  queue.

## Testing

Manual (no JS test harness in this project):

1. **First run** — clear the desktop config (or use a machine with none); launch
   → onboarding gate with "Searching". Power on an unpaired deck → confirm card
   with the code matching the device screen → Pair → "✓ Paired" → app lands in
   the Editor. Relaunch → opens straight to Editor (no gate).
2. **Skip** — first run, click "Skip for now" → Editor with no devices; relaunch
   → gate returns.
3. **Devices page** — Editor → Devices tab → paired deck listed online; power-
   cycle the deck → dot goes hollow/offline then back. Forget → row removed,
   deck drops and re-enters pairing on reconnect.
4. **Pair while editing** — with one deck paired, on the Editor tab, trigger a
   new deck's pairing → banner + Devices-tab badge appear (no hijack); clicking
   goes to Devices with the confirm card.
5. **Reject** — confirm card → Reject → device shows "Rejected"; no registry
   entry added.

## Out of scope

- Backend/firmware/protocol changes (none needed).
- A multi-device pairing queue (one-at-a-time is fine for v1).
- Renaming a device after pairing (still set-at-pair-time, per the prior spec).
- Persisting the "skip" choice across launches.
