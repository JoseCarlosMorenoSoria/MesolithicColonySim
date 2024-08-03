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
	void move(string direction);//use keyboard //maybe should use keyboard for movement?
	void move_to();//use mouse click

	//2 types of functions are needed, individual actions such as eat, sleep, etc. 
	//And cooperative functions where the player either initiates an interaction with an npc or responds to an interaction
	//from an npc. This requires npc's algorithms to therefore recognize the player as a player 
	//to then wait for a response rather than the fast paced data transfer between npcs.

	//_pc suffix prevents confusion and accidental overwriting of People functions
	void set_camp_pc();
	void remove_camp_pc();
	void chat_pc();
	void fight_pc();
	void sleep_pc();
	void eat_pc();
	void reproduce_pc();
	void drink_pc();
	void equip_pc();
	void unequip_pc();
	void carry_infant_pc();
	void drop_infant_pc();
	void speak_pc();
	void craft_pc();
	void drop_item_pc();
	void pick_up_item_pc();
	void set_trap_pc();
	void toggle_sprint_pc();
	void toggle_stealth_pc();
	void cut_down_tree_pc();
	void play_trumpet();//recreation option
	void bathe();
	void share_disposition();


	void mouse_info();//show info on whatever is being moused over
	void view_inventory();
	void pause_game();
	void view_own_data();//variables such as sex, tired_level, search_results, found_messages, etc.



	

	/*//player versions of these functions
	* these aren't yet implemented in People class, do there first before creating player versions
	bool give_tribute();//if submissive to someone, chance of giving them tribute, chance increased according to their authority
	bool rebel();//chance to remove submissive status towards someone
	*/





};




#endif