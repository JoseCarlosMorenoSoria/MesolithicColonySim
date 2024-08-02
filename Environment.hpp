#ifndef Environment_hpp
#define Environment_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>

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
	static Tile Map[50][100];
	static sky_tile Sky[10]; //strip of sky above map
	Environment();
	static void update(int hours_in_day, int hour_count);
};

#endif