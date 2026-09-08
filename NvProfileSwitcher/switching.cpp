#include "switching.h"
#include <cwctype>

namespace switching {

bool SameExecutable(const std::wstring& a, const std::wstring& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::towlower(a[i]) != std::towlower(b[i])) return false;
    }
    return true;
}

namespace {

// Automatic switching: the first enabled profile whose executable trigger
// matches the foreground process, or nullptr for the Windows profile.
const ProfileInfo* MatchExecutableTrigger(const std::vector<ProfileInfo>& profiles,
                                          const std::wstring& foregroundExe) {
    if (foregroundExe.empty()) return nullptr;
    for (const auto& p : profiles) {
        if (!p.enabled || p.exeBasename.empty()) continue;
        if (SameExecutable(p.exeBasename, foregroundExe)) return &p;
    }
    return nullptr;
}

const ProfileInfo* FindProfile(const std::vector<ProfileInfo>& profiles, const std::wstring& name) {
    for (const auto& p : profiles) {
        if (p.name == name) return &p;
    }
    return nullptr;
}

// Make `next` the active profile, emitting the action only if it differs from
// the current one. Leaves the override flag as it is.
Decision Activate(const State& current, const std::wstring& next) {
    Decision d{current, {}};
    if (next == current.activeProfile) return d;
    d.state.activeProfile = next;
    if (next == kWindowsProfileName) {
        d.action.kind = ActionKind::RestoreWindows;
    } else {
        d.action.kind = ActionKind::ApplyProfile;
        d.action.profile = next;
    }
    return d;
}

// End the override and return to the Windows profile. Automatic switching
// resumes on the next foreground event.
Decision EndOverride(const State& current) {
    Decision d = Activate(current, kWindowsProfileName);
    d.state.overrideActive = false;
    return d;
}

} // namespace

Decision Decide(const State& current, const std::vector<ProfileInfo>& profiles, const Event& event) {
    switch (event.kind) {
    case Event::Kind::Startup: {
        // The desktop always comes up in a known state; nothing carries over,
        // including any override.
        Decision d;
        d.state = State{};
        d.action.kind = ActionKind::RestoreWindows;
        return d;
    }
    case Event::Kind::ForegroundChanged: {
        // ADR 0001: while an override is in place executable triggers are ignored.
        if (current.overrideActive) return {current, {}};
        const ProfileInfo* hit = MatchExecutableTrigger(profiles, event.value);
        return Activate(current, hit ? hit->name : std::wstring(kWindowsProfileName));
    }
    case Event::Kind::HotkeyPressed: {
        // ADR 0001: a hotkey press pins its profile. Pressing the active
        // profile's own hotkey is a no-op, never a toggle.
        const ProfileInfo* p = FindProfile(profiles, event.value);
        if (!p || !p->enabled) return {current, {}};
        Decision d = Activate(current, p->name);
        d.state.overrideActive = true;
        return d;
    }
    case Event::Kind::ProfilesChanged: {
        // If the pinned profile was disabled or removed, the override ends so
        // the user is never stuck on colours they can no longer see or edit.
        if (!current.overrideActive) return {current, {}};
        const ProfileInfo* pinned = FindProfile(profiles, current.activeProfile);
        if (pinned && pinned->enabled) return {current, {}};
        return EndOverride(current);
    }
    }
    return {current, {}};
}

} // namespace switching
