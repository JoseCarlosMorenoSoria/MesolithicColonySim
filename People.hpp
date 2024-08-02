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
#include "ItemSys.hpp"
using namespace std;

class People {
	//NEED TO DO: pull out all constants such as the hunger_level threshold for being hungry out and into the Person struct to better be able to balance these variables with each other

	struct Position {
		int x = -1;
		int y = -1;
		bool operator==(Position const& pos2) {
			return this->x == pos2.x && this->y == pos2.y;
		}
		bool operator!=(Position const& pos2) {
			return this->x != pos2.x || this->y != pos2.y;
		}
		//should this include an overload of ! operator to check if pos == {-1,-1} which is the NULL equivalent?
	};

	struct Message {//need to tie Messages to tiles in map instead of current Message list
		int message_id = -1;
		int sender_id = -1;
		int reciever_id = -1; //-1 is a message for everyone/anyone
		string messsage = "";
		Position origin;
	};
	
	//structs for tracking function states	fo == function object


	struct progress_state { 
		int progress_done = -1;
		int progress = 0;
		bool progress_func() {
			if (progress == progress_done) {
				progress = 0;
				return true;
			}
			progress++;
			return false;
		}
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
		vector<int> item_inventory; //holds id's of items in inventory
		vector<int> group_members; //holds id's of group members
		bool is_alive = true;
		Position campsite_pos = {-1,-1};
		int campsite_age = -1; //used to track how long have lived at that campsite since it was placed to provide a bit of breathing room before removing to find another campsite
		int reproduction_cooldown = 0; 
		int spouse_id = -1;
		vector<int> children_id;
		int age = 0;
		bool being_carried = false;
		int carried_by_id = -1;
		//objects for tracking function states
		progress_state eating_progress = {4};//4 is the number of ticks/frames until progress is done
		int eating_food_index = -1;//this and eating_progress should be encapsulated in a function object

		map<string, progress_state> crafting;//key is product name		fix this: need to avoid case of progress saved, all ingredients lost (instead of being used up by starting the crafting process), new ingredients obtained, then picking up progress where it was last left off instead of starting over again

		bool child_is_hungry = false;//currently a flag to communicate between feed_own_children and searching_for_food and gathering_food. I need to rethink how functions communicate with each other and facilitate direct calls by one to another. Fix this.
		int hungry_child_index = -1;
		//these bools need to preserve their state outside each execution of the utility_function
		bool start_set_up_camp = false; //is set to true when search_for_new_campsite ends so that it only triggers on the next update after search_ ends.

		bool clean_image = false; //used by a function to return to default image on the next update

		vector<string> function_record;//useful for more than debugging, such as determining if did x within the last day (20 updates)
		vector<int> function_record_nums;//tracks how many times in a row each function was called to reduce size of function_record. Might make more sense combining as a struct of {record, times repeated}
		
		int radiusmax = -1;//largest radius for searching map. Is set or reset in relevant function

		vector<int> found_messages;//message id's
		map<string, vector<Position>> search_results;//for results of function find_all()
		
		bool message_clear_flag = false;//unsure if thise should be in Person or as part of the class

		bool printed = true;//for function record printing. set to true to prevent execution

		bool adopt_spouse_campsite = false;

		bool search_active = false;//a shared, generic search flag that reduces erratic movement by having a single search pattern serve all current targets
		Position general_search_dest;//this and search_active should be together in a function object
	};

	
	
	//vectors use more memory than necessary? Need to check
public:
	int people_id_iterator = 0;
	int message_id_iterator = 0;
	static vector<Person> pl;//people_list
	static vector<Message> message_list;
	static vector<int> Message_Map[Environment::map_y_max][Environment::map_x_max];//a map layer that holds messages by id according to their tile location. Makes clearing the map easier by being a separate layer. Each tile can have multiple messages.
	
	int campsite_distance_search = 5;

	People();
	bool check_death();
	void update_all(int day_count, int hour_count, int hours_in_day);
	void update(int day_count, int hour_count, int hours_in_day);
	void utility_function();
	void find_all();
	bool move_to(Position pos);
	Position walk_search_random_dest(); //returns a random destination for a random walk

	//fix this, these should be functions inside the Position struct
	bool valid_position(Position pos);
	Position make_position_valid(Position dest, int ux, int lx, int uy, int ly);
	int distance(Position pos1, Position pos2);
	
	//find a way to make these 2 functions automatic rather than having to be manually called
	int new_person_id();
	int new_message_id();

	//can these 2 functions be merged?
	int p_by_id(int id);//returns index of person in people list (pl).
	int message_by_id(int id);//returns index of message in message list.

	void add_func_record();
	vector<int> inventory_has(string target);
	void create_item(string item_type, Position pos);
	void pick_up_item(int item_id, Position pos);
	void delete_item(int item_id, Position pos, int index);

	//slated for replacement
	bool tile_has(string target, Position pos);
	bool give_food();
	bool moving_to_bed();//this should be integrated into sleeping rather than being a separate function
	Position empty_adjacent_tile();//should be replaced by drop_item()
	bool mate_check(int pers_id);//temp function, needs better replacement, either integrate into reproduce or turn into a generic filter on found items and found people
	
	bool search_for_new_campsite();
	bool set_up_camp();
	bool idle();
	bool sleeping();
	bool eating();
	bool searching_for_food();
	bool reproduce();
	bool feed_own_infants();
	//void carry_infant();		//need generic carry function
	//void drop_infant();

	void speak(string message_text, int receiver_id);
	bool craft(string product);
	bool drop_item(int index);//not done
	void general_search_walk();
	void check_tile_messages(Position pos);
	bool acquire(string target);

	vector<Position> filter_search_results();
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