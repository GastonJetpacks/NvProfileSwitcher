# 06: Reset hotkey field in Application Settings

**What to build:** A user assigns the reset hotkey from the UI. Application Settings gains a "Reset hotkey" capture field, placed to the right of the two rows of checkboxes, using the same control as ticket 05. Recording a combination saves and registers it immediately, like the other Application Settings toggles. Setting it to a combination already used by a profile is blocked with a message naming that profile. The unavailable state is shown the same way as for profile hotkeys.

**Blocked by:** 05 (Hotkey capture field in Profile Settings)

**Status:** ready-for-agent

**Spec:** `.scratch/hotkey-triggers/spec.md`, sections "Conflicts", "UI > Capture field", "UI > Application Settings"

## Notes

- Application Settings changes save on interaction today (no Save button), so a completed recording or a clear should save and re-register at once. If the conflict check fails, revert the field to the previous value and show the message.
- Update README to describe the reset hotkey and the override it ends. Add the item to the CHANGELOG Unreleased section.

## Acceptance criteria

Implemented and compiling; interactive verification is ticket 07's job.

- [x] Application Settings shows a labelled "Reset hotkey" field and clear button to the right of the existing checkboxes, visible for every selected profile.
- [x] Recording Ctrl+Alt+0 saves it, and pressing Ctrl+Alt+0 during an override restores the Windows profile without restarting. (record commits, saves, and re-registers at once)
- [x] The clear button unbinds and saves; pressing the old combination then does nothing.
- [x] Recording a combination already used by a profile shows a message naming that profile, and the field reverts to the previous value.
- [x] A binding Windows rejected shows with the "(unavailable)" suffix and the footer marker.
- [x] A fresh install shows the field empty. ("None" placeholder)
- [x] README and CHANGELOG describe the reset hotkey.

## Comments

**2026-09-08, implemented.** Same capture control and subclass as ticket 05, keyed by control id for the unavailable lookup. The field listens for the `HKN_CHANGED` notification the control posts after a recording or a clear and commits immediately: conflict check against every profile's hotkey, revert plus message on conflict, otherwise save and re-register.

Layout: the two checkbox rows end around 390 px into the right panel, so the reset control sits in the remaining strip at the right edge, label on the first row and field plus a compact "×" clear button on the second. The field is narrow (about 110 px), so long combinations are shortened with an ellipsis; the full text is still what gets saved. If that proves too cramped in ticket 07, the fix is a layout change to the Application Settings block, not to the control.
