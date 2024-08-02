#ifndef ItemSys_hpp
#define ItemSys_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>
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
	};
	//presets
	Item tent;
	Item grain;
	Item bread;
	Item berrybush;
	Item rock;
	Item mortar_pestle;

	static vector<Item> item_list;//global list
	static int item_by_id(int id);
	static int new_item_id();
};

#endif