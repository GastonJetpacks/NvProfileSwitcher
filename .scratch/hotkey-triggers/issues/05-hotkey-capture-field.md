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

- [ ] Selecting a non-Windows profile shows a hotkey field and clear button on the Enabled row; selecting the Windows profile hides them.
- [ ] Clicking the field and pressing Ctrl+Alt+1 shows `Ctrl+Alt+1`; pressing F9 alone shows `F9`.
- [ ] Pressing Escape while recording leaves the previous binding in place.
- [ ] Clicking outside the field while recording leaves the previous binding in place.
- [ ] Holding only modifiers never completes a recording.
- [ ] The clear button empties the field; saving then writes an empty `Hotkey` and unregisters the hotkey.
- [ ] Saving a profile whose hotkey matches another profile shows a message naming that profile and does not save.
- [ ] Saving a profile whose hotkey matches the reset hotkey shows a message naming the reset hotkey and does not save.
- [ ] After a successful save the hotkey works immediately without restarting.
- [ ] A binding Windows rejected shows with the "(unavailable)" suffix in the danger colour.
- [ ] README describes assigning and clearing a profile hotkey.
