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

} // namespace

Decision Decide(const State& current, const std::vector<ProfileInfo>& profiles, const Event& event) {
    switch (event.kind) {
    case Event::Kind::Startup: {
        // The desktop always comes up in a known state; nothing carries over.
        Decision d;
        d.state = State{};
        d.action.kind = ActionKind::RestoreWindows;
        return d;
    }
    case Event::Kind::ForegroundChanged: {
        const ProfileInfo* hit = MatchExecutableTrigger(profiles, event.value);
        return Activate(current, hit ? hit->name : std::wstring(kWindowsProfileName));
    }
    }
    return {current, {}};
}

} // namespace switching
