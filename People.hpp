#ifndef People_hpp
#define People_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "Environment.hpp"


using namespace std;

class People {


	//structs for tracking function states
	struct Position {
		int x = -1;
		int y = -1;
		bool operator==(Position const& pos2) {
			return this->x == pos2.x && this->y == pos2.y;
		}
		//should this include an overload of ! operator to check if pos == {-1,-1} which is the NULL equivalent?
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

	struct Message {//need to tie Messages to tiles in map instead of current Message list
		int sender_id = -1;
		int reciever_id = -1; //-1 is a message for everyone/anyone
		string messsage = "";
		Position pos;//might make more sense to have an origin position rather than a position for the whole range now that a map layer exists for messages
		Position origin;
	};

	struct Person {
		int id; //need to figure out a way to make new id automatic rather than a parameter, currently using an int but not automatic enough
		Position pos;
		bool sex; //true==male, false==female

		string current_image = "pics/human.png";
		string current_state = "idle"; //state/action
		int sightline_radius = 5; //how far around self can see objects
		int audioline_radius = 5; //how far around self can hear and be heard
		int hunger_level = 0;
		int hungry_time = 0;
		bool awake = true;
		int tired_level = 0;
		vector<int> food_inventory; //holds id's of food in inventory
		vector<int> group_members; //holds id's of group members
		bool is_alive = true;
		Position campsite_pos = {-1,-1};
		int campsite_age = -1; //used to track how long have lived at that campsite since it was placed to provide a bit of breathing room before removing to find another campsite
		int reproduction_cooldown = 0; 

		//objects for tracking function states
		fo_search_for_new_campsite fo1; //unsure if these should be kept or if it makes more sense to have a single shared variable, cuurent_dest
		fo_searching_for_food fo2;
		fo_eating fo3;
		Position fo4;

		//these bools need to preserve their state outside each execution of the utility_function
		bool start_set_up_camp = false; //is set to true when search_for_new_campsite ends so that it only triggers on the next update after search_ ends.
		bool start_gathering_food = false; //same as set up camp
		bool clean_image = false; //used by a function to return to default image on the next update

		vector<string> function_record;//useful for more than debugging, such as determining if did x within the last day (20 updates)
		vector<int> function_record_nums;//tracks how many times in a row each function was called to reduce size of function_record. Might make more sense combining as a struct of {record, times repeated}
		bool function_done = false;//flag used by all functions to declare them as done if they can't be interrupted

		int radiusmax = -1;//largest radius for searching map. Is set or reset in relevant function

		vector<Message> found_messages;
		vector<vector<Position>> all_found;//for results of function find_all()
		vector<string> potential_targets = { "food", "people", "mate", "no campsite", "messages" };//these should be class members not object members?
		map<string, int> target_index = { {"food", 0}, {"people", 1}, {"mate", 2}, {"no campsite", 3}, {"messages", 4} };
		vector<int> target_quantity_limit = {10, 1, 1, 1, -1};
		vector<bool> target_chosen = { true, true, true, true, true };
		//targets above might make more sense as a single list of a struct for targets as a type rather than multiple separate lists
	
		bool message_clear_flag = false;//unsure if thise should be in Person or as part of the class

		bool printed = true;//for function record printing. set to true to prevent execution
	};

	
	
	//vectors use more memory than necessary? Need to check
public:
	int people_id_iterator = 0;
	static vector<Person> pl;//people_list
	//DO NOW: give each message an id and store the id on the Message_Map to reduce the storage size of Message_Map
	static vector<Message> Message_Map[Environment::map_y_max][Environment::map_x_max];//a map layer that holds messages according to their tile location. Makes clearing the map easier by being a separate layer. Each tile can have multiple messages.
	static vector<int> Person_Map[Environment::map_y_max][Environment::map_x_max];//a map layer to hold people to access directly from a tile instead of searching the whole people list. Holds the id of each person
	//Need to test without the messages to see if that is the cause of lag

	int campsite_distance_search = 5;

	People();
	void update_all(int day_count, int hour_count, int hours_in_day);
	void update(int day_count, int hour_count, int hours_in_day);
	void utility_function();
	bool move_to(Position pos);
	bool valid_position(Position pos);
	Position make_position_valid(Position dest, int ux, int lx, int uy, int ly);
	int distance(Position pos1, Position pos2);
	bool check_death();
	Position walk_search_random_dest(); //returns a random destination for a random walk
	int new_person_id();
	int p_by_id(int id);//returns index of person in people list (pl).
	void add_func_record();
	
	//find_all could be further reduced in terms of time by checking for all people at once, so the number of iterations is simply the largest search rather than searching for every person separately
	vector<vector<Position>> find_all(vector<string> potential_targets); //instead of calling find for a single use and having to search the map multiple times, this function searches the map for all use cases and returns the results to be accessed instead.
	//Result categories: [0]: ? Unsure whether categories (food, people, mate, messages, etc) should have a fixed index or not, maybe not.
	bool find_check(Position pos, string target);//evaluates a single tile to determine if it contains the target being sought

	bool search_for_new_campsite();
	bool set_up_camp();
	bool idle();
	bool sleeping();
	bool eating();
	bool searching_for_food();
	bool gathering_food();
	bool sharing_food();
	void speak(string message_text);
	bool give_food();
	bool reproduce();
	bool moving_to_bed();

};

#endif

/*
* Format of action functions (not utility functions or main loop functions)
* 
* called from utility_function()
* check function trigger, if false return false, else continue
* action
* if action is not done this tick, return true; In Progress
* if action is done/ended this tick, return true; Done
*/

/*
* Actions generally take the format of:
* find x
* go to x
* perform action on/with x
* 
* Some actions are simply a composition of smaller actions that reduce to this format.
* Need to standardize format of functions so that they can be more easily read, combined, and simplified
*/