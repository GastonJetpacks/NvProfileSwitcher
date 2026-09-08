# Spec: Hotkey triggers

Status: ready-for-agent
Written: 2026-09-08
Sources: grilling session of 2026-09-07 (handoff), `CONTEXT.md`, ADR 0001, `docs/PRD.md`

## Problem Statement

NvProfileSwitcher only changes colours when a configured executable owns the foreground window. Users who want a profile's colours at other times, for example boosted vibrance while watching a video on the desktop, a dark-game gamma while a game runs under a launcher the app cannot match, or simply to compare two profiles, have no way to ask for it. They have to open the app, edit values, and press Save, which also rewrites the profile.

There is also no way to pin a profile. Alt-tabbing out of a game returns the desktop to the Windows profile whether or not the user wanted that.

## Solution

Each profile may carry an optional global hotkey. Pressing it applies that profile to every display and puts the application into an override: the active profile is pinned and executable triggers are ignored. A single application-wide reset hotkey returns to the Windows profile and ends the override. Restarting the application also ends it.

The only visible feedback is the footer, which gains an active-profile indicator that reads `<name> (hotkey)` while an override is in place. There is no notification, sound, or overlay.

Alongside this, the fork takes on its own identity: the update checker and GitHub links point at this fork, and the UI stops calling profiles "games", since a profile with only a hotkey trigger need not be a game at all.

## User Stories

1. As a user, I want to assign a key combination to a profile, so that I can apply its colours without the configured executable being in the foreground.
2. As a user, I want the hotkey to work globally while the application is minimized to the tray, so that I never need to bring the window up.
3. As a user, I want the hotkey to apply the profile to all of my displays at once, so that it behaves exactly like automatic switching.
4. As a user, I want a hotkey-activated profile to stay applied when I alt-tab into or out of a configured game, so that a deliberate key press is not silently undone.
5. As a user, I want pressing a different profile's hotkey to switch straight to that profile, so that I can compare profiles with a few key presses.
6. As a user, I want pressing the active profile's own hotkey again to do nothing, so that a double press never toggles me back to the desktop by surprise.
7. As a user, I want a reset hotkey that returns every display to the Windows profile and resumes automatic switching, so that I can end an override without opening the window.
8. As a user, I want the reset hotkey to be unbound by default, so that the application never grabs a key I did not choose.
9. As a user, I want to use any key, with or without modifiers, so that I can pick keys that suit my keyboard and games rather than being forced to hold Ctrl or Alt.
10. As a user, I want to record a hotkey by clicking a field and pressing the keys, so that I never have to type a key name by hand.
11. As a user, I want the recorded hotkey shown as readable text such as `Ctrl+Alt+1`, so that I can see at a glance what is bound.
12. As a user, I want a clear button next to the hotkey field, so that I can unbind a hotkey without recording a different one.
13. As a user, I want the reset hotkey configured in Application Settings using the same kind of field, so that both kinds of hotkey behave identically.
14. As a user, I want saving a profile to be blocked when its hotkey is already used by another profile or by the reset hotkey, with a message naming the conflicting binding, so that two bindings never race for one key.
15. As a user, I want to be told when Windows refuses to register a hotkey because another application already owns it, so that I understand why pressing it does nothing.
16. As a user, I want a refused hotkey to stay saved and be retried on the next start, so that a temporary clash with another program does not erase my binding.
17. As a user, I want the footer to show which profile is active and whether it was chosen by hotkey, so that I can tell why automatic switching appears to have stopped.
18. As a user, I want the override to end when the application restarts, so that the desktop always comes up in its normal colours.
19. As a user, I want disabling a profile to disable its hotkey along with its executable trigger, so that "Enabled" means one thing.
20. As a user, I want removing or disabling the profile that is currently pinned by hotkey to end the override, so that I am never stuck on colours I can no longer see or edit.
21. As a user, I want the Windows profile to have no hotkey field of its own, so that the reset hotkey is the one obvious way back to desktop colours.
22. As a user, I want my existing `profiles.json` to load unchanged after upgrading, so that adding hotkeys never costs me my profiles.
23. As a user, I want profiles without a hotkey to keep working exactly as before, so that I can adopt hotkeys one profile at a time.
24. As a user, I want the Add button and labels to say "profile" rather than "game", so that the UI matches what a hotkey-only profile actually is.
25. As a user of this fork, I want the update checker and the GitHub link to point at this fork's releases, so that I am offered the right builds.
26. As a maintainer, I want the switching decision covered by automated tests that run in CI, so that hotkey and executable trigger interactions cannot regress silently.
27. As a maintainer, I want the hotkey text format covered by tests, so that what the field shows always round-trips through `profiles.json`.

## Implementation Decisions

### Domain vocabulary

Use the terms in `CONTEXT.md` throughout: Profile, Windows profile, Active profile, Executable trigger, Hotkey trigger, Reset hotkey, Override, Automatic switching, Display. Never "game profile", "binding", "manual mode".

### Precedence (ADR 0001)

A hotkey press starts an override. While an override is in place the active profile is pinned and executable triggers are ignored. The override ends when the reset hotkey is pressed, when the pinned profile is disabled or removed, or when the application restarts. Pressing the active profile's own hotkey is a no-op, never a toggle. The override is not persisted.

### Switching decision module

The decision of what to do next is extracted from the timer handler into its own translation unit of pure C++ with no Win32 or NVAPI dependency. This is the single test seam for the feature.

- **Inputs**: the current switching state (active profile name, override flag), the list of profiles reduced to what the decision needs (name, executable basename, enabled flag), and one event.
- **Events**: Startup; ForegroundChanged carrying the foreground process basename; HotkeyPressed carrying a profile name; ResetHotkeyPressed; ProfilesChanged (a profile was saved, disabled, or removed).
- **Output**: the new switching state plus one action: none, apply the named profile to all displays, or restore the Windows profile to all displays.
- **Rules**:
  - Startup yields state `{Windows, no override}` and the restore action.
  - ForegroundChanged during an override yields no change.
  - ForegroundChanged without an override matches enabled profiles by executable basename, case-insensitive; the next active profile is the first match or Windows; an action is emitted only when it differs from the current active profile.
  - HotkeyPressed for an enabled profile sets the override flag; it emits the apply action only if that profile is not already active.
  - HotkeyPressed for a disabled or unknown profile yields no change.
  - ResetHotkeyPressed clears the override flag and, if the active profile is not Windows, emits the restore action with Windows as the new active profile. Automatic switching resumes on the next foreground event.
  - ProfilesChanged during an override whose pinned profile is now disabled or missing clears the override and behaves like a ResetHotkeyPressed. Otherwise no change.
- The Win32 layer owns polling, hotkey registration, and NVAPI calls; it translates the timer tick and the hotkey message into events, calls the module, and performs the returned action. The existing polling interval is unchanged.
- The Win32 layer also emits Startup at launch, which replaces the current hard-coded reset of the active profile name after the initial restore.

### Hotkey combination

- A hotkey is a set of modifiers (Ctrl, Shift, Alt, Win, each optional) plus one virtual key. **A modifier is not required**; a bare key such as `F9` or `Numpad1` is valid. The consequence, which the user accepted, is that a bare key is consumed system-wide while the application runs.
- Text form: modifiers in the fixed order `Ctrl`, `Shift`, `Alt`, `Win`, joined with `+`, followed by the key name. Key names: `A`..`Z`, `0`..`9`, `F1`..`F24`, `Numpad0`..`Numpad9`, `NumpadAdd`, `NumpadSubtract`, `NumpadMultiply`, `NumpadDivide`, `NumpadDecimal`, `Space`, `Tab`, `Enter`, `Esc`, `Backspace`, `Insert`, `Delete`, `Home`, `End`, `PageUp`, `PageDown`, `Up`, `Down`, `Left`, `Right`, `Pause`, `ScrollLock`, `PrintScreen`, `CapsLock`, `NumLock`, and the OEM punctuation keys by their US-layout character (semicolon, equals, comma, minus, period, slash, backtick, brackets, backslash, apostrophe). Anything else is rejected by the capture field.
- Parsing is case-insensitive and tolerates whitespace around `+`. Formatting is canonical so that two bindings compare equal by string equality after normalisation.
- Parse and format live in the same pure module as the switching decision (or a sibling pure unit) so they share the test executable.
- Registration uses the Win32 global hotkey API with the no-repeat flag and the corresponding hotkey window message. No low-level keyboard hook is installed, to stay clear of anti-cheat systems.
- Modifier-only combinations (for example `Ctrl+Shift` with no key) are rejected by the capture field.

### Persistence

- Each profile object gains an optional `"Hotkey"` string in the canonical text form. Absent or empty means unbound.
- The top level gains an optional `"ResetHotkey"` string with the same semantics.
- Files without these fields load unchanged. Saving always writes both fields, writing an empty string for unbound.
- A `Hotkey` string that fails to parse is preserved verbatim on save, never registered, and shown as unavailable in the capture field.

### Registration lifecycle

- All hotkeys (each enabled profile with a binding, plus the reset hotkey) are registered after settings load at startup and re-registered in full after any save, add, remove, or enable change. Re-registration unregisters everything first.
- Each registration result is remembered. A binding Windows rejects is marked unavailable; the binding stays saved and is retried at the next startup or re-registration.
- All hotkeys are unregistered on exit.

### Conflicts

- Saving a profile whose hotkey equals another profile's hotkey or the reset hotkey is blocked. The save does not happen, and a message box names the other profile (or "the reset hotkey").
- Saving the reset hotkey that equals a profile's hotkey is likewise blocked, naming the profile.
- Comparison is on the canonical text form, so `ctrl + a` conflicts with `Ctrl+A`.

### UI

- **Capture field**: a new owner-drawn control used in two places. Clicking it gives it focus and shows a "Press a key..." prompt; the next key press (with whatever modifiers are held) is recorded and shown as text; Escape while recording cancels without change. A clear button beside it unbinds. When the binding is unavailable the field shows the text with an "(unavailable)" suffix in the danger colour. Focus leaving the field ends recording.
- **Profile Settings**: the hotkey field appears for every profile except the Windows profile, on the same row as the Enabled checkbox, to its right. Selecting the Windows profile hides it along with the other per-profile controls.
- **Application Settings**: a "Reset hotkey" field appears to the right of the existing two rows of checkboxes.
- **Footer**: gains an active-profile indicator (label "Active" followed by the profile name) between the driver version and the version string. During an override the name is followed by ` (hotkey)`. While any binding is unavailable, the indicator is followed by a danger-coloured "hotkey unavailable" marker.
- **Wording**: the "Add game" button becomes "Add profile". The "Game executable" label becomes "Executable". README and CHANGELOG wording follows.
- The window remains fixed size; the new controls fit in the existing empty space on those rows.

### Fork identity

- The GitHub link, the update checker path, and the installer's URL fields point at `GastonJetpacks/NvProfileSwitcher`.
- The Ko-fi link and FUNDING file are left as they are.
- The manifest identity mismatch noted in the PRD is not touched by this spec.

## Testing Decisions

- A good test exercises observable behaviour of the pure module through its public interface: given a state, profile list, and event, assert the returned state and action. Tests never reach into globals, Win32, or NVAPI.
- **Switching decision** is tested for every rule above, including: startup restores Windows; a foreground change to a configured executable applies it; a foreground change during an override is ignored; a hotkey press for a non-active profile applies it and sets override; a hotkey press for the active profile is a no-op; reset clears the override and restores Windows; reset when Windows is already active emits no action; disabling the pinned profile ends the override; a hotkey for a disabled profile does nothing.
- **Hotkey text** is tested for: format of every modifier order, parse of lower-case and spaced input, round-trip of every supported key name, rejection of unknown key names and modifier-only strings, and canonical equality used for conflict detection.
- Tests are a single console executable, plain assert-style checks with a non-zero exit on failure, no test framework. CI builds it with a second compiler invocation and runs it before building the main executable. This is the first test target in the repo; there is no prior art.
- Win32 hotkey registration, the capture control, NVAPI calls, and JSON round-trip remain manually tested. The manual checklist: bind, press while minimized, alt-tab into a configured game, reset, restart, conflict message, unavailable marker when another app holds the key.

## Out of Scope

- Cycle-next / cycle-previous hotkeys.
- Any activation feedback other than the footer: no toast, sound, overlay, or tray balloon.
- Persisting the override across restarts.
- Low-level keyboard hooks or raw input.
- Hotkeys for the Windows profile other than the reset hotkey.
- Per-display hotkeys; a hotkey always targets all displays.
- Replacing the hand-rolled JSON parser, the polling timer, or splitting the rest of the single source file beyond the decision module.
- The other gaps listed in the PRD (display hot-plug, sleep/resume, exit restore, manifest identity, basename matching).

## Further Notes

- The footer indicator is new. Today the footer shows NVAPI status, driver version, and app version only; the active profile name is tracked but never displayed. ADR 0001 assumed a footer label existed, so this spec adds it.
- The switching module should also be where future events (display change, power resume) land if those PRD gaps are picked up later.
- All documentation files created so far (`CLAUDE.md`, `CONTEXT.md`, `docs/PRD.md`, `docs/adr/`, `docs/agents/`) and this `.scratch/` directory are untracked. There is no `.gitignore`. Whether `.scratch/` is committed or ignored is still an open call for the user.
