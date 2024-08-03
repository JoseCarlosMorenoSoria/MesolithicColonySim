#include "Environment.hpp"

using namespace std;
vector<int> Environment::people_in_stealth;//used so that Animal knows if any person is in stealth

Environment::Tile Environment::Map[50][100]; //this somehow resolved link error of People not accessing the Map, but might have unintended consequences?
vector<Environment::sky_tile> Environment::Sky;

ItemSys it;//used to access member functions and variables of ItemSys

int range1_minx = 0;
int range1_miny = 0;
int range1_maxx = 15;
int range1_maxy = 15;

int range2_minx = 85;
int range2_miny = 0;
int range2_maxx = Environment::map_x_max;
int range2_maxy = 15;

int range3_minx = 0;
int range3_miny = 35;
int range3_maxx = 15;
int range3_maxy = Environment::map_y_max;

int range4_minx = 85;
int range4_miny = 35;
int range4_maxx = Environment::map_x_max;
int range4_maxy = Environment::map_y_max;

void Environment::add_item_to_map(string item, int x, int y) {
	if (x < 0 || y < 0 || x >= map_x_max || y >= map_y_max) {
		return;//invalid position, should make this function a bool to return false
	}

	if (item == "berrybush" || item == "grain") {
		//return;
	}

	ItemSys::Item i = it.presets[item];
	i.item_id = ItemSys::new_item_id();
	ItemSys::item_list.push_back(i);
	Map[y][x].item_id = i.item_id;
}

Environment::Environment(int hours_in_day) {
	//for testing fire spread
	//Map[25][50].has_fire = true;

	for (int y = 0; y < map_y_max; y++) {
		for (int x = 0; x < map_x_max; x++) { //z controls the percent chance of tile having food
			if (Map[y][x].terrain != "water") {
				Map[y][x].terrain = "dirt";
			}

			int zr = rand() % 100;
			if (zr < 1) {
				add_item_to_map("rock",x, y);
			}
			if (zr > 1 && zr < 3) {
				add_item_to_map("tree", x, y);
			}

			int zw = rand() % 100;
			if (zw < 1) {
				if ((x > 5 && y > 5) && (x < map_x_max - 5 && y < map_y_max - 5)) {
					Map[y][x].terrain = "water";
					Map[y+1][x].terrain = "water";
					Map[y][x+1].terrain = "water";
					Map[y+1][x+1].terrain = "water";
				}
			}

			if((y >= range1_miny && y <= range1_maxy) && (x >= range1_minx && x <= range1_maxx)) {
					int z = rand() % 100;
					if (z < 20) {
						int f = rand() % 2;
						(f == 0) ? add_item_to_map("berrybush", x, y) : add_item_to_map("grain", x, y);
					}
			}

			else if ((y >= range2_miny && y <= range2_maxy) && (x >= range2_minx && x <= range2_maxx)) {
				int z = rand() % 100;
				if (z < 20) {
					int f = rand() % 2;
					(f == 0) ? add_item_to_map("berrybush", x, y) : add_item_to_map("grain", x, y);
				}
			}

			else if ((y >= range3_miny && y <= range3_maxy) && (x >= range3_minx && x <= range3_maxx)) {
				int z = rand() % 100;
				if (z < 20) {
					int f = rand() % 2;
					(f == 0) ? add_item_to_map("berrybush", x, y) : add_item_to_map("grain", x, y);
				}
			}

			else if ((y >= range4_miny && y <= range4_maxy) && (x >= range4_minx && x <= range4_maxx)) {
				int z = rand() % 100;
				if (z < 2) {
					int f = rand() % 2;
					(f == 0) ? add_item_to_map("berrybush", x, y) : add_item_to_map("grain", x, y);
				}
			}

			else {
				
				int z = rand() % 100;
				if (z < 2) {
					int f = rand() % 2;
					(f == 0) ? add_item_to_map("berrybush", x, y) : add_item_to_map("grain", x, y);
				}
				
			}
			
		}
	}

	for (int i = 0; i < hours_in_day/2; i++) {
		sky_tile s;
		Sky.push_back(s);
	}
}

static int season = 0;
static int day_count = 0;
int days_in_season = 6;
void Environment::update(int hours_in_day, int hour_count, int day_count) {
	int noon = hours_in_day / 2;
	if (hour_count < noon) {
		Sky[noon-1].has_moon = false;
		if (hour_count - 1 > -1) {
			Sky[hour_count - 1].has_sun = false;
		}
		Sky[hour_count].has_sun = true;
	}
	if (hour_count >= noon) {
		Sky[noon-1].has_sun = false;
		if (hour_count - noon - 1 > -1) {
			Sky[hour_count - noon-1].has_moon = false;
		}
		Sky[hour_count-noon].has_moon = true;
	}

	if (hour_count == 0) {
		if (day_count % days_in_season == 0) {
			season++;
			season = season % 4;
		}
	}
	
	//rain(); needs trigger
	fire_spread(); //needs triggers
	track_manager();

	if (season == 0) {
		int spawn_chance = rand() % 100;
		if (spawn_chance < 20) {
			int x = rand() % (range1_maxx - range1_minx) + range1_minx;
			int y = rand() % (range1_maxy - range1_miny) + range1_miny;
			if (Map[y][x].item_id == -1) {
				int f = rand() % 2;
				(f == 0) ? add_item_to_map("berrybush", x, y) : add_item_to_map("grain", x, y);
			}
		}
	}
	else if (season == 1) {
		int spawn_chance = rand() % 100;
		if (spawn_chance < 20) {
			int x = rand() % (range2_maxx - range2_minx) + range2_minx;
			int y = rand() % (range2_maxy - range2_miny) + range2_miny;
			if (Map[y][x].item_id == -1) {
				int f = rand() % 2;
				(f == 0) ? add_item_to_map("berrybush", x, y) : add_item_to_map("grain", x, y);
			}
		}
	}
	else if (season == 2) {
		int spawn_chance = rand() % 100;
		if (spawn_chance < 20) {
			int x = rand() % (range3_maxx - range3_minx) + range3_minx;
			int y = rand() % (range3_maxy - range3_miny) + range3_miny;
			if (Map[y][x].item_id == -1) {
				int f = rand() % 2;
				(f == 0) ? add_item_to_map("berrybush", x, y) : add_item_to_map("grain", x, y);
			}
		}
	}
	else if (season == 3) {
		int spawn_chance = rand() % 100;
		if (spawn_chance < 20) {
			int x = rand() % (range4_maxx - range4_minx) + range4_minx;
			int y = rand() % (range4_maxy - range4_miny) + range4_miny;
			if (Map[y][x].item_id == -1) {
				int f = rand() % 2;
				(f == 0) ? add_item_to_map("berrybush", x, y) : add_item_to_map("grain",x, y);
			}
		}
	}
	/* everywhere else
	int spawn_chance = rand() % 100;
	if (spawn_chance < 50) {
		int x = rand() % map_x_max;
		int y = rand() % map_y_max;
		if ((y >= range1_miny && y <= range1_maxy) && (x >= range1_minx && x <= range1_maxx)) {}
		else if ((y >= range2_miny && y <= range2_maxy) && (x >= range2_minx && x <= range2_maxx)) {}
		else if ((y >= range3_miny && y <= range3_maxy) && (x >= range3_minx && x <= range3_maxx)) {}
		else if ((y >= range4_miny && y <= range4_maxy) && (x >= range4_minx && x <= range4_maxx)) {}
		else {
			if (Map[y][x].item_id == -1) {
				ItemSys::Item food = it.food;
				food.item_id = ItemSys::new_item_id();
				ItemSys::item_list.push_back(food);
				Map[y][x].item_id = food.item_id;
			}
		}
	}
	*/


}

void Environment::fire_spread() {//FIX THIS. this function is inefficient. Fire should also burn out after a while when it runs out of flammable material and should leave behind either burned versions of items or ash and delete the items it burned. 
	for (int y = 0; y < map_y_max; y++) {
		for (int x = 0; x < map_x_max; x++) {
			if (Map[y][x].has_fire) {
				int rx = rand() % 3 - 1;
				int ry = rand() % 3 - 1;
				int chance = rand() % 100;
				if (chance<10 && !(x < 0 || y < 0 || x >= map_x_max || y >= map_y_max)) {
					Map[y + ry][x + rx].has_fire = true;
				}
			}
		}
	}
}
//all these functions that check the whole map every tick are inefficient, need to merge them together
bool rain_flip = false;
void Environment::rain() {
	for (int y = 0; y < map_y_max; y++) {
		for (int x = 0; x < map_x_max; x++) {
			if (rain_flip) {
				if (y % 2 == 0) {
					Map[y][x].has_rain = true;
				}
				else {
					Map[y][x].has_rain = false;
				}
			}
			else {
				if (y % 2 != 0) {
					Map[y][x].has_rain = true;
				}
				else {
					Map[y][x].has_rain = false;
				}
			}
		}
	}
	rain_flip = !rain_flip;
}


void Environment::track_manager() {//need to implement: tracks should be removable by the placement of other items on it or replaced by another animal or person walking on it
	for (int y = 0; y < map_y_max; y++) {
		for (int x = 0; x < map_x_max; x++) {
			if (Map[y][x].track.track_age!=-1) {
				if (Map[y][x].terrain == "stone") {//no tracks on stone, might be more efficient to simply not place tracks in the first place, fix?
					Map[y][x].track = {};//reset
				}
				else {
					if (Map[y][x].track.track_age > 10) {//10 ticks
						Map[y][x].track = {};//reset
					}
					else {
						Map[y][x].track.track_age++;
					}
				}
			}
		}
	}
}