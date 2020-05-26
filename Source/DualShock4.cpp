#include <windows.h>
#include "hidapi\hidapi.h"

#define DLLEXPORT extern "C" __declspec(dllexport)

#define NEX_GAMEPAD_DPAD_UP				0x0001
#define NEX_GAMEPAD_DPAD_DOWN			0x0002
#define NEX_GAMEPAD_DPAD_LEFT			0x0004
#define NEX_GAMEPAD_DPAD_RIGHT			0x0008
#define NEX_GAMEPAD_START				0x0010
#define NEX_GAMEPAD_BACK				0x0020
#define NEX_GAMEPAD_LEFT_THUMB			0x0040
#define NEX_GAMEPAD_RIGHT_THUMB			0x0080
#define NEX_GAMEPAD_LEFT_SHOULDER		0x0100
#define NEX_GAMEPAD_RIGHT_SHOULDER		0x0200
#define NEX_GAMEPAD_A					0x1000
#define NEX_GAMEPAD_B					0x2000
#define NEX_GAMEPAD_X					0x4000
#define NEX_GAMEPAD_Y					0x8000

#define NEX_CONTROLLER_WIRED			0
#define NEX_CONTROLLER_WIRELESS			1
#define NEX_BATTERY_NONE				0
#define NEX_BATTERY_LOW					1
#define NEX_BATTERY_FULL				5

#define NEX_INPUT_MAX_COUNT				4

#define ERROR_DEVICE_NOT_CONNECTED		1
#define ERROR_SUCCESS					0

#define NEX_UNKNOWN_CONTROLLER			0

#define MICROSOFT_XBOX_360_CONTROLLER	1
#define MICROSOFT_XBOX_ONE_CONTROLLER	2

#define SONY_DUALSHOCK_3_CONTROLLER		26
#define SONY_DUALSHOCK_4_CONTROLLER		27
#define SONY_DUALSHOCK_5_CONTROLLER		27

#define NINTENDO_SWITCH_PRO_CONTROLLER	51

typedef struct _NEX_INPUT_STATE
{
	WORD								Buttons;
	BYTE								LeftTrigger;
	BYTE								RightTrigger;
	SHORT								AxisLX;
	SHORT								AxisLY;
	SHORT								AxisRX;
	SHORT								AxisRY;
	float								Yaw;
	float								Pitch;
	float								Roll;
} NEX_INPUT_STATE, *PNEX_INPUT_STATE;

typedef struct _NEX_OUTPUT_STATE
{
	WORD								LeftMotorSpeed;
	WORD								RightMotorSpeed;
	BYTE								LEDBrightness;
	BYTE								LEDRed;
	BYTE								LEDGreen;
	BYTE								LEDBlue;
} NEX_OUTPUT_STATE, *PNEX_OUTPUT_STATE;

typedef struct _NEX_CONTROLLER_INFO
{
	WORD								ControllerType;
	BYTE								ConnectType;
	BYTE								BatteryLevel;
	bool								SupportRotation;
} NEX_CONTROLLER_INFO, *PNEX_CONTROLLER_INFO;


//DualShock4
#define DS4_VENDOR 0x054C
#define DS4_USB 0x05C4
#define DS4_V2_USB 0x09CC
#define DS4_BT 0x081F

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

//https://github.com/uucidl/pre.neige/blob/master/src/ds4.cpp
#pragma pack(push, 1)
struct DS4Touch {
	u32 id : 7;
	u32 inactive : 1;
	u32 x : 12;
	u32 y : 12;
};
struct DS4_USB_PACKET {
	u8 prepad;
	u8 AxisLX, AxisLY, AxisRX, AxisRY;
	u16 Buttons;
	u8 Trackpad : 2;
	u8 Timestamp : 6;
	u8 LeftTrigger, RightTrigger;
	u8 Pad[2];
	u8 Battery;
	short Accel[3], Gyro[3];
	u8 Pad0[35 - 25];
	DS4Touch Touch[2];
	u8 Pad2[64 - 43];
};
//https://github.com/ViGEm/ViGEmClient/blob/master/include/ViGEm/Common.h
#define DS4_BUTTON_THUMB_RIGHT		1 << 15
#define DS4_BUTTON_THUMB_LEFT		1 << 14
#define DS4_BUTTON_OPTIONS			1 << 13
#define DS4_BUTTON_SHARE			1 << 12
#define DS4_BUTTON_TRIGGER_RIGHT	1 << 11
#define DS4_BUTTON_TRIGGER_LEFT		1 << 10
#define DS4_BUTTON_SHOULDER_RIGHT	1 << 9
#define DS4_BUTTON_SHOULDER_LEFT	1 << 8
#define DS4_BUTTON_TRIANGLE			1 << 7
#define DS4_BUTTON_CIRCLE			1 << 6
#define DS4_BUTTON_CROSS			1 << 5
#define DS4_BUTTON_SQUARE			1 << 4

#define DS4_BUTTON_DPAD_NONE		0x8
#define DS4_BUTTON_DPAD_NORTHWEST	0x7
#define DS4_BUTTON_DPAD_WEST		0x6
#define DS4_BUTTON_DPAD_SOUTHWEST	0x5
#define DS4_BUTTON_DPAD_SOUTH		0x4
#define DS4_BUTTON_DPAD_SOUTHEAST	0x3
#define DS4_BUTTON_DPAD_EAST		0x2
#define DS4_BUTTON_DPAD_NORTHEAST	0x1
#define DS4_BUTTON_DPAD_NORTH		0x0


char gamepadCount = -1;
hid_device *gamepadHandle[NEX_INPUT_MAX_COUNT];

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			//Search all DualShock 4
			struct hid_device_info *devs, *cur_dev;
			//devs = hid_enumerate(0x0, 0x0);
			devs = hid_enumerate(DS4_VENDOR, 0x0);
			cur_dev = devs;

			while (cur_dev) {
				if (cur_dev->vendor_id == DS4_VENDOR)
					if (cur_dev->product_id == DS4_USB || cur_dev->product_id == DS4_V2_USB) //BT soon
					{
						gamepadCount++;
						gamepadHandle[gamepadCount] = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
						hid_set_nonblocking(gamepadHandle[gamepadCount], 1);

						if (gamepadCount == NEX_INPUT_MAX_COUNT - 1)
							break;
					}
				cur_dev = cur_dev->next;
			}
			//gamepadCount = 0;
			//gamepadHandle[gamepadCount] = hid_open(DS4_VENDOR, DS4_USB, NULL);

			hid_free_enumeration(devs);

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

	if (gamepadCount >= dwUserIndex) {
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
	if (gamepadCount >= dwUserIndex) {
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

	if (gamepadCount >= dwUserIndex)
		return ERROR_SUCCESS;
	else
		return ERROR_DEVICE_NOT_CONNECTED;
}

DLLEXPORT DWORD __stdcall NEXInputPowerOff(__in DWORD dwUserIndex)
{
	//Turn off controller
	if (gamepadCount >= dwUserIndex)
		return ERROR_SUCCESS;
	else
		return ERROR_DEVICE_NOT_CONNECTED;
}
