# NvProfileSwitcher

**Per-application NVIDIA display color profiles for Windows.**

NvProfileSwitcher is a lightweight native Windows utility that automatically
switches NVIDIA display color settings based on the application currently in
the foreground.

Configure your normal Windows colors once, create individual profiles for your
games and applications, and NvProfileSwitcher handles the switching
automatically.

When a configured application becomes active, its color profile is applied. As
soon as you switch back to Windows, your browser, Discord, or another
application, your Windows profile is automatically restored.

> This is a fork of [mgcarnevali/NvProfileSwitcher](https://github.com/mgcarnevali/NvProfileSwitcher).
> Releases and the in-app update check come from this repository.

> NvProfileSwitcher is not affiliated with, endorsed by, or sponsored by
> NVIDIA Corporation. NVIDIA is a trademark of NVIDIA Corporation.

## Features

- Automatic per-application profile switching
- Automatic Windows profile restoration
- Profiles matched by executable (`.exe`)
- Brightness control
- Contrast control
- Gamma control
- Digital Vibrance control
- Hue control
- Per-display profiles
- Multi-monitor support
- Separate Windows color settings for each configured display
- Individual executable icons
- Redesigned dark interface with dedicated Profiles, Profile Settings, and Application Settings sections
- Integrated NVIDIA API status, driver version, and application version footer
- Enable or disable individual profiles
- Start automatically with Windows
- Start minimized to the system tray
- Optional minimize-to-tray behavior
- Single-instance support
- Built-in update checker with optional automatic checks
- Lightweight native C++ application
- Portable — no installation required
- No .NET runtime required

## Screenshots

### Profile

![NvProfileSwitcher Profile](docs/screenshots/game-profile.png)

### Windows Profile

![NvProfileSwitcher Windows Profile](docs/screenshots/windows-profile.png)

### System Tray

![NvProfileSwitcher System Tray](docs/screenshots/system-tray.png)

## Interface

The interface is organized into dedicated **Profiles**, **Profile Settings**,
and **Application Settings** sections.

The footer provides at-a-glance NVIDIA API status, NVIDIA driver information,
the running NvProfileSwitcher version, and direct access to GitHub, support,
and application information.

## Installation

Download the latest version from the **Releases** section.

Before extracting the archive, Windows may mark the downloaded file as coming
from the Internet. To avoid Windows blocking the executable:

1. Right-click `NvProfileSwitcher-vX.X.X.zip`.
2. Select **Properties**.
3. Under the **General** tab, check **Unblock** if the option is available.
4. Click **Apply** and **OK**.
5. Extract the ZIP and run `NvProfileSwitcher.exe`.

No installer or additional runtime is required.

### Windows SmartScreen

NvProfileSwitcher is currently distributed as an unsigned executable. Because
of this, Microsoft Defender SmartScreen may display a warning when launching
the application for the first time.

This warning does not necessarily indicate that NvProfileSwitcher contains
malware. Unsigned applications with limited download reputation can trigger
SmartScreen until they establish sufficient reputation.

Always download NvProfileSwitcher from the official GitHub repository or its
Releases page.

### Requirements

- Windows 10 or Windows 11 (x64)
- NVIDIA GPU
- NVIDIA display driver with NVAPI support

## Usage

1. Launch `NvProfileSwitcher.exe`.
2. Select **Windows** and configure your normal desktop color settings.
3. Click **Add profile**.
4. Select the application's executable.
5. Choose the display where the application runs.
6. Configure the desired color settings. The selected display previews every
   change live, so you can tune by eye. Nothing is stored until you save;
   selecting another profile or display discards the preview.
7. Click **Save profile**. **Defaults** puts all five controls back to neutral
   values (brightness 100, contrast 100, gamma 1.00, vibrance 50, hue 0),
   previewed the same way.

NvProfileSwitcher will now detect when that application owns the foreground
window and automatically apply its profile.

Switch away from the application and your Windows profile is restored
automatically.

## Hotkeys

Any profile can also be activated with a global key combination.

1. Select the profile and click the **Hotkey** field in Profile Settings.
2. Press the key or combination you want, for example `Ctrl+Alt+1` or `F9`.
   A modifier key is optional.
3. Click **Save profile**.

Pressing the hotkey applies the profile to all displays, even while
NvProfileSwitcher is minimized to the tray, and starts an **override**: the
profile stays active and automatic switching is paused until you restart
NvProfileSwitcher. The footer shows the active profile with **(hotkey)** while
an override is in place. Pressing another profile's hotkey switches to that
profile; pressing the active profile's own hotkey does nothing.

Use **Clear** next to the field to remove a hotkey. Two profiles cannot share
a combination: saving is blocked with a message naming the other profile.

If another application already owns the combination, the field and the footer
show it as unavailable. The hotkey is kept and retried the next time
NvProfileSwitcher starts.

Press **Esc** while the field says "Press a key..." to cancel without changing
the hotkey. A key without a modifier is captured system-wide while
NvProfileSwitcher runs, so other applications will not see it.

### Reset hotkey

The **Reset hotkey** field in Application Settings sets one application-wide
combination that returns every display to the Windows profile and ends the
override, so automatic switching resumes. It is unbound by default. Recording
or clearing it takes effect immediately; no Save is needed. It cannot share a
combination with a profile's hotkey.

## Color controls

Each profile can independently configure:

| Setting | Range |
| --- | ---: |
| Brightness | 80–120 |
| Contrast | 80–120 |
| Gamma | 0.30–2.80 |
| Digital Vibrance | 0–100% |
| Hue | 0–359° |

The controls are applied directly through the NVIDIA display pipeline / NVAPI.

## Multi-monitor support

NvProfileSwitcher detects available NVIDIA displays and allows the Windows
profile and every other profile to store independent color settings for each
monitor.

Selecting a different display shows the values saved specifically for that
monitor.

When a configured application becomes active, NvProfileSwitcher applies the
profile's saved settings to the corresponding displays. When you leave the
application, each monitor returns to its own saved Windows color profile.

## System tray

NvProfileSwitcher can run silently from the Windows system tray.

The tray menu provides:

- Open NvProfileSwitcher
- Check for updates
- About NvProfileSwitcher
- Exit

The **Minimize to tray** option controls the behavior of the minimize button.

When enabled, minimizing NvProfileSwitcher hides it in the system tray. The tray icon is shown only while the application is minimized there and is removed again when the main window is restored.

When disabled, the application minimizes normally to the Windows taskbar.

The **Start with Windows** option allows NvProfileSwitcher to launch
automatically when you sign in to Windows.

The **Start minimized to tray** option allows it to start directly in the
system tray while handling profile switching in the background.

## Update checker

NvProfileSwitcher can automatically check GitHub for new releases when the
application starts.

Automatic update checks can be enabled or disabled using the
**Check for updates** option in the application settings.

You can always manually check for updates from the system tray menu, even when
automatic update checks are disabled.

If a newer version is available, a notification provides a direct link to the
latest GitHub release.

No automatic installation or background updater is used.

## Configuration

Profiles and application settings are stored in:

`%APPDATA%\NvProfileSwitcher\profiles.json`

The configuration file is created automatically.

## How it works

NvProfileSwitcher monitors the application that currently owns the foreground
window.

When its executable matches an enabled profile, the corresponding NVIDIA
color settings are applied to the configured displays.

When the foreground application no longer matches a configured profile,
NvProfileSwitcher restores the saved Windows profiles for each display.

Digital Vibrance and Hue are controlled through NVIDIA NVAPI.

Brightness, Contrast and Gamma are applied using the NVIDIA display pipeline
gamma correction LUT.

NvProfileSwitcher communicates directly with `nvapi64.dll` and does not need
the NVIDIA App to remain open.

> If another application is also controlling NVIDIA color settings, such as
> vibranceGUI, close it before using NvProfileSwitcher to prevent both
> applications from modifying the same display settings.

## Building

NvProfileSwitcher is built as a native x64 Windows application using:

- C++20
- MSVC x64
- Windows Win32 API
- NVIDIA NVAPI
- Static C/C++ runtime (`/MT`)

GitHub Actions automatically produces development builds from `main` and
versioned builds from release tags.

Development builds use:

`dev-<commit>`

Official releases use semantic version tags such as:

`v1.2.0`

## Support

If you find NvProfileSwitcher useful and would like to support its development:

☕ **Buy me a coffee on Ko-fi**

https://ko-fi.com/mgcarnevali

## Code signing

See the [Code signing policy](CODE_SIGNING.md).

## License

NvProfileSwitcher is licensed under the [MIT License](LICENSE).

Copyright © 2026 Maximiliano Carnevali.

## About

**NvProfileSwitcher** — Per-application NVIDIA display color profiles for Windows.
