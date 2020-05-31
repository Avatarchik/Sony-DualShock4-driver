#include <windows.h>
#include "hidapi\hidapi.h"
#include "NexInput.h"
#include "DualShock4.h"

#define DLLEXPORT extern "C" __declspec(dllexport)

DWORD gamepadCount = 0;
bool DS4Found = false;
hid_device *gamepadHandle[NEX_INPUT_MAX_COUNT];
//unsigned short gamepadType[NEX_INPUT_MAX_COUNT]; // USB, BT

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			//Search all DualShock 4
			struct hid_device_info *cur_dev;
			//cur_dev = hid_enumerate(0x0, 0x0);
			cur_dev = hid_enumerate(DS4_VENDOR, 0x0);

			while (cur_dev) {
					if (cur_dev->product_id == DS4_USB || cur_dev->product_id == DS4_V2_USB) // || cur_dev->product_id == DS4_BT) // BT soon
					{
						gamepadHandle[gamepadCount] = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
						hid_set_nonblocking(gamepadHandle[gamepadCount], 1);
						
						DS4Found = true;
						gamepadCount++;

						if (gamepadCount == NEX_INPUT_MAX_COUNT)
							break;
					}
				cur_dev = cur_dev->next;
			}
			if (DS4Found)
				gamepadCount--; // Back normal count

			//gamepadCount = 0;
			//gamepadHandle[gamepadCount] = hid_open(DS4_VENDOR, DS4_USB, NULL);

			if (cur_dev)
				hid_free_enumeration(cur_dev);

			break;
		}

		case DLL_PROCESS_DETACH:
		{
			for (int i = 0; i <= gamepadCount; i++)
				hid_close(gamepadHandle[i]);
			break;
		}
	}
	return true;
}

DLLEXPORT DWORD __stdcall NEXInputGetState(__in DWORD dwUserIndex, __out NEX_INPUT_STATE *pState)
{
	pState->Buttons = 0; //0 .. 255
	pState->LeftTrigger = 0; //0 .. 255
	pState->RightTrigger = 0; //0 .. 255
	pState->AxisLX = 0; //-32768 .. 32767
	pState->AxisLY = 0; //-32768 .. 32767
	pState->AxisRX = 0; //-32768 .. 32767
	pState->AxisRY = 0; //-32768 .. 32767

	pState->Yaw = 0;
	pState->Pitch = 0;
	pState->Roll = 0;

	if (DS4Found && gamepadCount >= dwUserIndex && gamepadHandle[dwUserIndex]) {
		DS4_USB_PACKET DS4State;
		if (hid_read(gamepadHandle[dwUserIndex], (u8 *)&DS4State, 64) == 64)
		{
			//https://github.com/ViGEm/VDX/blob/master/src/VDX.h
			pState->AxisLX = (DS4State.AxisLX + ((USHRT_MAX / 2) + 1)) * 257;
			pState->AxisLY = (-(DS4State.AxisLY + ((USHRT_MAX / 2) + 1)) * 257);
			pState->AxisRX = ((DS4State.AxisRX + ((USHRT_MAX / 2) + 1)) * 257);
			pState->AxisRY = (-(DS4State.AxisRY + ((USHRT_MAX / 2) + 1)) * 257);

			//Madgwicks AHRS filter 
			//pState->Yaw = MadgwickToYPR( DS4State.Accel[0], DS4State.Accel[1], DS4State.Accel[2], DS4State.Gyro[0], DS4State.Gyro[1], DS4State.Gyro[2] )->Yaw;
			//pState->Pitch = MadgwickToYPR( DS4State.Accel[0], DS4State.Accel[1], DS4State.Accel[2], DS4State.Gyro[0], DS4State.Gyro[1], DS4State.Gyro[2] )->Pitch;
			//pState->Roll = MadgwickToYPR( DS4State.Accel[0], DS4State.Accel[1], DS4State.Accel[2], DS4State.Gyro[0], DS4State.Gyro[1], DS4State.Gyro[2] )->Roll;

			pState->LeftTrigger = DS4State.LeftTrigger;
			pState->RightTrigger = DS4State.RightTrigger;

			pState->Buttons |= DS4State.Buttons & DS4_BUTTON_SHOULDER_LEFT ? NEX_GAMEPAD_LEFT_SHOULDER : 0;
			pState->Buttons |= DS4State.Buttons & DS4_BUTTON_SHOULDER_RIGHT ? NEX_GAMEPAD_RIGHT_SHOULDER : 0;

			pState->Buttons |= DS4State.Buttons & DS4_BUTTON_SHARE ? NEX_GAMEPAD_BACK : 0;
			pState->Buttons |= DS4State.Buttons & DS4_BUTTON_OPTIONS ? NEX_GAMEPAD_START : 0;

			pState->Buttons |= DS4State.Buttons & DS4_BUTTON_TRIANGLE ? NEX_GAMEPAD_Y : 0;
			pState->Buttons |= DS4State.Buttons & DS4_BUTTON_SQUARE ? NEX_GAMEPAD_X : 0;
			pState->Buttons |= DS4State.Buttons & DS4_BUTTON_CIRCLE ? NEX_GAMEPAD_B : 0;
			pState->Buttons |= DS4State.Buttons & DS4_BUTTON_CROSS ? NEX_GAMEPAD_A : 0;

			pState->Buttons |= DS4State.Buttons & DS4_BUTTON_THUMB_LEFT ? NEX_GAMEPAD_LEFT_THUMB : 0;
			pState->Buttons |= DS4State.Buttons & DS4_BUTTON_THUMB_RIGHT ? NEX_GAMEPAD_RIGHT_THUMB : 0;

			DS4State.Buttons &= 0xF;
			//https://github.com/JibbSmart/JoyShockLibrary/blob/master/JoyShockLibrary/InputHelpers.cpp
			pState->Buttons |= (DS4State.Buttons > 2) & (DS4State.Buttons < 6) ? NEX_GAMEPAD_DPAD_DOWN : 0; // down = SE | S | SW
			pState->Buttons |= (DS4State.Buttons == 7) | (DS4State.Buttons < 2) ? NEX_GAMEPAD_DPAD_UP : 0; // up = N | NE | NW
			pState->Buttons |= (DS4State.Buttons > 0) & (DS4State.Buttons < 4) ? NEX_GAMEPAD_DPAD_RIGHT : 0; // right = NE | E | SE
			pState->Buttons |= (DS4State.Buttons > 4) & (DS4State.Buttons < 8) ? NEX_GAMEPAD_DPAD_LEFT : 0; // left = SW | W | NW
		}

		return ERROR_SUCCESS;
	}
	else
		return ERROR_DEVICE_NOT_CONNECTED;
}

DLLEXPORT DWORD __stdcall NEXInputSetState(__in DWORD dwUserIndex, __in NEX_OUTPUT_STATE *pOutputState)
{
	if (DS4Found && gamepadCount >= dwUserIndex && gamepadHandle[dwUserIndex]) {
		unsigned char rumblePacket[31];
		memset(rumblePacket, 0, 31);
		rumblePacket[0] = 0x05;
		rumblePacket[1] = 0xff;
		rumblePacket[4] = pOutputState->LeftMotorSpeed / 257; //0 .. 65535
		rumblePacket[5] = pOutputState->RightMotorSpeed / 257; //0 .. 65535
		// RGB, soon
		rumblePacket[6] = pOutputState->LEDRed;
		rumblePacket[7] = pOutputState->LEDGreen;
		rumblePacket[8] = pOutputState->LEDBlue;
		// flash 1
		rumblePacket[9] = 0x00; //pOutputState->LEDBrightness?
		// flash 2
		rumblePacket[10] = 0x00;

		hid_write(gamepadHandle[dwUserIndex], rumblePacket, 31);

		return ERROR_SUCCESS;
	} else
	return ERROR_DEVICE_NOT_CONNECTED;
}

DLLEXPORT DWORD __stdcall NEXInputGetInfo(__in DWORD dwUserIndex, __out NEX_CONTROLLER_INFO *pControllerInfo)
{
	pControllerInfo->ControllerType = SONY_DUALSHOCK_4_CONTROLLER;
	pControllerInfo->ConnectType = NEX_CONTROLLER_WIRED; //or NEX_CONTROLLER_WIRELESS
	pControllerInfo->BatteryLevel = NEX_BATTERY_NONE; //1 .. 5 or NEX_BATTERY_NONE
	pControllerInfo->SupportRotation = true; 

	if (DS4Found && gamepadCount >= dwUserIndex && gamepadHandle[dwUserIndex])
		return ERROR_SUCCESS;
	else
		return ERROR_DEVICE_NOT_CONNECTED;
}

DLLEXPORT DWORD __stdcall NEXInputPowerOff(__in DWORD dwUserIndex)
{
	//Turn off controller
	if (DS4Found && gamepadCount >= dwUserIndex && gamepadHandle[dwUserIndex])
		return ERROR_SUCCESS;
	else
		return ERROR_DEVICE_NOT_CONNECTED;
}
