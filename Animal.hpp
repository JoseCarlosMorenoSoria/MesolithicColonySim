#ifndef Animal_hpp
#define Animal_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "Environment.hpp"
#include "ItemSys.hpp"
#include "Creature.hpp"
using namespace std;

//this is an outdated version, need to pass relevant functions and data to Creature and then add Animal specific functions/data here
//this is just a copy paste of People and Person but renamed for animals and with irrelevant functions removed (no crafting, no requests, etc)
//A better method might be to pull out common functions and structs into a separate class that is used by both People and Animal but for now this works

class Animal : public Creature{
	
public:
	static int ox;//origin.x for use in distance() function of Position struct.
	static int oy;
	struct Position {
		int x = -1;
		int y = -1;
		bool operator==(Position const& pos2) {
			return this->x == pos2.x && this->y == pos2.y;
		}
		bool operator!=(Position const& pos2) {
			return this->x != pos2.x || this->y != pos2.y;
		}
		static int distance(Position pos1, Position pos2);
		bool operator<(Position const& pos2) const {//sorting order is according to distance from an origin, the origin being the person's current position
			int d1 = distance(*this, { ox,oy });
			int d2 = distance(pos2, { ox,oy });
			if (d1 < d2) {
				return true;
			}
			else if (d1 == d2) {
				if (this->x < pos2.x) { return true; }
				if (this->x == pos2.x && this->y < pos2.y) { return true; }
			}
			return false;
		}
		//should this include an overload of ! operator to check if pos == {-1,-1} which is the NULL equivalent?
	};

	//fix this, these should be functions inside the Position struct
	bool valid_position(Position pos);
	Position make_position_valid(Position dest, int ux, int lx, int uy, int ly);


	struct Message {//need to tie Messages to tiles in map instead of current Message list
		int message_id = -1;//is map<> more efficient than keeping in a vector?
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

	struct animal {
		int id; //need to figure out a way to make new id automatic rather than a parameter, currently using an int but not automatic enough
		Position pos;
		bool sex; //true==male, false==female
		string current_image;
		int sightline_radius = 5; //how far around self can see objects
		int audioline_radius = 5; //how far around self can hear and be heard
		int hunger_level = 0;
		int hungry_time = 0;
		bool awake = true;
		int tired_level = 0;
		bool is_alive = true;
		int reproduction_cooldown = 0;
		int age = 0;
		//objects for tracking function states
		progress_state eating_progress = { 4 };//4 is the number of ticks/frames until progress is done
		int eating_food_index = -1;//this and eating_progress should be encapsulated in a function object

		//these bools need to preserve their state outside each execution of the utility_function

		bool clean_image = false; //used by a function to return to default image on the next update

		int radiusmax = -1;//largest radius for searching map. Is set or reset in relevant function

		vector<int> found_messages;//message id's
		map<string, vector<Position>> search_results;//for results of function find_all()


		Position general_search_dest;//this and search_active should be together in a function object
		
		string species;//deer, rabbit, wolf, etc
		string diet="";

		bool isdead = false;

		Position last_position;//for setting direction of tracks

		string meat_type;
		int speed = 1;
		bool fleeing = false;
	};
	//need to implement flee mechanic to animal and make animal faster than human but slow to react (later it should be slow to accelerate not react?)


	//vectors use more memory than necessary? Need to check
	static set<string> species_names;
	static int animal_id_iterator;
	int message_id_iterator = 0;
	static vector<animal> al;//animal_list
	static vector<Message> message_list;
	static vector<int> Message_Map[Environment::map_y_max][Environment::map_x_max];//a map layer that holds messages by id according to their tile location. Makes clearing the map easier by being a separate layer. Each tile can have multiple messages.
	bool message_clear_flag = false;

	Animal();
	Animal(int init);
	bool check_death();
	void update_all(int day_count, int hour_count, int hours_in_day);
	void update(int day_count, int hour_count, int hours_in_day);
	void utility_function();
	void find_all();
	bool move_to(Position pos);
	Position walk_search_random_dest(); //returns a random destination for a random walk

	static int new_animal_id();
	int new_message_id();

	int a_by_id(int id);
	int message_by_id(int id);//returns index of message in message list.

	void create_item(string item_type, Position pos);//needed for defecating
	void delete_item(int item_id, Position pos, int index);

	bool idle();
	bool sleeping();
	bool eating();
	bool reproduce();

	void speak(string message_text, int receiver_id);//for making sounds
	bool drop_item(int index);//not done
	void general_search_walk(string target);
	void check_tile_messages(Position pos);//for reacting to sounds/events such as another animal being killed
	bool acquire(string target);

	vector<Position> filter_search_results(string target);


	void create_tracks(Position pos);
};

#endif