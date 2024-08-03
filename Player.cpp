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

People pfunc;//for access to non static functions

Player::Player(){}

Player::Player(int a) {
	People::Person newpc = { People::new_person_id(), {0,0}, true };
	newpc.age = 11;
	People::pl.push_back(newpc);
	Environment::Map[newpc.pos.y][newpc.pos.x].person_id = newpc.id;
	pcindex = pfunc.p_by_id(newpc.id);
	cout << "Player id is " << newpc.id << "\n";//id should be 1 //player should be the first person in list

}

void Player::update() {
	People::p = pcindex;//this should always be 0 (first in pl list) but for now just in case use function
	pc = &People::pl[People::p];
	pc->current_image = "monument";
}

int sqdim1 = 16;
int spd = 2;
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