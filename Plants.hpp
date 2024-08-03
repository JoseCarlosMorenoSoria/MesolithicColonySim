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
#include "ProjUtil.hpp"
using namespace std;
using namespace proj_util;
//need to add decomposition of dead plants, same for animals and people
class Plants {//need to add plant functions and add them to Game, find_all, acquire(), and add the plant components as Items
public:
	
	struct Plant {
		int plant_id;
		string species;
		Position pos;
		bint light_hunger;//how long plant has been living without light
		bint current_temp;
		bint current_water;
		bint current_height;//height and radius shade out shorter plants to outcompete them (deprive them of sunlight and water)
		bint current_radius;//how far the tree shades out shorter plants or trees of the same height, or bushes doing the same to shorter plants
		vector<string> current_components;//Items: fruit, log, wood, branch, leaves, roots, sap, bark, fibers, etc
		//plant effects should be a property of their components? (leaves, fruit, etc) Same for calories?
		bint age;
		int growth_level;
		bool is_alive = true;
		//need to add diseases
	};
	
	
	//unsure about this idea ->, it might be more space efficient but also be more time heavy, and might be better replaced by just reducing attributes to just enough bits for the range of that attribute: might be able to reduce instances by reducing each instance to just its position which refers to one shared set of states, or even to shared invidual states. Such that instead of having 20 plants that all have their own instance of a water level, have a water level that is tied to certain Map position defined plant "instances", and if a specific plant has its water level change, it simply reties to another static water level state or creates one if none exists, the water level state erases itself if no position plant is tied to it.

	struct SpeciesPreset {
		string plant_type;//tree, flower, vine, grass, shrub, succulent, cactus, etc
		string soil_requirements;//soil fertility/nutrient levels necesssary to grow/live
		int light_min;//minimum light needed to grow and live
		int light_starvation;//how many max hours plant can survive without light
		int temp_min;
		int temp_max;//temperature limits for growth, death and leaf loss
		int water_min;
		int water_max;
		string nutrient_requirements;
		int max_height;
		int max_radius;
		vector<string> potential_components;//Items: fruit, log, wood, branch, leaves, roots, sap, bark, fibers, etc
		int reproduction_rate;//in days
		int reproduction_distance;//in tiles, distance plant spreads
		int lifespan;//also affects if plant is annual or perrenial, etc
		int growth_rate;
		bool domesticated;
	};

	//FIX THIS: need to change it such that constants are accessed directly from the species preset rather than each plant instance keeping a copy of the species constants
	//this requires presets to have a separate struct with only constants
	Plants(int a);
	void update_all(int hour);//hour is recieved from Game
	//check conditions, react to conditions (growth rate, leaf loss, drying up, drowning, reproduction, image changes, fruit production)
	void update(int hour);
	void reproduce();
	void check_death();
	void fill_presets();
	static map<string, SpeciesPreset> species_presets;//access by species name
	static vector<Plant> pln;//global list of plants
	Plant new_plant(string species, Position pos);

};

#endif