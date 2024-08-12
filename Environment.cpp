#include "Environment.hpp"

using namespace std;

Environment::Tile Environment::Map[50][100]; //this somehow resolved link error of People not accessing the Map, but might have unintended consequences?
vector<Environment::sky_tile> Environment::Sky;
map<string, Environment::Terrain> Environment::terrains;
ItemSys it;//used to access member functions and variables of ItemSys

//need to figure out a method of implementing regions again


void Environment::csv_fill_terrains() {
	vector<vector<string>> data = get_data("terrain csv");
	for (int i = 11; i < data.size();i++) {
		Terrain ter;
		int r = -1;
		ter.name=data[i][++r];
		ter.image = data[i][++r];
		ter.extracted_by = data[i][++r];
		ter.movement_cost = stoi(data[i][++r]);
		ter.uses = data[i][++r];
		terrains.insert({ ter.name,ter });
	}
}

//only items that should be added in this class are rocks according to the terrain type (granite terrain==granite rocks, dirt terrain == random rock type, etc)


Environment::Environment(int hours_in_day) {
	pu_map_x_max = map_x_max;
	pu_map_y_max = map_y_max;

	//for testing fire spread
	//Map[25][50].has_fire = true;

	for (int y = 0; y < map_y_max; y++) {
		for (int x = 0; x < map_x_max; x++) { //z controls the percent chance of tile having food
			if (Map[y][x].terrain_name != "water") {
				Map[y][x].terrain_name = "dirt";
			}

			int zw = rand() % 100;
			if (zw < 1) {
				if ((x > 5 && y > 5) && (x < map_x_max - 5 && y < map_y_max - 5)) {
					Map[y][x].terrain_name = "water";
					Map[y+1][x].terrain_name = "water";
					Map[y][x+1].terrain_name = "water";
					Map[y+1][x+1].terrain_name = "water";
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
	
	//rain();// needs trigger
	fire_spread(); //needs triggers
	track_manager();
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
				if (Map[y][x].terrain_name == "stone") {//no tracks on stone, might be more efficient to simply not place tracks in the first place, fix?
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