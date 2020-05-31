#pragma once

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