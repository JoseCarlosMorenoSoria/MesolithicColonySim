#include "People.hpp"

//currently works as expected, with the exception of a random unkown crash that crashes the whole computer. Maybe related to death but unsure, haven't tested death state.

//To Do Now: 
//Need to add food sharing. 

using namespace std;

People::Person People::p; //used to store currently updating Person
vector<People::Person> People::people_list; //used to store all Person instances

People::People() {
    Person p1 = { {10,10} };
    people_list.push_back(p1);
    Person p2 = { {20,20} };
    people_list.push_back(p2);

    for (int i = 0; i < people_list.size(); i++) {
        p = people_list[i];
        set_up_camp(); //for now, just set up a camp nearby at start
        people_list[i] = p;
    }
}

void People::update_all(int day_count, int hour_count) {
    for(int i = 0; i < people_list.size(); i++) {
        p = people_list[i]; //currently simply copying Person instance to utilize as an object when updating, but may be better to use a reference to the object in the list instead?
        if (p.is_alive) {
            update(day_count, hour_count);
        }
        people_list[i] = p;
    }
}

void People::update(int day_count, int hour_count) {
    if (p.hunger_level > 1000) {
        p.is_alive = false;
    }
    
    if (!p.is_alive) {
        p.current_state = "dead";
        p.current_image = "pics/human_dead.png";

        cout << "______" << endl;
        //print record of function history for debugging
        for (string s : p.function_record) {
            cout << s << ", " << endl;
        }

        return;
    }

	if (hour_count == 0) { //once a day check

        
		if (p.hunger_level > 50) { //tracks for continuous hungry days at the start of every day
			p.hungry_time++;
		}
		else {
			p.hungry_time = 0;
		}
        
	}

    p.hunger_level++; //hunger increases by 1 per hour, meaning it must decrease by 20 per day to stay at 0 daily average
    p.tired_level++; //same for tired level
    p.campsite_age++;

    utility_function();
}

bool People::valid_position(Position pos) {
    int map_max_x = 100;
    int map_min_x = 0;
    int map_max_y = 50;
    int map_min_y = 0;

    bool valid_x = map_min_x <= pos.x && pos.x < map_max_x;
    bool valid_y = map_min_y <= pos.y && pos.y < map_max_y;

    if (valid_x && valid_y) {
        return true;
    }
    return false;
}

bool People::move_to(Position dest) {//need to add speed of moving from one tile to another and how many tiles at a time. Also need to add a check to prevent it being called more than once per person per update.
    if (!valid_position(dest)) { //for debugging, kill npc if it tries to go off map
        p.is_alive = false;
        p.current_image = "pics/human_dead.png";
        return true;
    }


    if (p.pos.x < dest.x) {
        p.pos.x++;
    }
    else if (p.pos.x > dest.x) {
        p.pos.x--;
    }
    
    if (p.pos.y < dest.y) {
        p.pos.y++;
    }
    else if (p.pos.y > dest.y) {
        p.pos.y--;
    }

    bool reached = (p.pos.x == dest.x) && (p.pos.y == dest.y);
    return reached;
}


//bool People::tile_has(string target) { //need to move to environment class and make functional
//    if (target == "food") {
//
//    }
//}

//look in a radius around person to find x, return the position of the target if found, return a -1,-1 position if not found
vector<People::Position> People::find(string target, int radius, int quantity) {//currently only works for finding food. Also, a radius of 1 is the tile the person is on, a radius of 2 is all adjacent tiles to person
    vector<Position> pos_list;

    int tmpradius = 0;
    int quantity_found = 0;

    while (tmpradius != radius) { //this is just a way to return closest item found but uses far too many redundant loops, need to optimize by only checking each tile once in outward rings
        for (int y = p.pos.y - tmpradius; y < p.pos.y + tmpradius; y++) { //searches in a x tile radius around the person
            for (int x = p.pos.x - tmpradius; x < p.pos.x + tmpradius; x++) {
                if (valid_position({ x,y }) && Environment::Map[y][x].has_food) {//check if tile contains food, currently is simply tied directly to tile rather than being an item
                    pos_list.push_back({ x,y });
                    quantity_found++;
                    if (quantity_found == quantity) {//returns list of positions found once desired quantity is found
                        return pos_list;
                    }
                }
            }
        }
        tmpradius++;
    }
    return pos_list;
}

bool People::has_food() {
    if (p.food_inventory.size() > 0) {
        return true;
    }
    return false;
}



void People::utility_function() {//is currently actually just a behavior tree not a utility function

    //these bools determine which functions can be called, these are reset every time utility_function is called
    vector<Position> food_pos_list = find("food", p.sightline_radius, 1); //gets 1 result, assigns 1 result to food_pos
    Position food_pos;
    bool found_food = false;
    if (food_pos_list.size() > 0) {
        food_pos = food_pos_list[0];
        found_food = true;
    }
    else {
        food_pos = {-1,-1};
    }
     
    

    bool start_search_for_new_campsite = (!found_food || (found_food && (distance(food_pos, p.campsite_pos).x > campsite_distance_search || distance(food_pos, p.campsite_pos).y > campsite_distance_search))) && (p.campsite_pos.x == -1 || p.hungry_time >= 3) && (p.campsite_age > 10 || p.campsite_age == -1); //currently only creates a campsite after having been hungry 3 days, need a trigger for creating a campsite when there is abundant food. Still need to figure out when and when not to create a campsite.
    if (start_search_for_new_campsite) {//remove campsite to be able to be unbound and get food while searching for new campsite, figure out a cleaner way to organize order between finding food to eat now and finding a new campsite
        if (p.campsite_pos.x != -1) { 
            if (move_to(p.campsite_pos)) {//walk to campsite to remove
                Environment::Map[p.campsite_pos.y][p.campsite_pos.x].has_tent = false; //removes campsite
                p.campsite_pos = { -1,-1 };
                p.campsite_age = -1;
            }
        }
    }

    bool tired = p.tired_level > 50;
    bool very_tired = p.tired_level > 100;//might need to cap sleep such that a person can't ever have a tired_level over x_level as well as under y_level
    bool start_moving_to_bed = tired && p.campsite_pos.x != -1;
    bool start_sleeping = !p.awake || (tired && p.pos.x == p.campsite_pos.x && p.pos.y == p.campsite_pos.y && p.campsite_pos.x != -1) || very_tired; //Need to do: overload == to make comparing positions easier

    bool hungry = p.hunger_level > 50;
    //if (start_search_for_new_campsite) {//this doesn't trigger given the new numbers for when hungry triggers, is it actually necessary or was the problem just the other numbers?
    //    hungry = p.hunger_level > 40 && (p.function_record.back() != "searching_for_food" || p.function_record.back() != "gathering_food" || p.function_record.back() != "eating"); //allows more time searching for campsite instead of food, unless food search was already triggered in which case keep getting food to reset hunger padding
    //}
    
    bool start_eating = hungry && has_food();
    bool start_searching_for_food = hungry && !has_food();

    if (p.clean_image) {
        p.current_image = "pics/human.png";
        p.clean_image = false;
    }
    
    
    //select which specific function to call right now

    /*

whenever someone else is hungry and you have food and you are not hungry, share food


priority:
sleep
hunger
campsite

*/

    //this implementation allows functions to be interrupted by higher priority ones on every update, however this means that a function may not properly reset or preserve as needed for when it gets called again later, need to fix

    if (start_sleeping) {
        p.function_record.push_back("sleeping");
        p.current_state = "sleeping";
        sleeping();
    }
    else if (start_moving_to_bed) {
        p.function_record.push_back("moving to bed");
        move_to(p.campsite_pos); //go to campsite. This is the extent of this "function"
    }
    else if (start_eating) {
        p.function_record.push_back("eating");
        p.current_state = "eating";
        p.clean_image = eating(); //when this function ends (returns true), return to default image on next update
    }
    else if (start_searching_for_food && !p.start_gathering_food) {
        p.function_record.push_back("searching for food");
        p.current_state = "searching for food";
        p.start_gathering_food = searching_for_food();
    }
    else if (p.start_gathering_food) {
        p.function_record.push_back("gathering food");
        p.current_state = "gathering food";
        p.start_gathering_food = !gathering_food(); //resets function trigger to false when function ends
    }
    else if (start_search_for_new_campsite && !p.start_set_up_camp) {
        p.function_record.push_back("search for new campsite");
        p.current_state = "search for new campsite";
        p.start_set_up_camp = search_for_new_campsite();
    }
    else if (p.start_set_up_camp) {
        p.function_record.push_back("set up camp");
        p.current_state = "set up camp";
        p.start_set_up_camp = !set_up_camp(); //resets function trigger to false when function ends
        p.campsite_age = 0; //resets campsite age
    }
    else {
        //if all other functions are false, then idle.
        p.function_record.push_back("idle");
        p.current_state = "idle";
        idle();
    }
}

bool People::search_for_new_campsite(){ //need to bias search direction in the direction of wherever there is more food rather than waiting to randomly stumble on a site with enough food for campsite. Also need to add a system of not searching the same tile within too short a time frame.
	/*
	* searching for a new campsite
    start:
    - if have been continuosly hungry for 3 days (even if have eaten a bit)
    end:
    - if found an area with more food
	*/

    if (p.campsite_pos.x != -1) { //if have campsite, remove. //Later add an option to just abandon a campsite without removing the house. Should only decontruct if going to carry it to new location such as a tent/sleeping bag/lean to/etc.
        if (move_to(p.campsite_pos)) {//walk to campsite to remove
            Environment::Map[p.campsite_pos.y][p.campsite_pos.x].has_tent = false; //removes campsite
            p.campsite_pos = { -1,-1 };
            p.campsite_age = -1;
        }
    }

    //move x steps in a random direction, x is large to cover more distance, x is a multiple of 10 which is the radius to check for food in the area
    if (p.fo1.pos.x == -1 || move_to(p.fo1.pos)) {//initialize function object or reinitialize if reached destination
        bool valid_dest = false;
        while (!valid_dest) {
            int max = 100;
            int min = 10;
            int sign = (rand() % 2);
            (sign == 0) ? sign = -1 : sign = 1;
            p.fo1.pos.x = sign * ((rand() % (max - min)) + min); //select a destination
            max = 50;
            min = 10;
            sign = (rand() % 2);
            (sign == 0) ? sign = -1 : sign = 1;
            p.fo1.pos.y = sign * ((rand() % (max - min)) + min);
            valid_dest = valid_position(p.fo1.pos);
        }
    }//the move_to function triggers in the conditional

    
    vector<Position> food_pos_list = find("food", p.sightline_radius, 4);//get 4 results
    bool end = false;
    
    if (food_pos_list.size() >= 4) {//if there are 4 food items within sight, select area for campsite, else keep searching
        end = true;
    }
    else if (food_pos_list.size() > 0) {
        //need to add a method of investigating if any food found might have more food just out of current sightline, but this probably will require more complex modifiable pathfinding, as in have it be a detour from the current destination rather than a change in destination.
    }


    if (end) { // reset function object if ending function
        p.fo1.pos = {-1,-1};
    }
	return end; //return whether action has finished or not. True == end action  False == continue action
}

bool People::set_up_camp(){
	/*setting up camp
    start:
    - if found a new campsite
    end:
    - once new camp is done building
*/

    //need to ensure that the tile being placed in both exists and is not occupied, might need a separate function for this
    //place tent
    Environment::Map[p.pos.y+1][p.pos.x+1].has_tent = true; //for now simply have a tent be an attribute of the tile rather than a placed item
    p.campsite_pos = { p.pos.x + 1 , p.pos.y + 1 }; //store campsite location
    return true; //need to add an actual animation/progress to building the tent rather than immediate placement
}

bool People::idle(){
	/*idle
    start:
    - if nothing else to do
    end:
    - if have something else to do
*/
    //flip between idle and default image
    if (p.current_image == "pics/human.png") {
        p.current_image = "pics/human_idle.png"; //need to make image, just have human with raised hands
    }
    else {
        p.current_image = "pics/human.png";
    }
    return true;
}

bool People::sleeping(){
	/*sleeping
    start:
    - if tired
    end:
    - if not tired
*/
    p.current_image = "pics/human_sleeping.png";
    p.awake = false;

    p.tired_level-=11; //every call to this function reduces tired by 11, this means need 5 hours/updates to stop sleeping and sleep every 50 hours/updates. Is -11 so as to do -10 per hour and also -1 to negate the +1 tired in the regular update function

    if (p.tired_level <= 0) {
        p.current_image = "pics/human.png";
        p.awake = true;
        return true;
    }

    return false;

}

bool People::eating(){
	/*eating
    start: 
    - if hungry and have food
    end:
    - if not hungry or have no food
*/
    if (p.fo3.progress == 0) {
        p.current_image = "pics/human_eating.png";
    }
    p.fo3.progress++;
    if (p.fo3.progress == p.fo3.progress_done) {//makes eating take more than 1 frame
        p.fo3.progress = 0;//reset
        //need to delete food item in inventory from game
        p.food_inventory.pop_back();//delete item from inventory
        p.hunger_level -= 50; //reduce hungry level by 10, therefore need 2 meals a day to stay at 0 hunger_level average
        return true;//done eating
    }
     return false;
    
}

bool People::searching_for_food(){ //this function is far too similar to gathering_food() and should be consolidated or call gathering_food()
	/*searching for food
    start:
    - if hungry
    - if someone else is very hungry <-- other people not yet implemented
    end:
    - if found food
    - if other person is no longer hungry
*/

    //reset function if it was interrupted by another function
    if (p.function_record.back() != "searching for food") {
        p.fo2.pos = { -1,-1 }; //reset function object
    }

    //look around self for food
    Position food_pos = {-1,-1};
    bool found_food = false;
    vector<Position> food_pos_list = find("food", p.sightline_radius, 1);
    if (food_pos_list.size() > 0) {
        found_food = true;
        food_pos = food_pos_list[0];
    }
    else {

    }
    
     

     //move x steps in a random direction, x is large to cover more distance, x is a multiple of 10 which is the radius to check for food in the area
     if (p.fo2.pos.x == -1 || move_to(p.fo2.pos)) {//initialize function object or reinitialize if reached destination
         bool valid_dest = false;
         while (!valid_dest) {
             int max = 100;
             int min = 10;
             int sign = (rand() % 2);
             (sign == 0) ? sign = -1 : sign = 1;
             p.fo2.pos.x = sign * ((rand() % (max-min)) + min); //select a destination
             max = 50;
             min = 10;
             sign = (rand() % 2);
             (sign == 0) ? sign = -1 : sign = 1;
             p.fo2.pos.y = sign * ((rand() % (max - min)) + min);
             valid_dest = valid_position(p.fo2.pos);
             //limit search range to within x distance from campsite, if have campsite
             
             if (p.campsite_pos.x > -1) {
                 Position dist_pos = distance(p.campsite_pos, p.fo2.pos);
                 if (dist_pos.x > campsite_distance_search || dist_pos.y > campsite_distance_search) {
                     valid_dest = false;
                 }
             }
             
         }
     }//the move_to function triggers in the conditional

     if (found_food) {
         
         if (p.campsite_pos.x > -1) {
             Position dist_pos = distance(p.campsite_pos, food_pos);
             if (dist_pos.x > campsite_distance_search || dist_pos.y > campsite_distance_search) {
                 return false;
             }
         }
         
         p.fo2.pos = { -1,-1 }; //reset function object
         return true;
     }
     else {
     }

     return false;
}

People::Position People::distance(Position pos1, Position pos2) {
    Position pos3;
    pos3.x = abs(pos1.x - pos2.x);
    pos3.y = abs(pos1.y - pos2.y);
    return pos3;
}

bool People::gathering_food(){
    /*gathering food
    start:
    - if was searching and now found food and I or someone else is still hungry
    end:
    - if neither I nor someone else is hungry anymore
*/
    

    //look around self for food
    Position food_pos = { -1,-1 };
    bool found_food = false;
    vector<Position> food_pos_list = find("food", p.sightline_radius, 1);
    if (food_pos_list.size() > 0) {
        found_food = true;
        food_pos = food_pos_list[0];
    }
    else {

    }


    if (found_food) {
        
        if (p.campsite_pos.x > -1) {
            Position dist_pos = distance(p.campsite_pos, food_pos);
            if (dist_pos.x > campsite_distance_search || dist_pos.y > campsite_distance_search) {
                return true; //keep within x distance to campsite, if outside range then end function (return true)
            }
        }
        
        bool reached = move_to(food_pos); //move towards food
        if (reached) {//if at food location, pick up food
            p.current_image = "pics/human_gathering.png";
            //need to implement food item in the map to then pick up
            Environment::Map[food_pos.y][food_pos.x].has_food = false; //remove food from map
            p.food_inventory.push_back(1); //add food to inventory

            //this section, the part about getting more than 1 food, seems to sometimes work and sometimes not, don't know why, need to fix
            //if there is more food very close by and inventory has less than 4 items, then gather that as well, without moving to those positions. Need to adjust so it's not instant pickup
            if (p.food_inventory.size() < 4) {
                vector<Position> food_pos_list2 = find("food", 2, 4-p.food_inventory.size());//get as many additional results as needed to reach 4 items in inventory
                if (food_pos_list2.size() > 0) {
                    for (Position fpos : food_pos_list2) {
                        Environment::Map[fpos.y][fpos.x].has_food = false; //remove food from map
                        p.food_inventory.push_back(1); //add food to inventory
                    }
                }
            }

            return true;
        }
    }
    else { //food is no longer there and search function must begin again
        return true;
    }
    return false;

}

bool People::sharing_food(){
    /*sharing food
    start:
    - if have food and someone else asks for food, give food, keep track of group/family members and who is hungriest, distribute food according to need such that 
      everyone's hunger level is equalized
    end:
    - if no longer have food or no one is asking for food
*/

    return false;
}
