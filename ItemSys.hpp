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
using namespace std;

class ItemSys {//Item System

private:
	static int item_id_iterator;

public:
	ItemSys();

	struct Item {
		int item_id = -1;
		string item_name = "";
		string image = "";
		vector<string> tags;//include tags such as "food", "tool", "building", etc
		vector<string> ingredients;//includes tools (and later station) needed to craft item. Later can add quantity requirements for ingredients, etc
		bool consumable_ingredient;//true means item is consumed when used as an ingredient to craft something, false means it isn't
		bool can_pick_up = true;
		
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
	//presets
	map<string, Item> presets;
	map<string, vector<string>> tags; //caches all items by name under their associated tags for ease of lookup
	map<string, vector<string>> ingredients;//same as above tags map<>

	//remove redundancies inhereited by Item from these derived structs
	//need both the csv reader, to connect it to Item generic and to have combat functions utilize these stats. Need to adjust Equipment so that any item that can be picked up can be equipped in a hand or both
	struct Weapon : Item {//currently, all weapons can be picked up, none are consumable, all are craftable
		int item_id;
		string name;
		string image;
		string material;//this is determined at the time of crafting, not from the csv. Modifier on other stats.
		string tags;//currently weapons only have 1 tag
		int cut_damage;
		int slash_damage;
		int piercing_damage;
		int blunt_damage;
		int burn_damage;
		int stagger;
		int range;
		int mass;
		int speed;
		vector<string> ingredients;//current max is 3
	};

	struct Apparel : Item {//clothing, armor, jewelry, anything that is worn. All apparel can be picked up (for now), crafted, and is equippable according to its designated slots. None are consumable ingredients (for now)
		int item_id;
		string name;
		string image;
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
		vector<string> ingredients;
	};

	struct Container : Item{//need to modify inventory and find_all so as to use these containers, also person inventory requires division between that which can hold water and that which can't
		int item_id;
		string name;
		string image;
		vector<int> contents;//holds item id's of items contained inside
		vector<string> Tags;
		int capacity;
		bool holds_liquid;
		bool airtight;
		bool can_pick_up;
		vector<string> ingredients; 
	};

	//no building can be picked up or carried nor is a consumable ingredient
	//for now, buildings remain a subset of Item, need to later move to its own class and have buildings take up more than 1 tile and have its walls be impassable. A 1 person hut/tent should be 1 tile in size but have an entrance only on one side. (thin walls vs thick walls)
	struct Structure : Item {
		int item_id;
		string name;
		string material;
		string use;
		vector<string> ingredients;
		int insulation_cold;
	};

	void ItemPresetsCSVPull();
	static vector<Item> item_list;//global list
	static int item_by_id(int id);
	static int new_item_id();
	void fill_tag_lookup();
	void fill_ingredients_lookup();


};

#endif