#ifndef variables_define_file
#define variables_define_file

#include <graphx.h>

#include "gfx/gfx.h" 

#define FRAME_TIME 1600
#define CURSOR_BLINK_TIME 10
#define HOLD_TIME 3

#define DASH_WIDTH (dashleft_width + dashcenter_width + dashright_width)
#define DASH_HEIGHT dashleft_height
#define DASH_HOFFSET (GFX_LCD_WIDTH - DASH_WIDTH) / 2
#define DASH_VOFFSET (GFX_LCD_HEIGHT - DASH_HEIGHT)
#define DASH_HOFFSET_CENTER (DASH_HOFFSET + dashleft_width)
#define DASH_HOFFSET_RIGHT (DASH_HOFFSET + dashleft_width + dashcenter_width)

#define HEADER_Y 8
#define HEADER_DIVIDER_Y 20

#define xor_clipX (DASH_HOFFSET + 1)
#define xor_clipY 1
#define xor_clipWidth (DASH_WIDTH - 2)
#define xor_clipHeight (DASH_VOFFSET - 2)
#define xor_textRows (xor_clipHeight / 8)
#define xor_textCols (xor_clipWidth / 8)

#define LEFT_TEXT_INDENT 8

#define MM_SELBAR_WIDTH 120

#define LCL_MAP_HFIX 125
#define LCL_MAP_VFIX 95
#define LCL_MAP_DXMAX (LCL_MAP_HFIX / 4 - DASH_HOFFSET / 4)
#define LCL_MAP_DYMAX (LCL_MAP_VFIX / 2 - HEADER_DIVIDER_Y / 2)
#define GLX_MAP_HOFFSET (DASH_HOFFSET + (DASH_WIDTH - 256) / 2)
#define GLX_MAP_VOFFSET GLX_MAP_HOFFSET

#define CRS_SPREAD 10
#define CRS_SIZE 10
#define SML_CRS_SPREAD 4
#define SML_CRS_SIZE 4

#define RADAR_HCENTER (DASH_HOFFSET + dashleft_width + 74)
#define RADAR_XSCALE 256
#define RADAR_VCENTER (DASH_VOFFSET + 28)
#define RADAR_ZSCALE 1024
#define RADAR_YSCALE 512

#define COLOR_BLACK 0
#define COLOR_WHITE 1
#define COLOR_YELLOW 2

#define VIEW_HCENTER (DASH_WIDTH / 2 + DASH_HOFFSET)
#define VIEW_VCENTER (DASH_VOFFSET / 2)
#define SHPPT_WIDTH 3
#define SHPPT_HEIGHT 2
#define DISP_Z_SCALE 256

#define MAX_SHIPS 16
#define STARDUST_COUNT 12
#define NUM_TRADE_GOODS 17

#define PLAYER_MAX_SPEED 0x1c

#define cmdr_name "JAMESON"
#define cmdr_name_length 7

enum viewDirMode_t
{
	FRONT,
	LEFT,
	RIGHT,
	REAR
};

extern unsigned char drawCycle;

enum currentMenu_t {
	NONE,
	MAIN,
	STATUS,
	THIS_DATA,
	SEL_DATA,
	LOCAL_MAP,
	GALAXY_MAP,
	INVENTORY,
	MARKET,
	UPGRADES
};
#define NUM_MENU_OPTIONS 5
extern enum currentMenu_t currentMenu;

extern unsigned char player_fuel;
extern unsigned int player_money;

extern unsigned char player_cargo_space;
extern unsigned char player_cargo_cap;

#endif
