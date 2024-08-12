#ifndef People_hpp
#define People_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include "Animal.hpp"
class People : public Animal {
public:
	//Humans inherit Messages but have a far wider variety of Messages than Animal

	//only humans lay traps they check later, animal traps (if ever added) are more about waiting near the trap and only one trap at a time
	struct trap_check {
		Position pos;
		int last_day_checked = -1;
	};

	//Only human social dynamics will be handled for now
	struct submit_tracker {
		bool submissive_to = false;
		int fights_lost = 0;//if net fights lost to this person is equal or greater than 3, become submissive to that person
	};

	//this is for Humans only, need to figure out a system for pack animals to have an inventory
	struct equipped_items {//item id's
		map<string, int> equipment = {
			{"left_hand_holding", -1},//item/shield/bag
			{"right_hand_holding", -1},//item/weapon/bag
			{"person_carried", -1},//person id for carrying infants, sick, injured, and prisoners
			{"head", -1},//helmet, hat
			{"eyes", -1},//glasses
			{"neck", -1},//necklace
			{"shoulders", -1},//pauldrons, cape
			{"torso", -1},//shirt, tunic, poncho, jacket
			{"arms", -1},//sleeves
			{"legs", -1},//greaves/pants
			{"feet", -1}//shoes/sandals
		};

		bool equip(int item_id) {
			ItemSys::Apparel& item = *static_cast<ItemSys::Apparel*>(it2.item_list[item_id]);//FIX THIS, need to allow equipping non apparel (holding random item in hand such as weapon)
			
			for (auto& i : equipment) {
				if (i.first == item.body_part) {
					if (i.second != -1) {//slot is already filled
						return false;//failed to equip
					}
					else {
						i.second = item_id;
						return true;//item successfully equipped
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

	struct Person : animal {
		int p_id;//humans need there own id system apart from that of animals, humans should not get assigned an animal id
		string name = "no name";//will need a csv with names and a name generator function
		equipped_items equipped;
		//current_image = "pics/human.png"; this is set by the Human preset from species_presets
		vector<int> item_inventory; //holds id's of items in inventory		//note: when equipping/unequipping, the item stays in the inventory. Later migth be better to have equipment serve as a separate inventory such that if an item is equipped, it is removed from the reular inventory. Having separate inventories can work for bags as well so that bags can be dropped with all their contents still inside.
		int recreation_level = 0;
		bool beauty_need_satisfied = false;
		//vector<int> group_members; //holds id's of group members
		map<int, int> dispositions;//{Person_ID, favorability} holds id's of known people and whether and how much one likes or dislikes each
		map<int, submit_tracker> submissive_to;//{Person_ID, submissive_to/Not submissive_to} tracks people who have won a fight against self at least 3 times (them being present as a hotile during a fight that was lost against someone else counts)
		//being submissive to another means when a fight would normally break out, instead cede, show sign of submission, and every now and then bring a gift to that person with a higher chance and frequency of giving gift if the person has high authority
		//only Humans can craft, because it requires an inventory. Animals can only build.
		map<string, progress_state> crafting;//key is product name		fix this: need to avoid case of progress saved, all ingredients lost (instead of being used up by starting the crafting process), new ingredients obtained, then picking up progress where it was last left off instead of starting over again
		//unsure if this is needed, animals don't need it because they only feed their kids and not others
		bool child_is_hungry = false;//currently a flag to communicate between feed_own_children and searching_for_food and gathering_food. I need to rethink how functions communicate with each other and facilitate direct calls by one to another. Fix this.
		//int hungry_child_index = -1;
		//only keep records for humans
		vector<string> function_record;//useful for more than debugging, such as determining if did x within the last day (20 updates)
		vector<int> function_record_nums;//tracks how many times in a row each function was called to reduce size of function_record. Might make more sense combining as a struct of {record, times repeated}
		bool printed = false;//for function record printing. set to true to prevent execution
		vector<trap_check> traps_set;
		Position dropsite;//for trap
		bool monument_unlocked = false;//choice of stonehenge/great temple/etc
		int num_fights_won = 0;//net number of fights, so lost fights get subtracted
		int authority = 0;//the main goal of the game, to achieve x level of authority. unsure about the formula. Authority = (number of people who like you * amount they like you) * number of fights won * number of people dominated
		//player must be incentivized to both start and win fights, get many people to like him, get them to like him a lot, and use those people to both win more fights and get more likes and people. As well as winning fights in a way that causes enemies to submit and give tribute rather than just beating on the same enemy person many times or killing enemies
		//once x level of authority is met, can build a monument that when completed wins the game. Other npcs should be pursuing increasing their own authority and building a monument as well, and attacking the play if the player tries to build a monument.
		//getting tribute from having both high authority and many people submissive to oneself also allows spending the time of self and allies on time and resource costly actions such as crafting weapons, armor, walls, larger tents/clothing/mining/art that grant authority boosts, etc.
		//only humans track their allies and downed enemies
		vector<int> hostile_towards;//Person id's of people who were in this fight but might be downed, killed, surrendered. For accounting when battle ends.
		vector<int> combat_allies;//Person id's of people in current fight fighting on my side
		bool friend_camp_check = false;//used for searching for new campsite, check near friend first but only once
		//only humans have hygiene need for now
		int dirtiness;//how dirty one is, increases over time or all at once under certain circumstances such as when walking in mud, etc.
		progress_state bathing = { 4 };
		int time_waited = 0;//for angling
		Position active_fish_hook_pos;
		int num_people_liked_by = 0;
		int amount_liked = 0;
		int num_submissives = 0;
		bool dumping_not_done = false;
	};


	Animal::Species& con = Animal::species["human"];//access to human constants
	static vector<Person> pl;
	static int p;
	static int people_id_iterator;
	static vector<int> people_in_stealth;//used so that Animal knows if any person is in stealth
	//for human healing: Should consider skill of "doctor", severity, cleanliness of patient/wound and of doctor/his hands and the environment.
	//can include treatments such as brain panning, leeches, skin eating maggots, alcohol, tourniquets, etc. 
	//if someone is sick or injured, they should first try self treatment, then ask for help, and if severity increases seek out bed rest before being bedridden.
	/*
	* Need to add:
		have mood
		have comfort (temp/sitting/standing/aesthetics/stink/cramped/etc)
		pawns can be arrested and imprisoned, enslaved, recruited and converted
		have their own sleep spots and rooms
		have a schedule
		can have restrictions on allowed areas, allowed work, allowed food/clothes type
		have skills
		have health and capacities
		have unique appearances
		have traits
		have basic relationships
		have basic work passions
		have bonded pets - pets and farm animals must be cared for such as feeding, temp, etc just like a human child.
	Actions:
		Doctoring: should be an extension of treating self injuries and disease to treating others
		Guarding: guard an area, item or person either protecting or imprisoning. May be responsible for feeding, etc.
		Rescue: person, animal, item, remove them from danger and to a safe place or provide aid, overlaps with doctoring?
		Animal handling: tame wild animals, train animals such as dogs, the rest should be handled by simply treating animals as a child (feeding/protecting) and collecting resources they provide (including slaughtering them).
		Repair/Maintenance: need to include ownership of items, animals, buildings. A person will maintain items such as sharpening knives, or refuelling a campfire/hearth, or repair such as fixing a broken cuirass, patching a hole in the roof or wall, sewing a hole in clothing, etc. Some things cannot be repaired such as if something is too rusted/worn/broken/infested (in the case of a building)/etc and must be thrown out as trash or burned and replaced.
		Planting: harvesting plants should use the same mechanic for harvesting wild ones, the planting behavior should be learned and be wild horticulture first, then crops in various plots of various types. Over time includes weeding, surface scattering, sowing, tilling, watering, irrigation, etc.
		Need to add cleaning and include organizing in it (such that not only is trash thrown in a midden or stockpiled to be thrown away later, but also any belongings that aren't in a container/shelf/etc are placed in the corresponding place.
		Research: most tech/knowledge advancements are context driven and probabalistic such that boat tech is advanced by living near bodies of water and the type of craft according to the type of body of water (coast/river/lake) or better clothing techniques or tools by gaining a certain level of experience making clothes, etc.
		//studying a particular subject is available for some tech/knowledge but requires a lot of time investment by the same person or people sharing study progress and then that knowledge must be shared and taught to others in order to actually use it. It's not a global tech tree that is unlocked but the accumulation of individual knowledge that is shared and preserved.
	*/

	//Some of these functions are human versions of Animal functions, others are unique to Humans
	People();
	People(int init);
	void update_all(int day_count, int hour_count, int hours_in_day);
	void update(int day_count, int hour_count, int hours_in_day);
	void utility_function();
	bool search_for_new_campsite();
	bool idle();
	bool reproduce();
	bool beauty();
	bool need_light();
	bool health();
	bool craft(string product);//need to add some method of slowing it down, or else might craft multiple items within a single update. Maybe create an action flag that allows only a single action (walk/craft/speak/etc) per update)
	bool acquire(string target);
	bool answer_item_request();
	bool recreation();
	bool hygiene();
	bool exposure();

	void chat();//random chance to compliment or insult
	void authority_calc();

	void build_monument();//game victory condition. Player must complete before any NPC

	//do these count as Disposition funcs?
	bool give_tribute();//if submissive to someone, chance of giving them tribute, chance increased according to their authority
	bool rebel();//chance to remove submissive status towards someone

	bool inventory_dump();//if inventory is full, go back to camp and dump items. Encourages use of storage items and inventory expansion items as well as creates a better, cluttered, lived in environment

	//new funcs
	bool extinguish_fire();
	bool carry();//item or person, should merge with pick_up_item() though this is for carrying not placing in one's inventory
	bool drop();//item or person, should merge with drop_item() though this is just for ending carry();
	//add to this: (converting tree into wood, producing mineral item from mineral vein without deleting the latter (but depleting), using stations, carving/decorating/vandalizing an item, etc) 
	bool adjacency_acquire_handler(string target, string type, Position pos);//for cutting down trees, mining rock, digging out dirt, collecting water, etc
	bool coerce();//variation on request()
	//=====================================================================================================================
		//Animation.cpp Functions:


	//=====================================================================================================================
		//UtilityFuncs.cpp Functions:
	static int new_person_id();
	static int p_by_id(int id);//returns index of person in people list (pl).
	static Person& person(int id);
	//void add_func_record(string s);	too many things have changed, need to redo how I track function executions
	vector<int> inventory_has(string target);
	void create_item(string item_type, Position pos);
	void pick_up_item(int item_id, Position pos);
	void delete_item(int item_id, Position pos, int inventory_index);

	//need generic carry function

	void pickup_infants();//need to improve this function, temporary implementation. 
	bool drop_infants();//need to merge this with pickup_person?

	bool drop_item(int index);//not done
	//=====================================================================================================================
		//Subsistence.cpp Functions:
		// For now, only humans can hunt, later inherit from Animal. Need to include options to hunt other humans.
	//Should inherit from Animal, fix //bool drinking();
	bool hunting(string species);
	string choose_hunting_method(string species);
	bool tracking(string species);
	bool persistence_and_pick_up(string species, Animal::animal& a);
	bool ambush(string species, Animal::animal& a);
	bool angling(string species, Animal::animal& a);
	bool trap(string set_or_check, string species, Animal::animal& a);


	//=====================================================================================================================
		//Combat.cpp Functions:			Ideally, hunting and combat should overlap, unsure how to do so?
		//fight() needs to be simplified and/or broken up into smaller functions
	bool fight();//returns true fight is over, false otherwise. DO THIS: Need to figure out a way to have fights affect disposition of 3rd parties, maybe share_disposition is enough? The idea is to encourage player to fight common enemies of friends and not attack the friends of friends or at least weigh the consequences.
	bool hostile_detection();//detects if someone nearby is hostile to self


	//=====================================================================================================================
		//Disposition.cpp Functions:
	void share_disposition(int p_ind);
	void change_disposition(int p_id, int amount, string reason);//wrapper for changing disposition, might be useful to keep a record of all changes
	//might make sense to create a relationship struct to hold all data regarding one person's relation to another, including dispositions, history of interactions and disp changes, familial ties, formal ties (boss, employee), submission, etc
	//bool disposition_chance_check(int p2id);//unsure about use cases. checks probability of an action towards someone based on how they're liked. d=100 = always will, d=0= 50/50 chance, d=-100=never will. Could be used as inverse as well for negative actions. So always will do positive actions such as give food to liked people, but also always will do negative action such as insult to disliked people.


	//=====================================================================================================================
//Class Variables and Constants - Unique to Humans
	bool game_won = false;//for halting updates to check if game is winnable, for debugging
	//note: given how the acquire function is structured, berrybush is given priority over bread. Don't know if this is affected by order of which is searched for first or if the depth of the crafting chain has affect on which is chosen. To encourage farming/crafting/etc, need to bias choices towards things like bread. 
	int	HATED_THRESHOLD = -75;
	int DISLIKE_THRESHOLD = -25;
	int LIKE_THRESHOLD = 25;
	int LOVED_THRESHOLD = 75;
	int NEW_CAMP_CLOSE_TO_FRIEND = 10;//max distance to friend's camp
	int MIN_EXTRA_FOOD_IN_INVENTORY = 2;
	int RECREATION_TRIGGER = 500;
	int HYGIENE_TRIGGER = 500;
	int ANGLING_WAIT_TIME = 20;
	//note: skills have not been implemented for anything, nor knowledge of how to do something, nor memory to remember events and locations of items, etc.
//add associated behavior (avoidance, escalation, noticing a family member being attacked and going to defend without being asked, etc)
	// add disposition checks to actions such as give item so as not to give items to people who are disliked or to give items based on a percent chance affected by disposition level, or higher chance of setting up camp near someone liked or the inverse if disliked, same for marriage and item requests
//note: add more methods to cooperate be it hunting, gathering, building, partying, etc. Add methods to delegate tasks between npcs and maybe specialize tasks and roles as well, especially men for hunting and fighting, etc. Maybe one guy for butchering, another for pottery/basket making, another for bow/spear making or art/carvings. Skills can be add to aid in selecting who to delegate to. 
//note: what about handling the satisfaction of multiple needs at once? Such as if both thirsty and hungry, a watermelon would be ideal, rather than simply seeking out water then bread because each was handled independently?

};

#endif