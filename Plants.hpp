#ifndef Plants_hpp
#define Plants_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "Environment.hpp"
#include <stdexcept>
using namespace std;

//need to add decomposition of dead plants, same for animals and people
class Plants {//need to add plant functions and add them to Game, find_all, acquire(), and add the plant components as Items
public:
	
	struct Plant {
		int plant_id;
		string species;
		string soil_requirements;
		int light_min;//minimum light needed to grow and live
		int temp_min;
		int temp_max;//temperature limits for growth, death and leaf loss
		int current_temp;
		int water_min;
		int water_max;
		string nutrient_requirements;
		int max_height;
		int current_height;//height and radius shade out shorter plants to outcompete them (deprive them of sunlight and water)
		int max_radius;
		int current_radius;//how far the tree shades out shorter plants or trees of the same height, or bushes doing the same to shorter plants
		vector<string> components;//Items: fruit, log, wood, branch, leaves, roots, sap, bark, fibers, etc
		int reproduction_rate;//in days
		int reproduction_distance;//in tiles, distance plant spreads
		//plant effects should be a property of their components? (leaves, fruit, etc) Same for calories?
		int lifespan;
		int age;
		int growth_rate;
		bool domesticated;
		bool is_alive = true;
		//need to add diseases
	};

	Plants(int a);
	void update_all();
	//check conditions, react to conditions (growth rate, leaf loss, drying up, drowning, reproduction, image changes, fruit production)
	void update();
	void reproduce();
	void check_death();
	void fill_presets();
	static vector<Plant> species_presets;
	static vector<Plant> plants;//global list of plants

};

#endif