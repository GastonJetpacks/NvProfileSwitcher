// Hotkey combination text: parse and format.
//
// Pure C++: no Win32 headers. A hotkey is a set of modifiers plus one key.
// The canonical text form is the modifiers in the fixed order Ctrl, Shift,
// Alt, Win joined with '+', followed by the key name, e.g. "Ctrl+Alt+1".
// A modifier is optional: "F9" is a valid hotkey.
//
// Modifier bit values equal the Win32 MOD_* flags so the Win32 layer can pass
// them straight to RegisterHotKey. Key codes are Win32 virtual-key codes.
#pragma once
#include <optional>
#include <string>

namespace hotkey {

enum Modifier : unsigned {
    kAlt = 0x1,
    kCtrl = 0x2,
    kShift = 0x4,
    kWin = 0x8,
};

struct Hotkey {
    unsigned modifiers = 0; // bitwise OR of Modifier
    unsigned key = 0;       // virtual-key code, never 0 for a valid hotkey

    bool operator==(const Hotkey& o) const { return modifiers == o.modifiers && key == o.key; }
    bool operator!=(const Hotkey& o) const { return !(*this == o); }
};

// Parses text such as "ctrl + alt + 1". Case-insensitive, whitespace around
// '+' ignored. Returns nullopt for an empty string, an unknown key name, a
// modifier-only combination, or more than one key.
std::optional<Hotkey> Parse(const std::wstring& text);

// Canonical text for a hotkey. Returns an empty string if the key code has no
// name in the supported table.
std::wstring Format(const Hotkey& hk);

// Canonical text for arbitrary input: Format(Parse(text)), or empty when the
// text does not parse. Two hotkeys conflict when their canonical texts are
// equal and non-empty.
std::wstring Canonical(const std::wstring& text);

// Name for a virtual-key code, or empty if unsupported. Exposed for the
// capture field, which receives key codes from Windows.
std::wstring KeyName(unsigned key);

} // namespace hotkey
