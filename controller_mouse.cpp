#include <windows.h>
#include <winuser.h>
#include <xinput.h>
#include <stdio.h>
#include <shellapi.h>
#include <gdiplus.h>

#include "/lib/mw.h"
#include "controller_mouse.h"

// XInputGetState function definitions
#define X_INPUT_GET_STATE_EX(name) DWORD name(DWORD dwUserIndex, XINPUT_STATE * pState)
typedef X_INPUT_GET_STATE_EX (x_input_get_state_ex);
X_INPUT_GET_STATE_EX(XInputGetStateExStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
global x_input_get_state_ex *XInputGetStateEx_ = XInputGetStateExStub;
#define XInputGetStateEx XInputGetStateEx_
#define XINPUT_GAMEPAD_GUIDE 0x400

// XInputPowerOffController function definitions
#define XINPUT_POWER_OFF_CONTROLLER(name) DWORD name(DWORD dwUserIndex)
typedef XINPUT_POWER_OFF_CONTROLLER (xinput_power_off_controller);
XINPUT_POWER_OFF_CONTROLLER(XInputPowerOffControllerStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
global xinput_power_off_controller *XInputPowerOffController_ = XInputPowerOffControllerStub;
#define XInputPowerOffController XInputPowerOffController_

// NOTE XInputSetState function definitions
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION * pVibration)
typedef X_INPUT_SET_STATE (x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
global x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal void Win32LoadXInput()
{
    HMODULE xInputLibrary = LoadLibrary("xinput1_4.dll");
    if(!xInputLibrary) {
        xInputLibrary = LoadLibrary("xinput1_3.dll");
    }
    if(!xInputLibrary) {
        xInputLibrary = LoadLibrary("xinput9_1_0.dll");
    }
    if(xInputLibrary) {
        XInputGetStateEx = (x_input_get_state_ex *)GetProcAddress( xInputLibrary,
                                                                   (LPCSTR)100 );
        if (!XInputGetStateEx) {
            XInputGetStateEx = XInputGetStateExStub;
        }
        XInputSetState = (x_input_set_state *)GetProcAddress(xInputLibrary,
                                                             "XInputSetState");
        if (!XInputSetState) {
            XInputSetState = XInputSetStateStub;
        }
        XInputPowerOffController =
            (xinput_power_off_controller *)GetProcAddress(xInputLibrary, (LPCSTR)103);
        if (!XInputPowerOffController) {
            XInputPowerOffController = XInputPowerOffControllerStub;
        }
    }
} // Win32LoadXInput

void SendMouse(DWORD Flags, LONG X, LONG Y, DWORD data)
{
    INPUT Input = {};
    Input.type          = INPUT_MOUSE;
    Input.mi.dx         = X;
    Input.mi.dy         = Y;
    Input.mi.dwFlags    = Flags;
    Input.mi.mouseData  = data;
    SendInput( 1, &Input, sizeof(INPUT) );
}

void MoveMouseRelative(LONG X, LONG Y)
{
    SendMouse(MOUSEEVENTF_MOVE, X, Y, 0);
}

void SendMouseDown(bool32 IsLeft)
{
    DWORD Flags = 0;
    if (IsLeft) {
        Flags = MOUSEEVENTF_LEFTDOWN;
    } else {
        Flags = MOUSEEVENTF_RIGHTDOWN;
    }

    SendMouse(Flags, 0, 0, 0);
}

void SendMouseUp(bool32 IsLeft)
{
    DWORD Flags = 0;
    if (IsLeft) {
        Flags = MOUSEEVENTF_LEFTUP;
    } else {
        Flags = MOUSEEVENTF_RIGHTUP;
    }
    SendMouse(Flags, 0, 0, 0);
}

void SendMouseWheel(real32 Clicks, bool Horizontal)
{
    DWORD Event = MOUSEEVENTF_WHEEL;
    if (Horizontal) {
        Event = MOUSEEVENTF_HWHEEL;
    }
    SendMouse( Event, 0, 0, (uint32)(Clicks * 40) );
}

void SendKeyDown(uint16 KeyCode)
{
    INPUT Input = {};
    Input.type          = INPUT_KEYBOARD;
    Input.ki.wVk        = KeyCode;
    Input.ki.dwFlags    = 0;
    SendInput( 1, &Input, sizeof(INPUT) );
}

void SendKeyUp(uint16 KeyCode)
{
    INPUT Input = {};
    Input.type          = INPUT_KEYBOARD;
    Input.ki.wVk        = KeyCode;
    Input.ki.dwFlags    = KEYEVENTF_KEYUP;
    SendInput( 1, &Input, sizeof(INPUT) );
}

uint16 PressKey(uint16 KeyCode)
{
    SendKeyDown(KeyCode);
    SendKeyUp(KeyCode);
    return KeyCode;
}

// NOTE(cory): Letters must be captial
void InputCharacter(uint16 Key, uint8 Modifiers)
{
    if (Modifiers) {
        if (Modifiers & modifiers::Control) {
            SendKeyDown(VK_CONTROL);
        }
        if (Modifiers & modifiers::Shift) {
            SendKeyDown(VK_SHIFT);
        }
        if (Modifiers & modifiers::Alt) {
            SendKeyDown(VK_MENU);
        }
        if (Modifiers & modifiers::Windows) {
            SendKeyDown(VK_LWIN);
        }
    }

    PressKey(Key);

    if (Modifiers) {
        if (Modifiers & modifiers::Control) {
            SendKeyUp(VK_CONTROL);
        }
        if (Modifiers & modifiers::Shift) {
            SendKeyUp(VK_SHIFT);
        }
        if (Modifiers & modifiers::Alt) {
            SendKeyUp(VK_MENU);
        }
        if (Modifiers & modifiers::Windows) {
            SendKeyUp(VK_LWIN);
        }
    }
} // InputCharacter

int32 CalculateStickMagnitude(short StickValue, real32 Multiplier)
{
    LONG Result = 0;
    if ( (StickValue > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ||
         ( StickValue < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ) {
        int Modifier = 1;
        if (StickValue < 0) {
            Modifier = -1;
        }
        Result = (LONG)( Modifier * mw::math::Power(
                             (real32)StickValue / 0xfff * Multiplier, 2 ) );
    }
    return Result;
}

void SetButtonState(XINPUT_STATE *State, button *NewButton, button *OldButton, WORD Mask)
{
    NewButton->EndedDown        = ( (State->Gamepad.wButtons & Mask) != 0 );
    NewButton->HalfTransitions  = (OldButton->EndedDown ^ NewButton->EndedDown) ? 1 : 0;
}

void SetAnalogStickState(analog_stick *Stick, int16 X, int16 Y)
{
    Stick->X    = (real32)X / 32767;
    Stick->Y    = (real32)Y / 32767;
    real32 DeadZone = 0.20f;
    if ( (Stick->X < DeadZone) && (Stick->X > -DeadZone) ) {
        Stick->X = 0;
    }
    if ( (Stick->Y < DeadZone) && (Stick->Y > -DeadZone) ) {
        Stick->Y = 0;
    }
}

void UpdateControllerWithXInputState(controller *   OldController,
                                     controller *   NewController,
                                     XINPUT_STATE * State)
{
    SetButtonState(State, &NewController->Up, &OldController->Up, XINPUT_GAMEPAD_DPAD_UP);
    SetButtonState(State, &NewController->Down, &OldController->Down, XINPUT_GAMEPAD_DPAD_DOWN);
    SetButtonState(State, &NewController->Left, &OldController->Left, XINPUT_GAMEPAD_DPAD_LEFT);
    SetButtonState(State, &NewController->Right, &OldController->Right, XINPUT_GAMEPAD_DPAD_RIGHT);
    SetButtonState(State, &NewController->Start, &OldController->Start, XINPUT_GAMEPAD_START);
    SetButtonState(State, &NewController->Back, &OldController->Back, XINPUT_GAMEPAD_BACK);
    SetButtonState(State,
                   &NewController->LeftThumb,
                   &OldController->LeftThumb,
                   XINPUT_GAMEPAD_LEFT_THUMB);
    SetButtonState(State,
                   &NewController->RightThumb,
                   &OldController->RightThumb,
                   XINPUT_GAMEPAD_RIGHT_THUMB);
    SetButtonState(State,
                   &NewController->LeftShoulder,
                   &OldController->LeftShoulder,
                   XINPUT_GAMEPAD_LEFT_SHOULDER);
    SetButtonState(State,
                   &NewController->RightShoulder,
                   &OldController->RightShoulder,
                   XINPUT_GAMEPAD_RIGHT_SHOULDER);
    SetButtonState(State, &NewController->A, &OldController->A, XINPUT_GAMEPAD_A);
    SetButtonState(State, &NewController->B, &OldController->B, XINPUT_GAMEPAD_B);
    SetButtonState(State, &NewController->X, &OldController->X, XINPUT_GAMEPAD_X);
    SetButtonState(State, &NewController->Y, &OldController->Y, XINPUT_GAMEPAD_Y);
    SetButtonState(State, &NewController->Guide, &OldController->Guide, XINPUT_GAMEPAD_GUIDE);
    NewController->LeftTrigger  = State->Gamepad.bLeftTrigger;
    NewController->RightTrigger = State->Gamepad.bRightTrigger;
    SetAnalogStickState(&NewController->LeftStick, State->Gamepad.sThumbLX,
                        State->Gamepad.sThumbLY);
    SetAnalogStickState(&NewController->RightStick, State->Gamepad.sThumbRX,
                        State->Gamepad.sThumbRY);
} // UpdateControllerWithXInputState

bool32 ButtonDown(button Button)
{
    return Button.EndedDown;
}

bool32 ButtonPressed(button Button)
{
    return Button.HalfTransitions > 0 && !Button.EndedDown;
}

LRESULT CALLBACK WindowCallback(HWND Window, UINT Msg, WPARAM WParam, LPARAM LParam)
{
    return 1;
}

internal HWND MakeWindow(HINSTANCE instance, int windowWidth, int windowHeight)
{
    RECT r = {};
// TODO(cory): Remove the debug offset so window fits correctly
    r.right     = windowWidth;
    r.bottom    = windowHeight;

    // NOTE(cory): Init Window Class
    WNDCLASS windowClass = {};
    windowClass.style           = CS_OWNDC;
    windowClass.lpfnWndProc     = WindowCallback;
    windowClass.hInstance       = instance;
    windowClass.lpszClassName   = "ControllerMouseWindowClass";
    if ( !RegisterClass(&windowClass) ) {
        // TODO(cory): Log failure
        return 0;
    }

    // NOTE(cory): Create Window
    HWND window = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        windowClass.lpszClassName,
        "Hi",
        WS_POPUP | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        r.right - r.left,
        r.bottom - r.top,
        0,
        0,
        instance,
        0
        );
    if (!window) {
        // TODO(cory): Log failure
        return 0;
    }
    return window;
} // Win32CreateWindow

internal void Draw(HWND Window, Gdiplus::Bitmap *Background, POINT ptOrigin)
{
    Gdiplus::Color *Alpha = new Gdiplus::Color(0,0,0,0);
    HBITMAP BitmapHandle;
    Background->GetHBITMAP(*Alpha, &BitmapHandle);
    // get the size of the bitmap
    BITMAP bm;
    GetObject(BitmapHandle, sizeof(bm), &bm);
    SIZE sizeSplash = {
        bm.bmWidth, bm.bmHeight
    };

    // get the primary monitor's info
    POINT ptZero            = {
        0
    };
    HMONITOR hmonPrimary    = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorinfo = {
        0
    };
    monitorinfo.cbSize = sizeof(monitorinfo);
    GetMonitorInfo(hmonPrimary, &monitorinfo);

    // center the splash screen in the middle of the primary work area

    // create a memory DC holding the splash bitmap
    HDC hdcScreen       = GetDC(NULL);
    HDC hdcMem          = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmpOld     = (HBITMAP) SelectObject(hdcMem, BitmapHandle);

    // use the source image's alpha channel for blending
    BLENDFUNCTION blend = {
        0
    };
    blend.BlendOp               = AC_SRC_OVER;
    blend.SourceConstantAlpha   = 255;
    blend.AlphaFormat           = AC_SRC_ALPHA;

    // paint the window (in the right location) with the alpha-blended bitmap
    UpdateLayeredWindow(Window, hdcScreen, &ptOrigin, &sizeSplash,
                        hdcMem, &ptZero, RGB(0, 0, 0), &blend, ULW_ALPHA);

    // delete temporary objects
    SelectObject(hdcMem, hbmpOld);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
} // SetWindowImage

void Show(HWND Window)
{
    SetWindowPos(Window, 0, 0,0,0,0,SWP_SHOWWINDOW | SWP_NOREPOSITION);
}

void Hide(HWND Window)
{
    SetWindowPos(Window, 0, 0,0,0,0,SWP_HIDEWINDOW | SWP_NOREPOSITION);
}

void DrawString(Gdiplus::Graphics * Gx,
                char *              String,
                draw_string_format  Format,
                Gdiplus::PointF     Origin)
{
    int SizeNeeded      = MultiByteToWideChar( CP_ACP, 0, String, -1, 0, 0);
    wchar_t *WideString =
        (wchar_t *)VirtualAlloc(0, SizeNeeded * sizeof(wchar_t), MEM_COMMIT, PAGE_READWRITE);
    if (!WideString) {
        return;
    }
    MultiByteToWideChar(CP_ACP, 0, String, -1, WideString, SizeNeeded);
    Gx->DrawString(
        WideString,
        -1,
        Format.Font,
        Origin,
        Format.StringFormat,
        Format.Brush);
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCode)
{
    Win32LoadXInput();
    HWND Window = MakeWindow(Instance, 480, 480);

    // NOTE(cory): Set Up GDI
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    char EXEPath[MAX_PATH];
    mw::GetExecutablePath( EXEPath, sizeof(EXEPath) );
    char ResourcePath[MAX_PATH];
    mw::ConcatenateStrings( EXEPath, "\\keyboard_background.png", ResourcePath,
                            sizeof(ResourcePath) );
    wchar_t WideResourcePath[MAX_PATH];
    mw::AnsiStringToWideString( ResourcePath, WideResourcePath, sizeof(WideResourcePath) );

    // NOTE(cory): Go grab background!
    Gdiplus::Bitmap *Background = new Gdiplus::Bitmap(WideResourcePath);

    draw_string_format NormalText;
    NormalText.Font         = new Gdiplus::Font(L"Segoe UI",
                                                52,
                                                Gdiplus::FontStyleRegular,
                                                Gdiplus::UnitPoint);
    NormalText.StringFormat = new Gdiplus::StringFormat;
    NormalText.StringFormat->SetAlignment(Gdiplus::StringAlignmentCenter);
    NormalText.StringFormat->SetLineAlignment(Gdiplus::StringAlignmentCenter);
    NormalText.Brush        = new Gdiplus::SolidBrush( Gdiplus::Color(255, 255, 255, 255) );
    draw_string_format BigText;
    BigText                 = NormalText;
    BigText.Font            =
        new Gdiplus::Font(L"Segoe UI", 75, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);

    Gdiplus::Bitmap *NewBG = Background->Clone( 0, 0,
                                                Background->GetWidth(), Background->GetHeight(),
                                                Background->GetPixelFormat() );
    Gdiplus::Graphics Gx(NewBG);
    Gx.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

    Gdiplus::Color ClearColor(0,0,0,0);
    Gdiplus::CachedBitmap CachedBackground(Background, &Gx);

    // NOTE(cory): Hide Window and Clear out Background for Startup
    Gx.Clear(ClearColor);
    Hide(Window);

    bool32 MouseLeftDown    = false;
    bool32 MouseRightDown   = false;
    bool32 Paused           = false;
    uint32 PauseWaitTicks   = 0;

    controller Controller1  = {};
    controller Controller2 = {};
    controller *NewController   = &Controller1;
    controller *OldController   = &Controller2;

    bool32 TextInputMode        = false;
    char Input[2];
    Input[0]    = 0;
    Input[1]    = 0;
    char CurrentInput           = 0;
    bool32 TextInputUpperCase   = false;

    uint16 RepeatKey            = 0;
    button *RepeatButton        = 0;
    uint32 RepeatTicks          = 0;

    while(true) {
        Sleep(16);

        XINPUT_STATE State = {};
        DWORD Result        = XInputGetStateEx(0, &State);

        controller *Temp    = OldController;
        OldController   = NewController;
        NewController   = Temp;

        UpdateControllerWithXInputState(OldController, NewController, &State);

        if (Result == ERROR_SUCCESS) {
            if ( (PauseWaitTicks == 0)
                 && (NewController->LeftTrigger > 0) && (NewController->RightTrigger > 0)
                 && ButtonDown(NewController->Start) && ButtonDown(NewController->Back) ) {
                if ( ButtonDown(NewController->LeftShoulder)
                     && ButtonDown(NewController->RightShoulder) ) {
                    break;
                }
                Paused          = !Paused;
                PauseWaitTicks  = 30;
                continue;
            }
            if (PauseWaitTicks > 0) {
                PauseWaitTicks--;
            }
            if (Paused) {
                Hide(Window);
                TextInputMode       = false;
                CurrentInput        = 0;
                TextInputUpperCase  = false;
                RepeatKey           = 0;
                RepeatButton        = 0;
                RepeatTicks         = 0;
                continue;
            }
            if ( ButtonPressed(NewController->Guide) ) {
                XInputPowerOffController(0);
            }
            BlockInput(true);
            if (TextInputMode) {
                POINT Position;
                GetCursorPos(&Position);
                Gx.Clear(ClearColor);
                Gx.DrawCachedBitmap(&CachedBackground, 0, 0);

                int Page = 0;
                if (NewController->RightTrigger > 0) {
                    Page = 1;
                } else if (NewController->LeftTrigger > 0) {
                    Page = 2;
                }

                if (TextInputUpperCase) {
                    Page += 3;
                }

                Gdiplus::PointF *PositionStart  = KeyPositions;
                int NumKeys                     = 16;
                int StartIndex                  = 0;
                draw_string_format Format       = NormalText;

                if (CurrentInput > 0) {
                    StartIndex      = (4 * Input[0]);
                    PositionStart   = InputTwoPositions;
                    NumKeys         = 4;
                    Format          = BigText;
                }

                key *KeyPage = KeyboardArray[Page];

                for (int Index = 0; Index < NumKeys; Index++) {
                    DrawString( &Gx, (KeyPage + Index + StartIndex)->ShownText, Format,
                                *(PositionStart + Index) );
                }

                Draw( Window, NewBG, Position);
                char Direction = 0;
                if ( ButtonPressed(NewController->A) ) {
                    Direction = 1;
                } else if ( ButtonPressed(NewController->X) ) {
                    Direction = 2;
                } else if ( ButtonPressed(NewController->Y) ) {
                    Direction = 3;
                } else if ( ButtonPressed(NewController->B) ) {
                    Direction = 4;
                }
                if ( (CurrentInput == 0) && (Direction > 0) ) {
                    Input[0] = Direction - 1;
                    CurrentInput++;
                } else if ( (CurrentInput == 1) && (Direction > 0) ) {
                    Input[1]        = Direction - 1;
                    int Entry       = Input[0] * 4 + Input[1];
                    key InputKey    = KeyboardArray[Page][Entry];
                    InputCharacter(InputKey.Keycode, InputKey.Modifiers);
                    CurrentInput    = 0;
                }
                int RepeatWaitTicks = 25;
                if ( !RepeatButton && ButtonDown(NewController->RightShoulder) ) {
                    RepeatKey       = PressKey(VK_SPACE);
                    RepeatButton    = &NewController->RightShoulder;
                    RepeatTicks     = RepeatWaitTicks;
                } else if ( !RepeatButton && ButtonDown(NewController->LeftShoulder) ) {
                    RepeatKey       = PressKey(VK_BACK);
                    RepeatButton    = &NewController->LeftShoulder;
                    RepeatTicks     = RepeatWaitTicks;
                } else if ( !RepeatButton && ButtonDown(NewController->Left) ) {
                    RepeatKey       = PressKey(VK_LEFT);
                    RepeatButton    = &NewController->Left;
                    RepeatTicks     = RepeatWaitTicks;
                } else if ( !RepeatButton && ButtonDown(NewController->Right) ) {
                    RepeatKey       = PressKey(VK_RIGHT);
                    RepeatButton    = &NewController->Right;
                    RepeatTicks     = RepeatWaitTicks;
                } else if ( !RepeatButton && ButtonDown(NewController->Up) ) {
                    RepeatKey       = PressKey(VK_UP);
                    RepeatButton    = &NewController->Up;
                    RepeatTicks     = RepeatWaitTicks;
                } else if ( !RepeatButton && ButtonDown(NewController->Down) ) {
                    RepeatKey       = PressKey(VK_DOWN);
                    RepeatButton    = &NewController->Down;
                    RepeatTicks     = RepeatWaitTicks;
                }

                if (RepeatTicks > 0) {
                    RepeatTicks--;
                }
                if ( RepeatButton && ButtonDown(*RepeatButton) ) {
                    if (RepeatTicks == 0) {
                        PressKey(RepeatKey);
                        RepeatTicks = 5;
                    }
                } else {
                    RepeatButton = 0;
                }

                if ( ButtonPressed(NewController->Start) ) {
                    PressKey(VK_RETURN);
                }
                if ( ButtonPressed(NewController->LeftThumb) ) {
                    TextInputUpperCase = !TextInputUpperCase;
                }
                if ( ButtonPressed(NewController->Back) ) {
                    TextInputMode = false;
                    Hide(Window);
                }
            } else {
                if ( !MouseLeftDown && ButtonDown(NewController->A) ) {
                    MouseLeftDown = true;
                    SendMouseDown(true);
                } else if ( MouseLeftDown && !ButtonDown(NewController->A) ) {
                    MouseLeftDown = false;
                    SendMouseUp(true);
                }
                if ( !MouseRightDown && ButtonDown(NewController->B) ) {
                    MouseRightDown = true;
                    SendMouseDown(false);
                } else if ( MouseRightDown && !ButtonDown(NewController->B) ) {
                    MouseRightDown = false;
                    SendMouseUp(false);
                }
                if ( ButtonPressed(NewController->RightThumb) ) {
                    InputCharacter('P', modifiers::Shift | modifiers::Windows);
                }
                if ( ButtonPressed(NewController->Y) ) {
                    Input[0]        = 0;
                    Input[1]        = 0;
                    CurrentInput    = 0;
                    RepeatKey       = 0;
                    RepeatTicks     = 0;
                    RepeatButton    = 0;
                    TextInputMode   = true;
                    Show(Window);
                    continue;
                }

                if (NewController->RightStick.Y != 0) {
                    SendMouseWheel(NewController->RightStick.Y, false);
                }
                if (NewController->RightStick.X != 0) {
                    SendMouseWheel(NewController->RightStick.X, true);
                }

                LONG DeltaY         = 0;
                LONG DeltaX         = 0;
                real32 MouseSpeed   = 3.0f;
                MouseSpeed  *= (real32)NewController->RightTrigger / 255 + 1;
                DeltaX      = (LONG)(NewController->LeftStick.X * MouseSpeed * MouseSpeed / 2);
                DeltaY      = (LONG)(-NewController->LeftStick.Y * MouseSpeed * MouseSpeed / 2);
                MoveMouseRelative(DeltaX, DeltaY);
            }

            BlockInput(false);
        }
    }
} // WinMain
