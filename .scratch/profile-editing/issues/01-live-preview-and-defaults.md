# 01: Live preview while editing, and a Defaults button

**What to build:** Moving any slider in Profile Settings applies the values to the selected display immediately as a preview, without saving. Selecting another profile, another display, removing the profile, or minimizing the window discards the preview and puts the real active profile back. A Defaults button next to Save profile sets the five sliders to neutral values and previews them. Save profile persists as before.

**Blocked by:** None (can start immediately)

**Status:** ready-for-agent

**Spec:** `.scratch/profile-editing/spec.md`

## Acceptance criteria

Implemented and compiling; the display-facing items need a manual run on the NVIDIA machine (add to the ticket 07 checklist of the hotkey-triggers feature).

- [x] Dragging a slider changes the selected display's colours while dragging, with no visible stutter. (40 ms debounce; manual)
- [x] The preview affects only the display chosen in the Display box.
- [x] Selecting another profile without saving restores the real active profile's colours.
- [x] Changing the display in the Display box without saving restores the real colours on the previously previewed display.
- [x] Minimizing without saving restores the real colours.
- [x] Save profile stores the slider values; the saved profile behaves as before.
- [x] Saving a profile that is not the active one does not leave its colours on screen.
- [x] Defaults sets brightness 100, contrast 100, gamma 1.00, vibrance 50, hue 0, updates the labels, and previews; nothing is saved until Save profile.
- [x] Defaults is visible for both the Windows profile and other profiles and sits next to Save profile in both layouts.
- [x] A game coming to the foreground (or a hotkey) during a preview applies its real profile. (switching actions simply overwrite the preview)
- [x] CONTEXT.md gains the Preview term; README, CHANGELOG, and PRD rows C6 and C7 are updated.

## Comments

**2026-09-08, implemented.** Slider movement schedules a 40 ms timer; when it fires the slider values are applied to the display selected in the Display box through the same apply path the switching engine uses, and the preview is marked dirty. Discarding (selection change, display change, minimize, and implicitly Remove, which reloads the selection) re-applies the real state: the active profile, or the Windows profile. Save clears the dirty flag; saving the Windows profile while another profile is active re-applies that profile, and saving an inactive profile after a preview re-applies the active one, so a preview never lingers.

**2026-09-08, follow-up: janky sliders.** The first cut applied the preview on the UI thread from a 40 ms timer. Each apply is four synchronous driver calls (read and set vibrance, set hue, upload a 1024-entry gamma LUT), tens of milliseconds each, so the message loop stalled and the thumb froze mid-drag; the timer also fired in the micro-pauses of a drag, landing the stall in the middle of the motion. Fixed by moving preview application to a worker thread: the UI thread posts the latest slider values on every scroll message (no timer), the worker applies only the most recent request and skips controls whose values did not change, and all NVAPI apply calls from either thread go through one mutex. A generation counter cancels queued or in-flight previews before the real state is re-applied on discard or save, so a stale preview can never overwrite it. The worker starts on first use and is joined on window destroy before NVAPI unloads.

Defaults loads the struct's default values (the same neutral values new Windows profiles start from) into the sliders and schedules a preview. It sits left of Save profile with the same size and moves with it between the two layouts. Per-control reset stays out of scope, so PRD row C7 is "Partial".
