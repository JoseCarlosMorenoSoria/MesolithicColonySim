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
#include "ProjUtil.hpp"
#include <stdexcept>
#include "Plants.hpp"
using namespace std;
using namespace proj_util;

class Animal{
public:
	//Animals only send food request and alarm messages. Humans have a far wider variety of Messages
	struct Message {//need to tie Messages to tiles in map instead of current Message list
		int message_id = -1;//is map<> more efficient than keeping in a vector?
		int sender_id = -1;
		string species;//for distinguishing between humans and animals
		int reciever_id = -1; //-1 is a message for everyone/anyone
		string messsage = "";
		Position origin;
	};
	
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

	//note: when optimizing, convert for loops which are only for lookup into binary search lookups

	bool a_p_flip = false; //a==true, p==false, for sharing functions between animal and Person

	struct animal{//Need to organize/break up to be easier to read and improve
		string species;//deer, rabbit, wolf, human, etc
		vector<string> components;//bones, antlers, fur, hair, skull, etc
		int a_id=-1; //need to figure out a way to make new id automatic rather than a parameter, currently using an int but not automatic enough
		Position pos;
		bool sex; //true==male, false==female
		string current_image;
		bool fleeing = false;
		//need to implement flee mechanic to animal and make animal faster than human but slow to react (later it should be slow to accelerate not react?)
		//string current_state = "idle"; //state/action
		int sightline_radius = 5; //how far around self can see objects
		int audioline_radius = 5; //how far around self can hear and be heard
		int hunger_level = 0;
		int hungry_time = 0;
		int thirst_level = 0;
		bool awake = true;
		int tired_level = 0;//need to add exertion and tie somehow to hunger rate, such that someone who is sedentary gets tired and hungry slower than someone running and fighting and mining, etc
		//animals do not have recreation need for now, thought maybe later could be added for pets? And have child animals simply have random playful acts?
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
		//for herds? or simple congregating behavior is enough. humans should have a different implementation
		//vector<int> group_members; //holds id's of group members
		bool is_alive = true;
		//can serve for nesting behaviors such as bird nests and bear dens
		Position campsite_pos = {-1,-1};
		int campsite_age = -1; //used to track how long have lived at that campsite since it was placed to provide a bit of breathing room before removing to find another campsite
		int reproduction_cooldown = 0; 
		//if animal is monogamous
		int spouse_id = -1;
		//for feeding children
		vector<int> children_id;
		int age = 0;
		//for carrying children or being carried by a person (ex: a chicken that got caught)
		bool being_carried = false;
		int carried_by_id = -1;
		//objects for tracking function states
		progress_state eating_progress = {4};//4 is the number of ticks/frames until progress is done
		int eating_food_index = -1;//this and eating_progress should be encapsulated in a function object
		//animals have limited building (not crafting) such as beaver dams and nests including beehives, termite mounds, etc
		//these bools need to preserve their state outside each execution of the utility_function
		bool clean_image = false; //used by a function to return to default image on the next update
		int radiusmax = -1;//largest radius for searching map. Is set or reset in relevant function
		//animal messages are for feeding kids and sounding alarm about threats, as well as detecting sounds and smells
		vector<int> found_messages;//message id's
		map<string, vector<Position>> search_results;//for results of function find_all()
		//for nesting behavior?
		bool adopt_spouse_campsite = false;
		bool search_active = false;//a shared, generic search flag that reduces erratic movement by having a single search pattern serve all current targets
		Position general_search_dest;//this and search_active should be together in a function object
		bool end_search = false;//unsure if this is still used
		bool move_already = false;//ensures only move once per update
		bool general_search_called = false;//ensures search is the last move considered
		//this lock didn't work, caused person to freeze
		bool immobile = false;//a lock on movement, only released by same function that locked it
		//having these as sets would be useful for avoiding duplicates, but sets cause problems regarding access and ordering so for now vectors are better
		//for animal combat against each other and against people
		vector<int> active_hostile_towards;//Person id's of people currently fighting against, for knowing which enemies to go after
		progress_state fight_prog = {4};//fix this, maybe have delay be affected by dice roll, such that if one's dice roll is a lot larger or a lot smaller than that of opponent, fight is over faster. Longest fight is when tied.
		int dice=-1;
		bool downed = false;
		progress_state pregnancy = {10};
		Position lastpos;//for debugging find_all_helper. Unsure if actually bugged.
		int px_x = 0;//make these a Position?
		int px_y = 0;
		bool mov = false;
		Position dest;
		int my_temperature=75;
		int my_preffered_temperature=75;
		int sick_time = 0;
		bool am_sick = false;
		int injured_time = 0;
		bool am_injured = false;
		bool player_trigger = false;//kept here because player might need to trigger animal inherited functions
		Position main_dest;
		//animals should be able to carry and drop, to steal food and bring it to their kids. 
		//But not pickup because they have no inventory, nor push because it's not needed. 
		//However there is the case of pack animals and draft animals, so not sure yet.
		Position food_to_eat;//for animals
	};
	static vector<int> animals_in_stealth;//used so that Animal knows if any person is in stealth
	//need to implement a parasite, disease and injury system with types, triggers effects on NPC, and methods of healing. For an animal, the only treatment is rest.
	//childbirth should involve risks of maternal mortality and infants/children/elderly should be at higher risks of disease deaths.
	//diseases/injuries should have varying degrees of both severity and time to death and chance of dying. Need to add hereditary immunity chances and disease mutations to create cycles of epidemics and health.
	//some conditions should be chronic or from birth, such as asthma, blindness, etc.
//Need to fix such that Animal refers to Animal versions of the following functions and People only refers to its own versions when not using Animal's
//=====================================================================================================================
	//People.cpp Functions:
	Animal();
	Animal(int init);
	bool check_death();
	void update_all(int day_count, int hour_count, int hours_in_day);
	void update(int day_count, int hour_count, int hours_in_day);
	void utility_function();
	bool search_for_new_campsite();
	bool idle();//pull out socialize from idle and make it a need
	bool sleeping();
	bool reproduce();
	bool need_light();
	bool health();
	void speak(string message_text, int receiver_id);
	bool acquire(string target);
	bool answer_item_request();//only for giving kids food, but there's probably a simpler version if that's all an animal will do
	bool child_birth();
	bool exposure();

	//new funcs
	bool carry();//item or person, should merge with pick_up_item() though this is for carrying not placing in one's inventory
	bool drop();//item or person, should merge with drop_item() though this is just for ending carry();
	bool adjacency_acquire_handler();//for cutting down trees, mining rock, digging out dirt, collecting water, etc

//=====================================================================================================================
	//Animation.cpp Functions:


//=====================================================================================================================
	//UtilityFuncs.cpp Functions:
	//find a way to make these 2 functions automatic rather than having to be manually called
	static int new_animal_id();
	int new_message_id();

	//can these 2 functions be merged?
	int a_by_id(int a_id);//returns index of person in people list (pl).
	int message_by_id(int id);//returns index of message in message list.

	//void create_item(string item_type, Position pos);		need to make a different animal version
	//void delete_item(int item_id, Position pos, int inventory_index);		same as above, because animal doesn't have an inventory unlike Person

	//need generic carry function

	//have only humans pickup infants for now, if animal abandons nest, animal infants should get abandoned
	//void pickup_infants();//need to improve this function, temporary implementation. 
	//bool drop_infants();//need to merge this with pickup_person?

	void find_all();
	void check_tile_messages(Position pos);
	vector<Position> filter_search_results(string target);
//=====================================================================================================================
	//Subsistence.cpp Functions:
	bool eating();
	bool drinking();
	//For now, to keep things simple, don't have predatory animals, implement later
	//bool hunting(string species);
	//string choose_hunting_method(string species);
	//bool tracking(string species);
	//bool persistence_and_pick_up(string species, Animal::animal& a);
	//bool ambush(string species, Animal::animal& a);

//=====================================================================================================================
	//Movement.cpp Functions:
	//move_to should be modified to accept a distance variable such that it returns true when it reaches a given distance from the target, such as for going adjacent to a target
	bool move_to(Position pos, string caller);//string is the intended action calling move_to, such as hunting deer
	Position walk_search_random_dest(); //returns a random destination for a random walk
	void general_search_walk(string target);


//=====================================================================================================================
	//Combat.cpp Functions:			Ideally, hunting and combat should overlap, unsure how to do so?
	//fight() needs to be simplified and/or broken up into smaller functions
	//bool fight();//returns true fight is over, false otherwise.
	//bool hostile_detection();//detects if someone nearby is hostile to self
	//Leave Animal combat for later


//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//Class Contants and Static Variables:
	static ItemSys it2;
	//convert int's to bint's to automatically constrain to a range
	//vectors use more memory than necessary? Need to check
	static int pday_count;
	static int phour_count;
	static int animal_id_iterator;
	int message_id_iterator = 0;
	static vector<animal> al;//animal_list
	static vector<Message> message_list;
	static vector<int> Message_Map[Environment::map_y_max][Environment::map_x_max];//a map layer that holds messages by id according to their tile location. Makes clearing the map easier by being a separate layer. Each tile can have multiple messages.
	bool message_clear_flag = false;
	static int a;//animal index of currently updating animal

//balancing numbers. Everything should be relative to the length of a day, so need x calories per day, x water per day, etc.
//number one consumer of time is getting from A to B. Affected by density and distance of needed resources/people/places
//second consumer is amount of time in animation (crafting, sleeping, etc)
//third is number and frequency of tasks
	//currently there are 40hrs in a day
	int DAYLENGTH = 24*4;//measured in ticks, this is temporary, it should be set by Game class. Used to make other constants proportional to length of day
	int HOURLENGTH = DAYLENGTH / 24;
	static int TILE_PIXEL_SIZE;
	//these are currently the Human Constants
	struct Species {//holds constants for each species
		string species;//deer, rabbit, wolf, human, etc
		string diet;
		string meat_type;
		int HUNGER_REDUCTION_RATE;//per tick for now
		int HUNGRY_LEVEL;//assume only need 1 meal per day
		int DAYS_HUNGRY_MOVE_CAMP;
		int STARVATION_LEVEL;//record is 61 days no food, general max is 30 days no food, average is 8 to 21 days no food. For now set to 8

		int THIRST_REDUCTION_RATE;//per tick for now
		int THIRSTY_LEVEL;//assume only need one drink per day
		int DEHYDRATION_LEVEL;//rule of thumb is survive without water for 3 days, not sure about average or max record, set to 3 days for now

		int MAX_INFANT_AGE;
		int MIN_ADULT_AGE;
		int MAX_AGE;

		int REPRODUCTION_TRIGGER;//once every 7 days

		int spouse_distance_limit;//something like this could serve to keep packs/herds close together?

		int NEW_CAMP_PROBATION_TIME;//a third of a day

		int SLEEP_REST_RATE;
		int SLEEP_TRIGGER;//sleep every 2/3rds of a day
		int FORCE_SLEEP_LEVEL;//record is 11 days of no sleep, but extreme symptoms start at 36 hours (1.5 days). For now just set at 2x sleep trigger

		int campsite_distance_search;//should be half a day's walk from camp, fix this

		//humidity lowers heat tolerance such that at 100% humidity, humans die at 95F of heat in 6 hours. Need to implement humidity.
		int HEAT_DEATH_TEMPERATURE;//max heat that can be survived is between 104 and 122 F. For now set at 110F;
		//need to implement a time component to surviving temperatures. 
		//40 to 50 degrees can cause death in one to three hours. 32 to 40 degrees can cause death in 30 to 90 minutes. 32 degrees or less can cause death in as little as 15 to 45 minutes.
		int COLD_DEATH_TEMPERATURE;//measured in F

		int SPRINT_LIMIT;

		int WALK_SPEED;
		int STEALTH_SPEED;
		int SPRINT_SPEED;
		int AMBUSH_DISTANCE;
		int STEALTH_DISTANCE;//distance from prey at which person enters stealth

		int SICK_TIME_DEATH;//later replace by specific illness/injury and body part mechanics such as bleeding out and organ failure
		int INJURED_TIME_DEATH;


		//Not yet added to CSV
		//skull/long bones/tusk/antler/fur/hide/wool/hair/blood/sinew/intestines/liver/teeth/claws/short bones/fat/blubber/poison/etc
		vector<string> components;//bones, antlers, fur, hair, skull, etc
		//components need to be their own items
		//calories when killed
		//hunting methods
		//actions: ram/bite/trample/etc
		//migration/territoriality
		//can fly?
		//can swim?
		//can burrow?
		//breathes water/air?
		//other roles: alarm/guard/war dog, draft animal, ridable, guide(honey / hound), messenger pigeon, hunting aid(dog / falcon)
		//pack animal? carry capacity
		//domesticated?
		//tameable?
		//social type
		//fight/flight 

	};
	void fill_species_presets();
	static map<string,Species> species;
	
};

#endif
//note: is stamina the same as tired level? what about the difference between long distance endurance and ability for short sprints (humans are slower than cheetahs but run far longer)

//DO THIS:
/*
* Balance:	needs to handle total animal+human population
Rate of plant respawning per day + death rate due to childbirth, sickness, old age
									=
				Plants eaten per person per day + rate of birth

*/

//TO DO NOW:
// need to fix why the campsite restriction and trigger to move isn't working
// to optimize, need to cache more data to reduce function calls
//note: convert vectors to map or sets? whatever has the fastest lookup
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





