---
status: accepted
date: 2026-09-07
---

# A hotkey-activated profile is a sticky override that suspends executable triggers

Profiles can now be activated two ways: automatically when a configured executable owns the foreground window, or manually by a global hotkey. When both could apply, a hotkey press wins and keeps winning: it puts the application into an **override** state where the active profile is pinned and executable triggers are ignored, until the reset hotkey is pressed or the application restarts. Pressing the active profile's own hotkey again is a no-op, not a toggle.

## Considered options

- **Most recent event wins.** A hotkey press and a foreground change are peers. Rejected because any alt-tab into a configured game would silently undo a deliberate hotkey choice, making hotkeys nearly useless on the desktop.
- **Global mode switch** (Automatic vs. Manual). Rejected because users forget which mode they are in, and it needs its own UI and hotkey.
- **Sticky override** (chosen). Matches the mental model "I pressed a key, so the colours stay until I say otherwise", and gives hotkeys a distinct purpose alongside automatic switching: overriding the automatic choice for one session.

## Consequences

- A user who forgets they pressed a hotkey will see automatic switching "stop working". The footer therefore labels the active profile with "(hotkey)" while an override is in place.
- The override is deliberately not persisted; startup always applies the Windows profile.
- Disabling a profile unregisters its hotkey as well as its executable trigger, so "Enabled" keeps one meaning.
