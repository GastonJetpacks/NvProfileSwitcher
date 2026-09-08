// Switching decision module.
//
// Pure C++: no Win32, no NVAPI, no globals. Given the current switching
// state, the configured profiles, and one event, decide the next state and
// the single action the caller must perform. The Win32 layer owns polling,
// hotkey registration, and applying colours; it only translates messages into
// events and carries out the returned action.
//
// Profiles are identified by an opaque id chosen by the caller. It must be
// unique among the profiles passed in and must never equal kWindowsId. Names
// are not identities: two profiles may share a name, and a user may name a
// profile "Windows".
//
// Vocabulary follows CONTEXT.md: Profile, Windows profile, Active profile,
// Executable trigger, Hotkey trigger, Override, Automatic switching.
#pragma once
#include <string>
#include <vector>

namespace switching {

// Reserved id of the Windows profile.
inline const wchar_t* const kWindowsId = L"Windows";

// What the decision needs to know about one profile. The executable basename
// is the file name without directory or extension; empty means the profile has
// no executable trigger.
struct ProfileInfo {
    std::wstring id;
    std::wstring exeBasename;
    bool enabled = true;
};

struct State {
    std::wstring activeId = kWindowsId;
    bool overrideActive = false;
};

enum class ActionKind {
    None,
    ApplyProfile,   // apply the profile `profileId` to all displays
    RestoreWindows, // apply the Windows profile to all displays
};

struct Action {
    ActionKind kind = ActionKind::None;
    std::wstring profileId; // set when kind == ApplyProfile
};

struct Event {
    enum class Kind {
        Startup,           // application launched
        ForegroundChanged, // `value` is the foreground process basename (may be empty)
        HotkeyPressed,     // `value` is the id of the profile whose hotkey trigger fired
        ResetHotkeyPressed,// the application-wide reset hotkey fired
        ProfilesChanged,   // a profile was saved, added, removed, enabled or disabled
    };
    Kind kind;
    std::wstring value;

    static Event Startup() { return {Kind::Startup, {}}; }
    static Event ForegroundChanged(std::wstring exeBasename) {
        return {Kind::ForegroundChanged, std::move(exeBasename)};
    }
    static Event HotkeyPressed(std::wstring profileId) {
        return {Kind::HotkeyPressed, std::move(profileId)};
    }
    static Event ResetHotkeyPressed() { return {Kind::ResetHotkeyPressed, {}}; }
    static Event ProfilesChanged() { return {Kind::ProfilesChanged, {}}; }
};

struct Decision {
    State state;
    Action action;
};

// Case-insensitive comparison of executable basenames.
bool SameExecutable(const std::wstring& a, const std::wstring& b);

Decision Decide(const State& current, const std::vector<ProfileInfo>& profiles, const Event& event);

} // namespace switching
