/*
SDLPoP, a port/conversion of the DOS game Prince of Persia.
Copyright (C) 2013-2025  Dávid Nagy

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

#ifdef USE_SCREENSHOT

char screenshots_folder[POP_MAX_PATH] = "screenshots";
char screenshot_filename[POP_MAX_PATH] = "screenshot.png";
int screenshot_index = 0;

// Use incrementing numbers and a separate folder, like DOSBox.
void make_screenshot_filename(void) {
	// Create the screenshots directory in SDLPoP's directory, even if the current directory is something else.
	snprintf_check(screenshots_folder, sizeof(screenshots_folder), "%s", locate_file("screenshots"));
	// Create the folder if it doesn't exist yet:
#if defined WIN32 || _WIN32 || WIN64 || _WIN64
	mkdir (screenshots_folder);
#else
	mkdir (screenshots_folder, 0700);
#endif
	// Find the first unused filename:
	for (;;) {
		snprintf_check(screenshot_filename, sizeof(screenshot_filename), "%s/screenshot_%03d.png", screenshots_folder, screenshot_index);
		if (! file_exists(screenshot_filename)) {
			return;
		}
		screenshot_index++;
	}
}

#define EVENT_OFFSET 0 // Add this number to displayed event numbers. Use 1 for Apoplexy compatibility.

#define NUMBER_OF_ROOMS 24

void show_result(int result, const char* what) {
	char sprintf_temp[100];
	if (result == 0) {
		printf("Saved %s to \"%s\".\n", what, screenshot_filename);
		snprintf(sprintf_temp, sizeof(sprintf_temp), "Saved %s", what);
	} else {
		printf("Could not save %s to \"%s\". Error: %s\n", what, screenshot_filename, IMG_GetError());
		snprintf(sprintf_temp, sizeof(sprintf_temp), "Could not save %s", what);
	}
	display_text_bottom(sprintf_temp);
	text_time_total = 24;
	text_time_remaining = 24;
}

// Save a screenshot.
void save_screenshot() {
	int result;
	make_screenshot_filename();
	result = IMG_SavePNG(get_final_surface(), screenshot_filename);
	show_result(result, "screenshot");
}

// Switch to the given room and draw it.
void switch_to_room(int room) {
	drawn_room = room;
	load_room_links();

	if (custom->tbl_level_type[current_level]) {
		gen_palace_wall_colors();
	}

	// for guards
	Guard.direction = dir_56_none;
	guardhp_curr = 0; // otherwise guard HPs stay on screen
	draw_guard_hp(0, 10); // otherwise guard HPs still stay on screen if some guards have extra HP
	enter_guard(); // otherwise the guard won't show up
	check_shadow(); // otherwise the shadow won't appear on level 6

	{
	int tilepos;
	for (tilepos=0;tilepos<30;tilepos++) {
		int tile_type = curr_room_tiles[tilepos] & 0x1F;
		if (tile_type == tiles_10_potion) {
			int modifier = curr_room_modif[tilepos];
			if ((modifier & 7) == 0) curr_room_modif[tilepos]++;
		}
	}
	}

	redraw_screen(1);
}

bool event_used[256] = {false};
bool has_trigger_potion = false;

// delta vectors for room links
const int dx[4] = {-1, +1,  0,  0};
const int dy[4] = { 0,  0, -1, +1};

int xpos[NUMBER_OF_ROOMS+1] = {0};
int ypos[NUMBER_OF_ROOMS+1] = {0};

// Show annotations for non-visible things, like: room bounds, room numbers, door events, loose floors, potion types, special events, ...
// (this will make the function even more like a cheat)
// TODO: fake tiles?
void draw_extras(void) {
	int tilepos;
	char room_num[6];
	rect_type text_rect;
	rect_type vline;
	rect_type hline;
	for (tilepos=0;tilepos<30;tilepos++) {
		int tile_type = curr_room_tiles[tilepos] & 0x1F;
		int modifier = curr_room_modif[tilepos];
		int row = tilepos/10;
		int col = tilepos%10;
		int y = row * 63 + 3;
		int x = col * 32;
		rect_type floor_rect;
		bool is_trob_here = false;
		int index;
		char events[256*4];
		int events_pos;
		char* special_event = NULL;
		int direction;

		floor_rect.top = y+60-3; floor_rect.left = x; floor_rect.bottom = y+63-3; floor_rect.right = x+32;

		if (tile_type == tiles_11_loose) {
			int color = color_15_brightwhite;
			if (curr_room_tiles[tilepos] & 0x20) color = color_13_brightmagenta;
			show_text_with_color(&floor_rect, halign_center, valign_top, "~~~~", color);
		}

		if (tile_type == tiles_15_opener) {
			show_text_with_color(&floor_rect, halign_center, valign_top, "^^^^", color_10_brightgreen);
		}
		if (tile_type == tiles_6_closer) {
			floor_rect.top -= 2;
			show_text_with_color(&floor_rect, halign_center, valign_top, "xxxx", color_12_brightred);
		}

		for (index = 0; index < trobs_count; ++index) {
			trob = trobs[index];
			if (trob.room == drawn_room && trob.tilepos == tilepos) {
				is_trob_here = true;
				break;
			}
		}

		if (!is_trob_here) { // It's not stuck if it's currently animated.
			// harmless spikes
			if (tile_type == tiles_2_spike) {
				if (modifier >= 5) { // harmless
					rect_type spike_rect = {y+50, x, y+60, x+32};
					show_text_with_color(&spike_rect, halign_center, valign_top, "safe", color_10_brightgreen);
				}
			}

			// stuck chompers
			if (tile_type == tiles_18_chomper) {
				int frame = (modifier & 0x7F);
				if (frame != 0) {
					rect_type chomper_rect = {y, x-10, y+60, x+32+10};
					int color = color_10_brightgreen;
					if (frame == 2) color = color_12_brightred;
					show_text_with_color(&chomper_rect, halign_center, valign_middle, "stuck", color);
				}
			}
		}

		// potion types
		if (tile_type == tiles_10_potion) {
			struct pot_type {
				int color;
				const char* text;
			} pot_types[7] = {
				{color_7_lightgray, "x"},
				{color_12_brightred, "+1"},
				{color_12_brightred, "+++"},
				{color_10_brightgreen, "slow\nfall"},
				{color_10_brightgreen, "flip"},
				{color_9_brightblue, "-1"},
				{color_9_brightblue, "trig"},
			};
			int potion_type = modifier >> 3;
			int color;
			const char* text;
			char temp_text[4];
			rect_type pot_rect;
			if (potion_type >= 0 && potion_type < 7) {
				color = pot_types[potion_type].color;
				text = pot_types[potion_type].text;
			} else {
				color = color_15_brightwhite;
				snprintf(temp_text, sizeof(temp_text), "%d", potion_type);
				text = temp_text;
			}
			pot_rect.top = y+40; pot_rect.left = x; pot_rect.bottom = y+60; pot_rect.right = x+32;
			show_text_with_color(&pot_rect, halign_center, valign_top, text, color);
		}

		// triggered door events
		if (tile_type == tiles_6_closer || tile_type == tiles_15_opener
			// These tiles are triggered even if they are not buttons!
			/*
			|| (current_level == 1 && drawn_room == 5 && tilepos == 2) // triggered at start
			|| (current_level == 13 && drawn_room == 24 && tilepos == 0) // triggered when player enters any room from the right after Jaffar died
			*/
			|| (has_trigger_potion && drawn_room == 8 && tilepos == 0) // triggered when player drinks an open potion
		) {
			int first_event = modifier;
			int last_event = modifier;
			char btn_events[256*4];
			int btn_events_pos = 0;
			int event;
			rect_type buttonmod_rect;
			while (last_event<256 && get_doorlink_next(last_event)) last_event++;
			btn_events[0] = '\0';
			for (event=first_event; event<=last_event && btn_events_pos<(int)sizeof(btn_events); event++) {
				int len = snprintf(btn_events+btn_events_pos, sizeof(btn_events)-btn_events_pos, "%d ", event+EVENT_OFFSET);
				if (len < 0) break;
				btn_events_pos += len;
			}
			--btn_events_pos;
			if (btn_events_pos>0 && btn_events_pos<(int)sizeof(btn_events)) btn_events[btn_events_pos]='\0';
			buttonmod_rect.top = y; buttonmod_rect.left = x; buttonmod_rect.bottom = y+60-3; buttonmod_rect.right = x+32;
			show_text_with_color(&buttonmod_rect, halign_center, valign_bottom, btn_events, color_14_brightyellow);
		}

		events[0] = '\0';
		events_pos = 0;
		{
		int event;
		for (event=0; event<256 && events_pos<(int)sizeof(events); event++) {
			if (event_used[event] && get_doorlink_room(event) == drawn_room && get_doorlink_tile(event) == tilepos) {
				int len = snprintf(events+events_pos, (int)sizeof(events)-events_pos, "%d ", event+EVENT_OFFSET);
				if (len < 0) break;
				events_pos += len;
			}
		}
		}
		--events_pos;
		if (events_pos>0 && events_pos<(int)sizeof(events)) events[events_pos]='\0';
		if (events[0] != '\0') {
			rect_type events_rect;
			events_rect.top = y; events_rect.left = x; events_rect.bottom = y+63-3; events_rect.right = x+32-7;
			show_text_with_color(&events_rect, halign_center, valign_bottom, events, color_14_brightyellow);
		}

#ifdef USE_TELEPORTS
		if (tile_type == tiles_23_balcony_left && modifier != 0) {
			char number[4];
			rect_type number_rect;
			snprintf(events, sizeof(number), "%d", modifier);
			number_rect.top = y; number_rect.left = x+32; number_rect.bottom = y+63; number_rect.right = x+64;
			show_text_with_color(&number_rect, halign_center, valign_top, events, color_14_brightyellow);
		}
#endif

		special_event = NULL;

		if (current_level == 0 && drawn_room == /*24*/ custom->demo_end_room) {
			special_event = "exit"; // exit by entering this room
		}

		// not marked: level 1 falling entry

		if (current_level == 1 && drawn_room == 5 && tilepos == 2) {
			special_event = "start\ntrig"; // triggered at start
		}

		if (current_level == 3 && drawn_room == 7 && col == 0) {
			special_event = "<-\nchk point"; // checkpoint activation
		}

		if (current_level == /*3*/ custom->checkpoint_level &&
			drawn_room == /*7*/ custom->checkpoint_clear_tile_room &&
			tilepos == /*4*/ custom->checkpoint_clear_tile_col * 10 + custom->checkpoint_clear_tile_row
		) {
			special_event = "removed"; // this loose floor is removed when restarting at the checkpoint
		}

		if (current_level == 3 && drawn_room == 2 && tile_type == tiles_4_gate) {
			special_event = "loud"; // closing can be heard everywhere
		}

		if (current_level == /*3*/ custom->checkpoint_level &&
			drawn_room == /*2*/ custom->checkpoint_respawn_room &&
			tilepos == /*6*/ custom->checkpoint_respawn_tilepos
		) {
			special_event = "check point"; // restart at checkpoint
			// TODO: Show this room (and connected rooms) even if it is unreachable from the start via room links?
		}

		if (current_level == /*3*/ custom->skeleton_level &&
			drawn_room == /*1*/ custom->skeleton_room &&
			tilepos == /*15*/ custom->skeleton_row * 10 + custom->skeleton_column &&
			tile_type == tiles_21_skeleton
		) {
			special_event = "skel wake"; // skeleton wakes
		}

		if (current_level == /*3*/ custom->skeleton_level &&
			drawn_room == /*3*/ custom->skeleton_reappear_room &&
			tilepos == /*14*/ custom->skeleton_reappear_row * 10 + (custom->skeleton_reappear_x - 58) / 14
		) {
			special_event = "skel cont"; // skeleton continues here if it falls into this room
		}

		if (current_level == /*4*/ custom->mirror_level &&
			drawn_room == /*4*/ custom->mirror_room &&
			tilepos == /*4*/ custom->mirror_row * 10 + custom->mirror_column
		) {
			special_event = "mirror"; // mirror appears
		}

		// not marked: level 4 mirror clip

		// not marked: level 5 shadow, required opening gate

		if (current_level == /*5*/ custom->shadow_steal_level &&
			drawn_room == /*24*/ custom->shadow_steal_room &&
			tilepos == 3 &&
			tile_type == tiles_10_potion
		) {
			special_event = "stolen"; // stolen potion
		}

		// not marked: level 6 shadow (it's already visible)

		if (current_level == /*6*/ custom->falling_exit_level &&
			drawn_room == /*1*/ custom->falling_exit_room &&
			row == 2
		) {
			special_event = "exit\ndown"; // exit by falling
		}

		// not marked: level 7 falling entry

		if (current_level == /*8*/ custom->mouse_level &&
			drawn_room == /*16*/ custom->mouse_room &&
			tilepos == 9 // top right corner
		) {
			special_event = "mouse"; // mouse comes
		}

		if (current_level == 12 && drawn_room == 15 && tilepos == 1 && tile_type == tiles_22_sword) {
			special_event = "disapp"; // the sword disappears from here
		}

		if (current_level == 12 && drawn_room == 18 && col == 9) {
			special_event = "disapp\n->"; // the sword disappears if you exit this room
		}

		// not marked: level 12 shadow

		if (current_level == 12 && row == 0 && (drawn_room == 2 || (drawn_room == 13 && col >= 6))) {
			special_event = "floor"; // floors appear
		}

		if (/*current_level == 12 &&*/ drawn_room == /*23*/ custom->tbl_seamless_exit[current_level]) {
			special_event = "exit"; // exit by entering this room
		}

		if (current_level == /*13*/ custom->loose_tiles_level &&
			(drawn_room == level.roomlinks[/*23*/ custom->loose_tiles_room_1 - 1].up ||
				drawn_room == level.roomlinks[/*16*/ custom->loose_tiles_room_2 - 1].up) &&
			(tilepos >= /*22*/ custom->loose_tiles_first_tile && tilepos <= /*27*/ custom->loose_tiles_last_tile)
		) {
			special_event = "fall"; // falling loose floors
		}

		if (current_level == 13 && drawn_room == 3 && col == 9) {
			special_event = "meet\n->"; // meet Jaffar
		}

		// not marked: flash

		if (current_level == 13 && drawn_room == 24 && tilepos == 0) {
			special_event = "Jffr\ntrig"; // triggered when player enters any room from the right after Jaffar died
		}

		if (current_level == /*14*/ custom->win_level && drawn_room == /*5*/ custom->win_room) {
			special_event = "end"; // end of game
		}

		if (has_trigger_potion && drawn_room == 8 && tilepos == 0) {
			special_event = "blue\ntrig"; // triggered when player drinks an open potion
		}

		if (special_event) {
			rect_type event_rect;
			event_rect.top = y; event_rect.left = x-10; event_rect.bottom = y+63; event_rect.right = x+32+10;
			show_text_with_color(&event_rect, halign_center, valign_middle, special_event, color_14_brightyellow);
		}

		{
		byte* roomlinks = (byte*)(&level.roomlinks[drawn_room-1]);
		for (direction = 0; direction < 4; direction++) {
			int other_room = roomlinks[direction];
			if (other_room >= 1 && other_room <= NUMBER_OF_ROOMS) {
				int other_x = xpos[drawn_room] + dx[direction];
				int other_y = ypos[drawn_room] + dy[direction];
				if (xpos[other_room] != other_x || ypos[other_room] != other_y) {
					int center_x = 160+dx[direction]*150;
					int center_y = 96+dy[direction]*85;
					rect_type text_rect;
					char room_num[4];
					text_rect.top = center_y-6; text_rect.left = center_x-10; text_rect.bottom = center_y+6; text_rect.right = center_x+10;
					snprintf(room_num, sizeof(room_num), "%d", other_room);
					method_5_rect(&text_rect, 0, color_4_red);
					show_text_with_color(&text_rect, halign_center, valign_middle, room_num, color_15_brightwhite);
				}
			}
		}
		}

		if (level.start_room == drawn_room && level.start_pos == tilepos) {
			byte start_dir = level.start_dir;
			char* start_text;
			rect_type start_rect;
			if (current_level == 1 || current_level == 13) start_dir ^= 0xFF;
			start_text = (start_dir == dir_0_right) ? "start\n->" : "start\n<-";
			start_rect.top = y; start_rect.left = x-10; start_rect.bottom = y+63; start_rect.right = x+32+10;
			show_text_with_color(&start_rect, halign_center, valign_middle, start_text, color_14_brightyellow);
		}

		if (Guard.direction != dir_56_none && tilepos == Guard.curr_row * 10 + Guard.curr_col) {
			int screen_x;
			rect_type event_rect;
			char guard_info[20];
			loadshad();
			load_frame_to_obj();
			screen_x = calc_screen_x_coord(obj_x);
			if (Guard.direction == dir_0_right) screen_x -= 10; else screen_x += 10;

			event_rect.top = y+2; event_rect.left = screen_x-16-10; event_rect.bottom = y+63; event_rect.right = screen_x+16+10;
			snprintf(guard_info, sizeof(guard_info), "s%d h%d", guard_skill, guardhp_max);
			show_text_with_color(&event_rect, halign_center, valign_top, guard_info, color_14_brightyellow);
		}

	}

	snprintf(room_num, sizeof(room_num), "%d", drawn_room);
	text_rect.top = 10; text_rect.left = 10; text_rect.bottom = 21; text_rect.right = 30;
	method_5_rect(&text_rect, 0, color_8_darkgray);
	show_text_with_color(&text_rect, halign_center, valign_middle, room_num, color_15_brightwhite);

	vline.top = 0; vline.left = 0; vline.bottom = 192; vline.right = 1;
	method_5_rect(&vline, 0, color_12_brightred);
	hline.top = 3; hline.left = 0; hline.bottom = 4; hline.right = 320;
	method_5_rect(&hline, 0, color_12_brightred);
}

// Save a "screenshot" of the whole level.
void save_level_screenshot(bool want_extras) {
	#define MAX_MAP_SIZE NUMBER_OF_ROOMS
	bool processed[NUMBER_OF_ROOMS+1];
	int queue[NUMBER_OF_ROOMS];
	int queue_start = 0;
	int queue_end = 1;
	int min_x=0, max_x=0, min_y=0, max_y=0;
	int clash_y, clash_x;
	int map[MAX_MAP_SIZE][MAX_MAP_SIZE];
	int map_width, map_height;
	int image_width, image_height;
	SDL_Surface* map_surface;
	int old_room;
	int result;
	int room, direction, tilepos, x, y;

	if (!cheats_enabled) return;

	upside_down = 0;

	memset(processed, 0, sizeof(processed));
	for (room=1;room<=NUMBER_OF_ROOMS;room++) {
		xpos[room] = 0;
		ypos[room] = 0;
	}
	xpos[drawn_room] = 0;
	ypos[drawn_room] = 0;
	processed[drawn_room] = true;
	queue[0] = drawn_room;

	while (queue_start < queue_end) {
		int rm = queue[queue_start];
		byte* roomlinks;
		queue_start++;
		roomlinks = (byte*)(&level.roomlinks[rm-1]);
		for (direction = 0; direction < 4; direction++) {
			int other_room = roomlinks[direction];
			if (other_room >= 1 && other_room <= NUMBER_OF_ROOMS && !processed[other_room]) {
				int other_x = xpos[rm] + dx[direction];
				int other_y = ypos[rm] + dy[direction];
				xpos[other_room] = other_x;
				ypos[other_room] = other_y;
				processed[other_room] = true;
				printf("Adding room %d to map.\n", other_room);
				if (queue_end >= NUMBER_OF_ROOMS) { printf("Queue overflow!\n"); break; }
				queue[queue_end] = other_room;
				queue_end++;
			}
		}
	}

	for (room=1;room<=NUMBER_OF_ROOMS;room++) {
		if (xpos[room] < min_x) min_x = xpos[room];
		if (xpos[room] > max_x) max_x = xpos[room];
		if (ypos[room] < min_y) min_y = ypos[room];
		if (ypos[room] > max_y) max_y = ypos[room];
	}

	clash_y = max_y + 1;
	clash_x = min_x;

	memset(map, 0, sizeof(map));
	for (room=1;room<=NUMBER_OF_ROOMS;room++) {
		if (processed[room]) {
			again:
			y = ypos[room] - min_y;
			x = xpos[room] - min_x;
			if (x>=0 && y>=0 && x<MAX_MAP_SIZE && y<MAX_MAP_SIZE) {
				if (map[y][x]) {
					printf("Warning: room %d was mapped to the same place as room %d!\n", room, map[y][x]);
					xpos[room] = clash_x;
					ypos[room] = clash_y;
					clash_x++;
					if (xpos[room] > max_x) max_x = xpos[room];
					if (ypos[room] > max_y) max_y = ypos[room];
					goto again;
				}
				map[y][x] = room;
			} else {
				printf("Warning: room %d was mapped outside the map: x = %d, y = %d.\n", room, x, y);
			}
		}
	}

	map_width = max_x-min_x+1;
	map_height = max_y-min_y+1;

	image_width = map_width*320;
	image_height = map_height*189+3+8;

	map_surface = SDL_CreateRGBSurface(0, image_width, image_height, 32, Rmsk, Gmsk, Bmsk, Amsk);
	if (map_surface == NULL) {
		sdlperror("SDL_CreateRGBSurface (map_surface)");
		return;
	}

	has_trigger_potion = false;

	for (room=1;room<=NUMBER_OF_ROOMS;room++) {
		if (processed[room]) {
			get_room_address(room);
			for (tilepos=0;tilepos<30;tilepos++) {
				int tile_type = curr_room_tiles[tilepos] & 0x1F;
				if (tile_type == tiles_10_potion && curr_room_modif[tilepos] >> 3 == 6) {
					has_trigger_potion = true;
				}
			}
		}
	}

	memset(event_used, 0, sizeof(event_used));

	for (room=1;room<=NUMBER_OF_ROOMS;room++) {
		if (processed[room]) {
			get_room_address(room);
			for (tilepos=0;tilepos<30;tilepos++) {
				int tile_type = curr_room_tiles[tilepos] & 0x1F;
				if (tile_type == tiles_6_closer || tile_type == tiles_15_opener
					|| (has_trigger_potion && room == 8 && tilepos == 0)
				) {
					int modifier = curr_room_modif[tilepos];
					int index;
					for (index = modifier; index < 256; index++) {
						event_used[index] = true;
						if (!get_doorlink_next(index)) break;
					}
				}
			}
		}
	}

	old_room = drawn_room;
	for (y=0;y<map_height;y++) {
		for (x=0;x<map_width;x++) {
			int rm = map[y][x];
			if (rm) {
				SDL_Rect dest_rect;
				dest_rect.x = x*320;
				dest_rect.y = y*189;
				switch_to_room(rm);

				if (want_extras) draw_extras();

				SDL_BlitSurface(onscreen_surface_, NULL, map_surface, &dest_rect);
			}
		}
	}
	switch_to_room(old_room);

	make_screenshot_filename();
	result = IMG_SavePNG(map_surface, screenshot_filename);
	show_result(result, "level map");

	SDL_FreeSurface(map_surface);
}

bool want_auto = false;
bool want_auto_whole_level = false;
bool want_auto_extras = false;

void init_screenshot() {
	// Command-line options to automatically save a screenshot at startup.
	const char* screenshot_param = check_param("--screenshot");
	if (screenshot_param != NULL) {
		// We require megahit+levelnumber.
		if (start_level < 0) {
			printf("You must supply a level number if you want to make an automatic screenshot!\n");
			exit(1);
		} else {
			want_auto = true;
			want_auto_whole_level = (check_param("--screenshot-level") != NULL);
			want_auto_extras = (check_param("--screenshot-level-extras") != NULL);
		}
	}
}

// TODO: Don't open a window if the user wants an auto screenshot.

// To skip cutscenes, etc.
bool want_auto_screenshot() {
	return want_auto;
}

// Called when the level is drawn for the first time.
void auto_screenshot() {
	if (!want_auto) return;

	if (want_auto_whole_level) {
		save_level_screenshot(want_auto_extras);
	} else {
		save_screenshot();
	}

	quit(1);
}

#endif

