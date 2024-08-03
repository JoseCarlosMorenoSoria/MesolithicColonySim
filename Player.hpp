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

class Player : public People{
public:
	Player();
	Player(int a);
	void update();
	Person* pc;//player character
	int pcindex;
	int continue_func = -1;//for running a function in update after being triggered manually by player. Turn this later into a map<> with the function name for more clarity.
	void move(string direction);//use keyboard //maybe should use keyboard for movement?
	void move_to_pc(Position dest);//use mouse click
	Position mouse_click_dest = {-1,-1};
	string crafting_item = "";
	int target_person = -1;
	bool fight_mode = false;//if fight_mode==true, then when right clicking on an npc, attack them. If ==false, then chat (disposition change) with the npc
	bool valence = true;//valence==true -> random positive value disposition change, else inverse. Used to determine whether next intentional social interaction is positive or negative, set by player
	//2 types of functions are needed, individual actions such as eat, sleep, etc. 
	//And cooperative functions where the player either initiates an interaction with an npc or responds to an interaction
	//from an npc. This requires npc's algorithms to therefore recognize the player as a player 
	//to then wait for a response rather than the fast paced data transfer between npcs.

	//_pc suffix prevents confusion and accidental overwriting of People functions
	void toggle_set_and_remove_camp_pc();
	void toggle_speed_pc();

	void chat_pc(int pid);
	void sleep_pc();
	void eat_pc(int index);
	void reproduce_pc();
	void drink_pc();
	void equip_pc(int index);
	void unequip_pc(int index);
	void carry_infant_pc();
	void drop_infant_pc();
	void speak_pc();
	void craft_pc(string product);
	void drop_item_pc(int index);
	void pick_up_item_pc();
	void set_trap_pc();
	void cut_down_tree_pc();
	void play_trumpet();//recreation option
	void bathe();
	void share_disposition();
	void attack_person(int pid);


	vector<string> view_own_data();//variables such as sex, tired_level, search_results, found_messages, etc.
	vector<string> view_inventory();
	vector<string> view_equipment();
	vector<string> view_dispositions();
	vector<string> view_craftable();
	

	/*//player versions of these functions
	* these aren't yet implemented in People class, do there first before creating player versions
	bool give_tribute();//if submissive to someone, chance of giving them tribute, chance increased according to their authority
	bool rebel();//chance to remove submissive status towards someone
	*/





};




#endif