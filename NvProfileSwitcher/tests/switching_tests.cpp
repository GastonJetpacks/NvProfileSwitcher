// Console tests for the switching decision module. No framework: each CHECK
// records a failure and the process exits non-zero if any failed.
#include "../switching.h"
#include <cstdio>
#include <string>
#include <vector>

using namespace switching;

static int gFailures = 0;
static int gChecks = 0;

#define CHECK(cond)                                                                 \
    do {                                                                            \
        ++gChecks;                                                                  \
        if (!(cond)) {                                                              \
            ++gFailures;                                                            \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);            \
        }                                                                           \
    } while (0)

static std::vector<ProfileInfo> Profiles() {
    return {
        {L"Doom", L"doom", true},
        {L"Elden Ring", L"eldenring", true},
        {L"Disabled Game", L"disabled", false},
        {L"No Exe", L"", true},
    };
}

static State Windows() { return State{}; }
static State Active(const wchar_t* name) { return State{name, false}; }

static void StartupRestoresWindows() {
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::Startup());
    CHECK(d.state.activeProfile == L"Windows");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::RestoreWindows);
}

static void ForegroundMatchAppliesProfile() {
    Decision d = Decide(Windows(), Profiles(), Event::ForegroundChanged(L"doom"));
    CHECK(d.state.activeProfile == L"Doom");
    CHECK(d.state.overrideActive == false);
    CHECK(d.action.kind == ActionKind::ApplyProfile);
    CHECK(d.action.profile == L"Doom");
}

static void ForegroundMatchIsCaseInsensitive() {
    Decision d = Decide(Windows(), Profiles(), Event::ForegroundChanged(L"DOOM"));
    CHECK(d.state.activeProfile == L"Doom");
    CHECK(d.action.kind == ActionKind::ApplyProfile);
    CHECK(d.action.profile == L"Doom");
}

static void ForegroundLeavingProfileRestoresWindows() {
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::ForegroundChanged(L"explorer"));
    CHECK(d.state.activeProfile == L"Windows");
    CHECK(d.action.kind == ActionKind::RestoreWindows);
}

static void ForegroundEmptyRestoresWindows() {
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::ForegroundChanged(L""));
    CHECK(d.state.activeProfile == L"Windows");
    CHECK(d.action.kind == ActionKind::RestoreWindows);
}

static void ForegroundSwitchesBetweenProfiles() {
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::ForegroundChanged(L"eldenring"));
    CHECK(d.state.activeProfile == L"Elden Ring");
    CHECK(d.action.kind == ActionKind::ApplyProfile);
    CHECK(d.action.profile == L"Elden Ring");
}

static void DisabledProfileNeverMatches() {
    Decision d = Decide(Windows(), Profiles(), Event::ForegroundChanged(L"disabled"));
    CHECK(d.state.activeProfile == L"Windows");
    CHECK(d.action.kind == ActionKind::None);
}

static void ProfileWithoutExecutableNeverMatches() {
    Decision d = Decide(Windows(), Profiles(), Event::ForegroundChanged(L""));
    CHECK(d.state.activeProfile == L"Windows");
    CHECK(d.action.kind == ActionKind::None);
}

static void NoActionWhenMatchedProfileAlreadyActive() {
    Decision d = Decide(Active(L"Doom"), Profiles(), Event::ForegroundChanged(L"doom"));
    CHECK(d.state.activeProfile == L"Doom");
    CHECK(d.action.kind == ActionKind::None);
}

static void NoActionWhenWindowsAlreadyActiveAndNothingMatches() {
    Decision d = Decide(Windows(), Profiles(), Event::ForegroundChanged(L"explorer"));
    CHECK(d.state.activeProfile == L"Windows");
    CHECK(d.action.kind == ActionKind::None);
}

static void FirstMatchingProfileWins() {
    std::vector<ProfileInfo> twice = {
        {L"First", L"game", true},
        {L"Second", L"game", true},
    };
    Decision d = Decide(Windows(), twice, Event::ForegroundChanged(L"game"));
    CHECK(d.state.activeProfile == L"First");
    CHECK(d.action.profile == L"First");
}

static void SameExecutableIgnoresCase() {
    CHECK(SameExecutable(L"Doom", L"dOOm"));
    CHECK(!SameExecutable(L"doom", L"doom2"));
    CHECK(!SameExecutable(L"", L"doom"));
    CHECK(SameExecutable(L"", L""));
}

int main() {
    StartupRestoresWindows();
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

    if (gFailures) {
        std::printf("%d of %d checks failed\n", gFailures, gChecks);
        return 1;
    }
    std::printf("All %d checks passed\n", gChecks);
    return 0;
}
