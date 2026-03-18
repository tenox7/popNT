/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2025  David Nagy

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

The authors of this program may be contacted at https://forum.princed.org
*/

#include "common.h"

word text_time_remaining;
word text_time_total;
word is_show_time;
word checkpoint;
word upside_down;
word resurrect_time;
word dont_reset_time;
short rem_min;
word rem_tick;
word hitp_beg_lev;
word need_level1_music;
surface_type* offscreen_surface;

byte sound_flags = 0;
const rect_type screen_rect = {0, 0, 200, 320};
word draw_mode;
short start_level = -1;
byte * guard_palettes;
chtab_type *chtab_addrs[10];

#ifdef USE_COPYPROT
word copyprot_plac;
word copyprot_idx;
const char copyprot_letter[] = {'A','A','B','B','C','C','D','D','E','F','F','G','H','H','I','I','J','J','K','L','L','M','M','N','O','O','P','P','R','R','S','S','T','T','U','U','V','Y','W','Y'};
word cplevel_entr[14];
#endif

dialog_type* copyprot_dialog;
dialog_settings_type dialog_settings = {
	add_dialog_rect,
	dialog_method_2_frame,
	4, 4, 4, 4, 3, 4, 1
};
rect_type dialog_rect_1 = {60, 56, 124, 264};
rect_type dialog_rect_2 = {61, 56, 120, 264};

word drawn_room;

byte curr_tile;
byte curr_modifier;

tile_and_mod leftroom_[3];
tile_and_mod row_below_left_[10];
const word tbl_line[] = {0, 10, 20};

word loaded_room;
byte* curr_room_tiles;
byte* curr_room_modif;
word draw_xh;
word current_level = -1;
byte graphics_mode = 0;

word room_L;
word room_R;
word room_A;
word room_B;
word room_BR;
word room_BL;
word room_AR;
word room_AL;

level_type level;

#ifdef USE_COLORED_TORCHES
byte torch_colors[24+1][30];
#endif

short table_counts[5];
short drects_count;
short peels_count;

back_table_type foretable[200];
back_table_type backtable[200];
midtable_type midtable[50];
peel_type* peels_table[50];
rect_type drects[30];

sbyte obj_direction;
const byte chtab_flip_clip[10] = {1,0,1,1,1,1,0,0,0,0};
short obj_clip_left;
short obj_clip_top;
short obj_clip_right;
short obj_clip_bottom;
wipetable_type wipetable[300];
const byte chtab_shift[10] = {0,1,0,0,0,0,1,1,1,0};
word need_drects;
word is_blind_mode;

const rect_type rect_top = {0, 0, 192, 320};
const rect_type rect_bottom_text = {193, 70, 202, 250};

word leveldoor_right;
word leveldoor_ybottom;

byte palace_wall_colors[44*3];

word seed_was_init = 0;
dword random_seed;

surface_type* current_target_surface = NULL;

byte* doorlink2_ad;
byte* doorlink1_ad;

sbyte control_shift;
sbyte control_y;
sbyte control_x;

#ifdef USE_FADE
word is_global_fading;
palette_fade_type* fade_palette_buffer;
#endif

char_type Kid;
word is_keyboard_mode = 0;
word is_paused;
word is_restart_level;
byte sound_mode = 0;
word is_joyst_mode;
byte is_sound_on = 0x0F;
word next_level;
short guardhp_delta;
word guardhp_curr;
word next_room;
word hitp_curr;
word hitp_max;
short hitp_delta;
word flash_color;
word flash_time;
char_type Guard;

word need_quotes;
short roomleave_result;
word different_room;
sound_buffer_type* sound_pointers[58];
word guardhp_max;
word is_feather_fall;
chtab_type* chtab_title40;
chtab_type* chtab_title50;
short hof_count;

#ifdef USE_SUPER_HIGH_JUMP
byte super_jump_timer = 0;
byte super_jump_fall = 0;
byte super_jump_room;
sbyte super_jump_col;
sbyte super_jump_row;
#endif

word demo_mode = 0;

word is_cutscene;
bool is_ending_sequence;

cutscene_ptr_type tbl_cutscenes[16] = {
	NULL,
	NULL,
	cutscene_2_6,
	NULL,
	cutscene_4,
	NULL,
	cutscene_2_6,
	NULL,
	cutscene_8,
	cutscene_9,
	NULL,
	NULL,
	cutscene_12,
	NULL,
	NULL,
	NULL,
};

short mobs_count;
short trobs_count;
short next_sound;
word grab_timer;
short can_guard_see_kid;
word holding_sword;
short united_with_shadow;
word leveldoor_open;
word demo_index;
short demo_time;
word have_sword;

char_type Char;
char_type Opp;

short knock;
word is_guard_notice;

byte wipe_frames[30];
sbyte wipe_heights[30];
byte redraw_frames_anim[30];
byte redraw_frames2[30];
byte redraw_frames_floor_overlay[30];
byte redraw_frames_full[30];
byte redraw_frames_fore[30];
byte tile_object_redraw[30];
byte redraw_frames_above[10];
word need_full_redraw;
short n_curr_objs;
objtable_type objtable[50];
short curr_objs[50];

byte obj_xh;
byte obj_xl;
byte obj_y;
byte obj_chtab;
byte obj_id;
byte obj_tilepos;
short obj_x;

frame_type cur_frame;
word seamless;
trob_type trob;
trob_type trobs[TROBS_MAX];
short redraw_height;

/*const*/ byte sound_interruptible[] = {
	0,
	1,
	1,
	1,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	0,
	0,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

byte curr_tilepos;
short curr_room;
mob_type curmob;
mob_type mobs[14];
short tile_col;

const short y_land[] = {-8, 55, 118, 181, 244};

word curr_guard_color;
byte key_states[SDL_NUM_SCANCODES];

const byte x_bump[] = {-12, 2, 16, 30, 44, 58, 72, 86, 100, 114, 128, 142, 156, 170, 184, 198, 212, 226, 240, 254};

word is_screaming;
word offguard;
word droppedout;

#ifdef USE_COPYPROT
/*const*/ word copyprot_room[] = {3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4};
const word copyprot_tile[] = {1,  5,  7,  9, 11, 21,  1,  3,  7, 11, 17, 21, 25, 27};
#endif

word exit_room_timer;
short char_col_right;
short char_col_left;
short char_top_row;
short prev_char_top_row;
short prev_char_col_right;
short prev_char_col_left;
short char_bottom_row;
short guard_notice_timer;
short jumped_through_mirror;
const short y_clip[] = {-60, 3, 66, 129, 192};
byte curr_tile2;
short tile_row;

word char_width_half;
word char_height;
short char_x_left;
short char_x_left_coll;
short char_x_right_coll;
short char_x_right;
short char_top_y;
byte fall_frame;
byte through_tile;
sbyte infrontx;
const sbyte dir_front[] = {-1, 1};
const sbyte dir_behind[] = {1, -1};
word current_sound;
sbyte control_shift2;
sbyte control_forward;
word guard_skill;
sbyte control_backward;
sbyte control_up;
sbyte control_down;
sbyte ctrl1_forward;
sbyte ctrl1_backward;
sbyte ctrl1_up;
sbyte ctrl1_down;
sbyte ctrl1_shift2;

word shadow_initialized;

word guard_refrac;
word kid_sword_strike;

byte edge_type;

SDL_Surface* onscreen_surface_;
SDL_Surface* overlay_surface;
SDL_Surface* merged_surface;
SDL_Renderer* renderer_;
bool is_renderer_targettexture_supported;
SDL_Window* window_;
bool is_overlay_displayed;
SDL_Texture* texture_sharp;
SDL_Texture* texture_fuzzy;
SDL_Texture* texture_blurry;
SDL_Texture* target_texture;

SDL_GameController* sdl_controller_ = 0;
SDL_Joystick* sdl_joystick_;
byte using_sdl_joystick_interface;
int joy_axis[JOY_AXIS_NUM];
int joy_axis_max[JOY_AXIS_NUM];
int joy_left_stick_states[2];
int joy_right_stick_states[2];
int joy_button_states[JOYINPUT_NUM];
SDL_Haptic* sdl_haptic;

Uint64 perf_counters_per_tick;
Uint64 perf_frequency;
float milliseconds_per_counter;

char** sound_names;

int g_argc;
char** g_argv;

sbyte collision_row;
sbyte prev_collision_row;

sbyte prev_coll_room[10];
sbyte curr_row_coll_room[10];
sbyte below_row_coll_room[10];
sbyte above_row_coll_room[10];
byte curr_row_coll_flags[10];
byte above_row_coll_flags[10];
byte below_row_coll_flags[10];
byte prev_coll_flags[10];

short pickup_obj_type;

word justblocked;

word last_loose_sound;

int last_key_scancode;
int last_any_key_scancode;

#ifdef USE_TEXT
font_type hc_font = {0x01,0xFF, 7,2,1,1, NULL};
textstate_type textstate = {0,0,0,15,&hc_font};
#endif

int need_quick_save = 0;
int need_quick_load = 0;

#ifdef USE_REPLAY
byte recording = 0;
byte replaying = 0;
dword num_replay_ticks = 0;
byte need_start_replay = 0;
byte need_replay_cycle = 0;
char replays_folder[POP_MAX_PATH] = "replays";
byte special_move;
dword saved_random_seed;
dword preserved_seed;
sbyte keep_last_seed;
byte skipping_replay;
byte replay_seek_target;
byte is_validate_mode;
dword curr_tick = 0;
#endif

#ifdef __PSP__
byte start_fullscreen = 1;
word pop_window_width = 480;
word pop_window_height = 272;
#else
byte start_fullscreen = 0;
word pop_window_width = 640;
word pop_window_height = 400;
#endif

byte use_custom_levelset = 0;
char levelset_name[POP_MAX_PATH];
char mod_data_path[POP_MAX_PATH];
bool skip_mod_data_files;
bool skip_normal_data_files;

byte use_fixes_and_enhancements = 0;
byte enable_copyprot = 0;
byte enable_music = 1;
byte enable_fade = 1;
byte enable_flash = 1;
byte enable_text = 1;
byte enable_info_screen = 1;
byte enable_controller_rumble = 0;
byte joystick_only_horizontal = 0;
int joystick_threshold = 8000;
char gamecontrollerdb_file[POP_MAX_PATH] = "";
byte enable_quicksave = 1;
byte enable_quicksave_penalty = 1;
byte enable_replay = 1;

#ifdef __PSP__
byte use_hardware_acceleration = 1;
#else
byte use_hardware_acceleration = 2;
#endif

byte use_correct_aspect_ratio = 0;
byte use_integer_scaling = 0;
byte scaling_type = 0;

#ifdef USE_LIGHTING
byte enable_lighting = 0;
image_type* lighting_mask;
#endif

fixes_options_type fixes_saved;
fixes_options_type fixes_disabled_state;
fixes_options_type* fixes = &fixes_disabled_state;
byte use_custom_options;
custom_options_type custom_saved;
custom_options_type custom_defaults = {
	/* start_minutes_left */ 60,
	/* start_ticks_left */ 719,
	/* start_hitp */ 3,
	/* max_hitp_allowed */ 10,
	/* saving_allowed_first_level */ 3,
	/* saving_allowed_last_level */ 13,
	/* start_upside_down */ 0,
	/* start_in_blind_mode */ 0,
	/* copyprot_level */ 2,
	/* drawn_tile_top_level_edge */ tiles_1_floor,
	/* drawn_tile_left_level_edge */ tiles_20_wall,
	/* level_edge_hit_tile */ tiles_20_wall,
	/* allow_triggering_any_tile */ 0,
	/* enable_wda_in_palace */ 0,
	/* vga_palette */ VGA_PALETTE_DEFAULT,
	/* first_level */ 1,
	/* skip_title */ 0,
	/* shift_L_allowed_until_level */ 4,
	/* shift_L_reduced_minutes */ 15,
	/* shift_L_reduced_ticks */ 719,
	/* demo_hitp */ 4,
	/* demo_end_room */ 24,
	/* intro_music_level */ 1,
	/* have_sword_from_level */ 2,
	/* checkpoint_level */ 3,
	/* checkpoint_respawn_dir */ dir_FF_left,
	/* checkpoint_respawn_room */ 2,
	/* checkpoint_respawn_tilepos */ 6,
	/* checkpoint_clear_tile_room */ 7,
	/* checkpoint_clear_tile_col */ 4,
	/* checkpoint_clear_tile_row */ 0,
	/* skeleton_level */ 3,
	/* skeleton_room */ 1,
	/* skeleton_trigger_column_1 */ 2,
	/* skeleton_trigger_column_2 */ 3,
	/* skeleton_column */ 5,
	/* skeleton_row */ 1,
	/* skeleton_require_open_level_door */ 1,
	/* skeleton_skill */ 2,
	/* skeleton_reappear_room */ 3,
	/* skeleton_reappear_x */ 133,
	/* skeleton_reappear_row */ 1,
	/* skeleton_reappear_dir */ dir_0_right,
	/* mirror_level */ 4,
	/* mirror_room */ 4,
	/* mirror_column */ 4,
	/* mirror_row */ 0,
	/* mirror_tile */ tiles_13_mirror,
	/* show_mirror_image */ 1,
	/* shadow_steal_level */ 5,
	/* shadow_steal_room */ 24,
	/* shadow_step_level */ 6,
	/* shadow_step_room */ 1,
	/* falling_exit_level */ 6,
	/* falling_exit_room */ 1,
	/* falling_entry_level */ 7,
	/* falling_entry_room */ 17,
	/* mouse_level */ 8,
	/* mouse_room */ 16,
	/* mouse_delay */ 150,
	/* mouse_object */ 24,
	/* mouse_start_x */ 200,
	/* loose_tiles_level */ 13,
	/* loose_tiles_room_1 */ 23,
	/* loose_tiles_room_2 */ 16,
	/* loose_tiles_first_tile */ 22,
	/* loose_tiles_last_tile */ 27,
	/* jaffar_victory_level */ 13,
	/* jaffar_victory_flash_time */ 18,
	/* hide_level_number_from_level */ 14,
	/* level_13_level_number */ 12,
	/* victory_stops_time_level */ 13,
	/* win_level */ 14,
	/* win_room */ 5,
	/* loose_floor_delay */ 11,
	/* tbl_level_type */ {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0},
	/* tbl_level_color */ {0, 0, 0, 1, 0, 0, 0, 1, 2, 2, 0, 0, 3, 3, 4, 0},
	/* tbl_guard_type */ {0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 4, 3, -1, -1},
	/* tbl_guard_hp */ {4, 3, 3, 3, 3, 4, 5, 4, 4, 5, 5, 5, 4, 6, 0, 0},
	/* tbl_cutscenes_by_index */ {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	/* tbl_entry_pose */ {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
	/* tbl_seamless_exit */ {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 23, -1, -1, -1},
	/* strikeprob */    { 61,100, 61, 61, 61, 40,100,220,  0, 48, 32, 48},
	/* restrikeprob */  {  0,  0,  0,  5,  5,175, 16,  8,  0,255,255,150},
	/* blockprob */     {  0,150,150,200,200,255,200,250,  0,255,255,255},
	/* impblockprob */  {  0, 61, 61,100,100,145,100,250,  0,145,255,175},
	/* advprob */       {255,200,200,200,255,255,200,  0,  0,255,100,100},
	/* refractimer */   { 16, 16, 16, 16,  8,  8,  8,  8,  0,  8,  0,  0},
	/* extrastrength */ {  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0},
	/* init_shad_6 */  {0x0F, 0x51, 0x76, 0, 0, 1, 0, 0},
	/* init_shad_5 */  {0x0F, 0x37, 0x37, 0, 0xFF, 0, 0, 0},
	/* init_shad_12 */ {0x0F, 0x51, 0xE8, 0, 0, 0, 0, 0},
	/* demo_moves */ {{0x00, 0}, {0x01, 1}, {0x0D, 0}, {0x1E, 1}, {0x25, 5}, {0x2F, 0}, {0x30, 1}, {0x41, 0}, {0x49, 2}, {0x4B, 0}, {0x63, 2}, {0x64, 0}, {0x73, 5}, {0x80, 6}, {0x88, 3}, {0x9D, 7}, {0x9E, 0}, {0x9F, 1}, {0xAB, 4}, {0xB1, 0}, {0xB2, 1}, {0xBC, 0}, {0xC1, 1}, {0xCD, 0}, {0xE9,-1}},
	/* shad_drink_move */ {{0x00, 0}, {0x01, 1}, {0x0E, 0}, {0x12, 6}, {0x1D, 7}, {0x2D, 2}, {0x31, 1}, {0xFF,-2}},
	/* base_speed */ 5,
	/* fight_speed */ 6,
	/* chomper_speed */ 15,
	/* no_mouse_in_ending */ 0,
};
custom_options_type* custom = &custom_defaults;

full_image_type full_image[MAX_FULL_IMAGES] = {
	/* [TITLE_MAIN]     */ { 0, &chtab_title50, blitters_0_no_transp, 0, 0 },
	/* [TITLE_PRESENTS] */ { 1, &chtab_title50, blitters_0_no_transp, 96, 106 },
	/* [TITLE_GAME]     */ { 2, &chtab_title50, blitters_0_no_transp, 96, 122 },
	/* [TITLE_POP]      */ { 3, &chtab_title50, blitters_10h_transp, 24, 107 },
	/* [TITLE_MECHNER]  */ { 4, &chtab_title50, blitters_0_no_transp, 48, 184 },
	/* [HOF_POP]        */ { 3, &chtab_title50, blitters_10h_transp, 24, 24 },
	/* [STORY_FRAME]    */ { 0, &chtab_title40, blitters_0_no_transp, 0, 0 },
	/* [STORY_ABSENCE]  */ { 1, &chtab_title40, blitters_white, 24, 25 },
	/* [STORY_MARRY]    */ { 2, &chtab_title40, blitters_white, 24, 25 },
	/* [STORY_HAIL]     */ { 3, &chtab_title40, blitters_white, 24, 25 },
	/* [STORY_CREDITS]  */ { 4, &chtab_title40, blitters_white, 24, 26 },
};

word cheats_enabled = 0;
#ifdef USE_DEBUG_CHEATS
byte debug_cheats_enabled = 0;
byte is_timer_displayed = 0;
byte is_feather_timer_displayed = 0;
#endif

#ifdef USE_MENU
font_type hc_small_font = {32, 126, 5, 2, 1, 1, NULL};
bool have_mouse_input;
bool have_keyboard_or_controller_input;
int mouse_x, mouse_y;
bool mouse_moved;
bool mouse_clicked;
bool mouse_button_clicked_right;
bool pressed_enter;
bool escape_key_suppressed;
int menu_control_scroll_y;
sbyte is_menu_shown;
byte enable_pause_menu = 1;
#endif

char mods_folder[POP_MAX_PATH] = "mods";

int play_demo_level;

#ifdef USE_REPLAY
int g_deprecation_number;
#endif

byte always_use_original_music;
byte always_use_original_graphics;

int key_left       = SDL_SCANCODE_LEFT;
int key_right      = SDL_SCANCODE_RIGHT;
int key_up         = SDL_SCANCODE_UP;
int key_down       = SDL_SCANCODE_DOWN;
int key_jump_left  = SDL_SCANCODE_HOME;
int key_jump_right = SDL_SCANCODE_PAGEUP;
int key_action     = SDL_SCANCODE_RSHIFT;
int key_enter      = SDL_SCANCODE_RETURN;
int key_esc        = SDL_SCANCODE_ESCAPE;

#ifdef FIX_ONE_HP_STOPS_BLINKING
bool global_blink_state = false;
#endif
