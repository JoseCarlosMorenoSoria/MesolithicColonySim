#ifndef ItemSys_hpp
#define ItemSys_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
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
	};
	//presets
	map<string, Item> presets;
	map<string, vector<string>> tags; //caches all items by name under their associated tags for ease of lookup

	static vector<Item> item_list;//global list
	static int item_by_id(int id);
	static int new_item_id();
	void fill_tag_lookup();
};

#endif