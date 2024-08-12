#ifndef Environment_hpp
#define Environment_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>
#include "ItemSys.hpp"
#include "ProjUtil.hpp"
using namespace std;
using namespace proj_util;
//TO DO NOW:
//remember close family to avoid mating with parents/etc. Children exclusion done.
//Unsure if need to add a way to remember past locations that had food or if random searching is good enough for now in order to create cyclical seasonal camp movement
//add a way to compare value of foods according to calorie count and work/time cost

//add fish
//add (social fights, avoidance, escalation, calling for family/friends to join fight, noticing a family member being attacked and going to defend without being asked, sharing dispositions with others, etc)

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

	struct Terrain {
		string name;
		string image;
		string extracted_by;
		int movement_cost;
		string uses;
	};

	static map<string, Terrain> terrains;
	void csv_fill_terrains();

	struct Tile {
		int person_id = -1;//helps enforce only 1 person per tile. Maybe make an exception for carrying infants or wounded? Or might be better to tie the carried infants/wounded in a the inventory of the carrier 
		int animal_id = -1;
		int plant_id=-1;
		//tent/house/campsite is currently implemented as an item, not a building, implement buildings later
		int item_id=-1;//holds the id's of item on tile, currently restricted to one per tile, later on add a method to handle stacking items up to a limit, including of different item types
		string terrain_name;//increase types of terrain: ice/snow/mud/stone type/mineral veins/sand/cracked earth/etc
		//terrain type should also affect type of building allowed, so marsh should require stilts before building on it, sand and mud require water draining or removal or constructing a foundation, etc.
		//terrain also needs to affect movement speed (sand, mud, snow, soil, stone, dirt path, stone road, wood bridge, etc)
		bool has_fire = false;
		//fire should spread according to whether there is a flammable item or terrain and consume the item/terrain leaving behind ash, fire should have a lifetime of how long it takes to consume different items/terrain
		//fires should be started naturally by lightning or extreme heat
		bool has_rain = false;
		//add current_weather and have weather affect movement speed, rain puts out fires, rain and snow distribute water and snow, wind speed which can move (tornado debris) or damage things if too high as well as affects temperature and windmills and direction and speed of fire spread/smoke/clouds, lightning that causes fires,
		//weather should be seasonal and should give a few hours warning time (view clouds on the horizon), 
		//tiles in a column should affect the cloud types in the sky row, the whole map shouldn't be always identical, look into microclimates
		//need to include events such as natural disaster, floods, blizzards, (landslides?), 
		//what about heat waves and cold snaps or is there a way to make these organic rather than random and discrete?
		Tracks track;
		int light_level = 4;//max light
		int temperature = 75;//temperature should be determined by time of day, time of year and latitude
		//temperature, rain, wind, etc should be nullified by having a tile be classed as indoors and roofed. These should also affect item degradation / food/corpse rotting
		//add water level. one level for surface water and another for underground water. (for flooding, wells, mud, cracked earth, plant fertility)
		//terrain then should have 4 levels, underground, the terrain itself, and the surface of the tile and the sky above the tile and its water level that determines cloud cover, clouds should be able to be at the surface as fog. So the terrain could be soil with a level of underground water of 50% and snow at the surface
		string underground_terrain;//subsurface minerals/deposits/etc
		int surface_water_level;//for flooding, runoff, rivers
		int underground_water_level; //for plant roots and wells
		//smoke at the surface should cause breathing problems, smoke/fog at the surface should affect sightline radius. Include methane emissions from the ground as disaster?
		int soil_fertility_level;//should be replenished by plant/animal corpses and manure, depleted by plants growing. What about salinity such as what happened in Mesopotamia?
		//Sunlight level should vary based on cloud/smoke cover, time of day, time of year, and latitude.
		//tiles should also hold sound and smell, though maybe those are better handled as tracks?

		//biome placement should be preset but then allowed to change organically, such that if climate in a region changes, a fertile grassland can become a desert, or a desert be turned into a fertile forest, etc. Biomes would then be marked by the local flora, fauna, climate and topography of each block of x tiles.
		//biomes should have transition regions when placed such that they don't have stark borders.
		int total_movement_cost;//terrain+subsurface_water_level (mud is slower, wet sand is faster)+surface_water_level+plant_movement_cost+flooring	//later +slope+surface roughness
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

	//need to replace with add rock or other valid items only
	//static void add_item_to_map(string item, int x, int y);

	static void fire_spread();
	static void rain();
	static void track_manager();

};

#endif