#ifndef CONTROLLER_MOUSE_H
#define CONTROLLER_MOUSE_H

struct button {
    int HalfTransitions;
    bool32 EndedDown;
};

struct analog_stick {
    real32 X;
    real32 Y;
};

struct controller {
    button Up;
    button Down;
    button Left;
    button Right;
    button Start;
    button Back;
    button LeftThumb;
    button RightThumb;
    button LeftShoulder;
    button RightShoulder;
    button A;
    button B;
    button X;
    button Y;
    button Guide;
    uint8 LeftTrigger;
    uint8 RightTrigger;
    analog_stick LeftStick;
    analog_stick RightStick;
};

namespace modifiers {
uint8 Control = 1 << 1;
uint8 Shift = 1 << 2;
uint8 Alt = 1 << 3;
uint8 Windows = 1 << 4;
}

#define INIT_KEY(KEYCODE, MODIFIERS, SHOWNTEXT) { KEYCODE, MODIFIERS, SHOWNTEXT }

struct key {
    uint16 Keycode;
    uint8 Modifiers;
    char *ShownText;
};

global key KeyboardArray[6][16] = {
    {
        // NOTE(cory): No Trigger
        INIT_KEY('A', 0, "a"),
        INIT_KEY('E', 0, "e"),
        INIT_KEY('O', 0, "o"),
        INIT_KEY('I', 0, "i"),

        INIT_KEY('S', 0, "s"),
        INIT_KEY('T', 0, "t"),
        INIT_KEY('N', 0, "n"),
        INIT_KEY('R', 0, "r"),

        INIT_KEY('B', 0, "b"),
        INIT_KEY('C', 0, "c"),
        INIT_KEY('D', 0, "d"),
        INIT_KEY('F', 0, "f"),

        INIT_KEY('H', 0, "h"),
        INIT_KEY('J', 0, "j"),
        INIT_KEY('K', 0, "k"),
        INIT_KEY('G', 0, "g")
    },
    {
        // NOTE(cory): Right Trigger

        INIT_KEY('W', 0, "w"),
        INIT_KEY('U', 0, "u"),
        INIT_KEY('Z', 0, "z"),
        INIT_KEY('V', 0, "v"),

        INIT_KEY('M', 0, "m"),
        INIT_KEY('P', 0, "p"),
        INIT_KEY('L', 0, "l"),
        INIT_KEY('Q', 0, "q"),

        INIT_KEY(VK_OEM_7, 0, "'"),
        INIT_KEY(VK_OEM_MINUS, 0, "-"),
        INIT_KEY(VK_OEM_7, modifiers::Shift, "\""),
        INIT_KEY(VK_OEM_COMMA, 0, ","),

        INIT_KEY(VK_OEM_2, modifiers::Shift, "?"),
        INIT_KEY('X', 0, "x"),
        INIT_KEY('Y', 0, "y"),
        INIT_KEY(VK_OEM_PERIOD, 0, ".")
    },
    {
        // NOTE(cory): Left Trigger
        INIT_KEY('1', 0, "1"),
        INIT_KEY('2', 0, "2"),
        INIT_KEY('3', 0, "3"),
        INIT_KEY('4', 0, "4"),

        INIT_KEY('5', 0, "5"),
        INIT_KEY('6', 0, "6"),
        INIT_KEY('7', 0, "7"),
        INIT_KEY('8', 0, "8"),

        INIT_KEY('9', 0, "9"),
        INIT_KEY('0', 0, "0"),
        INIT_KEY(VK_OEM_2, 0, "/"),
        INIT_KEY(VK_OEM_5, 0, "\\"),

        INIT_KEY(VK_OEM_MINUS, 0, "-"),
        INIT_KEY(VK_OEM_PLUS, 0, "="),
        INIT_KEY(VK_OEM_MINUS, modifiers::Shift, "_"),
        INIT_KEY(VK_OEM_PLUS, modifiers::Shift, "+")
    },
    {
        // NOTE(cory): No Trigger Shifted
        INIT_KEY('A', modifiers::Shift, "A"),
        INIT_KEY('E', modifiers::Shift, "E"),
        INIT_KEY('O', modifiers::Shift, "O"),
        INIT_KEY('I', modifiers::Shift, "I"),

        INIT_KEY('S', modifiers::Shift, "S"),
        INIT_KEY('T', modifiers::Shift, "T"),
        INIT_KEY('N', modifiers::Shift, "N"),
        INIT_KEY('R', modifiers::Shift, "R"),

        INIT_KEY('B', modifiers::Shift, "B"),
        INIT_KEY('C', modifiers::Shift, "C"),
        INIT_KEY('D', modifiers::Shift, "D"),
        INIT_KEY('F', modifiers::Shift, "F"),

        INIT_KEY('H', modifiers::Shift, "H"),
        INIT_KEY('J', modifiers::Shift, "J"),
        INIT_KEY('K', modifiers::Shift, "K"),
        INIT_KEY('G', modifiers::Shift, "G")
    },
    {
        // NOTE(cory): Right Trigger Shifted

        INIT_KEY('W', modifiers::Shift, "W"),
        INIT_KEY('U', modifiers::Shift, "U"),
        INIT_KEY('Z', modifiers::Shift, "Z"),
        INIT_KEY('V', modifiers::Shift, "V"),

        INIT_KEY('M', modifiers::Shift, "M"),
        INIT_KEY('P', modifiers::Shift, "P"),
        INIT_KEY('L', modifiers::Shift, "L"),
        INIT_KEY('Q', modifiers::Shift, "Q"),

        INIT_KEY(VK_OEM_7, 0, "'"),
        INIT_KEY(VK_OEM_3, 0, "`"),
        INIT_KEY(VK_OEM_7, modifiers::Shift, "\""),
        INIT_KEY(VK_OEM_COMMA, 0, ","),

        INIT_KEY(VK_OEM_2, modifiers::Shift, "?"),
        INIT_KEY('X', modifiers::Shift, "X"),
        INIT_KEY('Y', modifiers::Shift, "Y"),
        INIT_KEY(VK_OEM_PERIOD, 0, ".")
    },
    {
        // NOTE(cory): Left Trigger Shifted
        INIT_KEY('1', modifiers::Shift, "!"),
        INIT_KEY('2', modifiers::Shift, "@"),
        INIT_KEY('3', modifiers::Shift, "#"),
        INIT_KEY('4', modifiers::Shift, "$"),

        INIT_KEY('5', modifiers::Shift, "%"),
        INIT_KEY('6', modifiers::Shift, "^"),
        INIT_KEY('7', modifiers::Shift, "&"),
        INIT_KEY('8', modifiers::Shift, "*"),

        INIT_KEY('9', modifiers::Shift, "("),
        INIT_KEY('0', modifiers::Shift, ")"),
    }
};

global Gdiplus::PointF KeyPositions[16] = {
    {244.0f, 434.0f},
    {194.0f, 380.0f},
    {244.0f, 324.0f},
    {294.0f, 380.0f},

    {104.0f, 294.0f},
    { 54.0f, 240.0f},
    {104.0f, 184.0f},
    {154.0f, 240.0f},

    {244.0f, 154.0f},
    {195.0f, 100.0f},
    {244.0f,  44.0f},
    {295.0f, 100.0f},

    {384.0f, 294.0f},
    {334.0f, 240.0f},
    {384.0f, 184.0f},
    {434.0f, 240.0f}
};

global Gdiplus::PointF InputTwoPositions[4] = {
    {245.0f, 380.0f},
    {100.0f, 245.0f},
    {245.0f, 100.0f},
    {380.0f, 245.0f},
};

struct draw_string_format {
    Gdiplus::Font *Font;
    Gdiplus::StringFormat *StringFormat;
    Gdiplus::Brush *Brush;
};

#endif
