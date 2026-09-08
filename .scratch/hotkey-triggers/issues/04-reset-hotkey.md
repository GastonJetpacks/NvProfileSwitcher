# 04: Reset hotkey ends the override

**What to build:** A single application-wide reset hotkey, stored as top-level `ResetHotkey` in `profiles.json`, restores the Windows profile on all displays and ends any override, so automatic switching resumes. It ships unbound. As with ticket 03 the binding is entered by editing the JSON; the capture field arrives in ticket 05.

**Blocked by:** 03 (Hotkey trigger activates a profile as an override)

**Status:** ready-for-agent

**Spec:** `.scratch/hotkey-triggers/spec.md`, sections "Precedence", "Switching decision module", "Persistence", "Registration lifecycle"

## Notes

- The decision module gains the `ResetHotkeyPressed` event: clear the override; if the active profile is not Windows, restore it and make Windows active; if Windows is already active, no action.
- After a reset, automatic switching resumes on the next foreground event. If a configured game is in the foreground at that moment, its profile is applied on the next tick. That is the intended behaviour.
- The reset hotkey is registered and re-registered alongside the profile hotkeys, uses the same unavailable handling, and is unregistered on exit.
- Absent or empty `ResetHotkey` means unbound; save always writes the field.

## Acceptance criteria

- [ ] With `"ResetHotkey": "Ctrl+Alt+0"` at the top level of `profiles.json`, pressing Ctrl+Alt+0 during an override restores the Windows profile on all displays and the footer drops the `(hotkey)` suffix.
- [ ] After the reset, bringing a configured game to the foreground applies its profile again.
- [ ] Pressing the reset hotkey when Windows is already active and no override is in place does nothing visible.
- [ ] A fresh install or a file without the field has no reset hotkey bound.
- [ ] Save writes `"ResetHotkey"` (empty string when unbound).
- [ ] When another application owns the combination, the footer shows the "hotkey unavailable" marker and the binding stays in the file.
- [ ] Tests cover reset during override, reset when Windows is already active, and reset followed by a foreground change to a configured executable.
