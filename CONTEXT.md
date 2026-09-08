# NvProfileSwitcher

A Windows utility that applies NVIDIA display color settings as named profiles, switching between them when a configured game takes the foreground or when the user presses a configured hotkey.

## Language

### Profiles

**Profile**:
A named set of color values (brightness, contrast, gamma, digital vibrance, hue) held independently for each display. Not an NVIDIA driver 3D profile.
_Avoid_: Game profile, preset, scheme, color profile (ambiguous with ICC profiles)

**Windows profile**:
The one special profile representing normal desktop colors. It cannot be deleted, is applied at startup, and is applied whenever no other profile is active.
_Avoid_: Desktop profile, default profile, baseline

**Active profile**:
The profile most recently applied to the displays. Exactly one profile is active at any time.
_Avoid_: Current profile, selected profile (that means highlighted in the UI list)

### Triggers

**Trigger**:
A condition that causes a profile to become active. A profile may have zero, one, or both kinds of trigger.

**Executable trigger**:
A trigger that fires when a process with the configured executable name owns the foreground window.
_Avoid_: Auto trigger, game detection, process match

**Hotkey trigger**:
A trigger that fires when the user presses the profile's configured global key combination.
_Avoid_: Keybinding, shortcut, binding

**Reset hotkey**:
The single application-wide key combination that activates the Windows profile and ends any override.
_Avoid_: Default hotkey, clear hotkey, Windows hotkey

### Switching

**Override**:
The state entered when a hotkey trigger fires. While an override is in place the active profile is pinned and executable triggers are ignored. It ends when the reset hotkey is pressed or the application restarts.
_Avoid_: Manual mode, lock, pinned mode

**Automatic switching**:
The behaviour when no override is in place: the active profile follows executable triggers, falling back to the Windows profile when none matches.
_Avoid_: Auto mode, detection

### Displays

**Display**:
One physical monitor driven by the NVIDIA GPU, as identified to both Windows and NVAPI. Profiles hold one set of values per display.
_Avoid_: Monitor, screen, target, output

**Primary display**:
The display Windows marks as primary. Preselected when editing a profile.
