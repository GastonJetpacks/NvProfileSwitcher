# 05: Hotkey capture field in Profile Settings

**What to build:** A user assigns a profile's hotkey from the UI. Profile Settings gains a hotkey field on the Enabled row, to the right of the checkbox. Clicking it shows "Press a key...", the next key press (with whatever modifiers are held, or none) is recorded and shown as text such as `Ctrl+Alt+1`, Escape cancels, and a clear button unbinds. Saving the profile persists and registers the binding. Saving is blocked with a message naming the other profile when the combination is already used by another profile (or by the reset hotkey). If Windows rejected the binding, the field shows it with an "(unavailable)" suffix in the danger colour.

**Blocked by:** 03 (Hotkey trigger activates a profile as an override), 04 (Reset hotkey ends the override)

**Status:** ready-for-agent

**Spec:** `.scratch/hotkey-triggers/spec.md`, sections "Hotkey combination", "Conflicts", "UI > Capture field", "UI > Profile Settings"

## Notes

- Build the capture field as a reusable owner-drawn control (one implementation, two placements), styled like the existing dark fields. Ticket 06 reuses it for the reset hotkey.
- Recording state: focus in and click starts recording; the first non-modifier key down ends it and records the combination; Escape while recording cancels without change; focus loss ends recording without change. Modifier-only presses never complete a recording.
- Reject keys not in the spec's key-name list; the field simply stays in recording state.
- The field is hidden when the Windows profile is selected, like the other per-profile controls.
- Conflict check runs on Save, using canonical text equality, and compares against every other profile's binding and the reset hotkey. The message box names the conflicting profile or says "the reset hotkey". The save does not happen.
- The window is fixed size; the field and clear button fit in the free space to the right of the "Enable automatic profile" label.
- Update README to describe assigning a hotkey.

## Acceptance criteria

All UI behaviour below is implemented and compiles; interactive verification is ticket 07's job (needs a real display and keyboard session).

- [x] Selecting a non-Windows profile shows a hotkey field and clear button on the Enabled row; selecting the Windows profile hides them.
- [x] Clicking the field and pressing Ctrl+Alt+1 shows `Ctrl+Alt+1`; pressing F9 alone shows `F9`.
- [x] Pressing Escape while recording leaves the previous binding in place.
- [x] Clicking outside the field while recording leaves the previous binding in place. (focus loss cancels)
- [x] Holding only modifiers never completes a recording.
- [x] The clear button empties the field; saving then writes an empty `Hotkey` and unregisters the hotkey.
- [x] Saving a profile whose hotkey matches another profile shows a message naming that profile and does not save.
- [x] Saving a profile whose hotkey matches the reset hotkey shows a message naming the reset hotkey and does not save.
- [x] After a successful save the hotkey works immediately without restarting. (save re-registers)
- [x] A binding Windows rejected shows with the "(unavailable)" suffix in the danger colour.
- [x] README describes assigning and clearing a profile hotkey.

## Comments

**2026-09-08, implemented.** The capture field is an owner-drawn button with a window subclass, the same mechanism the footer links use. Clicking it (or Space while focused) starts recording: the field shows "Press a key..." with an accent border; the next non-modifier key-down, with the held modifiers read from the keyboard state, is formatted through the hotkey module and shown; Escape alone cancels; focus loss cancels; the matching key-up is swallowed so it cannot re-click the button. While recording, every registered hotkey is released so an already-bound combination can be captured, and they are re-registered when recording ends.

Decisions made while implementing:
- **Escape with a modifier is recordable** (`Ctrl+Esc`, `Shift+Esc`); only a bare Escape cancels, since the spec lists `Esc` as a valid key name and also makes Escape the cancel key.
- The field reads back the saved binding in canonical form; an unparseable saved string is shown verbatim and, because it is never registered, is drawn with the "(unavailable)" suffix.
- "(unavailable)" is shown only when the field text equals the saved binding, so an edited-but-unsaved value is never flagged.
- The Clear button is a plain text owner-drawn button; clearing takes effect on Save, like every other Profile Settings change.
- The conflict check runs before any field is copied into the profile, so a blocked save leaves the profile untouched.
- Layout: label at the right of "Enable automatic profile", then the field, then Clear flush with the right edge of the panel. Fixed window, no layout changes elsewhere.

Reusable pieces for ticket 06: the subclass proc, the draw routine (keyed on control id for the unavailable lookup), and a `HKN_CHANGED` notification the field posts to the main window when a binding is recorded or cleared. The Profile Settings field ignores it (Save applies); the reset field will save on it.
