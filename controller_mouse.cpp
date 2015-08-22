#include <Windows.h>
#include <Winuser.h>
#include <xinput.h>
#include <stdio.h>
#include "/lib/mw.h"

#define SEND_INPUT(name) UINT WINAPI name(UINT NumInputs, LPINPUT Inputs, int SizeofInput)
typedef SEND_INPUT (send_input);
SEND_INPUT(SendInputStub) {
    return 0;
}
global send_input *SendInput_ = SendInputStub;
#define SendInput SendInput_
internal void LoadUser32()
{
    HMODULE User32Library = LoadLibrary("user32.dll");
    if(User32Library) {
        SendInput = (send_input *)GetProcAddress(User32Library, "SendInput");
        if (!SendInput) {
            SendInput = SendInputStub;
        }
    }
}

// XInputGetState function definitions
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE * pState)
typedef X_INPUT_GET_STATE (x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
global x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_
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
        XInputGetState = (x_input_get_state *)GetProcAddress(xInputLibrary, "XInputGetState");
        if (!XInputGetState) {
            XInputGetState = XInputGetStateStub;
        }
        XInputSetState = (x_input_set_state *)GetProcAddress(xInputLibrary, "XInputSetState");
        if (!XInputSetState) {
            XInputSetState = XInputSetStateStub;
        }
    }
} // Win32LoadXInput

/*
   internal void Win32GetXInputState(game_input *input)
   {
    int32 maxUsers = ArrayCount(input->controllers);
    if (maxUsers > XUSER_MAX_COUNT) {
        maxUsers = XUSER_MAX_COUNT;
    }
    for (int i = 0; i < maxUsers; i++) {
        int cIndex = i + 1;
        game_controller oldController = input->controllers[cIndex];
        game_controller *newController = &input->controllers[cIndex];
        XINPUT_STATE state = {};
        DWORD result = XInputGetState(i, &state);
        if (result == ERROR_SUCCESS) {
            newController->isConnected = true;
            newController->isAnalog = true;
            XINPUT_GAMEPAD pad = state.Gamepad;
            Win32SetDigitalButtonState(&newController->actionDown,
                                       &oldController.actionDown,
                                       pad.wButtons,
                                       XINPUT_GAMEPAD_A);
            Win32SetDigitalButtonState(&newController->actionRight,
                                       &oldController.actionRight,
                                       pad.wButtons,
                                       XINPUT_GAMEPAD_B);
            Win32SetDigitalButtonState(&newController->actionLeft,
                                       &oldController.actionLeft,
                                       pad.wButtons,
                                       XINPUT_GAMEPAD_X);
            Win32SetDigitalButtonState(&newController->actionUp,
                                       &oldController.actionUp,
                                       pad.wButtons,
                                       XINPUT_GAMEPAD_Y);
            Win32SetDigitalButtonState(&newController->start,
                                       &oldController.start,
                                       pad.wButtons,
                                       XINPUT_GAMEPAD_START);
            Win32SetDigitalButtonState(&newController->back,
                                       &oldController.back,
                                       pad.wButtons,
                                       XINPUT_GAMEPAD_BACK);
                 input->DPad.Up = pad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
               input->DPad.Down = pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
               input->DPad.Left = pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
               input->DPad.Right = pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
            Win32SetAnalogStickState(newController, pad.sThumbLX, pad.sThumbLY);
            //Win32SetAnalogStickState(newController, pad.sThumbRX, pad.sThumbRY);
        } else {
            newController->isConnected = false;
            newController->isAnalog = oldController.isAnalog;
        }
    }
   } // Win32GetXInputState
 */
void SendMouse(DWORD Flags, LONG X, LONG Y, DWORD data)
{
    INPUT Input = {};
    Input.type = INPUT_MOUSE;
    Input.mi.dx = X;
    Input.mi.dy = Y;
    Input.mi.dwFlags = Flags;
    Input.mi.mouseData = data;
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

void SendMouseWheel(int Clicks)
{
    SendMouse(MOUSEEVENTF_WHEEL, 0, 0, Clicks * 120);
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCode)
{
    Win32LoadXInput();
    LoadUser32();
    int32 NormalizationFactor = 4000;
    bool32 MouseLeftDown = false;
    bool32 MouseRightDown = false;
    bool32 Paused = false;
    uint32 PauseWaitTicks = 0;
    while(true) {
        Sleep(16);
        XINPUT_STATE State = {};
        DWORD Result = XInputGetState(0, &State);
        if (Result == ERROR_SUCCESS) {
            if ( (PauseWaitTicks == 0)
                 && (State.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
                 && ( State.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
                 && State.Gamepad.wButtons & XINPUT_GAMEPAD_START
                 && State.Gamepad.wButtons & XINPUT_GAMEPAD_BACK ) {
                if (State.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER
                    && State.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
                    break;
                }
                Paused = !Paused;
                PauseWaitTicks = 30;
                continue;
            }
            if (PauseWaitTicks > 0) {
                PauseWaitTicks--;
            }
            if (Paused) {
                continue;
            }
            /*
               if (State.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) {
                NormalizationFactor += 100;
               }
               if (State.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
                NormalizationFactor -= 100;
               }
               if (NormalizationFactor < 1000) {
                NormalizationFactor = 1000;
               }
               if (NormalizationFactor > 16000) {
                NormalizationFactor = 16000;
               }
             */

            if (!MouseLeftDown && State.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
                MouseLeftDown = true;
                SendMouseDown(true);
            }
            if ( MouseLeftDown && !(State.Gamepad.wButtons & XINPUT_GAMEPAD_A) ) {
                MouseLeftDown = false;
                SendMouseUp(true);
            }
            if (!MouseRightDown && State.Gamepad.wButtons & XINPUT_GAMEPAD_B) {
                MouseRightDown = true;
                SendMouseDown(false);
            }
            if ( MouseRightDown && !(State.Gamepad.wButtons & XINPUT_GAMEPAD_B) ) {
                MouseRightDown = false;
                SendMouseUp(false);
            }

            LONG DeltaY = 0;
            LONG DeltaX = 0;
            if ( (State.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ||
                 ( State.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ) {
                DeltaY = (-State.Gamepad.sThumbLY) / NormalizationFactor;
            }
            if ( (State.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ||
                 ( State.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) ) {
                DeltaX = (State.Gamepad.sThumbLX) / NormalizationFactor;
            }
            if ( (State.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) ||
                 ( State.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) ) {
                if (State.Gamepad.sThumbRY > 0) {
                    SendMouseWheel(1);
                } else {
                    SendMouseWheel(-1);
                }
            }
            MoveMouseRelative(DeltaX, DeltaY);
        }
    }
} // main
