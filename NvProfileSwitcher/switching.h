// Switching decision module.
//
// Pure C++: no Win32, no NVAPI, no globals. Given the current switching
// state, the configured profiles, and one event, decide the next state and
// the single action the caller must perform. The Win32 layer owns polling,
// hotkey registration, and applying colours; it only translates messages into
// events and carries out the returned action.
//
// Vocabulary follows CONTEXT.md: Profile, Windows profile, Active profile,
// Executable trigger, Override, Automatic switching.
#pragma once
#include <string>
#include <vector>

namespace switching {

inline const wchar_t* const kWindowsProfileName = L"Windows";

// What the decision needs to know about one profile. The executable basename
// is the file name without directory or extension; empty means the profile has
// no executable trigger.
struct ProfileInfo {
    std::wstring name;
    std::wstring exeBasename;
    bool enabled = true;
};

struct State {
    std::wstring activeProfile = kWindowsProfileName;
    bool overrideActive = false;
};

enum class ActionKind {
    None,
    ApplyProfile,   // apply `profile` to all displays
    RestoreWindows, // apply the Windows profile to all displays
};

struct Action {
    ActionKind kind = ActionKind::None;
    std::wstring profile; // set when kind == ApplyProfile
};

struct Event {
    enum class Kind {
        Startup,           // application launched
        ForegroundChanged, // `value` is the foreground process basename (may be empty)
    };
    Kind kind;
    std::wstring value;

    static Event Startup() { return {Kind::Startup, {}}; }
    static Event ForegroundChanged(std::wstring exeBasename) {
        return {Kind::ForegroundChanged, std::move(exeBasename)};
    }
};

struct Decision {
    State state;
    Action action;
};

// Case-insensitive comparison of executable basenames.
bool SameExecutable(const std::wstring& a, const std::wstring& b);

Decision Decide(const State& current, const std::vector<ProfileInfo>& profiles, const Event& event);

} // namespace switching
