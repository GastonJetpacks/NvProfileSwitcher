# 02: Fork identity and "profile" wording

**What to build:** The application presents itself as this fork. The GitHub footer link and the About dialog open `GastonJetpacks/NvProfileSwitcher`, the update checker queries that fork's latest release, and the installer's URL fields point there too. The UI stops calling profiles "games": the Add button reads "Add profile" and the executable label reads "Executable". README and CHANGELOG wording follows.

**Blocked by:** None (can start immediately)

**Status:** ready-for-agent

**Spec:** `.scratch/hotkey-triggers/spec.md`, sections "Fork identity" and "UI > Wording"

## Notes

- Leave the Ko-fi support link and the FUNDING file untouched.
- Do not touch the manifest identity mismatch; it is out of scope.
- The JSON keys `Games Profiles` and `Windows Profiles` must not change; existing files have to load unchanged.
- README: rename the "Game Profile" section and the "Add game" step, and describe profiles rather than game profiles where the text is generic. Keep the screenshots as they are.
- CHANGELOG: start an Unreleased section noting the rename and the fork URLs; later tickets append to it.

## Acceptance criteria

- [x] Footer GitHub link and tray About dialog open the fork's repository page.
- [x] Update check requests the fork's latest release and compares against it.
- [x] Installer publisher, support, and updates URLs point at the fork.
- [x] The button under the profile list reads "Add profile"; the executable field label reads "Executable".
- [x] No user-visible string in the application says "game" or "Game" in the sense of a profile.
- [x] README and CHANGELOG reflect the wording change and the fork URLs.
- [x] An existing `profiles.json` loads without change. (JSON keys untouched.)

## Comments

**2026-09-08, implemented.** Source URL constants, installer URL, "Add profile", "Executable", and the About dialog tagline ("Per-application NVIDIA display color profiles for Windows") changed. README reworded and gained a fork note under the title; CHANGELOG gained an Unreleased section that also records ticket 01. Application rebuilt locally.

Left alone on purpose: the Ko-fi link and FUNDING file (per spec), the installer publisher name and the licence copyright (the original author's), and `CODE_SIGNING.md`, which describes upstream's SignPath policy and still links upstream because this fork's builds are not signed under it. The screenshot file name `game-profile.png` is unchanged; only its caption was reworded.
