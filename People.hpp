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

	//These are currently in the UtilityFincs.cpp file
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

	struct equipped_items {//item id's
		map<string, int> equipment = {
			{"left_hand_holding_eq", -1},//item/shield/bag
			{"right_hand_holding_eq", -1},//item/weapon/bag
			{"person_carried_eq", -1},//person id for carrying infants, sick, injured, and prisoners
			{"head_wearing_eq", -1},//helmet, hat
			{"eyes_wearing_eq", -1},//glasses
			{"neck_wearing_eq", -1},//necklace
			{"shoulders_wearing_eq", -1},//pauldrons, cape
			{"torso_wearing_eq", -1},//shirt, tunic, poncho, jacket
			{"arms_wearing_eq", -1},//sleeves
			{"legs_wearing_eq", -1},//greaves/pants
			{"feet_wearing_eq", -1}//shoes/sandals
		};
		

		bool equip(int item_id) {
			ItemSys::Item& item = ItemSys::item_list[ItemSys::item_by_id(item_id)];
			for (string t : item.tags) {
				if (t.find("_eq") != -1) {//find the tag that relates to where it is worn
					for (auto& i : equipment) {
						if (i.first == t) {
							if (i.second != -1) {//slot is already filled
								return false;//failed to equip
							}
							else {
								i.second = item_id;
								return true;//item successfully equipped
							}
						}
					}

				}
			}
			return false;//item is invalid for equipping
		}

		int unequip(string slot) {//return item_id
			for (auto& i : equipment) {
				if (i.first == slot) {
					int item_id = i.second;
					i.second = -1;//item unequipped
					return item_id;
				}
			}
			return -1;//slot is already empty
		}
	};

	//note: when optimizing, convert for loops which are only for lookup into binary search lookups

	struct Person {//Need to organize/break up Person to be easier to read and improve
		int id; //need to figure out a way to make new id automatic rather than a parameter, currently using an int but not automatic enough
		Position pos;
		bool sex; //true==male, false==female

		string name = "no name";
		string current_image = "pics/human.png";

		equipped_items equipped;

		//string current_state = "idle"; //state/action
		int sightline_radius = 5; //how far around self can see objects
		int audioline_radius = 5; //how far around self can hear and be heard
		int hunger_level = 0;
		int hungry_time = 0;
		int thirst_level = 0;
		bool awake = true;
		int tired_level = 0;//need to add exertion and tie somehow to hunger rate, such that someone who is sedentary gets tired and hungry slower than someone running and fighting and mining, etc
		vector<int> item_inventory; //holds id's of items in inventory		//note: when equipping/unequipping, the item stays in the inventory. Later migth be better to have equipment serve as a separate inventory such that if an item is equipped, it is removed from the reular inventory. Having separate inventories can work for bags as well so that bags can be dropped with all their contents still inside.

		int recreation_level = 0;
		bool beauty_need_satisfied = false;

		map<string,progress_state> progress_states;
		int protected_func = -1;
		progress_state drinking = { 4 };

		bool walk_flip = false;
		string speed_type="walking";
		int speed = 4;
		bool in_stealth = false;//if this is true, animal should have chance of not seeing person. Later, when line of sight and obstructions are implemented, then have person in stealth attempt to go from cover to cover and only when animal is facing away, on approach
		int sprint_stamina = 0;//for limiting sprint time
		int animal_id_ambush = -1;//to check if the animal being stalked and chased is no longer in range, to return to regular walk speed. 
		//DO THIS NOW: Need to implement a method for certain actions/functions to both cache their data such as current target prey, and increase their priority such that ambushing prey can't be interrupted by anything other than being injured/attacked/etc

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

		int px_x = 0;
		int px_y = 0;
		bool mov = false;
		Position dest;

		Position last_position;//for setting direction of tracks

		int dirtiness;//how dirty one is, increases over time or all at once under certain circumstances such as when walking in mud, etc.
		progress_state bathing = { 4 };

		int my_temperature=75;
		int my_preffered_temperature=75;

		int time_waited = 0;

		Position active_fish_hook_pos;

		int sick_time = 0;
		bool am_sick = false;

		int injured_time = 0;
		bool am_injured = false;


		bool player_trigger = false;

		int num_people_liked_by = 0;//set all factors to 1 to avoid any 0 making authority == 0; For debugging. This worked to allow someone to win the game. 
		int amount_liked = 0;
		int num_submissives = 0;

		Position main_dest;

		bool dumping_not_done = false;
	};

//=====================================================================================================================
	//People.cpp Functions:
	People();
	People(int initint);
	bool check_death();
	void update_all(int day_count, int hour_count, int hours_in_day);
	void update(int day_count, int hour_count, int hours_in_day);
	void utility_function();

	bool search_for_new_campsite();
	bool idle();
	bool sleeping();
	bool reproduce();
	bool beauty();
	bool need_light();
	bool health();
	void speak(string message_text, int receiver_id);
	bool craft(string product);//need to add some method of slowing it down, or else might craft multiple items within a single update. Maybe create an action flag that allows only a single action (walk/craft/speak/etc) per update)
	bool acquire(string target);
	bool answer_item_request();
	bool cut_down_tree();//should be made generic for interacting with an adjacent item (converting tree into wood, producing mineral item from mineral vein without deleting the latter (but depleting), using stations, carving/decorating/vandalizing an item, etc) 
	bool recreation();
	bool child_birth();
	bool hygiene();
	bool exposure();

	void chat();//random chance to compliment or insult
	void authority_calc();

	void build_monument();//game victory condition. Player must complete before any NPC
	
	//do these count as Disposition funcs?
	bool give_tribute();//if submissive to someone, chance of giving them tribute, chance increased according to their authority
	bool rebel();//chance to remove submissive status towards someone
	
	bool inventory_dump();//if inventory is full, go back to camp and dump items. Encourages use of storage items and inventory expansion items as well as creates a better, cluttered, lived in environment
//=====================================================================================================================
	//Animation.cpp Functions:


//=====================================================================================================================
	//UtilityFuncs.cpp Functions:
	//find a way to make these 2 functions automatic rather than having to be manually called
	static int new_person_id();
	int new_message_id();

	//can these 2 functions be merged?
	int p_by_id(int id);//returns index of person in people list (pl).
	int message_by_id(int id);//returns index of message in message list.

	void add_func_record(string s);
	vector<int> inventory_has(string target);
	void create_item(string item_type, Position pos);
	void pick_up_item(int item_id, Position pos);
	void delete_item(int item_id, Position pos, int inventory_index);

	//need generic carry function

	void pickup_infants();//need to improve this function, temporary implementation. 
	bool drop_infants();//need to merge this with pickup_person?

	void find_all();
	void check_tile_messages(Position pos);
	vector<int> remove_dup(vector<int> v);//to remove duplicates from vector but preserve original order
	bool drop_item(int index);//not done
	vector<Position> filter_search_results(string target);
//=====================================================================================================================
	//Subsistence.cpp Functions:
	bool eating();
	bool drinking();
	bool hunting(string species);
	string choose_hunting_method(string species);
	bool tracking(string species);
	bool persistence_and_pick_up(string species, Animal::animal& a);
	bool ambush(string species, Animal::animal& a);
	bool angling(string species, Animal::animal& a);
	bool trap(string set_or_check, string species, Animal::animal& a);

//=====================================================================================================================
	//Movement.cpp Functions:
	//move_to should be modified to accept a distance variable such that it returns true when it reaches a given distance from the target, such as for going adjacent to a target
	bool move_to(Position pos, string caller);//string is the intended action calling move_to, such as hunting deer
	Position walk_search_random_dest(); //returns a random destination for a random walk
	void general_search_walk(string target);


//=====================================================================================================================
	//Combat.cpp Functions:			Ideally, hunting and combat should overlap, unsure how to do so?
	//fight() needs to be simplified and/or broken up into smaller functions
	bool fight();//returns true fight is over, false otherwise. DO THIS: Need to figure out a way to have fights affect disposition of 3rd parties, maybe shrain_disposition is enough? The idea is to encourage player to fight common enemies of friends and not attack the friends of friends or at least weigh the consequences.
	bool hostile_detection();//detects if someone nearby is hostile to self


//=====================================================================================================================
	//Disposition.cpp Functions:
	void share_disposition(int p_ind);
	void change_disposition(int p_id, int amount, string reason);//wrapper for changing disposition, might be useful to keep a record of all changes
	//might make sense to create a relationship struct to hold all data regarding one person's relation to another, including dispositions, history of interactions and disp changes, familial ties, formal ties (boss, employee), submission, etc
	//bool disposition_chance_check(int p2id);//unsure about use cases. checks probability of an action towards someone based on how they're liked. d=100 = always will, d=0= 50/50 chance, d=-100=never will. Could be used as inverse as well for negative actions. So always will do positive actions such as give food to liked people, but also always will do negative action such as insult to disliked people.


//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
	//Class Contants and Static Variables:
	
	static ItemSys it2;
	static Animal anim1;//to access Animal class functions


	bool game_won = false;//for halting updates to check if game is winnable, for debugging

	//need to place all variables in custom data types such that they are gaurunteed to be constrained between a certain range to avoid invalid values?
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

	static int p;

//balancing numbers. Everything should be relative to the length of a day, so need x calories per day, x water per day, etc.
//number one consumer of time is getting from A to B. Affected by density and distance of needed resources/people/places
//second consumer is amount of time in animation (crafting, sleeping, etc)
//third is number and frequency of tasks
	//currently there are 40hrs in a day
	int DAYLENGTH = 24*4;//measured in ticks, this is temporary, it should be set by Game class. Used to make other constants proportional to length of day
	int HOURLENGTH = DAYLENGTH / 24;

	int HUNGER_REDUCTION_RATE = 50;//per tick for now
	int HUNGRY_LEVEL = HUNGER_REDUCTION_RATE*DAYLENGTH;//assume only need 1 meal per day
	int DAYS_HUNGRY_MOVE_CAMP = 3;
	int STARVATION_LEVEL = HUNGRY_LEVEL*21;//record is 61 days no food, general max is 30 days no food, average is 8 to 21 days no food. For now set to 8

	int THIRST_REDUCTION_RATE = 25;//per tick for now
	int THIRSTY_LEVEL = THIRST_REDUCTION_RATE*DAYLENGTH;//assume only need one drink per day
	int DEHYDRATION_LEVEL = THIRSTY_LEVEL*3;//rule of thumb is survive without water for 3 days, not sure about average or max record, set to 3 days for now

	int MAX_INFANT_AGE = 5;
	int MIN_ADULT_AGE = 10;
	int MAX_AGE = 50;


	int REPRODUCTION_TRIGGER = DAYLENGTH*7;//once every 7 days

	int spouse_distance_limit = 10;

	int	HATED_THRESHOLD = -75;
	int DISLIKE_THRESHOLD = -25;
	int LIKE_THRESHOLD = 25;
	int LOVED_THRESHOLD = 75;

	int NEW_CAMP_PROBATION_TIME = DAYLENGTH/3;//a third of a day
	int NEW_CAMP_CLOSE_TO_FRIEND = 10;//max distance to friend's camp

	int SLEEP_REST_RATE = 11;
	int SLEEP_TRIGGER = SLEEP_REST_RATE*((2*DAYLENGTH)/3);//sleep every 2/3rds of a day
	int FORCE_SLEEP_LEVEL = SLEEP_TRIGGER*2;//record is 11 days of no sleep, but extreme symptoms start at 36 hours (1.5 days). For now just set at 2x sleep trigger

	int MIN_EXTRA_FOOD_IN_INVENTORY = 2;

	int campsite_distance_search = 5;//should be half a day's walk from camp, fix this

	int RECREATION_TRIGGER = 500;

	int HYGIENE_TRIGGER = 500;

	//humidity lowers heat tolerance such that at 100% humidity, humans die at 95F of heat in 6 hours. Need to implement humidity.
	int HEAT_DEATH_TEMPERATURE = 110;//max heat that can be survived is between 104 and 122 F. For now set at 110F;
	//need to implement a time component to surviving temperatures. 
	//40 to 50 degrees can cause death in one to three hours. 32 to 40 degrees can cause death in 30 to 90 minutes. 32 degrees or less can cause death in as little as 15 to 45 minutes.
	int COLD_DEATH_TEMPERATURE = 50;//measured in F

	int SPRINT_LIMIT = 20;


	static int TILE_PIXEL_SIZE;
	static int WALK_SPEED;
	static int STEALTH_SPEED;
	static int SPRINT_SPEED;
	int AMBUSH_DISTANCE = 3;
	int STEALTH_DISTANCE = 6;//distance from prey at which person enters stealth
	int ANGLING_WAIT_TIME = 20;

	int SICK_TIME_DEATH = 250;//later replace by specific illness/injury and body part mechanics such as bleeding out and organ failure
	int INJURED_TIME_DEATH = 250;
};

#endif

//Need to add a way to pull data from a spreadsheet for items, images, stats, etc


//note: is stamina the same as tired level? what about the difference between long distance endurance and ability for short sprints (humans are slower than cheetahs but run far longer)

//note: skills have not been implemented for anything, nor knowledge of how to do something, nor memory to remember events and locations of items, etc.

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