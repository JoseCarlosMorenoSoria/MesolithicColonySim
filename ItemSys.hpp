#ifndef ItemSys_hpp
#define ItemSys_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <fstream>
#include <sstream>
#include "ProjUtil.hpp"
using namespace std;
using namespace proj_util;

class ItemSys {//Item System

private:
	static int item_id_iterator;

public:
	ItemSys();


	//FIX THIS: need to separate item instance from item type, such that the instance only contains data that changes such as position, deterioration, material, quality, etc and no constants at all.

	struct Item {
		int item_id = -1;
		string item_name = "";
		string image = "";
		vector<string> tags;
		vector<string> ingredients;//includes tools (and later station) needed to craft item. Later can add quantity requirements for ingredients, etc
		bool consumable_ingredient=true;//true means item is consumed when used as an ingredient to craft something, false means it isn't
		bool can_pick_up = true;
		string item_type="misc";
		//many items have deterioration which is affected by time, temperature and being outdoors
		//humans and animals should have diets which restrict what they can eat and what they seek out as food, this can be facilitated by using tags to mark an item as fitting a certain diet. Animals should be able to eat corpses raw. Vegetarian, omnivore, carnivore, insectivore, infant animals should drink their mother's milk, egg-ivore?
		//eggs should be fertilized or unfertilized for certain animals such as chickens.
		//some food items are products of processing such as bread/soup
		//both crafted and natural items should have a quality level, such that a certain region might have higher quality stones/metal ore and the skill+chance of person should affect the quality of crafted items including food. Quality should then affect stats such as damage, durability, food poisoning, beauty, etc.

		//need to figure out a better system for the same item to be made from different materials without being classed 100% as a separate item, tags help but don't feel scalable
		//need to ensure there is a set of tags for how to interact with certain items (cut down trees, mine stone source, extract berries from bush, paint rock wall, etc)
		//trees and certain plants should lose their leaves either seasonally or according to temperature. FLowering plants can even close their flowers at night. Plants should have needs like animals and reproduce by spawning near each other.
		//need to include decoration/furniture such as stools, rugs, tapestry, etc.
		//need to include art and a related beauty mechanic with a need to drive it

		//need to implement walls and buildings larger than 1 tile
		//need to include crafted herbal remedies for various illnesses/symptoms and their natural ingredients

		//need to include certain bone types such as skulls for animals and humans

		//every crafted item needs a related crafting method, not just a generic crafting act. So wood might be carved, hammered, nailed, burnt, etc. A basket weaved, clothes cut, sewn, tied, tanned, etc.
		//Need to ensure every item has the right drives to ensure that it actually is sought out successfully and test how often it is done so. For example, if there are item A and item B, but in most or every instance only item A is made/sought, then it needs to be reworked to allow B to also be made/sought
		//need to add flooring as well
	};

	map<string, vector<string>> tags; //caches all items by name under their associated tags for ease of lookup
	map<string, vector<string>> ingredients;//same as above tags map<>

	//need to have combat functions utilize these stats. Need to adjust Equipment so that any item that can be picked up can be equipped in a hand or both
	struct Weapon : Item {//currently, all weapons can be picked up, none are consumable, all are craftable
		string material;//this is determined at the time of crafting, not from the csv. Modifier on other stats.
		//currently weapons only have 1 tag
		int cut_damage;
		int slash_damage;
		int piercing_damage;
		int blunt_damage;
		int burn_damage;
		int stagger;
		int range;
		int mass;
		int speed;
		//current max ingredients is 3
	};

	struct Apparel : Item {//clothing, armor, jewelry, anything that is worn. All apparel can be picked up (for now), crafted, and is equippable according to its designated slots. None are consumable ingredients (for now)
		string material;//this is determined at the time of crafting, not from the csv

		int cut_defense;
		int slash_defense;
		int piercing_defense;
		int blunt_defense;
		int burn_defense;
		int weight;//should this be mass for consistency?
		string body_part;
		int insulation_cold;
		int beauty;
	};

	struct Container : Item{//need to modify inventory and find_all so as to use these containers, also person inventory requires division between that which can hold water and that which can't
		vector<int> contents;//holds item id's of items contained inside
		vector<string> Tags;
		int capacity;
		bool holds_liquid;
		bool airtight;
	};

	struct Material : Item {//Materials are items that serve as ingredients and affect the stats of items made from them.
		string source;
		int mass;
		int sharpness;
		int beauty;
		int crafting_time;
		int insulation;
	};

	struct Tool : Item{//these are either items required to craft, build or extract other items or affect the speed or quality of doing so.
		string material;//determined at crafting time

		string use_case;
		string crafting_process;
		int crafting_time;
		vector<string> ingredients;
	};

	//for now, buildings remain a subset of Item, need to later move to its own class and have buildings take up more than 1 tile and have its walls be impassable. A 1 person hut/tent should be 1 tile in size but have an entrance only on one side. (thin walls vs thick walls)
	struct Structure : Item {
		string material;
		string use;
		int insulation_cold;
	};


	//FIX THIS: implement master lists and functions to pass around or store Item children as pointers and simply cast pointer to a Child pointer if child properties need to be accessed.


	static map<string, Item*> presets;//master list
	//presets
	static map<string, Item> misc_presets;
	static map<string, Weapon> weapon_presets;
	static map<string, Apparel> apparel_presets;
	static map<string, Tool> tool_presets;
	static map<string, Material> material_presets;
	static map<string, Structure> structure_presets;
	static map<string, Container> container_presets;

	void ItemPresetsCSVPull();

	static map<int, Item*> item_list;//master list <id,ptr>

	static map<int, Item> misc_item_list;
	static map<int, Weapon> weapon_item_list;
	static map<int, Apparel> apparel_item_list;
	static map<int, Structure> structure_item_list;
	static map<int, Tool> tool_item_list;
	static map<int, Material> material_item_list;
	static map<int, Container> container_item_list;
	

	static int new_item_id();
	void fill_tag_lookup();
	void fill_ingredients_lookup();
	int create_item(string item_name);//returns item_id of created item
	void delete_item(int id);
	void update_item_list();
};

#endif