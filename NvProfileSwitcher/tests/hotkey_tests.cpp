// Tests for hotkey text parse/format. Compiled into the same test executable
// as switching_tests.cpp; see RunHotkeyTests().
#include "../hotkey.h"
#include "test_check.h"
#include <string>
#include <vector>

using namespace hotkey;

// Virtual-key codes used by the tests, spelled out so the test file needs no
// Windows headers either.
namespace vk {
constexpr unsigned kA = 0x41, kZ = 0x5A, k0 = 0x30, k1 = 0x31, k9 = 0x39;
constexpr unsigned kF1 = 0x70, kF9 = 0x78, kF12 = 0x7B, kF24 = 0x87;
constexpr unsigned kNumpad0 = 0x60, kNumpad9 = 0x69, kNumpadAdd = 0x6B;
constexpr unsigned kSpace = 0x20, kTab = 0x09, kEnter = 0x0D, kEsc = 0x1B;
constexpr unsigned kOemSemicolon = 0xBA, kOemBacktick = 0xC0, kOemBackslash = 0xDC;
constexpr unsigned kLeftShift = 0xA0, kControl = 0x11, kMenu = 0x12, kLWin = 0x5B;
} // namespace vk

static void ParsesSingleModifierAndDigit() {
    auto h = Parse(L"Ctrl+Alt+1");
    CHECK(h.has_value());
    if (!h) return;
    CHECK(h->modifiers == (kCtrl | kAlt));
    CHECK(h->key == vk::k1);
}

static void ParsesBareKeyWithoutModifier() {
    auto h = Parse(L"F9");
    CHECK(h.has_value());
    if (!h) return;
    CHECK(h->modifiers == 0);
    CHECK(h->key == vk::kF9);
}

static void ParseIsCaseInsensitiveAndIgnoresSpaces() {
    auto a = Parse(L"ctrl + alt + a");
    auto b = Parse(L"CTRL+ALT+A");
    auto c = Parse(L"  Ctrl+Alt+A  ");
    CHECK(a.has_value() && b.has_value() && c.has_value());
    if (!(a && b && c)) return;
    CHECK(*a == *b);
    CHECK(*b == *c);
    CHECK(a->key == vk::kA);
}

static void ParseAcceptsModifierAliases() {
    auto a = Parse(L"Control+Shift+Windows+F1");
    CHECK(a.has_value());
    if (!a) return;
    CHECK(a->modifiers == (kCtrl | kShift | kWin));
    CHECK(a->key == vk::kF1);
}

static void ParseAcceptsModifiersInAnyOrder() {
    auto a = Parse(L"Alt+Ctrl+1");
    auto b = Parse(L"Ctrl+Alt+1");
    CHECK(a.has_value() && b.has_value());
    if (!(a && b)) return;
    CHECK(*a == *b);
}

static void ParseRejectsEmpty() {
    CHECK(!Parse(L"").has_value());
    CHECK(!Parse(L"   ").has_value());
}

static void ParseRejectsModifierOnly() {
    CHECK(!Parse(L"Ctrl").has_value());
    CHECK(!Parse(L"Ctrl+Shift").has_value());
    CHECK(!Parse(L"Ctrl+").has_value());
}

static void ParseRejectsUnknownKey() {
    CHECK(!Parse(L"Ctrl+Banana").has_value());
    CHECK(!Parse(L"F25").has_value());
    CHECK(!Parse(L"Numpad10").has_value());
}

static void ParseRejectsTwoKeys() {
    CHECK(!Parse(L"A+B").has_value());
    CHECK(!Parse(L"Ctrl+A+B").has_value());
}

static void ParseRejectsModifierAsKey() {
    // A modifier key code on its own is not a hotkey.
    CHECK(!Parse(L"Shift+Ctrl").has_value());
}

static void FormatUsesCanonicalModifierOrder() {
    Hotkey h{kWin | kAlt | kShift | kCtrl, vk::kA};
    CHECK(Format(h) == L"Ctrl+Shift+Alt+Win+A");
}

static void FormatBareKey() {
    CHECK(Format(Hotkey{0, vk::kF12}) == L"F12");
    CHECK(Format(Hotkey{0, vk::kSpace}) == L"Space");
}

static void FormatUnsupportedKeyIsEmpty() {
    CHECK(Format(Hotkey{kCtrl, 0}) == L"");
    CHECK(Format(Hotkey{0, vk::kLeftShift}) == L"");
    CHECK(Format(Hotkey{0, vk::kControl}) == L"");
    CHECK(Format(Hotkey{0, vk::kMenu}) == L"");
    CHECK(Format(Hotkey{0, vk::kLWin}) == L"");
}

static void EverySupportedKeyRoundTrips() {
    const std::vector<std::wstring> names = {
        L"A", L"Z", L"0", L"9",
        L"F1", L"F12", L"F24",
        L"Numpad0", L"Numpad9", L"NumpadAdd", L"NumpadSubtract", L"NumpadMultiply", L"NumpadDivide", L"NumpadDecimal",
        L"Space", L"Tab", L"Enter", L"Esc", L"Backspace",
        L"Insert", L"Delete", L"Home", L"End", L"PageUp", L"PageDown",
        L"Up", L"Down", L"Left", L"Right",
        L"Pause", L"ScrollLock", L"PrintScreen", L"CapsLock", L"NumLock",
        L";", L"=", L",", L"-", L".", L"/", L"`", L"[", L"\\", L"]", L"'",
    };
    for (const auto& n : names) {
        auto h = Parse(n);
        CHECK(h.has_value());
        if (!h) { std::printf("  (key name %ls)\n", n.c_str()); continue; }
        CHECK(Format(*h) == n);
        if (Format(*h) != n) std::printf("  (key name %ls -> %ls)\n", n.c_str(), Format(*h).c_str());
    }
}

static void SpecificKeyCodes() {
    CHECK(Parse(L"A")->key == vk::kA);
    CHECK(Parse(L"Z")->key == vk::kZ);
    CHECK(Parse(L"0")->key == vk::k0);
    CHECK(Parse(L"9")->key == vk::k9);
    CHECK(Parse(L"F24")->key == vk::kF24);
    CHECK(Parse(L"Numpad0")->key == vk::kNumpad0);
    CHECK(Parse(L"Numpad9")->key == vk::kNumpad9);
    CHECK(Parse(L"NumpadAdd")->key == vk::kNumpadAdd);
    CHECK(Parse(L"Tab")->key == vk::kTab);
    CHECK(Parse(L"Enter")->key == vk::kEnter);
    CHECK(Parse(L"Esc")->key == vk::kEsc);
    CHECK(Parse(L";")->key == vk::kOemSemicolon);
    CHECK(Parse(L"`")->key == vk::kOemBacktick);
    CHECK(Parse(L"\\")->key == vk::kOemBackslash);
}

static void CanonicalNormalisesForConflictDetection() {
    CHECK(Canonical(L"ctrl + a") == L"Ctrl+A");
    CHECK(Canonical(L"alt+ctrl+1") == L"Ctrl+Alt+1");
    CHECK(Canonical(L"Ctrl+A") == Canonical(L"CTRL+a"));
    CHECK(Canonical(L"") == L"");
    CHECK(Canonical(L"Ctrl") == L"");
    CHECK(Canonical(L"Nonsense") == L"");
    CHECK(Canonical(L"Ctrl+A") != Canonical(L"Ctrl+B"));
}

static void KeyNameLookup() {
    CHECK(KeyName(vk::kA) == L"A");
    CHECK(KeyName(vk::kF9) == L"F9");
    CHECK(KeyName(vk::kOemSemicolon) == L";");
    CHECK(KeyName(vk::kLeftShift) == L"");
    CHECK(KeyName(0) == L"");
}

void RunHotkeyTests() {
    ParsesSingleModifierAndDigit();
    ParsesBareKeyWithoutModifier();
    ParseIsCaseInsensitiveAndIgnoresSpaces();
    ParseAcceptsModifierAliases();
    ParseAcceptsModifiersInAnyOrder();
    ParseRejectsEmpty();
    ParseRejectsModifierOnly();
    ParseRejectsUnknownKey();
    ParseRejectsTwoKeys();
    ParseRejectsModifierAsKey();
    FormatUsesCanonicalModifierOrder();
    FormatBareKey();
    FormatUnsupportedKeyIsEmpty();
    EverySupportedKeyRoundTrips();
    SpecificKeyCodes();
    CanonicalNormalisesForConflictDetection();
    KeyNameLookup();
}
