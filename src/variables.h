#ifndef variables_define_file
#define variables_define_file

#include <stdbool.h>
#include <graphx.h>
#include "gfx/gfx.h" 

#define SAVE_VAR_NAME "ELITESAV"

#define FRAME_TIME 1600
#define CURSOR_BLINK_TIME 10
#define FLTMSG_MED_TIME 25
#define HOLD_TIME 3
#define STAR_JUMP_HOLD_TIME 50
#define GALAXY_JUMP_HOLD_TIME 120
#define TUNNEL_FRAME_TIME 400
#define TUNNEL_HOLD_TIME 16000
#define DEATH_SCREEN_TIME 128000

#define DASH_WIDTH (dashleft_width + dashcenter_width + dashright_width)
#define DASH_HEIGHT dashleft_height
#define DASH_HOFFSET (GFX_LCD_WIDTH - DASH_WIDTH) / 2
#define DASH_VOFFSET (GFX_LCD_HEIGHT - DASH_HEIGHT)
#define DASH_HOFFSET_CENTER (DASH_HOFFSET + dashleft_width)
#define DASH_HOFFSET_RIGHT (DASH_HOFFSET + dashleft_width + dashcenter_width)

#define SOI_INDIC_POS_X (DASH_HOFFSET + dashleft_width + 142)
#define SOI_INDIC_POS_Y (DASH_VOFFSET + 42)

#define COMPASS_HCENTER (DASH_HOFFSET_CENTER + 145)
#define COMPASS_VCENTER (DASH_VOFFSET + 10)
#define COMPASS_SCALE (256 / 9)

#define HEADER_Y 7
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
#define COLOR_GREEN 3
#define COLOR_RED 4

#define VIEW_HCENTER (DASH_WIDTH / 2 + DASH_HOFFSET)
#define VIEW_VCENTER (DASH_VOFFSET / 2)
#define SHPPT_WIDTH 3
#define SHPPT_HEIGHT 2
#define DISP_Z_SCALE 256

#define MAX_SHIPS 16
#define STARDUST_COUNT 12
#define NUM_PLAYER_DEATH_CANS 2

#define NUM_TRADE_GOODS 17
#define NUM_UPGRADES 13

#define PLAYER_MAX_SPEED 0x1c

#define TTL_SHIP_ONE BP_COBRA
#define TTL_SHIP_TWO BP_VIPER
#define TTL_SHIP_START_Z 0x1200
#define TTL_SHIP_END_Z 0x0200
#define TTL_SHIP_ZOOM_RATE 0x90

#define CMDR_NAME_MAX_LENGTH 16

#define FLTMSG_MAX_LENGTH 24
#define FLTMSG_VOFFSET (DASH_VOFFSET - 20)

#define PULSE_LASER_INTERVAL 5
#define LASER_OVERHEAT_TEMP 242
#define LASER_MAX_RANGE 682

#define LASER_LINE_SPREAD 8
#define LASER_LINE_X_ONE 25
#define LASER_LINE_X_TWO 50

#define MISSILE_LAUNCH_Y 20
#define MISSILE_LAUNCH_Z 15
#define MISSILE_LAUNCH_SPEED 24
#define MAX_MISSILE_LOCK_RANGE 63*256

#define EXPLOSION_START_SIZE 18
#define EXPLOSION_GROW_RATE 4
#define EXPLOSION_PARTICLE_SIZE 2
#define EXPLOSION_SCALE 1024

extern char cmdr_name[CMDR_NAME_MAX_LENGTH];
extern unsigned char cmdr_name_length;

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
#define NUM_MENU_OPTIONS 6
extern enum currentMenu_t currentMenu;

extern bool player_dead;

extern unsigned char player_fuel;
extern unsigned int player_money;
extern unsigned char player_outlaw;
extern unsigned int player_kills;
extern unsigned char player_energy;

extern unsigned char player_cargo_space;

#endif
