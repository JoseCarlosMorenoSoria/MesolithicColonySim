#ifndef Environment_hpp
#define Environment_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>
#include "ItemSys.hpp"
using namespace std;

//FIX NOW: person sometimes doesn't move at all, might have to do with search_for_new_campsite(), though it happened even when the person created a campsite, fix this

//TO DO NOW:
//remember close family to avoid mating with parents/etc. Children exclusion done.
//Unsure if need to add a way to remember past locations that had food or if random searching is good enough for now in order to create cyclical seasonal camp movement
//add a way to compare value of foods according to calorie count and work/time cost

//add deer and rabbits
//add hunting deer and rabbits and trapping rabbits
//add water and thirst and fish and fishing
//add dispositions between people and associated behavior (social fights, avoidance, escalation, calling for family/friends to join fight, noticing a family member being attacked and going to defend without being asked, sharing dispositions with others, etc)

//add method to map and label natural cliques formed through the network of dispositions
//add method to map or create internal hierarchies among cliques, as well as ways to break/shift the hierarchies
//ensure natural methods for cliques not to become too stable so that groups are always changing, being formed, dissappearing, splitting, merging
//add more clique based behavior such as fighting over land if hungry, blood feuds, defend each other, raid each other, etc

class Environment {
public:
	struct Tracks {//might serve for the creation of cow trails as well (natural roads/paths)
		//int id = -1; //id might not be necessary
		string creature;//the animal/person that caused the tracks
		int track_age=-1;//for controlling the lifetime of tracks according to terrain type. (tracks last longer in snow, no tracks made on stone (add feces and urine, etc later which can be found on stone))
		int direction_x;//which way the tracks are pointing
		int direction_y;
	};

	struct Tile {
		int person_id = -1;//helps enforce only 1 person per tile. Maybe make an exception for carrying infants or wounded? Or might be better to tie the carried infants/wounded in a the inventory of the carrier 
		int animal_id = -1;
		//tent/house/campsite is currently implemented as an item, not a building, implement buildings later
		int item_id=-1;//holds the id's of item on tile, currently restricted to one per tile, later on add a method to handle stacking items up to a limit, including of different item types
		string terrain;
		bool has_fire = false;
		bool has_rain = true;
		Tracks track;
		int light_level = 4;//max light
		int temperature = 75;
	};

	struct sky_tile {
		bool has_sun = false;
		bool has_moon = false;
	};


	static const int map_y_max = 50;
	static const int map_x_max = 100;
	static Tile Map[map_y_max][map_x_max];
	static vector<sky_tile> Sky; //strip of sky above map
	Environment(int hours_in_day);
	static void update(int hours_in_day, int hour_count, int day_count);

	static void add_item_to_map(string item, int x, int y);

	static void fire_spread();
	static void rain();
	static void track_manager();

	static vector<int> people_in_stealth;//used so that Animal knows if any person is in stealth
};

#endif