# 07: End-to-end manual verification and documentation pass

**What to build:** The whole feature is verified on a real machine with an NVIDIA GPU against the manual checklist in the spec, README and CHANGELOG are complete and consistent with `CONTEXT.md` vocabulary, and any defect found is fixed or filed as a new issue in this directory. This is the release gate for the feature.

**Blocked by:** 02 (Fork identity and "profile" wording), 06 (Reset hotkey field in Application Settings)

**Status:** ready-for-human

**Spec:** `.scratch/hotkey-triggers/spec.md`, section "Testing Decisions"

## Notes

- This ticket needs a human at a Windows machine with an NVIDIA GPU; the agent can prepare the build and the checklist but cannot press keys against a real driver.
- Record results as a `## Comments` entry in this file, one line per checklist item.
- **Build ready (2026-09-08):** tickets 01 to 06 are committed on branch `feature/hotkey-triggers`. A local release-flags build of that branch is at `NvProfileSwitcher/NvProfileSwitcher.exe` (gitignored). It reads and writes the same `%APPDATA%\NvProfileSwitcher\profiles.json` as the installed version, adding `Hotkey` and `ResetHotkey` fields on first save; the old version ignores them, so switching back is safe. Close any running NvProfileSwitcher first (single instance).
- Points worth extra attention during the run, from the implementation notes: the reset field is narrow (about 110 px), and Escape with a modifier is recordable while bare Escape cancels.
- **Code review (2026-09-08)** of the whole branch found six confirmed bugs, all fixed in the follow-up commit: profiles are now identified by a runtime id rather than by name (duplicate names and a profile named "Windows" no longer misroute switching, and renaming a pinned profile keeps the override); saving the Windows profile during an override re-applies the pinned profile instead of leaving screen and state apart; the capture field's key-up flag is cleared on focus loss and the conflict message is posted after the key-up; an unparseable stored hotkey is drawn unavailable even on a disabled profile; hotkey ids derive from the profile id so a queued press cannot land on the wrong profile after a re-registration; and recording no longer unregisters hotkeys, the main window forwards a registered combination to the field instead. Hotkey text is canonicalised once at load. Extra manual checks worth doing: re-record a combination that is already bound to another profile (should capture, then the conflict message on Save), and two profiles with the same name each with their own hotkey.

## Acceptance criteria

- [ ] Bind a profile hotkey via the UI, minimize to tray, press it: colours change on every display, footer shows `(hotkey)`.
- [ ] Alt-tab into a configured game during the override: colours do not change.
- [ ] Press the reset hotkey: Windows colours return, footer drops `(hotkey)`, and the game's profile applies again on the next tick if it is still in the foreground.
- [ ] Restart the application during an override: Windows colours at startup, no override.
- [ ] Conflict messages appear for profile-vs-profile and profile-vs-reset in both directions.
- [ ] Bind a key another application already holds: field and footer show unavailable, binding survives a restart, and it registers once the other application is closed.
- [ ] A bare key hotkey (no modifier) works and is consumed only while the application runs.
- [ ] An existing pre-hotkey `profiles.json` loads unchanged and gains the new fields on the next save.
- [ ] README, CHANGELOG, and every user-visible string use `CONTEXT.md` vocabulary; no "game profile" remains.
- [ ] CI is green with the test executable running.
