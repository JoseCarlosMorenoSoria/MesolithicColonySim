#include "Environment.hpp"

using namespace std;

Environment::Tile Environment::Map[50][100]; //this somehow resolved link error of People not accessing the Map, but might have unintended consequences?
vector<Environment::sky_tile> Environment::Sky;

Environment::Environment(int hours_in_day) {
	int range1_minx = 0;
	int range1_miny = 0;
	int range1_maxx = 15;
	int range1_maxy = 15;

	int range2_minx = 85;
	int range2_miny = 0;
	int range2_maxx = map_x_max;
	int range2_maxy = 15;

	int range3_minx = 0;
	int range3_miny = 35;
	int range3_maxx = 15;
	int range3_maxy = map_y_max;

	int range4_minx = 85;
	int range4_miny = 35;
	int range4_maxx = map_x_max;
	int range4_maxy = map_y_max;

	for (int y = 0; y < map_y_max; y++) {
		for (int x = 0; x < map_x_max; x++) { //z controls the percent chance of tile having food

			if((y >= range1_miny && y <= range1_maxy) && (x >= range1_minx && x <= range1_maxx)) {
					int z = rand() % 100;
					if (z < 20) {
						Map[y][x].has_food = true;
					}
			}

			else if ((y >= range2_miny && y <= range2_maxy) && (x >= range2_minx && x <= range2_maxx)) {
				int z = rand() % 100;
				if (z < 20) {
					Map[y][x].has_food = true;
				}
			}

			else if ((y >= range3_miny && y <= range3_maxy) && (x >= range3_minx && x <= range3_maxx)) {
				int z = rand() % 100;
				if (z < 20) {
					Map[y][x].has_food = true;
				}
			}

			else if ((y >= range4_miny && y <= range4_maxy) && (x >= range4_minx && x <= range4_maxx)) {
				int z = rand() % 100;
				if (z < 20) {
					Map[y][x].has_food = true;
				}
			}

			else {
				int z = rand() % 100;
				if (z < 1) {
					Map[y][x].has_food = true;
				}
			}
			
		}
	}

	for (int i = 0; i < hours_in_day/2; i++) {
		sky_tile s;
		Sky.push_back(s);
	}
}


void Environment::update(int hours_in_day, int hour_count) {
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

}