# 03: Hotkey trigger activates a profile as an override

**What to build:** A profile whose `Hotkey` field in `profiles.json` holds a combination such as `Ctrl+Alt+1` can be activated from anywhere, including while the application is minimized to the tray, by pressing that combination. The profile is applied to all displays and the application enters an override: executable triggers are ignored until the reset hotkey (next ticket) or a restart. The footer gains an active-profile indicator that shows the profile name and, during an override, the `(hotkey)` suffix. There is no capture UI yet; the binding is entered by editing the JSON file by hand, which is enough to demo and verify the whole path.

**Blocked by:** 01 (Extract the switching decision into a pure, tested module)

**Status:** ready-for-agent

**Spec:** `.scratch/hotkey-triggers/spec.md`, sections "Precedence", "Switching decision module", "Hotkey combination", "Persistence", "Registration lifecycle", "UI > Footer"

## Notes

- Hotkey text parse and format are pure functions living beside the decision module and covered by the same test executable. Canonical modifier order is Ctrl, Shift, Alt, Win; key names are listed in the spec. Parsing is case-insensitive and whitespace-tolerant. A modifier is optional.
- The decision module gains the `HotkeyPressed(profile name)` and `ProfilesChanged` events and the override rules: a hotkey press for an enabled profile sets the override and applies the profile if it is not already active; pressing the active profile's own hotkey is a no-op; foreground changes during an override yield no change; if the pinned profile is disabled or removed the override ends and Windows is restored.
- Registration uses the Win32 global hotkey API with the no-repeat flag. Never a low-level keyboard hook. Every enabled profile with a parseable binding is registered after load and fully re-registered (unregister all, register all) after any save, add, remove, or enable change, and all are unregistered on exit.
- A registration Windows rejects is remembered as unavailable, the binding stays in the file, and it is retried on the next re-registration. The footer shows a danger-coloured "hotkey unavailable" marker while any binding is unavailable.
- A `Hotkey` string that does not parse is preserved verbatim on save and never registered.
- Startup emits the module's `Startup` event, so the override is always clear after a restart.
- The active-profile indicator is new; today the footer never displays the active profile. Place it between the driver version and the version string, label "Active".

## Acceptance criteria

Items marked "(module tests)" are proven by the console tests; items marked "(manual)" are built and wired but await ticket 07's run on a real GPU.

- [x] With `"Hotkey": "Ctrl+Alt+1"` on a profile in `profiles.json`, pressing Ctrl+Alt+1 applies that profile to all displays, including while minimized to the tray. (manual)
- [x] While that override is in place, alt-tabbing into a different configured game does not change the active profile, and alt-tabbing to the desktop does not restore Windows. (module tests)
- [x] Pressing the hotkey of a second profile switches to it and keeps the override. (module tests)
- [x] Pressing the active profile's own hotkey does nothing. (module tests)
- [x] Disabling or removing the pinned profile ends the override and restores the Windows profile. (module tests; Save/Add/Remove all route through the module)
- [x] Restarting the application restores the Windows profile and clears the override. (module tests for the Startup event)
- [x] A disabled profile's hotkey is not registered and does nothing. (registration skips disabled profiles; module ignores the event regardless)
- [x] A bare key with no modifier (for example `F9`) is accepted and works. (module tests for parsing; registration manual)
- [x] Save writes `"Hotkey"` for every profile (empty string when unbound); files without the field load unchanged.
- [x] When another application already owns the combination, the footer shows the "hotkey unavailable" marker and the binding stays in the file. (manual)
- [x] Footer shows `Active <name>` normally and `Active <name> (hotkey)` during an override. (manual)
- [x] Tests cover every override rule listed in the spec and the hotkey text round-trip, lower-case and spaced parsing, rejection of unknown names and modifier-only strings, and canonical equality.
- [x] CHANGELOG Unreleased section mentions hotkey triggers.

## Comments

**2026-09-08, implemented.** New pure unit `hotkey.h` / `hotkey.cpp` (parse, format, canonical text, key-name lookup; modifier bits equal the Win32 `MOD_*` values). The switching module gained `HotkeyPressed` and `ProfilesChanged` events with the override rules from ADR 0001. Tests: 225 checks across `tests/switching_tests.cpp` and `tests/hotkey_tests.cpp`, one executable.

Win32 layer: profile `hotkey` field, JSON load/save, a registered-hotkey table with full re-registration after any profile change, the hotkey window message dispatching to the module, unregistration on window destroy, registration at startup after the window exists. Save/Add/Remove now go through one commit helper (save, re-register, emit `ProfilesChanged`). Saving a profile re-applies it if it is the active one, otherwise hands the current foreground to the module, so a save never bypasses an override. The footer gained the "Active" indicator with the `(hotkey)` suffix and the danger-coloured "hotkey unavailable" marker; long names are shortened with an ellipsis so the footer links are never overlapped.

Decisions made while implementing, all within the spec:
- A binding that does not parse counts as unavailable (footer marker), the same as one Windows rejected.
- Renaming the profile that is pinned by an override ends the override on save, because the module sees the old name as removed. Acceptable edge case; noted for ticket 07.
- Saving the Windows profile during an override no longer marks Windows as active; the override stays pinned.
- Hotkey ids start at 100 and are assigned per registration pass, so the id-to-profile table is rebuilt every time.

Not done here: the capture UI (ticket 05), the reset hotkey (ticket 04), and any run against a real GPU (ticket 07).
