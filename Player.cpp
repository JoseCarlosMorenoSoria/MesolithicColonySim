//gameplay - sandbox colony sim with rpg mechanics - survival social management game - Nomad King
//survive (keep your npc alive from wolves, other npcs, get enough food, water, avoid dying from cold/heat, etc)
//raise a family (get a mate, have kids, keep them alive by bringing them food, give them commands for tasks to prioritize such as hunting, gathering, crafting, building, etc)
//raise your disposition with other people to gain advantages in resource access and number of people you can delegate tasks too
//achieve enough social influence to be declared tribal chief and beat out competitors for the title both external and internal, as well as settling disputes within your group and defending against outside attacks
//can delegate tasks, roles and partition territory but npcs might rebel according to their dispositions to you and each other and internal motives
//ensure your chosen heir successfully takes over after your death, keep your dynasty alive. 

//create save system

#include "Player.hpp"
Animal::Species& con = Animal::species["human"];//access to human constants

//once player is done, test if can win game, then improve player by turning actions into context actions or tooltip actions to reduce need to go into menu


Player::Player(){}

Player::Player(int a) {
	Person newpc;//unsure if player is actually inserted into index 0, if it doesn't matter then need to fix People's update_all to also update index 0
	newpc.p_id = new_person_id();
	newpc.pos = { 50,10 };
	newpc.sex = true;
	newpc.age = 11;
	pl.push_back(newpc);//unsure if need a variable in Person that marks which Person is a Player to prevent update_all from updating it.
	Environment::Map[newpc.pos.y][newpc.pos.x].person_id = newpc.p_id;
	pcindex = p_by_id(newpc.p_id);
	cout << "Player id is " << newpc.p_id << "\n";//id should be 1 //player should be the first person in list
	
}



//note: need to to constantly update pcindex in update() in case a person gets deleted from list and the pcindex is no longer correct

int speed_toggle = 0;
//fix this now: need to add a way to lock in an animation until player provides a different input. So for example if eating, then have to initiate and run eating progress in update() until done unless player interrupts by giving another input
void Player::update() {//this should always be 0 (first in pl list) but for now just in case use function
	p = pcindex;
	pc = &pl[p];
	//pc->current_image = "player";

	check_death();

	if (!pl[p].awake || pl[p].tired_level >= con.FORCE_SLEEP_LEVEL) {
		//pl[p].awake = false;
		sleeping();
	}

	if (pl[p].sprint_stamina >= con.SPRINT_LIMIT) {
		pl[p].speed = con.WALK_SPEED;
		speed_toggle = 0;
	}

	if (!pl[p].active_hostile_towards.empty() && continue_func==-1) {//if someone is actively hostile to pc and no player function is currently being continuosly called, fight in self defense
		pl[p].move_already = false;
		fight();
	}

	switch (continue_func) {
	case 0:
		bathe();
		break;
	case 1:
		play_trumpet();
		break;
	case 2:
		move_to_pc(mouse_click_dest);
		break;
	case 3:
		drink_pc();//fix this, need to cap stats at 0 minimum and prevent consumption of water/food/etc if already at that cap
		break;
	case 4:
		craft_pc(crafting_item);
		break;
	case 5:
		eat_pc(pl[p].eating_food_index);
		break;
	case 6:
		attack_person(target_person);
		break;
	case 7:
		break;
	case 8:
		break;
	default:
		break;
	}

	if (phour_count == 0) {
		pl[p].age++;
	}
	
	//need to set upper and lower caps for these (upper for some is death) Might also be better to track some by the hour or day rather than by the tick
	pl[p].hunger_level++; //hunger increases by 1 per hour, meaning it must decrease by 20 per day to stay at 0 daily average
	pl[p].tired_level++; //same for tired level
	pl[p].campsite_age++;
	pl[p].reproduction_cooldown++; //for when to find mate and create new person
	pl[p].thirst_level++;
	pl[p].recreation_level++;
	pl[p].dirtiness++;

	if (pl[p].am_injured) {
		pl[p].injured_time++;
	}
	if (pl[p].am_sick) {
		pl[p].sick_time++;
	}

	int insulation_cold = 0;//unsure if insulation from heat makes sense, except from hats and maybe light white clothing? Only insulation from cold used for now.
	for (auto const& i : pl[p].equipped.equipment) {
		if (i.second != -1) {
			insulation_cold += ItemSys::item_list[ItemSys::item_by_id(i.second)].insulation_from_cold;
		}
	}
	if (Environment::Map[pl[p].pos.y][pl[p].pos.x].temperature > pl[p].my_temperature) {//like other needs, having this update every tick is not ideal and should be changed.
		pl[p].my_temperature++;//fix this, need to make increase/decrease a percent of the difference between my_temp and tile_temp. Such that a large difference causes a large change in my_temp but a small one does not. Do the same for the clothing insulation below.

	}
	else if (Environment::Map[pl[p].pos.y][pl[p].pos.x].temperature < pl[p].my_temperature) {
		if (insulation_cold == 0) {
			pl[p].my_temperature--; //for now, insulation is essentially a bool that prevents getting colder than already are. Need to implement a more thought out system of person temp change and insulation. Fix.
		}
	}

	if (pl[p].clean_image) {
		pl[p].current_image = "pics/human.png";
		pl[p].clean_image = false;
	}

	authority_calc();
}

//int sqdim1 = 16;
//int spd = 8;
//Not working properly sometimes. Goes off the map and sometimes gets stuck in a single direction regardless of key pressed
void Player::move(string direction) {
	p = pcindex;
	pc = &pl[p];
	pl[p].move_already = false;

	//People::Position pos = pc->pos;
	//Environment::Map[pos.y][pos.x].person_id = -1;
	Position dest = pl[p].pos;

	if (direction == "E") {
		dest.x++;

		/*
		pc->px_x+=1*spd;
		pc->px_x %= sqdim1;
		if (pc->px_x == 8) {
			pc->pos.x++;
			pc->px_x *= -1;
		}
		*/
	}
	else if (direction == "W") {
		dest.x--;

		/*
		pc->px_x-= 1 * spd;
		pc->px_x %= sqdim1;
		if (pc->px_x == 8) {
			pc->pos.x--;
			pc->px_x *= -1;
		}
		*/
	}
	else if (direction == "N") {
		dest.y--;

		/*
		pc->px_y-= 1 * spd;
		pc->px_y %= sqdim1;
		if (pc->px_y == 8) {
			pc->pos.y--;
			pc->px_y *= -1;
		}
		*/
	}
	else if (direction == "S") {
		dest.y++;

		/*
		pc->px_y+= 1 * spd;
		pc->px_y %= sqdim1;
		if (pc->px_y == 8) {
			pc->pos.y++;
			pc->px_y *= -1;
		}
		*/
	}

	move_to(dest,"player moving");//need to figure out why movement is slow, also need to handle being able to move diagonally

	//Environment::Map[pos.y][pos.x].person_id = pc->id;//is there a danger of this overwriting the position of an npc and causing a problem?
	//if (Environment::Map[pos.y][pos.x].person_id != -1) {
	//	pc->pos = pos;
	//}
}

void Player::toggle_speed_pc() {//not sure if working properly
	p = pcindex;
	pc = &pl[p];

	speed_toggle++;
	speed_toggle %= 3;
	if (speed_toggle == 0) {
		pl[p].speed = con.WALK_SPEED;
	}
	else if (speed_toggle == 1) {
		if (pl[p].speed != con.SPRINT_SPEED && pl[p].sprint_stamina < con.SPRINT_LIMIT) {
			pl[p].speed = con.SPRINT_SPEED;
		}
	}
	else if (speed_toggle == 2) {
		pl[p].speed = con.STEALTH_SPEED;
	}
}

//this is the player version
void Player::move_to_pc(Position dest) {
	p = pcindex;
	pc = &pl[p];

	if (!Position::valid_position(dest)) {
		continue_func = -1;
		return;
	}

	mouse_click_dest = dest;
	continue_func = 2;
	pl[p].move_already = false;
	if (move_to(dest, "mouse click move")) {
		continue_func = -1;
		mouse_click_dest = { -1,-1 };
	}
}//use mouse click

void Player::toggle_set_and_remove_camp_pc() {
	p = pcindex;//need to reset p to player's index for every function just in case, to ensure that it is correct before executing a People function
	pc = &pl[p];
	if (Environment::Map[pc->pos.y][pc->pos.x].item_id==-1 && pc->campsite_pos.x == -1) {//if valid empty spot found and have no campsite
		create_item("tent", pc->pos);//create and place tent
		pl[p].campsite_pos = pc->pos; //store campsite location
	}
	else if (pc->campsite_pos.x != -1 && pc->pos==pc->campsite_pos) {
		int item_id = Environment::Map[pl[p].campsite_pos.y][pl[p].campsite_pos.x].item_id;
		delete_item(item_id, pl[p].campsite_pos, -1);
		pl[p].campsite_pos = { -1,-1 };
		pl[p].campsite_age = -1;
	}
}

void Player::sleep_pc() {
	p = pcindex;
	pc = &pl[p];
	if (pl[p].campsite_pos.x==-1 || pl[p].pos == pl[p].campsite_pos) {//if don't have campsite or if at campsite
		pl[p].awake = false;
		sleeping();
	}
}

void Player::bathe() {
	p = pcindex;
	pc = &pl[p];
	if (Environment::Map[pl[p].pos.y][pl[p].pos.x].terrain == "water" && pl[p].dirtiness>0) {
		//play bathing animation
		//reduce dirtiness
		continue_func = 0;
		pl[p].current_image = "bathing";
		if (pl[p].bathing.progress_func()) {
			pl[p].dirtiness = 0;
			continue_func = -1;
			pl[p].clean_image = true;
		}
	}
}

void Player::drop_item_pc(int index) {
	p = pcindex;
	pc = &pl[p];
	find_all();
	drop_item(index);
	pl[p].search_results.clear();
}

void Player::pick_up_item_pc() {
	//pick up item on the same tile as player. Later when add facing direction, add picking up item in front of player
	p = pcindex;
	pc = &pl[p];
	int item_id = Environment::Map[pl[p].pos.y][pl[p].pos.x].item_id;
	if (item_id != -1) {
		ItemSys::Item& item = ItemSys::item_list[ItemSys::item_by_id(item_id)];
		if (item.can_pick_up) {//fix this, need to add it to NPC as well
			pick_up_item(Environment::Map[pl[p].pos.y][pl[p].pos.x].item_id, pl[p].pos);//need to add a constraint for items that shouldn't be able to be picked up, such as active traps, trees and tents, etc.
		}
	}
	else {
		cut_down_tree_pc();//if no item on player's tile, try to cut down nearest tree
	}
}

void Player::drink_pc() {//if next to water, drink water
	p = pcindex;
	pc = &pl[p];
	find_all();
	if (Position::distance(pl[p].pos, pl[p].search_results["water"][0]) == 1) {
		pl[p].thirst_level -= con.THIRST_REDUCTION_RATE;
		continue_func = 3;
		if (pl[p].thirst_level == 0) {
			continue_func = -1;
		}
	}
	pl[p].search_results.clear();
}

void Player::craft_pc(string product) {
	p = pcindex;
	pc = &pl[p];
	if (craft(product)) {
		continue_func = -1;
	}
	continue_func = 4;
	crafting_item = product;
}

void Player::eat_pc(int index) {//might be better to break up NPC functions such that both the player and the NPC call the same functions the same way rather than having a simplified version for the player, and only differing in where the input is coming from (AI vs player input)
	p = pcindex;
	pc = &pl[p];

	pl[p].eating_food_index = index;//of item in inventory
	continue_func = 5;

	if (pl[p].eating_progress.progress == 0) {
		pl[p].current_image = "pics/human_eating.png";
	}
	if (pl[p].eating_progress.progress_func()) {//makes eating take more than 1 frame
		int index = pl[p].eating_food_index;
		int food_id = pl[p].item_inventory[index];
		delete_item(food_id, { -1,-1 }, index);//delete food from game
		pl[p].hunger_level -= con.HUNGER_REDUCTION_RATE; //reduce hungry level by 10, therefore need 2 meals a day to stay at 0 hunger_level average
		pl[p].clean_image = true; //when this function ends, return to default image on next update
		continue_func = -1;
	}

	//do this later for this and all functions, including for npcs
	//from inventory menu
	//equip food item <--
	//exit menu
	//play eating animation
	//consume item
}

void Player::equip_pc(int index) {
	//while in inventory menu, select item to equip
	p = pcindex;
	pc = &pl[p];
	ItemSys::Item item = ItemSys::item_list[ItemSys::item_by_id(pl[p].item_inventory[index])];
	for (string t : item.tags) {
		if (t == "ready food") {
			eat_pc(index);//if item being equipped is food, eat item.
			return;
		}
	}
	pl[p].equipped.equip(pl[p].item_inventory[index]);
}

void Player::unequip_pc(int index) {
	//while in equipment menu, select item to unequip
	p = pcindex;
	pc = &pl[p];//this might be unneccessary given the use of pl[p].
	int ind = 0;
	for (auto& i : pl[p].equipped.equipment) {
		if (ind == index) {
			pl[p].equipped.unequip(i.first);
		}
	}
}

void Player::play_trumpet() {//need to cap values given that player will probably push them below 0, fix this now
	p = pcindex;
	pc = &pl[p];
	if (!inventory_has("trumpet").empty()) {
		//play trumpet animation
		pl[p].current_image = "playing_trumpet";
		pl[p].recreation_level -= 50;//each recreation type should have its own reduction value?
		continue_func = 1;
		if (pl[p].recreation_level <= 0) {
			continue_func = -1;
			pl[p].clean_image = true;
		}
	}
}//recreation option

// currently called by pick_up_item_pc if no item is on pc's tile
void Player::cut_down_tree_pc() {//temporary implementation, cuts down tree if next to it, if adding an animation then it would cause the pc to move to the nearest tree in its sightline, but the proper implementation should be to have the player click the tree to cut down.
	p = pcindex;
	pc = &pl[p];
	find_all();

	cut_down_tree();

	pl[p].search_results.clear();
}

void Player::attack_person(int pid) {
	p = pcindex;
	pc = &pl[p];
	if (pid == -1 || pid == pl[p].p_id) {
		return;//not sure why it recieves a -1 sometimes
	}

	if (pid != target_person) {
		target_person = pid;
		pl[p].hostile_towards.push_back(pid);
		pl[p].active_hostile_towards.push_back(pid);
	}
	pl[p].move_already = false;
	fight();
	continue_func = 6;
	if (pl[p].hostile_towards.empty()) {
		continue_func = -1;
		target_person = -1;
		pl[p].clean_image = true;
	}
}

void Player::chat_pc(int pid) {
	p = pcindex;
	pc = &pl[p];

	if (Position::distance(pl[p].pos, pl[p_by_id(pid)].pos) > pl[p].audioline_radius) {
		return;//only chat if person is close enough
	}
	cout << "player chatted";

	//player gives either compliment or insult to a specific npc
	int p2_ind = p_by_id(pid);
	int p2_id = pl[p2_ind].p_id;
	//compliment or insult
	int comment = (rand() % 15);
	if (!valence) {//if valence==false, set topic to negative (slight/insult). 
		comment *= -1;
	}
	change_disposition(p2_id,0,"insert if not found");
	if (pl[p].dispositions[p2_id] >= 0) {//if like person, make compliment bigger, if like person, make insult smaller. Inverse for dislike person
		if (comment >= 0) {
			comment = comment * ((pl[p].dispositions[p2_id] / 100) * 2 + 1);
		}
		else {
			comment = comment * ((pl[p].dispositions[p2_id] / 100) * 0.5 + 1);
		}
	}
	else {
		if (comment >= 0) {
			comment = comment * ((pl[p].dispositions[p2_id] / 100) * 0.5 + 1);
		}
		else {
			comment = comment * ((pl[p].dispositions[p2_id] / 100) * 2 + 1);
		}
	}
	int op = p;
	p = p2_ind;
	string comment_type;
	(comment < 0) ? comment_type = "insult" : comment_type = "compliment";
	change_disposition(pl[op].p_id, comment, comment_type);//positive is compliment, negative is insult
	p = op;
}



//do these next

void Player::reproduce_pc() {}//need to set so that player decides when to propose or accept reproduction with a specific npc


void Player::speak_pc() {}
void Player::share_disposition() {}

void Player::carry_infant_pc() {
	//if near infant and infant is mine, pick up
}
void Player::drop_infant_pc() {
	//if empty tile available, drop any infants being carried
}
void Player::set_trap_pc() {}





vector<string> Player::view_own_data() {
	p = pcindex;
	pc = &pl[p];
	vector<string> stats;
	pc->name = "Jose";//need to implement both npc names and player naming, fix this
	stats.push_back("Name: "+pc->name);
	stats.push_back("Chat Mood: " + string((valence) ? "Compliment" : "Insult"));
	stats.push_back("Current Mode: " + string((fight_mode) ? "Fight" : "Chat"));
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
	stats.push_back("Dirtiness Level: "+to_string(pc->dirtiness));
	stats.push_back("My Temperature: "+to_string(pc->my_temperature));
	stats.push_back("Sickness: " + string((!pc->am_sick) ? "none" : "Yes - Sick Time: " + to_string(pc->sick_time)));
	stats.push_back("Injured: " + string((!pc->am_injured) ? "none" : "Yes - Injured Time: " + to_string(pc->injured_time)));
	stats.push_back("Num liked by: " + to_string(pl[p].num_people_liked_by));
	stats.push_back("Amount liked: " + to_string(pl[p].amount_liked));
	stats.push_back("Num Submissives: " + to_string(pl[p].num_submissives));
	stats.push_back("Fights Won: " + to_string(pc->num_fights_won));
	stats.push_back("Authority Level: " + to_string(pc->authority));
	stats.push_back("Monument Unlocked: " + string((pc->monument_unlocked) ? "True" : "False"));
	return stats;

	//separate menu lists?
	/*
	map<int, submit_tracker> submissive_to;//{Person_ID, submissive_to/Not submissive_to} tracks people who have won a fight against self at least 3 times (them being present as a hotile during a fight that was lost against someone else counts)
	
	This should be shown on the map in real time as speech bubbles above speaker
	vector<int> found_messages;//message id's

	*/



}//variables such as sex, tired_level, search_results, found_messages, etc.

vector<string> Player::view_inventory() {
	p = pcindex;
	pc = &pl[p];
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
	p = pcindex;
	pc = &pl[p];
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

vector<string> Player::view_dispositions() {
	p = pcindex;
	pc = &pl[p];
//These should be manually set by the player, such that if the player chooses to hate someone, they can set their disposition to that person as hate and therefore be able to share the disposition with other npcs
//map<int, int> dispositions;//{Person_ID, favorability} holds id's of known people and whether and how much one likes or dislikes each
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

vector<string> Player::view_craftable() {//this function is very inefficient, maybe use sets to find the intersection between inventory items and the ingredients of items or something?
	p = pcindex;
	pc = &pl[p];
	vector<string> craftable;
	for (int& i : pc->item_inventory) {
		ItemSys::Item& item = ItemSys::item_list[ItemSys::item_by_id(i)];
		for (string j : it2.ingredients[item.item_name]) {
			craftable.push_back(j);
		}
	}

	set<string> cr;
	for (string s : craftable) {
		cr.insert(s);
	}
	craftable.clear();
	for (auto& i : cr) {
		craftable.push_back(i);
	}
		for (int i = craftable.size()-1; i > -1; i--) {
			ItemSys::Item& product = it2.presets[craftable[i]];
			for (string s : product.ingredients) {
				if (inventory_has(s).empty()) {
					craftable.erase(craftable.begin() + i);
					break;
				}
			}
		}
	if (craftable.empty()) {
		craftable.push_back("Can't craft anything right now.");
	}
	return craftable;
}





//To do today:
//refactor code to make it cleaner, compact, readable and modifiable/modular. Such as making People and Animal children of a parent Creature class, or rather make the shared functions more like components rather than inherited. As well as a utility class that contains things like Position which is used by every class.
//need to implement zoom levels, as these will determine the necessary detail in sprites. 
//need to make sprites and animations
//then start adding more items/weather/terrain/etc variety, keep new behavior/code light