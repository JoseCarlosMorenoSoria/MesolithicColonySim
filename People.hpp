#ifndef People_hpp
#define People_hpp
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
#include "Animal.hpp"
#include <stdexcept>
using namespace std;

class People {
	//NEED TO DO: pull out all constants such as the hunger_level threshold for being hungry out and into the Person struct to better be able to balance these variables with each other
public:
	static int ox;//origin.x for use in distance() function of Position struct. Uses current Person position
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
			int d1 = distance(*this,{ox,oy});
			int d2 = distance(pos2, { ox,oy });
			if (d1 < d2) {
				return true;
			}
			else if(d1==d2){
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

	struct trap_check {
		Position pos;
		int last_day_checked = -1;
	};

	struct submit_tracker {
		bool submissive_to = false;
		int fights_lost = 0;//if net fights lost to this person is equal or greater than 3, become submissive to that person
	};

	struct Person {
		int id; //need to figure out a way to make new id automatic rather than a parameter, currently using an int but not automatic enough
		Position pos;
		bool sex; //true==male, false==female

		string current_image = "pics/human.png";
		//string current_state = "idle"; //state/action
		int sightline_radius = 5; //how far around self can see objects
		int audioline_radius = 5; //how far around self can hear and be heard
		int hunger_level = 0;
		int hungry_time = 0;
		int thirst_level = 0;
		bool awake = true;
		int tired_level = 0;
		vector<int> item_inventory; //holds id's of items in inventory
		
		Position current_direction;//based on last tile moved from. Rotations and direction only used for circumnavigating obstacles for now.
		vector<Position> rotations = { {0,-1},{1,-1},{1,0} ,{1,1} ,{0,1} ,{-1,1} ,{-1,0} ,{-1,-1} };//includes diagonals
		//clockwise rotation
		// 0,-1 N - up
		// 1,0 E - right
		// 0,1 S - down
		//-1,0 W - left
		
		//vector<int> group_members; //holds id's of group members
		map<int, int> dispositions;//{Person_ID, favorability} holds id's of known people and whether and how much one likes or dislikes each
		
		map<int, submit_tracker> submissive_to;//{Person_ID, submissive_to/Not submissive_to} tracks people who have won a fight against self at least 3 times (them being present as a hotile during a fight that was lost against someone else counts)
		//being submissive to another means when a fight would normally break out, instead cede, show sign of submission, and every now and then bring a gift to that person with a higher chance and frequency of giving gift if the person has high authority

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
		//int hungry_child_index = -1;
		
		//these bools need to preserve their state outside each execution of the utility_function
		
		bool clean_image = false; //used by a function to return to default image on the next update

		vector<string> function_record;//useful for more than debugging, such as determining if did x within the last day (20 updates)
		vector<int> function_record_nums;//tracks how many times in a row each function was called to reduce size of function_record. Might make more sense combining as a struct of {record, times repeated}
		
		int radiusmax = -1;//largest radius for searching map. Is set or reset in relevant function

		vector<int> found_messages;//message id's
		map<string, vector<Position>> search_results;//for results of function find_all()

		bool printed = false;//for function record printing. set to true to prevent execution

		bool adopt_spouse_campsite = false;

		bool search_active = false;//a shared, generic search flag that reduces erratic movement by having a single search pattern serve all current targets
		Position general_search_dest;//this and search_active should be together in a function object
		bool end_search = false;

		bool move_already = false;//ensures only move once per update
		bool general_search_called = false;//ensures search is the last move considered

		bool saw_rabbit_recently = false;//resets every 3 days
		int day_I_saw_rabbit = -1;
		vector<trap_check> traps_set;
		Position dropsite;//for trap

		//this lock didn't work, caused person to freeze
		bool immobile = false;//a lock on movement, only released by same function that locked it

		bool monument_unlocked = false;//choice of stonehenge/great temple/etc
		int num_fights_won = 0;//net number of fights, so lost fights get subtracted
		int authority=0;//the main goal of the game, to achieve x level of authority. unsure about the formula. Authority = (number of people who like you * amount they like you) * number of fights won * number of people dominated
		//player must be incentivized to both start and win fights, get many people to like him, get them to like him a lot, and use those people to both win more fights and get more likes and people. As well as winning fights in a way that causes enemies to submit and give tribute rather than just beating on the same enemy person many times or killing enemies
		//once x level of authority is met, can build a monument that when completed wins the game. Other npcs should be pursuing increasing their own authority and building a monument as well, and attacking the play if the player tries to build a monument.
		//getting tribute from having both high authority and many people submissive to oneself also allows spending the time of self and allies on time and resource costly actions such as crafting weapons, armor, walls, larger tents/clothing/mining/art that grant authority boosts, etc.
		
		//having these as sets would be useful for avoiding duplicates, but sets cause problems regarding access and ordering so for now vectors are better
		vector<int> active_hostile_towards;//Person id's of people currently fighting against, for knowing which enemies to go after
		vector<int> hostile_towards;//Person id's of people who were in this fight but might be downed, killed, surrendered. For accounting when battle ends.
		vector<int> combat_allies;//Person id's of people in current fight fighting on my side
		progress_state fight_prog = {4};//fix this, maybe have delay be affected by dice roll, such that if one's dice roll is a lot larger or a lot smaller than that of opponent, fight is over faster. Longest fight is when tied.
		int dice=-1;
		bool downed = false;

		progress_state pregnancy = {10};
		bool friend_camp_check = false;//used for searching for new campsite, check near friend first but only once
	
		People::Position lastpos;//for debugging find_all_helper. Unsure if actually bugged.
	};

	//note: given how the acquire function is structured, berrybush is given priority over bread. Don't know if this is affected by order of which is searched for first or if the depth of the crafting chain has affect on which is chosen. To encourage farming/crafting/etc, need to bias choices towards things like bread. 
	
	//vectors use more memory than necessary? Need to check
	static int pday_count;
	static int phour_count;
	static int people_id_iterator;
	int message_id_iterator = 0;
	static vector<Person> pl;//people_list
	static vector<Message> message_list;
	static vector<int> Message_Map[Environment::map_y_max][Environment::map_x_max];//a map layer that holds messages by id according to their tile location. Makes clearing the map easier by being a separate layer. Each tile can have multiple messages.
	bool message_clear_flag = false;
	int campsite_distance_search = 5;

	People();
	People(int initint);
	bool check_death();
	void update_all(int day_count, int hour_count, int hours_in_day);
	void update(int day_count, int hour_count, int hours_in_day);
	void utility_function();
	void find_all();
	void find_all_helper(Position pos, string type);
	bool move_to(Position pos, string caller);//string is the intended action calling move_to, such as hunting deer
	Position walk_search_random_dest(); //returns a random destination for a random walk
	vector<int> remove_dup(vector<int> v);//to remove duplicates from vector but preserve original order
	
	//find a way to make these 2 functions automatic rather than having to be manually called
	int new_person_id();
	int new_message_id();

	//can these 2 functions be merged?
	int p_by_id(int id);//returns index of person in people list (pl).
	int message_by_id(int id);//returns index of message in message list.

	void add_func_record(string s);
	vector<int> inventory_has(string target);
	void create_item(string item_type, Position pos);
	void pick_up_item(int item_id, Position pos);
	void delete_item(int item_id, Position pos, int index);

	bool search_for_new_campsite();
	bool idle();
	bool sleeping();
	bool eating();
	bool reproduce();
	bool drinking();
	//void carry_infant();		//need generic carry function
	//void drop_infant();

	void speak(string message_text, int receiver_id);
	bool craft(string product);//need to add some method of slowing it down, or else might craft multiple items within a single update. Maybe create an action flag that allows only a single action (walk/craft/speak/etc) per update)
	bool drop_item(int index);//not done
	void general_search_walk(string target);
	void check_tile_messages(Position pos);
	bool acquire(string target);

	vector<Position> filter_search_results(string target);

	bool answer_item_request();
	bool hunting(string species);
	bool cut_down_tree();

	bool child_birth();

	void change_disposition(int p_id, int amount, string reason);//wrapper for changing disposition, might be useful to keep a record of all changes
	void chat();//random chance to compliment or insult
	void authority_calc();
	bool fight();//returns true fight is over, false otherwise. DO THIS: Need to figure out a way to have fights affect disposition of 3rd parties, maybe shrain_disposition is enough? The idea is to encourage player to fight common enemies of friends and not attack the friends of friends or at least weigh the consequences.
	void share_disposition(int p_ind);
	void build_monument();//game victory condition. Player must complete before any NPC
	bool give_tribute();//if submissive to someone, chance of giving them tribute, chance increased according to their authority
	bool rebel();//chance to remove submissive status towards someone
	bool hostile_detection();//detects if someone nearby is hostile to self
	//might make sense to create a relationship struct to hold all data regarding one person's relation to another, including dispositions, history of interactions and disp changes, familial ties, formal ties (boss, employee), submission, etc
	//bool disposition_chance_check(int p2id);//unsure about use cases. checks probability of an action towards someone based on how they're liked. d=100 = always will, d=0= 50/50 chance, d=-100=never will. Could be used as inverse as well for negative actions. So always will do positive actions such as give food to liked people, but also always will do negative action such as insult to disliked people.
};

#endif

//DO THIS:
/*
* Balance:
Rate of plant respawning per day + death rate due to childbirth, sickness, old age
									=
				Plants eaten per person per day + rate of birth

*/

//TO DO NOW:
// need to fix why the campsite restriction and trigger to move isn't working
// to optimize, need to cache more data to reduce function calls

//add associated behavior (avoidance, escalation, noticing a family member being attacked and going to defend without being asked, etc)
	// add disposition checks to actions such as give item so as not to give items to people who are disliked or to give items based on a percent chance affected by disposition level, or higher chance of setting up camp near someone liked or the inverse if disliked, same for marriage and item requests


//note: convert vectors to map or sets? whatever has the fastest lookup

//note: add more methods to cooperate be it hunting, gathering, building, partying, etc. Add methods to delegate tasks between npcs and maybe specialize tasks and roles as well, especially men for hunting and fighting, etc. Maybe one guy for butchering, another for pottery/basket making, another for bow/spear making or art/carvings. Skills can be add to aid in selecting who to delegate to. 

//note: what about handling the satisfaction of multiple needs at once? Such as if both thirsty and hungry, a watermelon would be ideal, rather than simply seeking out water then bread because each was handled independently?

//note: to add speed and better movement between tiles, Person must hold their pixel position relative to the tile they are on until a threshold is passed midway between tiles and the npc is tied to the next tile.

//fix this, to make movement less random, have npc remember the last x amount of tiles seen and move towards tiles not yet seen. Also, to get past obstacles, have direction and always turn right or left. Also, within npc's visual range, if path is blocked x tiles away, head for first empty tile to the right or left instead of waiting to turn when next to obstacle. If walking randomly and come across an obstacle, can also just choose a new random destination to go in another direction

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