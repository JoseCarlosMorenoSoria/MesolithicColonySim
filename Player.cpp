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
void Player::pause_game() {}


vector<string> Player::view_own_data() {
	vector<string> stats;
	pc->name = "Jose";//need to implement both npc names and player naming, fix this
	stats.push_back("Name: "+pc->name);
	stats.push_back("Sex: " + string((pc->sex) ? "male" : "female"));
	stats.push_back("Hunger Level: "+to_string(pc->hunger_level));
	stats.push_back("Thirst Level: " + to_string(pc->thirst_level));
	stats.push_back("Tired Level: " + to_string(pc->tired_level));
	stats.push_back("Recreation Level: " + to_string(pc->recreation_level));
	stats.push_back("Beauty Satisfied: " + string((pc->beauty_need_satisfied) ? "True" : "False"));
	stats.push_back("Current Camp Location: " + string((pc->campsite_pos.x == -1) ? "none" : to_string(pc->campsite_pos.x) + ", " + to_string(pc->campsite_pos.y)));
	stats.push_back("Current Camp Age: " + string((pc->campsite_age == -1) ? "N/A" : to_string(pc->campsite_age)));
	stats.push_back("Spouse: " + string((pc->spouse_id != -1) ? pl[p_by_id(pc->spouse_id)].name : "none"));
	string children= "Children:";
	if (pc->children_id.empty()) { children += " none"; }
	else {
		for (int i : pc->children_id) {
			children += " " + pl[p_by_id(i)].name + ",";
		}//need to remove last comma after loop ends
	}
	stats.push_back(children);
	stats.push_back("Age: "+to_string(pc->age));
	stats.push_back("Monument Unlocked: " + string((pc->monument_unlocked) ? "True" : "False"));
	stats.push_back("Fights Won: "+to_string(pc->num_fights_won));
	stats.push_back("Authority Level: "+to_string(pc->authority));
	stats.push_back("Dirtiness Level: "+to_string(pc->dirtiness));
	stats.push_back("My Temperature: "+to_string(pc->my_temperature));
	stats.push_back("Sickness: " + string((!pc->am_sick) ? "none" : "Yes - Sick Time: " + to_string(pc->sick_time)));
	stats.push_back("Injured: " + string((!pc->am_injured) ? "none" : "Yes - Injured Time: " + to_string(pc->injured_time)));

	return stats;

	//separate menu lists?
	/*
	map<int, submit_tracker> submissive_to;//{Person_ID, submissive_to/Not submissive_to} tracks people who have won a fight against self at least 3 times (them being present as a hotile during a fight that was lost against someone else counts)
	
	This should be shown on the map in real time as speech bubbles above speaker
	vector<int> found_messages;//message id's

	*/



}//variables such as sex, tired_level, search_results, found_messages, etc.


vector<string> Player::view_inventory() {
	vector<string> inventory;
	for (int& i : pc->item_inventory) {
		ItemSys::Item& item = ItemSys::item_list[ItemSys::item_by_id(i)];
		inventory.push_back(item.item_name + " : ");
	}//need to remove last colon after loop is done
	if (inventory.empty()) {
		inventory.push_back("Inventory is Empty");
	}
	return inventory;
}

vector<string> Player::view_equipment() {
	vector<string> equipment;
	for (auto& i : pc->equipped.equipment) {
		string s = i.first + ": ";
		if (i.second != -1) {
			ItemSys::Item& item = ItemSys::item_list[ItemSys::item_by_id(i.second)];
			s += item.item_name + " - Insulation = " + to_string(item.insulation_from_cold);
			equipment.push_back(s);
		}
	}//need to remove last colon after loop is done
	if (equipment.empty()) {
		equipment.push_back("Nothing Equipped");
	}
	return equipment;
}

//These should be manually set by the player, such that if the player chooses to hate someone, they can set their disposition to that person as hate and therefore be able to share the disposition with other npcs
//map<int, int> dispositions;//{Person_ID, favorability} holds id's of known people and whether and how much one likes or dislikes each

vector<string> Player::view_dispositions() {
	vector<string> dispositions;
	for (auto& i : pc->dispositions) {
		string s = "Name: " + pl[p_by_id(i.first)].name + " - ";
		
		if (i.second < HATED_THRESHOLD) {
			s += "Hate(" + to_string(i.second) + ")";
		}
		else if (i.second < DISLIKE_THRESHOLD) {
			s += "Dislike(" + to_string(i.second) + ")";
		}
		else if (i.second < LIKE_THRESHOLD) {
			s += "Neutral(" + to_string(i.second) + ")";
		}
		else if (i.second < LOVED_THRESHOLD) {
			s += "Like(" + to_string(i.second) + ")";
		}
		else {
			s += "Love(" + to_string(i.second) + ")";
		}

		dispositions.push_back(s);
	}
	if (dispositions.empty()) {
		dispositions.push_back("No Opinions on Anyone");
	}
	return dispositions;
}
