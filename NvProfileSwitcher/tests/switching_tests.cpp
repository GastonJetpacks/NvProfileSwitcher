// Console tests for the pure modules (switching decision, hotkey text).
// No framework: each CHECK records a failure and the process exits non-zero
// if any failed.
#include "../switching.h"
#include "test_check.h"
#include <cstdio>
#include <string>
#include <vector>

int gFailures = 0;
int gChecks = 0;

void RunHotkeyTests(); // hotkey_tests.cpp

using namespace switching;

static std::vector<ProfileInfo> Profiles() {
    return {
        {L"Doom", L"doom", true},
        {L"Elden Ring", L"eldenring", true},
        {L"Disabled", L"disabled", false},
        {L"No Exe", L"", true},
    };
}

static State Windows() { return State{}; }
static State Active(const wchar_t* name) { return State{name, false}; }
static State Override(const wchar_t* name) { return State{name, true}; }

// --- Automatic switching -------------------------------------------------

static void StartupRestoresWindows() {
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::Startup());
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::RestoreWindows);
}

static void StartupClearsOverride() {
    Decision d = Decide(Override(L"Doom"), Profiles(), Event::Startup());
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::RestoreWindows);
}

static void ForegroundMatchAppliesProfile() {
    Decision d = Decide(Windows(), Profiles(), Event::ForegroundChanged(L"doom"));
    CHECK(d.state.activeId == L"Doom");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::ApplyProfile);
    CHECK(d.action.profileId == L"Doom");
}

static void ForegroundMatchIsCaseInsensitive() {
    Decision d = Decide(Windows(), Profiles(), Event::ForegroundChanged(L"DOOM"));
    CHECK(d.state.activeId == L"Doom");
    CHECK(d.action.kind == ActionKind::ApplyProfile);
    CHECK(d.action.profileId == L"Doom");
}

static void ForegroundLeavingProfileRestoresWindows() {
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::ForegroundChanged(L"explorer"));
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.action.kind == ActionKind::RestoreWindows);
}

static void ForegroundEmptyRestoresWindows() {
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::ForegroundChanged(L""));
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.action.kind == ActionKind::RestoreWindows);
}

static void ForegroundSwitchesBetweenProfiles() {
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::ForegroundChanged(L"eldenring"));
    CHECK(d.state.activeId == L"Elden Ring");
    CHECK(d.action.kind == ActionKind::ApplyProfile);
    CHECK(d.action.profileId == L"Elden Ring");
}

static void DisabledProfileNeverMatches() {
    Decision d = Decide(Windows(), Profiles(), Event::ForegroundChanged(L"disabled"));
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.action.kind == ActionKind::None);
}

static void ProfileWithoutExecutableNeverMatches() {
    Decision d = Decide(Windows(), Profiles(), Event::ForegroundChanged(L""));
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.action.kind == ActionKind::None);
}

static void NoActionWhenMatchedProfileAlreadyActive() {
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::ForegroundChanged(L"doom"));
    CHECK(d.state.activeId == L"Doom");
    CHECK(d.action.kind == ActionKind::None);
}

static void NoActionWhenWindowsAlreadyActiveAndNothingMatches() {
    Decision d = Decide(Windows(), Profiles(), Event::ForegroundChanged(L"explorer"));
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.action.kind == ActionKind::None);
}

static void FirstMatchingProfileWins() {
    std::vector<ProfileInfo> twice = {
        {L"First", L"game", true},
        {L"Second", L"game", true},
    };
    Decision d = Decide(Windows(), twice, Event::ForegroundChanged(L"game"));
    CHECK(d.state.activeId == L"First");
    CHECK(d.action.profileId == L"First");
}

static void SameExecutableIgnoresCase() {
    CHECK(SameExecutable(L"Doom", L"dOOm"));
    CHECK(!SameExecutable(L"doom", L"doom2"));
    CHECK(!SameExecutable(L"", L"doom"));
    CHECK(SameExecutable(L"", L""));
}

// --- Hotkey triggers and override (ADR 0001) -----------------------------

static void HotkeyAppliesProfileAndStartsOverride() {
    Decision d = Decide(Windows(), Profiles(), Event::HotkeyPressed(L"Doom"));
    CHECK(d.state.activeId == L"Doom");
    CHECK(d.state.overrideActive == true);
    CHECK(d.action.kind == ActionKind::ApplyProfile);
    CHECK(d.action.profileId == L"Doom");
}

static void HotkeyForAlreadyActiveProfilePinsWithoutReapplying() {
    // Profile became active through its executable trigger; the hotkey pins it.
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::HotkeyPressed(L"Doom"));
    CHECK(d.state.activeId == L"Doom");
    CHECK(d.state.overrideActive == true);
    CHECK(d.action.kind == ActionKind::None);
}

static void HotkeyForPinnedProfileIsNoOpNotToggle() {
    Decision d = Decide(Override(L"Doom"), Profiles(), Event::HotkeyPressed(L"Doom"));
    CHECK(d.state.activeId == L"Doom");
    CHECK(d.state.overrideActive == true);
    CHECK(d.action.kind == ActionKind::None);
}

static void HotkeySwitchesToAnotherProfileKeepingOverride() {
    Decision d = Decide(Override(L"Doom"), Profiles(), Event::HotkeyPressed(L"Elden Ring"));
    CHECK(d.state.activeId == L"Elden Ring");
    CHECK(d.state.overrideActive == true);
    CHECK(d.action.kind == ActionKind::ApplyProfile);
    CHECK(d.action.profileId == L"Elden Ring");
}

static void HotkeyForDisabledProfileDoesNothing() {
    Decision d = Decide(Windows(), Profiles(), Event::HotkeyPressed(L"Disabled"));
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::None);
}

static void HotkeyForUnknownProfileDoesNothing() {
    Decision d = Decide(Override(L"Doom"), Profiles(), Event::HotkeyPressed(L"Nope"));
    CHECK(d.state.activeId == L"Doom");
    CHECK(d.state.overrideActive == true);
    CHECK(d.action.kind == ActionKind::None);
}

static void HotkeyWorksForProfileWithoutExecutable() {
    Decision d = Decide(Windows(), Profiles(), Event::HotkeyPressed(L"No Exe"));
    CHECK(d.state.activeId == L"No Exe");
    CHECK(d.state.overrideActive == true);
    CHECK(d.action.kind == ActionKind::ApplyProfile);
}

static void ForegroundChangeDuringOverrideIsIgnored() {
    Decision d = Decide(Override(L"Doom"), Profiles(), Event::ForegroundChanged(L"eldenring"));
    CHECK(d.state.activeId == L"Doom");
    CHECK(d.state.overrideActive == true);
    CHECK(d.action.kind == ActionKind::None);

    Decision e = Decide(Override(L"Doom"), Profiles(), Event::ForegroundChanged(L""));
    CHECK(e.state.activeId == L"Doom");
    CHECK(e.state.overrideActive == true);
    CHECK(e.action.kind == ActionKind::None);
}

static void ResetHotkeyEndsOverrideAndRestoresWindows() {
    Decision d = Decide(Override(L"Doom"), Profiles(), Event::ResetHotkeyPressed());
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::RestoreWindows);
}

static void ResetHotkeyWhenWindowsAlreadyActiveDoesNothing() {
    Decision d = Decide(Windows(), Profiles(), Event::ResetHotkeyPressed());
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::None);
}

static void ResetHotkeyDuringAutomaticSwitchingRestoresWindows() {
    // Not an override, but the user asked for Windows colours: honour it. The
    // next foreground event decides whether the profile comes back.
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::ResetHotkeyPressed());
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::RestoreWindows);
}

static void ResetHotkeyThenForegroundGameReappliesItsProfile() {
    Decision reset = Decide(Override(L"Doom"), Profiles(), Event::ResetHotkeyPressed());
    Decision next = Decide(reset.state, Profiles(), Event::ForegroundChanged(L"doom"));
    CHECK(next.state.activeId == L"Doom");
    CHECK(next.state.overrideActive == false);
    CHECK(next.action.kind == ActionKind::ApplyProfile);
    CHECK(next.action.profileId == L"Doom");
}

static void ResetHotkeyThenForegroundDesktopStaysOnWindows() {
    Decision reset = Decide(Override(L"Doom"), Profiles(), Event::ResetHotkeyPressed());
    Decision next = Decide(reset.state, Profiles(), Event::ForegroundChanged(L"explorer"));
    CHECK(next.state.activeId == L"Windows");
    CHECK(next.action.kind == ActionKind::None);
}

static void ProfilesChangedEndsOverrideWhenPinnedProfileDisabled() {
    std::vector<ProfileInfo> profiles = Profiles();
    profiles[0].enabled = false; // Doom
    Decision d = Decide(Override(L"Doom"), profiles, Event::ProfilesChanged());
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::RestoreWindows);
}

static void ProfilesChangedEndsOverrideWhenPinnedProfileRemoved() {
    std::vector<ProfileInfo> profiles = {{L"Elden Ring", L"eldenring", true}};
    Decision d = Decide(Override(L"Doom"), profiles, Event::ProfilesChanged());
    CHECK(d.state.activeId == L"Windows");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::RestoreWindows);
}

static void ProfilesChangedKeepsOverrideWhenPinnedProfileStillEnabled() {
    Decision d = Decide(Override(L"Doom"), Profiles(), Event::ProfilesChanged());
    CHECK(d.state.activeId == L"Doom");
    CHECK(d.state.overrideActive == true);
    CHECK(d.action.kind == ActionKind::None);
}

static void ProfilesChangedWithoutOverrideIsNoChange() {
    std::vector<ProfileInfo> profiles = Profiles();
    profiles[0].enabled = false;
    Decision d = Decide(Active(L"Doom"), profiles, Event::ProfilesChanged());
    CHECK(d.state.activeId == L"Doom");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::None);
}

int main() {
    StartupRestoresWindows();
    StartupClearsOverride();
    ForegroundMatchAppliesProfile();
    ForegroundMatchIsCaseInsensitive();
    ForegroundLeavingProfileRestoresWindows();
    ForegroundEmptyRestoresWindows();
    ForegroundSwitchesBetweenProfiles();
    DisabledProfileNeverMatches();
    ProfileWithoutExecutableNeverMatches();
    NoActionWhenMatchedProfileAlreadyActive();
    NoActionWhenWindowsAlreadyActiveAndNothingMatches();
    FirstMatchingProfileWins();
    SameExecutableIgnoresCase();

    HotkeyAppliesProfileAndStartsOverride();
    HotkeyForAlreadyActiveProfilePinsWithoutReapplying();
    HotkeyForPinnedProfileIsNoOpNotToggle();
    HotkeySwitchesToAnotherProfileKeepingOverride();
    HotkeyForDisabledProfileDoesNothing();
    HotkeyForUnknownProfileDoesNothing();
    HotkeyWorksForProfileWithoutExecutable();
    ForegroundChangeDuringOverrideIsIgnored();
    ResetHotkeyEndsOverrideAndRestoresWindows();
    ResetHotkeyWhenWindowsAlreadyActiveDoesNothing();
    ResetHotkeyDuringAutomaticSwitchingRestoresWindows();
    ResetHotkeyThenForegroundGameReappliesItsProfile();
    ResetHotkeyThenForegroundDesktopStaysOnWindows();
    ProfilesChangedEndsOverrideWhenPinnedProfileDisabled();
    ProfilesChangedEndsOverrideWhenPinnedProfileRemoved();
    ProfilesChangedKeepsOverrideWhenPinnedProfileStillEnabled();
    ProfilesChangedWithoutOverrideIsNoChange();

    RunHotkeyTests();

    if (gFailures) {
        std::printf("%d of %d checks failed\n", gFailures, gChecks);
        return 1;
    }
    std::printf("All %d checks passed\n", gChecks);
    return 0;
}
