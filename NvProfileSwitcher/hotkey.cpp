#include "hotkey.h"
#include <cwctype>
#include <vector>

namespace hotkey {

namespace {

// Virtual-key codes (Win32 values, spelled out to keep this unit free of
// Windows headers).
constexpr unsigned kVkBack = 0x08, kVkTab = 0x09, kVkReturn = 0x0D, kVkPause = 0x13, kVkCapital = 0x14;
constexpr unsigned kVkEscape = 0x1B, kVkSpace = 0x20, kVkPrior = 0x21, kVkNext = 0x22, kVkEnd = 0x23, kVkHome = 0x24;
constexpr unsigned kVkLeft = 0x25, kVkUp = 0x26, kVkRight = 0x27, kVkDown = 0x28, kVkSnapshot = 0x2C;
constexpr unsigned kVkInsert = 0x2D, kVkDelete = 0x2E;
constexpr unsigned kVk0 = 0x30, kVkA = 0x41;
constexpr unsigned kVkNumpad0 = 0x60, kVkMultiply = 0x6A, kVkAdd = 0x6B, kVkSubtract = 0x6D, kVkDecimal = 0x6E, kVkDivide = 0x6F;
constexpr unsigned kVkF1 = 0x70; // F1..F24 = 0x70..0x87
constexpr unsigned kVkNumlock = 0x90, kVkScroll = 0x91;
constexpr unsigned kVkOem1 = 0xBA, kVkOemPlus = 0xBB, kVkOemComma = 0xBC, kVkOemMinus = 0xBD, kVkOemPeriod = 0xBE;
constexpr unsigned kVkOem2 = 0xBF, kVkOem3 = 0xC0, kVkOem4 = 0xDB, kVkOem5 = 0xDC, kVkOem6 = 0xDD, kVkOem7 = 0xDE;

struct NamedKey {
    const wchar_t* name;
    unsigned key;
};

// Keys with a name that is not derivable by pattern (letters, digits, F-keys,
// Numpad digits are handled arithmetically).
const NamedKey kNamedKeys[] = {
    {L"NumpadAdd", kVkAdd},
    {L"NumpadSubtract", kVkSubtract},
    {L"NumpadMultiply", kVkMultiply},
    {L"NumpadDivide", kVkDivide},
    {L"NumpadDecimal", kVkDecimal},
    {L"Space", kVkSpace},
    {L"Tab", kVkTab},
    {L"Enter", kVkReturn},
    {L"Esc", kVkEscape},
    {L"Backspace", kVkBack},
    {L"Insert", kVkInsert},
    {L"Delete", kVkDelete},
    {L"Home", kVkHome},
    {L"End", kVkEnd},
    {L"PageUp", kVkPrior},
    {L"PageDown", kVkNext},
    {L"Up", kVkUp},
    {L"Down", kVkDown},
    {L"Left", kVkLeft},
    {L"Right", kVkRight},
    {L"Pause", kVkPause},
    {L"ScrollLock", kVkScroll},
    {L"PrintScreen", kVkSnapshot},
    {L"CapsLock", kVkCapital},
    {L"NumLock", kVkNumlock},
    {L";", kVkOem1},
    {L"=", kVkOemPlus},
    {L",", kVkOemComma},
    {L"-", kVkOemMinus},
    {L".", kVkOemPeriod},
    {L"/", kVkOem2},
    {L"`", kVkOem3},
    {L"[", kVkOem4},
    {L"\\", kVkOem5},
    {L"]", kVkOem6},
    {L"'", kVkOem7},
};

std::wstring Lower(std::wstring s) {
    for (auto& c : s) c = (wchar_t)std::towlower(c);
    return s;
}

std::wstring Trim(const std::wstring& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::iswspace(s[a])) ++a;
    while (b > a && std::iswspace(s[b - 1])) --b;
    return s.substr(a, b - a);
}

bool EqualsNoCase(const std::wstring& a, const wchar_t* b) {
    size_t i = 0;
    for (; i < a.size() && b[i]; ++i) {
        if (std::towlower(a[i]) != std::towlower(b[i])) return false;
    }
    return i == a.size() && b[i] == 0;
}

// Returns the modifier bit for a modifier name, or 0 if the token is not one.
unsigned ModifierFromName(const std::wstring& token) {
    if (EqualsNoCase(token, L"Ctrl") || EqualsNoCase(token, L"Control")) return kCtrl;
    if (EqualsNoCase(token, L"Shift")) return kShift;
    if (EqualsNoCase(token, L"Alt")) return kAlt;
    if (EqualsNoCase(token, L"Win") || EqualsNoCase(token, L"Windows")) return kWin;
    return 0;
}

// Parses a positive integer with no sign and no leading zeros beyond "0".
bool ParseSmallInt(const std::wstring& s, unsigned& out) {
    if (s.empty() || s.size() > 2) return false;
    unsigned v = 0;
    for (wchar_t c : s) {
        if (c < L'0' || c > L'9') return false;
        v = v * 10 + (unsigned)(c - L'0');
    }
    if (s.size() == 2 && s[0] == L'0') return false;
    out = v;
    return true;
}

// Returns the key code for a key name, or 0 if unknown.
unsigned KeyFromName(const std::wstring& token) {
    if (token.empty()) return 0;

    if (token.size() == 1) {
        wchar_t c = (wchar_t)std::towupper(token[0]);
        if (c >= L'A' && c <= L'Z') return kVkA + (unsigned)(c - L'A');
        if (c >= L'0' && c <= L'9') return kVk0 + (unsigned)(c - L'0');
        // Punctuation falls through to the named table.
    }

    std::wstring lower = Lower(token);

    if (lower.size() >= 2 && lower[0] == L'f') {
        unsigned n = 0;
        if (ParseSmallInt(lower.substr(1), n) && n >= 1 && n <= 24) return kVkF1 + (n - 1);
    }

    const std::wstring numpad = L"numpad";
    if (lower.size() == numpad.size() + 1 && lower.compare(0, numpad.size(), numpad) == 0) {
        wchar_t d = lower[numpad.size()];
        if (d >= L'0' && d <= L'9') return kVkNumpad0 + (unsigned)(d - L'0');
    }

    for (const auto& nk : kNamedKeys) {
        if (EqualsNoCase(token, nk.name)) return nk.key;
    }
    return 0;
}

} // namespace

std::wstring KeyName(unsigned key) {
    if (key >= kVkA && key <= kVkA + 25) return std::wstring(1, (wchar_t)(L'A' + (key - kVkA)));
    if (key >= kVk0 && key <= kVk0 + 9) return std::wstring(1, (wchar_t)(L'0' + (key - kVk0)));
    if (key >= kVkF1 && key <= kVkF1 + 23) return L"F" + std::to_wstring(key - kVkF1 + 1);
    if (key >= kVkNumpad0 && key <= kVkNumpad0 + 9) return L"Numpad" + std::to_wstring(key - kVkNumpad0);
    for (const auto& nk : kNamedKeys) {
        if (nk.key == key) return nk.name;
    }
    return {};
}

std::optional<Hotkey> Parse(const std::wstring& text) {
    // Split on '+'. A trailing '+' or an empty token is invalid; this also
    // means '+' itself cannot be a key (use '=' for that physical key).
    std::vector<std::wstring> tokens;
    std::wstring cur;
    for (wchar_t c : text) {
        if (c == L'+') {
            tokens.push_back(Trim(cur));
            cur.clear();
        } else {
            cur += c;
        }
    }
    tokens.push_back(Trim(cur));

    Hotkey hk;
    bool haveKey = false;
    for (const auto& t : tokens) {
        if (t.empty()) return std::nullopt;
        if (unsigned m = ModifierFromName(t)) {
            hk.modifiers |= m;
            continue;
        }
        unsigned k = KeyFromName(t);
        if (!k || haveKey) return std::nullopt;
        hk.key = k;
        haveKey = true;
    }
    if (!haveKey) return std::nullopt;
    return hk;
}

std::wstring Format(const Hotkey& hk) {
    std::wstring key = KeyName(hk.key);
    if (key.empty()) return {};
    std::wstring out;
    if (hk.modifiers & kCtrl) out += L"Ctrl+";
    if (hk.modifiers & kShift) out += L"Shift+";
    if (hk.modifiers & kAlt) out += L"Alt+";
    if (hk.modifiers & kWin) out += L"Win+";
    out += key;
    return out;
}

std::wstring Canonical(const std::wstring& text) {
    auto hk = Parse(text);
    return hk ? Format(*hk) : std::wstring();
}

} // namespace hotkey
