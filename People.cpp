#include "People.hpp"

//currently works as expected, with the exception of a random unkown crash that crashes the whole computer. Maybe related to death but unsure, haven't tested death state.

//To Do Now: 
//For some unkown reason, one npc placed a campsite near where there was only 1 food rather than the required 4. In another case it didn't place a campsite despite passing enough food. Need to fix. 
//Encapsulate more into each function in the utility function
//FIX NOW: not sure if reproduce is working properly, npcs walk far away at times probably due to this function, also stand still sometimes, unsure why

using namespace std;

People::Person People::p; //used to store currently updating Person
vector<People::Person> People::people_list; //used to store all Person instances
vector<People::Message> People::map_message_list;

People::People() {
    Person p1 = { new_person_id(), {5,5}, true};
    people_list.push_back(p1);
    Person p2 = { new_person_id(), {8,8}, false};
    people_list.push_back(p2);

    for (int i = 0; i < people_list.size(); i++) {
        p = people_list[i];
        set_up_camp(); //for now, just set up a camp nearby at start
        people_list[i] = p;
    }
}

int People::new_person_id() {//unsure if this function is redundant with how int++ works or if there's a better method
    people_id_iterator++;
    return people_id_iterator;
}

bool message_clear_flag = false;
void People::update_all(int day_count, int hour_count, int hours_in_day) {
    for(int i = 0; i < people_list.size(); i++) {
        p = people_list[i]; //currently simply copying Person instance to utilize as an object when updating, but may be better to use a reference to the object in the list instead?
        if (p.is_alive) {
            update(day_count, hour_count, hours_in_day);
        }
        people_list[i] = p;
    }
    //clear global message list every other update. One issue with npcs cooperating or communicating is sequence order, 
    //as in if npc1 updates before npc2, anything npc2 says or does won't be witnessed unless it is still there on the next update, NEED TO FIX
    if (message_clear_flag) {
        map_message_list.clear();
    }
    message_clear_flag = !message_clear_flag;
    
}

bool People::check_death() {

    bool starvation = p.hunger_level > 1000;

    bool death = starvation;

    if (death) {
        p.is_alive = false;
        p.current_state = "dead";
        p.current_image = "pics/human_dead.png";

        /* //print record of function history for debugging
        cout << "______" << endl;
        for (string s : p.function_record) {
            cout << s << ", " << endl;
        } */

        return true;
    }
}

void People::update(int day_count, int hour_count, int hours_in_day) {
    if (check_death()) {
        return;
    }

	if (hour_count == 0) { //once a day check
		if (p.hunger_level > 50) { //tracks for continuous hungry days at the start of every day
			p.hungry_time++;
		}
		else {
			p.hungry_time = 0;
		}
        p.function_record.erase(p.function_record.begin(), p.function_record.end()-5);//once a day, erases all function records except the last 5
	}

    p.hunger_level++; //hunger increases by 1 per hour, meaning it must decrease by 20 per day to stay at 0 daily average
    p.tired_level++; //same for tired level
    p.campsite_age++;
    p.reproduction_cooldown++; //for when to find mate and create new person

    utility_function();
}

bool People::valid_position(Position pos) {
    bool valid_x = 0 <= pos.x && pos.x < Environment::map_x_max;
    bool valid_y = 0 <= pos.y && pos.y < Environment::map_y_max;
    if (valid_x && valid_y) {
        return true;
    }
    return false;
}

//person sometimes stops moving, need to check functions that call move_to to make sure they aren't asking to move to the same tile person is on
bool People::move_to(Position dest) {//need to add speed of moving from one tile to another and how many tiles at a time. Also need to add a check to prevent it being called more than once per person per update.
    if (!valid_position(dest)) { //for debugging, kill npc if it tries to go off map
        p.is_alive = false;
        p.current_image = "pics/debug.png";
        return true;
    }

    if (p.pos.x < dest.x) {//for future optimization, see: https://stackoverflow.com/questions/14579920/fast-sign-of-integer-in-c
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

    bool reached = p.pos == dest;
    return reached;
}

bool People::reproduce() {
    vector<Position> pos_list = find("mate", p.sightline_radius, 1);
    Person p2;
    bool mate_found = false;
    bool mate_willing = false;
    if (pos_list.size() > 0) {
        mate_found = true;
        for (Person pers : people_list) {//get person at one's same position, won't work once collision is implemented
            if (pers.sex!=p.sex && pers.pos == p.pos && pers.reproduction_cooldown > 100) {//if potential mate is willing to mate
                p2 = pers;
                mate_willing = true;
                break;
            }
        }

        if (mate_found && move_to(pos_list[0])) {
            if (mate_willing) {
                //create a new human, add pregnancy later, only female creates child
                if (!p.sex) {
                    int sex = rand() % 2;
                    Person child = { new_person_id(), p.pos, sex };
                    people_list.push_back(child);
                    p.reproduction_cooldown = 0;//reset
                    p2.reproduction_cooldown = 0;//unsure if this is the best way to handle interaction between 2 people, speaking or some other function might be better to avoid 2 people not being in sync
                    update_person(p2);
                }
                else {
                    //if male, simply wait for female to update one's reproduction_cooldown
                }
                if (p.reproduction_cooldown == 0) {
                    return true;
                }
            }
        }
    }
    else {//if no mate found, walk to search
        if (p.fo4.x == -1 || move_to(p.fo4)) {//initialize function object or reinitialize if reached destination
            p.fo4 = walk_search_random_dest();
        }//the move_to function triggers in the conditional
    }
    return false;
}

void People::update_person(Person pers) {
    for (int i = 0; i < people_list.size(); i++) {
        if (people_list[i].id == pers.id) {
            people_list[i] = pers;
        }
    }
}


void People::find_replacement(int radiusmax) {
    Position o = p.pos;//origin

    for (int radius = 0; radius <= radiusmax; radius++) { //this function checks tilemap in outward rings by checking top/bottom then left/right ring boundaries
        if (radius == 0) {//avoids double checking origin
            std::cout << Map[o.y][o.x].x;
            continue;
        }

        int xmin = o.x - radius;
        int xmax = o.x + radius;
        for (int x = xmin; x <= xmax; x++) {
            std::cout << Map[o.y + radius][x].x;
            std::cout << Map[o.y - radius][x].x;
        }

        int ymin = o.y - radius;
        int ymax = o.y + radius;
        for (int y = ymin + 1; y <= ymax - 1; y++) {//+1 and -1 to avoid double checking corners
            std::cout << Map[y][o.x + radius].x;
            std::cout << Map[y][o.y - radius].x;
        }
    }

}

void People::find_check(Position pos, string target) {
    if (valid_position({ pos.x,pos.y })) {
        if (target == "food") {
            if (Environment::Map[y][x].has_food) {//check if tile contains food, currently is simply tied directly to tile rather than being an item
                found = true;
            }
        }
        else if (target == "people" || target == "mate") {
            for (Person pers : people_list) {//checks the positions of all people in global people list and adds those that have a position matching the current tile being checked
                Position tmppos = { x,y };
                if (pers.pos == tmppos && p.id != pers.id) {
                    found = true;
                    if (target == "mate") {
                        if (pers.sex != p.sex) {
                            found = true;
                        }
                        else {
                            found = false;
                        }
                    }

                    break;//this assumes there's only 1 person per tile, but collision has not yet been implemented so a tile can have many people, NEED TO FIX
                }
            }
        }

        if (found) {
            found = false;
            pos_list.push_back({ x,y });
            quantity_found++;
            if (quantity_found == quantity) {//returns list of positions found once desired quantity is found
                return pos_list;
            }
        }
    }
}

//if quantity parameter is < 0 (usually -1), then returns all results
vector<People::Position> People::find(string target, int radius, int quantity) {//A radius of 1 is the tile the person is on, a radius of 2 is all adjacent tiles to person
    vector<Position> pos_list;
    int tmpradius = 0;
    int quantity_found = 0;

    bool found = false;
    while (tmpradius != radius) { //this is just a way to return closest item found but uses far too many redundant loops, need to optimize by only checking each tile once in outward rings
        for (int y = p.pos.y - tmpradius; y < p.pos.y + tmpradius; y++) { //searches in a x tile radius around the person
            for (int x = p.pos.x - tmpradius; x < p.pos.x + tmpradius; x++) {



                find_check({ x,y }, target);



            }
        }
        tmpradius++;
    }
    return pos_list;
}

//need to make generic for general inventory
bool People::has_food() {
    if (p.food_inventory.size() > 0) {
        return true;
    }
    return false;
}

//need to make generic for general messages
void People::speak(string message_text) {//currently all messages are to everyone
    //current valid messages include:
    //"food request"
    //"giving food"
    Message m = { p.id, -1, message_text };//creates message
    for (int y = p.pos.y - p.audioline_radius; y < p.pos.y + p.audioline_radius; y++) {//creates copies of message for each map position it reaches then adds to global message list
        for (int x = p.pos.x - p.audioline_radius; x < p.pos.x + p.audioline_radius; x++) {
            if (valid_position({ x,y })) {
                m.pos = { x,y };
                map_message_list.push_back(m);
            }
        }
    }
}

vector<People::Message> People::check_messages() {//might also serve as a generic for reacting to sounds
    vector<Message> found_messages;
    for (int y = p.pos.y - p.audioline_radius; y < p.pos.y + p.audioline_radius; y++) {
        for (int x = p.pos.x - p.audioline_radius; x < p.pos.x + p.audioline_radius; x++) {
            if (valid_position({ x,y })) {
                for (Message m : map_message_list) {
                    Position tmppos = { x,y };
                    if (m.pos == tmppos) {
                        if (found_messages.size() > 0) {//vector has a check empty function, should use that instead in all cases, need to fix
                            bool repeated_message = false;
                            for (Message m1 : found_messages) {
                                if (m.messsage == m1.messsage && m.sender_id == m1.sender_id && m.reciever_id == m1.reciever_id) {//avoids copying messages that differ only in their location
                                    repeated_message = true;
                                    break;
                                }
                            }
                            if (!repeated_message && m.sender_id != p.id) {
                                found_messages.push_back(m);
                            }
                        }
                        else if(m.sender_id!=p.id){
                            found_messages.push_back(m);
                        }
                    }
                }
            }
        }
    }
    return found_messages;
}

//this function is getting cluttered, encapsulate more in each function and add categorization to functions to keep them better organized
void People::utility_function() {//is currently actually just a behavior tree not a utility function
    //these bools determine which functions can be called

    bool someone_requested_food = false;
    bool someone_is_giving_food = false;
    Message food_request;
    vector<Message> nearby_messages = check_messages();
    for (Message m : nearby_messages) {
        if (m.messsage == "food request") {
            someone_requested_food = true;
            food_request = m;
        }
        else if (m.messsage == "giving food") {
            someone_is_giving_food = true;
        }
    }
    


    
    //check for nearby food
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

    //check for nearby people, later adjust so check nearby tiles not people_list
    vector<Position> nearby_people = find("people", p.sightline_radius, -1);//-1 means return all results, quantity unlimited
    bool people_found = false;
    if (nearby_people.size() > 0) {
        people_found = true;
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
    bool start_sleeping = !p.awake || (tired && p.pos == p.campsite_pos && p.campsite_pos.x != -1) || very_tired; //Need to do: overload == to make comparing positions easier

    bool hungry = p.hunger_level > 50;
    //if (start_search_for_new_campsite) {//this doesn't trigger given the new numbers for when hungry triggers, is it actually necessary or was the problem just the other numbers?
    //    hungry = p.hunger_level > 40 && (p.function_record.back() != "searching_for_food" || p.function_record.back() != "gathering_food" || p.function_record.back() != "eating"); //allows more time searching for campsite instead of food, unless food search was already triggered in which case keep getting food to reset hunger padding
    //}
    
    bool start_eating = hungry && has_food();
    bool start_searching_for_food = hungry && !has_food();

    bool start_request_food = start_searching_for_food && people_found && !found_food;
    if (start_request_food) {//spoken messages might need their own priority tree separate from the movement related actions below
        if (someone_is_giving_food) {//if someone is giving food, do nothing, however this might cause problems by interrupting things like eating/sleep/etc until one recieves food
            //currently doesn't actually do nothing due to sequence order issue of NPCs updating, therefore only does nothing every other turn. NEED TO FIX
            return;
        }
        speak("food request");
    }

    bool start_give_food = !hungry && has_food() && someone_requested_food; 

    bool start_mate_search = false && p.reproduction_cooldown > 100;//currently set to false to prevent execution while optimizing

    if (p.clean_image) {
        p.current_image = "pics/human.png";
        p.clean_image = false;
    }
    
    //select which specific function to call right now
    //this implementation allows functions to be interrupted by higher priority ones on every update, however this means that a function may not properly reset or preserve as needed for when it gets called again later, need to fix
    //to further encapsulate each function, it may make sense to move the condition for execution inside each function and have each function return an int where 1 == done, 0 == in progress and -1 == condition not met, then have each function execute in a priority sequence and exit out of the utility function if any returns either 0 or 1

    if (start_sleeping) {
        p.function_record.push_back("sleeping");
        p.current_state = "sleeping";
        sleeping();
    }
    else if (start_give_food) {
        p.function_record.push_back("give_food");
        p.current_state = "give_food";
        give_food(food_request);
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
    else if (start_mate_search) {
        p.function_record.push_back("reproduce");
        p.current_state = "reproduce";
        reproduce();
    }
    else {
        //if all other functions are false, then idle.
        p.function_record.push_back("idle");
        p.current_state = "idle";
        idle();
    }
}

bool People::give_food(Message m) {
    p.current_image = "pics/human_giving_food.png";
    speak("giving food");

    Person p2;
    for (Person p1 : people_list) {//store this person in a function object so as not to repeat search every call, fix this
        if (p1.id == m.sender_id) {
            p2 = p1;
            break;
        }
    }
    if (move_to(p2.pos)) {
        p2.food_inventory.push_back(1); //give food
        p.food_inventory.pop_back();//remove from own inventory
        p.current_image = "pics/human.png";
        //update people_list
        for (int i = 0; i < people_list.size(); i++) {
            if (people_list[i].id == p2.id) {
                people_list[i] = p2;
                break;
            }
        }
        return true;
    }//move towards p2 until reached
    return false;
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
        p.fo1.pos = walk_search_random_dest();
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

People::Position People::walk_search_random_dest() {
    Position dest;
    bool valid_dest = false;
    while (!valid_dest) {
        int max = Environment::map_x_max;
        int min = 10;
        int sign = (rand() % 2);
        (sign == 0) ? sign = -1 : sign = 1;
        dest.x = sign * ((rand() % (max - min)) + min); //select a destination
        max = Environment::map_y_max;
        min = 10;
        sign = (rand() % 2);
        (sign == 0) ? sign = -1 : sign = 1;
        dest.y = sign * ((rand() % (max - min)) + min);
        valid_dest = valid_position(dest);

        //limit search range to within x distance from campsite, if have campsite
        if (p.campsite_pos.x > -1) {
            Position dist_pos = distance(p.campsite_pos, dest);
            if (dist_pos.x > campsite_distance_search || dist_pos.y > campsite_distance_search) {
                valid_dest = false;
            }
        }
    }
    return dest;
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

     //move x steps in a random direction, x is large to cover more distance, x is a multiple of 10 which is the radius to check for food in the area
     if (p.fo2.pos.x == -1 || move_to(p.fo2.pos)) {//initialize function object or reinitialize if reached destination
             p.fo2.pos = walk_search_random_dest();
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
     return false;
}

People::Position People::distance(Position pos1, Position pos2) {//should return double instead of Position, fix later
    return { abs(pos1.x - pos2.x) , abs(pos1.y - pos2.y) };
}

bool People::gathering_food(){
    /*gathering food
    //start:
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
