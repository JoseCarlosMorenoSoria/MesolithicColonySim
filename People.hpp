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
		int sender_id;
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
		fo_search_for_new_campsite fo1;
		fo_searching_for_food fo2;
		fo_eating fo3;
		Position fo4;

		//these bools need to preserve their state outside each execution of the utility_function
		bool start_set_up_camp = false; //is set to true when search_for_new_campsite ends so that it only triggers on the next update after search_ ends.
		bool start_gathering_food = false; //same as set up camp
		bool clean_image = false; //used by a function to return to default image on the next update

		vector<string> function_record;//useful for more than debugging, such as determining if did x within the last day (20 updates)

		int radiusmax = -1;//largest radius for searching map. Is set or reset in relevant function

		vector<Message> found_messages;
		vector<vector<Position>> all_found;//for results of function find_all()
		vector<string> potential_targets = { "food", "people", "mate", "no campsite", "messages" };//these should be class members not object members?
		map<string, int> target_index = { {"food", 0}, {"people", 1}, {"mate", 2}, {"no campsite", 3}, {"messages", 4} };
	};

	static Person p; 
	
	//vectors use more memory than necessary? Need to check
public:
	int people_id_iterator = 0;
	static vector<Person> people_list;
	static vector<Message> Message_Map[Environment::map_y_max][Environment::map_x_max];//a map layer that holds messages according to their tile location. Makes clearing the map easier by being a separate layer. Each tile can have multiple messages.

	int campsite_distance_search = 5;

	People();
	void update_all(int day_count, int hour_count, int hours_in_day);
	void update(int day_count, int hour_count, int hours_in_day);
	void utility_function();
	bool move_to(Position pos);
	bool has_food();
	bool valid_position(Position pos);
	Position distance(Position pos1, Position pos2);
	bool check_death();
	Position walk_search_random_dest(); //returns a random destination for a random walk
	int new_person_id();
	void update_person(Person pers);//used to reinsert a person that was copied out and modified. A pointer might be better but I'm not fully certain to avoiding memory leaks, so using this for now.
	
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
	bool give_food(Message m);
	bool reproduce();

};

#endif