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

//TO DO NOW:
// TO DO NOW NOW: implement behavior for feeding one's infants and moving them to current campsite or carry them while searching for new campsite
//remember close family to avoid mating with siblings/parents/etc. Children exclusion done.
//implement an age system to slow down person multiplication, add simple behavior for infants and children
//add a restriction for family to stay together most of the time rather than wander apart, especially spouse and kids
//add seasonal varieties of berrybushes (to create nomadism or cyclical camp movement) and have them respawn so that a certain area only has food during x days and people have to move following the food seasons. Unsure if need to add a way to remember past locations that had food or if random searching is good enough for now.
//include grain and a method to process it, as well as a way to compare value and pick either berries or grain according to calorie count and work/time cost
//then implement a crafting system and a tool requirement for processing grain (instead of just bringing it back to camp to perform an animation)
//consolidate functions into generics such as acquire(x) which is composed of search_for(x) and move_to(x) and pick_up(x), or if craft(x) is chosen then acquire(ingredients/tools)
//add deer and rabbits
//add hunting deer and rabbits and trapping rabbits
//add water and thirst and fish and fishing
//add dispositions between people and associated behavior (social fights, avoidance, escalation, calling for family/friends to join fight, noticing a family member being attacked and going to defend without being asked, sharing dispositions with others, etc)
//add method to map and label natural cliques formed through the network of dispositions
//add method to map or create internal hierarchies among cliques, as well as ways to break/shift the hierarchies
//ensure natural methods for cliques not to become too stable so that groups are always changing, being formed, dissappearing, splitting, merging
//add more clique based behavior such as fighting over land if hungry, blood feuds, defend each other, raid each other, etc

class Environment {

	struct Tile {


		int person_id = -1;//helps enforce only 1 person per tile. Maybe make an exception for carrying infants or wounded? Or might be better to tie the carried infants/wounded in a the inventory of the carrier 
		//tent/house/campsite is currently implemented as an item, not a building, implement buildings later
		int item_id=-1;//holds the id's of item on tile, currently restricted to one per tile, later on add a method to handle stacking items up to a limit, including of different item types
		
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