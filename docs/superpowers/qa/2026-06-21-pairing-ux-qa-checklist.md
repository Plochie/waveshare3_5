# Pairing UX Redesign — Screenshot QA Checklist

Walk these in order. For each row: do the action, take the screenshot named in
**Capture**, confirm **Expect**, and tick the box. Note anything off in **Notes**.

**Setup**
- [ ] Device flashed with current firmware: `~/.platformio/penv/bin/pio run -e waveshare_esp32s3_35 -t upload -t monitor`
- [ ] Desktop app running the current build: `cd desktop && npm run tauri dev`
- [ ] Device and Mac on the same Wi-Fi (not a guest/isolated network)
- [ ] Start from a clean slate: no devices paired yet (delete the desktop config / forget all devices, and delete `/deck/pairing.json` on the SD card so the device is unpaired)

---

## 1. First-run onboarding gate

| Step | Detail |
|---|---|
| Do | Launch the desktop app with zero devices paired |
| **Capture** | `01-gate-searching.png` |
| **Expect** | Full-window split screen: left = "Connect your deck" + numbered steps + "Skip for now"; right = spinner pill "Searching for your deck…". No editor, no tabs. |

- [ ] Pass  — Notes: ____________________

## 2. Code-confirm card appears

| Step | Detail |
|---|---|
| Do | Power on the unpaired deck; wait for it to connect |
| **Capture** | `02-confirm-card.png` (capture the device screen too, or note its code) |
| **Expect** | Right panel swaps to the large-code card: "New device wants to pair", device-id chip, big 6-digit code, name field, Reject / Pair. The code matches the code shown on the deck's screen. |

- [ ] Pass  — Notes: ____________________

## 3. Pair → success → lands in Editor

| Step | Detail |
|---|---|
| Do | (Optional) type a name, then click **Pair** |
| **Capture** | `03a-paired-success.png` (the "✓ Paired" flash), then `03b-editor.png` |
| **Expect** | Brief green "✓ Paired", then the app transitions into the Editor (grid + tabs visible). The device shows its grid. |

- [ ] Pass  — Notes: ____________________

## 4. Relaunch skips the gate

| Step | Detail |
|---|---|
| Do | Restart the desktop app (it has a paired device now) |
| **Capture** | `04-relaunch-editor.png` |
| **Expect** | Opens straight to the **Editor** — no onboarding gate. |

- [ ] Pass  — Notes: ____________________

## 5. Devices tab — paired list & live status

| Step | Detail |
|---|---|
| Do | Click the **Devices** tab; then power-cycle the deck and watch |
| **Capture** | `05a-devices-online.png`, `05b-devices-offline.png` |
| **Expect** | Left: the paired deck with a filled green dot, name, `device_id · fw …`, a **Forget** button. Right: "Add a device" / "Waiting for a device…". On power-cycle the dot goes hollow/offline, then back to green. |

- [ ] Pass  — Notes: ____________________

## 6. Forget → re-pair

| Step | Detail |
|---|---|
| Do | Click **Forget** on the paired deck; watch the device reconnect |
| **Capture** | `06a-after-forget.png`, `06b-repair-card.png` |
| **Expect** | The row disappears; the deck drops, reconnects, and its confirm card reappears in the "Add a device" panel (device shows its pairing screen again). |

- [ ] Pass  — Notes: ____________________

## 7. Skip-for-now (session only)

| Step | Detail |
|---|---|
| Do | Forget all devices so the next launch has none → relaunch → on the gate click **Skip for now**; then relaunch again |
| **Capture** | `07a-skip-into-editor.png`, `07b-gate-returns.png` |
| **Expect** | Skip drops you into the Editor with no devices. Relaunching with still-zero devices shows the gate again (skip is session-only). |

- [ ] Pass  — Notes: ____________________

## 8. Pair-while-editing (non-hijacking)

| Step | Detail |
|---|---|
| Do | With one deck already paired, sit on the **Editor** tab. Trigger a *new* (or forgotten/re-pairing) deck's pairing |
| **Capture** | `08a-editor-banner.png`, `08b-devices-confirm.png` |
| **Expect** | The editor is **not** hijacked: a banner "A deck wants to pair — review in Devices" appears + the Devices tab shows a green badge dot. Clicking "review in Devices" switches to the Devices tab where the confirm card is waiting. |

- [ ] Pass  — Notes: ____________________

## 9. Reject

| Step | Detail |
|---|---|
| Do | On a confirm card (onboarding or Devices), click **Reject** |
| **Capture** | `09-reject.png` (device screen showing "Rejected") |
| **Expect** | The device shows "✗ Rejected" then returns; no entry is added to the paired list. |

- [ ] Pass  — Notes: ____________________

---

### Result
- [ ] All scenarios pass → ready to merge to `main`
- [ ] Issues found (list screenshot names + notes above) → send them over and I'll fix

> Tip: drop the screenshots in a folder and send them back; if anything looks
> off I can adjust spacing/colors/copy from the images.
