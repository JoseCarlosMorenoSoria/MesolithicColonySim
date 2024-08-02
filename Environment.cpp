#include "Environment.hpp"

using namespace std;

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

void Environment::add_berrybush(int x, int y) {
	ItemSys::Item food = it.presets["berrybush"];
	food.item_id = ItemSys::new_item_id();
	ItemSys::item_list.push_back(food);
	Map[y][x].item_id = food.item_id;
}
void Environment::add_grain(int x, int y) {
	ItemSys::Item food = it.presets["grain"];
	food.item_id = ItemSys::new_item_id();
	ItemSys::item_list.push_back(food);
	Map[y][x].item_id = food.item_id;
}

void Environment::add_rock(int x, int y) {
	ItemSys::Item food = it.presets["rock"];
	food.item_id = ItemSys::new_item_id();
	ItemSys::item_list.push_back(food);
	Map[y][x].item_id = food.item_id;
}

Environment::Environment(int hours_in_day) {
	
	for (int y = 0; y < map_y_max; y++) {
		for (int x = 0; x < map_x_max; x++) { //z controls the percent chance of tile having food
			int zr = rand() % 100;
			if (zr < 1) {
				add_rock(x, y);
			}

			if((y >= range1_miny && y <= range1_maxy) && (x >= range1_minx && x <= range1_maxx)) {
					int z = rand() % 100;
					if (z < 20) {
						int f = rand() % 2;
						(f == 0) ? add_berrybush(x, y) : add_grain(x, y);
					}
			}

			else if ((y >= range2_miny && y <= range2_maxy) && (x >= range2_minx && x <= range2_maxx)) {
				int z = rand() % 100;
				if (z < 20) {
					int f = rand() % 2;
					(f == 0) ? add_berrybush(x, y) : add_grain(x, y);
				}
			}

			else if ((y >= range3_miny && y <= range3_maxy) && (x >= range3_minx && x <= range3_maxx)) {
				int z = rand() % 100;
				if (z < 20) {
					int f = rand() % 2;
					(f == 0) ? add_berrybush(x, y) : add_grain(x, y);
				}
			}

			else if ((y >= range4_miny && y <= range4_maxy) && (x >= range4_minx && x <= range4_maxx)) {
				int z = rand() % 100;
				if (z < 2) {
					int f = rand() % 2;
					(f == 0) ? add_berrybush(x, y) : add_grain(x, y);
				}
			}

			else {
				
				int z = rand() % 100;
				if (z < 2) {
					int f = rand() % 2;
					(f == 0) ? add_berrybush(x, y) : add_grain(x, y);
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
	
	if (season == 0) {
		int spawn_chance = rand() % 100;
		if (spawn_chance < 20) {
			int x = rand() % (range1_maxx - range1_minx) + range1_minx;
			int y = rand() % (range1_maxy - range1_miny) + range1_miny;
			if (Map[y][x].item_id == -1) {
				int f = rand() % 2;
				(f == 0) ? add_berrybush(x, y) : add_grain(x, y);
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
				(f == 0) ? add_berrybush(x, y) : add_grain(x, y);
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
				(f == 0) ? add_berrybush(x, y) : add_grain(x, y);
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
				(f == 0) ? add_berrybush(x, y) : add_grain(x, y);
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






