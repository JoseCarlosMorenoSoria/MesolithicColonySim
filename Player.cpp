//gameplay - sandbox colony sim with rpg mechanics - survival social management game - Nomad King
//survive (keep your npc alive from wolves, other npcs, get enough food, water, avoid dying from cold/heat, etc)
//raise a family (get a mate, have kids, keep them alive by bringing them food, give them commands for tasks to prioritize such as hunting, gathering, crafting, building, etc)
//raise your disposition with other people to gain advantages in resource access and number of people you can delegate tasks too
//achieve enough social influence to be declared tribal chief and beat out competitors for the title both external and internal, as well as settling disputes within your group and defending against outside attacks
//can delegate tasks, roles and partition terrirory but npcs might rebel according to their dispositions to you and each other and internal motives
//ensure your chosen heir successfully takes over after your death, keep your dynasty alive. 

//when npc's and world mechanics are done, make copies of each potential action for player reacting to player input and clearly presenting possible actions to player
//also make copies of npc and world data so that player can see and understand relevant info such as one's disposition to others and their dispositions to self, etc
//create menu and save system for player

#include "Player.hpp"

Player::Player(){}

Player::Player(int a) {
	Person newpc = { new_person_id(), {0,0}, true };//unsure if player is actually inserted into index 0, if it doesn't matter then need to fix People's update_all to also update index 0
	newpc.age = 11;
	pl.push_back(newpc);//unsure if need a variable in Person that marks which Person is a Player to prevent update_all from updating it.
	Environment::Map[newpc.pos.y][newpc.pos.x].person_id = newpc.id;
	pcindex = p_by_id(newpc.id);
	cout << "Player id is " << newpc.id << "\n";//id should be 1 //player should be the first person in list
	
}

void Player::update() {
	p = pcindex;//this should always be 0 (first in pl list) but for now just in case use function
	pc = &pl[p];
	pc->current_image = "monument";

	check_death();
}

int sqdim1 = 16;
int spd = 8;
void Player::move(string direction) {
	//People::Position pos = pc->pos;
	//Environment::Map[pos.y][pos.x].person_id = -1;
	if (direction == "E") {
		pc->px_x+=1*spd;
		pc->px_x %= sqdim1;
		if (pc->px_x == 8) {
			pc->pos.x++;
			pc->px_x *= -1;
		}
	}
	else if (direction == "W") {
		pc->px_x-= 1 * spd;
		pc->px_x %= sqdim1;
		if (pc->px_x == 8) {
			pc->pos.x--;
			pc->px_x *= -1;
		}
	}
	else if (direction == "N") {
		pc->px_y-= 1 * spd;
		pc->px_y %= sqdim1;
		if (pc->px_y == 8) {
			pc->pos.y--;
			pc->px_y *= -1;
		}
	}
	else if (direction == "S") {
		pc->px_y+= 1 * spd;
		pc->px_y %= sqdim1;
		if (pc->px_y == 8) {
			pc->pos.y++;
			pc->px_y *= -1;
		}
	}
	//Environment::Map[pos.y][pos.x].person_id = pc->id;//is there a danger of this overwriting the position of an npc and causing a problem?
	//if (Environment::Map[pos.y][pos.x].person_id != -1) {
	//	pc->pos = pos;
	//}
}

void Player::move_to() {}//use mouse click
void Player::set_camp_pc() {}
void Player::remove_camp_pc() {}
void Player::chat_pc() {}
void Player::fight_pc() {}
void Player::sleep_pc() {}
void Player::eat_pc() {}
void Player::reproduce_pc() {}
void Player::drink_pc() {}
void Player::equip_pc() {}
void Player::unequip_pc() {}
void Player::carry_infant_pc() {}
void Player::drop_infant_pc() {}
void Player::speak_pc() {}
void Player::craft_pc() {}
void Player::drop_item_pc() {}
void Player::pick_up_item_pc() {}
void Player::set_trap_pc() {}
void Player::toggle_sprint_pc() {}
void Player::toggle_stealth_pc() {}
void Player::cut_down_tree_pc() {}
void Player::play_trumpet() {}//recreation option
void Player::bathe() {}
void Player::share_disposition() {}

void Player::mouse_info() {}//show info on whatever is being moused over
void Player::view_inventory() {}
void Player::pause_game() {}
void Player::view_own_data() {}//variables such as sex, tired_level, search_results, found_messages, etc.








