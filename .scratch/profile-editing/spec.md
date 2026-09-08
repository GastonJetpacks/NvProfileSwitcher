# Spec: Live preview and defaults while editing a profile

Status: ready-for-agent
Written: 2026-09-08
Source: user request of 2026-09-08; closes PRD gaps C6 (live preview) and C7 (reset to defaults)

## Problem Statement

Editing a profile is blind. Moving a slider changes a number on screen but not the colours on the display until Save profile is clicked, so tuning is a loop of guess, save, look, adjust. There is also no quick way back to neutral colours once a profile has drifted; every slider has to be dragged back by hand.

## Solution

Slider changes are applied to the selected display immediately, as a preview, while the profile is being edited. Nothing is saved until Save profile is clicked; leaving the profile or changing the display discards the preview and puts the real active profile back on screen. A Defaults button next to Save profile returns the five sliders to neutral values, previewed the same way.

## User Stories

1. As a user, I want to see the colours change on the display while I drag a slider, so that I can tune a profile by eye.
2. As a user, I want the preview to hit the display I have selected in the Display box, so that multi-monitor profiles can be tuned one monitor at a time.
3. As a user, I want the preview to be discarded if I select another profile or another display without saving, so that unsaved experiments never stick.
4. As a user, I want the preview to be discarded when I minimize the window without saving, so that the desktop does not stay in half-edited colours.
5. As a user, I want Save profile to keep working exactly as before, so that what I see is what gets stored.
6. As a user, I want a Defaults button that sets brightness, contrast, gamma, vibrance and hue back to neutral, so that I can start over without dragging five sliders.
7. As a user, I want Defaults to preview but not save, so that I can still cancel by selecting another profile.
8. As a user, I want automatic switching and hotkey overrides to win over a preview, so that a game coming to the foreground shows its real profile.

## Implementation Decisions

- **Preview** is a new glossary term: unsaved slider values applied to the selected display while a profile is being edited. It is not a profile and never becomes the active profile.
- The preview applies on slider movement with a short debounce (a few tens of milliseconds) so dragging does not flood the driver. It targets only the display selected in the Display box, using the same apply path the switching engine uses.
- A preview is "dirty" from the first slider move until it is discarded or saved. Discarding means re-applying the real state: the active profile to all its displays, or the Windows profile if none is active. Discard happens on profile selection change, display selection change, profile removal, and minimize.
- Save profile already applies the real values; after a save of a profile that is not active, the real active state is re-applied so the preview does not linger on a display.
- Defaults sets the sliders to brightness 100, contrast 100, gamma 1.00, digital vibrance 50, hue 0 (the driver-neutral values already used for new Windows profiles), refreshes the labels, and schedules a preview. It does not save.
- Defaults sits to the left of Save profile, same size, muted styling, and moves with it between the Windows and other-profile layouts.
- Switching actions performed by the module are unaffected; they simply overwrite whatever the preview put on screen.

## Testing Decisions

- The pure switching module is untouched; no new tests there.
- Preview and Defaults are Win32 and NVAPI behaviour and are verified manually: drag a slider and watch the display; select another profile and watch it revert; change display; minimize; press Defaults and watch neutral colours appear; save and confirm persistence.

## Out of Scope

- Per-control reset (one slider at a time).
- Previewing on displays other than the selected one.
- A Cancel or Revert button (selecting another profile does that).
- Undo history.
