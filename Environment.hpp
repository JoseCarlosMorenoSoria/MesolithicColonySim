#ifndef Environment_hpp
#define Environment_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Environment {

	struct Tile {
		bool has_tent = false;
		bool has_food = false;

	};

	struct sky_tile {
		bool has_sun = false;
		bool has_moon = false;
	};

public:
	static const int map_y_max = 50;
	static const int map_x_max = 100;
	static Tile Map[map_y_max][map_x_max];
	static vector<sky_tile> Sky; //strip of sky above map
	Environment(int hours_in_day);
	static void update(int hours_in_day, int hour_count);
};

#endif