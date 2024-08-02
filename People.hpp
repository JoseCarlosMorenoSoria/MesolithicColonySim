#ifndef People_hpp
#define People_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>
#include "Environment.hpp"

using namespace std;

class People {


	//structs for tracking function states
	struct Position {
		int x = -1;
		int y = -1;
	};
	
	//fo == function object
	struct fo_search_for_new_campsite { //might make more sense to have a search movement function to be used for both finding a new campsite and for finding food
		Position pos = {-1,-1}; //current destination for search
	};
	struct fo_searching_for_food {
		Position pos = { -1,-1 }; //current destination for search
	};
	struct fo_eating {//could probably be better as a generic time delay or animation delay so as to be used in other functions. 
		int progress = 0;
		int progress_done = 4;
	};

	struct Person {
		//int id;
		Position pos;
		string current_image = "pics/human.png";
		string current_state = "idle"; //state/action
		int sightline_radius = 5; //how far around self can see objects
		int hunger_level = 0;
		int hungry_time = 0;
		bool awake = true;
		int tired_level = 0;
		vector<int> food_inventory; //holds id's of food in inventory
		vector<int> group_members; //holds id's of group members
		bool is_alive = true;
		Position campsite_pos = {-1,-1};
		int campsite_age = -1; //used to track how long have lived at that campsite since it was placed to provide a bit of breathing room before removing to find another campsite

		//objects for tracking function states
		fo_search_for_new_campsite fo1;
		fo_searching_for_food fo2;
		fo_eating fo3;

		//these bools need to preserve their state outside each execution of the utility_function
		bool start_set_up_camp = false; //is set to true when search_for_new_campsite ends so that it only triggers on the next update after search_ ends.
		bool start_gathering_food = false; //same as set up camp
		bool clean_image = false; //used by a function to return to default image on the next update

		vector<string> function_record;

	};

	static Person p; 
	

public:
	static vector<Person> people_list;

	People();
	void update_all(int day_count, int hour_count); //does this need to be static?
	void update(int day_count, int hour_count);
	void utility_function();
	bool move_to(Position pos);
	bool has_food();
	vector<Position> find(string target, int radius, int quantity);
	bool valid_position(Position pos);
	Position distance(Position pos1, Position pos2);
	int campsite_distance_search = 5;

	bool tile_has(string target);

	bool search_for_new_campsite();
	bool set_up_camp();
	bool idle();
	bool sleeping();
	bool eating();
	bool searching_for_food();
	bool gathering_food();
	bool sharing_food();

};

#endif