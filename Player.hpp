#ifndef Player_hpp
#define Player_hpp
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
#include "People.hpp"
#include <stdexcept>
using namespace std;

class Player {
public:
	Player();
	Player(int a);
	void update();
	People::Person* pc;//player character
	int pcindex;
	void move(string direction);

	void move_to();//use mouse click

	//player versions of these functions

	/*
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
	static int new_person_id();
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

	*/





};




#endif