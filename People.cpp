#include "People.hpp"
using namespace std;
//currently works as expected, with the exception of a random unkown crash that crashes the whole computer. Maybe related to death but unsure, haven't tested death state.

//To Do Now: 
//For some unkown reason, one npc placed a campsite near where there was only 1 food rather than the required 4. In another case it didn't place a campsite despite passing enough food. Need to fix. Unsure if this is still an issue

//need to fix: Many functions have redundant code in them given the reorganization of including the function triggers inside the functions themselves

//People::Person* People::p; //used to store currently updating Person, by pointer.
vector<People::Person> People::pl; //pl, using the name pl because of the frequency of use, used to store all Person instances
vector<People::Message> People::Message_Map[Environment::map_y_max][Environment::map_x_max];
int p = -1;//index for accessing current person. Using index to access instead of a pointer because list may change such as when a new person is born or dies which invalidates pointers to pl (people_list)
vector<int> People::Person_Map[Environment::map_y_max][Environment::map_x_max];

People::People() {
    Person p1 = { new_person_id(), {5,5}, true};
    pl.push_back(p1);
    Person_Map[p1.pos.y][p1.pos.x].push_back(p1.id);
    Person p2 = { new_person_id(), {8,8}, false};
    pl.push_back(p2);
    Person_Map[p2.pos.y][p2.pos.x].push_back(p2.id);
}

int People::new_person_id() {//unsure if this function is redundant with how int++ works or if there's a better method
    people_id_iterator++;
    return people_id_iterator;
}

void People::update_all(int day_count, int hour_count, int hours_in_day) {
    for(int i = 0; i < pl.size(); i++) {
        p = i;
        update(day_count, hour_count, hours_in_day);
        
    }
    //clear global message list every other update. One issue with npcs cooperating or communicating is sequence order, 
    //as in if npc1 updates before npc2, anything npc2 says or does won't be witnessed unless it is still there on the next update, NEED TO FIX
    if (pl[p].message_clear_flag) {
        for (int y = 0; y < Environment::map_y_max; y++) {//unsure if this is the most efficient way to clear Message_Map
            for (int x = 0; x < Environment::map_x_max; x++) {
                Message_Map[y][x].clear();
            }
        }
    }
    pl[p].message_clear_flag = !pl[p].message_clear_flag;
    

    //for debugging
    if (day_count == -1) {//kill all people. set to -1 to prevent execution
        for (Person& pip : pl) {
            pip.is_alive = false;
        }
    }
}

bool People::check_death() {

    bool starvation = pl[p].hunger_level > 1000;

    bool death = !pl[p].is_alive || starvation;

    

    if (death) {
        pl[p].is_alive = false;
        pl[p].current_state = "dead";
        pl[p].current_image = "pics/human_dead.png";

        if(!pl[p].printed){
            //print record of function history for debugging
            cout << "\n______\n"<<pl[p].id<<" ~ ";
            for (int i = 0; i < pl[p].function_record.size(); i++) {
                cout << pl[p].function_record[i] << " - " << pl[p].function_record_nums[i] << " ::: ";
            }
            cout<<"\n"
            <<"\nsex: " << pl[p].sex
            << "\nhunger_level: " << pl[p].hunger_level
            << "\nhungry_time: " << pl[p].hungry_time
            << "\ntired_level: " << pl[p].tired_level
            << "\nfood_inventory size: " << pl[p].food_inventory.size()
            << "\nCampsite pos: x- " << pl[p].campsite_pos.x <<" y-"<<pl[p].campsite_pos.y
            << "\ncampsite_age: " << pl[p].campsite_age
            << "\nreproduction_cooldown: " << pl[p].reproduction_cooldown
            << "\nfound_messages size: " << pl[p].found_messages.size();

            pl[p].printed = true;
        }

        return true;
    }
    return false;
}

void People::update(int day_count, int hour_count, int hours_in_day) {
    if (check_death()) {
        return;
    }

	if (hour_count == 0) { //once a day check
		if (pl[p].hunger_level > 50) { //tracks for continuous hungry days at the start of every day
			pl[p].hungry_time++;
		}
		else {
			pl[p].hungry_time = 0;
		}
        //if (pl[p].function_record.size() >= 5) {
        //    pl[p].function_record.erase(pl[p].function_record.begin(), pl[p].function_record.end() - 5);//once a day, erases all function records except the last 5
        //}
    }

    pl[p].hunger_level++; //hunger increases by 1 per hour, meaning it must decrease by 20 per day to stay at 0 daily average
    pl[p].tired_level++; //same for tired level
    pl[p].campsite_age++;
    pl[p].reproduction_cooldown++; //for when to find mate and create new person

    utility_function();
    pl[p].found_messages.clear();
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
    if (!valid_position(dest)) { //for debugging, kill npc if it tries to go off map or is asked to move to the spot it is already at
        pl[p].is_alive = false;
        pl[p].current_image = "pics/debug.png";
        return true;
    }
    for (int i = 0; i < Person_Map[pl[p].pos.y][pl[p].pos.x].size(); i++) {
        if (Person_Map[pl[p].pos.y][pl[p].pos.x][i] == pl[p].id) {
            Person_Map[pl[p].pos.y][pl[p].pos.x].erase(Person_Map[pl[p].pos.y][pl[p].pos.x].begin() + i);//remove person from Person_Map
        }
    }
    if (pl[p].pos.x < dest.x) {//for future optimization, see: https://stackoverflow.com/questions/14579920/fast-sign-of-integer-in-c
        pl[p].pos.x++;
    }
    else if (pl[p].pos.x > dest.x) {
        pl[p].pos.x--;
    }
    
    if (pl[p].pos.y < dest.y) {
        pl[p].pos.y++;
    }
    else if (pl[p].pos.y > dest.y) {
        pl[p].pos.y--;
    }

    Person_Map[pl[p].pos.y][pl[p].pos.x].push_back(pl[p].id);//add person back to Person_Map at new location
    bool reached = pl[p].pos == dest;
    return reached;
}

void People::add_func_record() {
    if (pl[p].function_record.empty()) {
        pl[p].function_record.push_back(pl[p].current_state);
        pl[p].function_record_nums.push_back(1);
    }
    else {
        if (pl[p].function_record.back() == pl[p].current_state) {
            pl[p].function_record_nums.back()++;
        }
        else {
            pl[p].function_record.push_back(pl[p].current_state);
            pl[p].function_record_nums.push_back(1);
        }
    }
}

int People::p_by_id(int id){//uses binary search to find and return index to person in people list (pl)
    int low = 0;
    int high = pl.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (pl[mid].id == id) {
            return mid;
        }
        (pl[mid].id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}

bool People::reproduce() {
    if (!(pl[p].reproduction_cooldown > 100)) {//function trigger
        return false;
    }
    pl[p].current_state = "reproduce";
    vector<Position> &pos_list = pl[p].all_found[pl[p].target_index["mate"]];//note: using reference (&) reduces copying
    int p2 = -1;
    bool mate_willing = false;
    if (!pos_list.empty()) {
        for (int i = 0; i < Person_Map[pos_list[0].y][pos_list[0].x].size();i++) {
            int index = p_by_id(Person_Map[pos_list[0].y][pos_list[0].x][i]);
            if (index == -1) {
                return true;//person not found, end function
            }
            if (pl[index].reproduction_cooldown > 100 && pl[index].sex != pl[p].sex) {
                mate_willing = true;
                p2 = index;
                break;
            }
        }
        if (mate_willing && move_to(pl[p2].pos)) {
                //create a new human, add pregnancy later, only female creates child
                if (!pl[p].sex) {
                    int sex = rand() % 2;
                    Person child = { new_person_id(), pl[p].pos, sex };
                    Person_Map[child.pos.y][child.pos.x].push_back(child.id);
                    pl.push_back(child);
                    pl[p].reproduction_cooldown = 0;//reset
                    pl[p2].reproduction_cooldown = 0;//unsure if this is the best way to handle interaction between 2 people, speaking or some other function might be better to avoid 2 people not being in sync
                    return true;//male will simply no longer call reproduce() given the cooldown==0, so only female needs to return true
                }
                else {
                    //if male, simply wait for female to update one's reproduction_cooldown
                }
        }
    }
    else {//if no mate found, walk to search
        if (pl[p].fo4.x == -1 || move_to(pl[p].fo4)) {//initialize function object or reinitialize if reached destination
            pl[p].fo4 = walk_search_random_dest();
        }//the move_to function triggers in the conditional
    }
    return true;//in progress
}

//if quantity parameter is < 0 (usually -1), then returns all results. Quantity limit not implemented
vector<vector<People::Position>> People::find_all(vector<string> potential_targets) {//All potential targets are searched for
    int radius_options[2] = {//all radius options
        pl[p].sightline_radius, pl[p].audioline_radius
    };
    if(pl[p].radiusmax == -1){//used to store result instead of calling every time, only resets if one of the radius options changes such as damaged eyesight, etc
        for (int i = 0; i < 2; i++) {//selects largest radius
            if (i == 0) {
                pl[p].radiusmax = radius_options[i];
            }
            else if (pl[p].radiusmax < radius_options[i]) {
                pl[p].radiusmax = radius_options[i];
            }
        }
    }
    //int quantity; // still need to adjust find_check and handle quantity limits for some targets
    Position o = pl[p].pos;//origin
    vector<vector<Position>> pos_list;
    vector<int> target_quantity_current;
    //int quantity_found = 0;       Need to implement a quantity limit for each potential target to cut down on iterations
    for (int radius = 0; radius <= pl[p].radiusmax; radius++) { //this function checks tilemap in outward rings by checking top/bottom and left/right ring boundaries
        if (radius == 0) {//avoids double checking origin
            for (int i = 0; i < potential_targets.size(); i++) {
                target_quantity_current.push_back(0);//initialize all to 0
                vector<Position> emptylist;
                pos_list.push_back(emptylist);//initialize all target lists as empty vectors
                if (potential_targets[i] == "food" || potential_targets[i] == "people" || potential_targets[i] == "mate" || potential_targets[i] == "no campsite") {
                    if (radius > pl[p].sightline_radius) {
                        continue;
                    }
                }
                else if (potential_targets[i] == "messages") {
                    if (radius > pl[p].audioline_radius) {
                        continue;
                    }
                }
                if (find_check({ o.x,o.y }, potential_targets[i])) {
                    pos_list[i].push_back({o.x,o.y});
                    target_quantity_current[i]++;
                    /*
                    quantity_found++;
                    if (quantity_found == quantity) {//returns list of positions found once desired quantity is found
                        return pos_list;
                    }
                    */
                }
            }
        }
        int xmin = o.x - radius;
        int xmax = o.x + radius;
        int ymin = o.y - radius + 1;//+1 and -1 to avoid double checking corners
        int ymax = o.y + radius - 1;
        for (int x = xmin, y = ymin; x <= xmax; x++, y++) {
            for (int sign = -1; sign<=1; sign+=2) {//sign == -1, then sign == 1
                for (int i = 0; i < potential_targets.size(); i++) {
                    //handle which targets to check or not check
                    if (!pl[p].target_chosen[i]) {
                        continue;
                    }
                    //handles quantity limits
                    if (target_quantity_current[i] >= pl[p].target_quantity_limit[i]) {
                        continue;
                    }
                    //Handle differing radius limits for different potential targets
                    if (potential_targets[i]=="food" || potential_targets[i]=="people") {
                        if (radius > pl[p].sightline_radius) {
                            continue;
                        }
                    }
                    else if (potential_targets[i] == "messages") {
                        if (radius > pl[p].audioline_radius) {
                            continue;
                        }
                    }
                    if (find_check({ x, o.y + (sign * radius) }, potential_targets[i])) {
                        pos_list[i].push_back({ x, o.y + (sign * radius) });
                        target_quantity_current[i]++;
                        /*
                        quantity_found++;
                        if (quantity_found == quantity) {//returns list of positions found once desired quantity is found
                            return pos_list;
                        }
                        */
                    }
                    if (y <= ymax) {
                            if (find_check({ o.x + (sign * radius), y }, potential_targets[i])) {
                                pos_list[i].push_back({ o.x + (sign * radius), y });
                                target_quantity_current[i]++;
                                /*
                                quantity_found++;
                                if (quantity_found == quantity) {//returns list of positions found once desired quantity is found
                                    return pos_list;
                                }
                                */
                            }
                    }
                }
            }
        }
    }
    return pos_list;
}

bool People::find_check(Position pos, string target) {
    bool found = false;
    if (valid_position({ pos.x,pos.y })) {
        if (target == "food") {
            if (Environment::Map[pos.y][pos.x].has_food) {//check if tile contains food, currently is simply tied directly to tile rather than being an item
                found = true;
            }
        }
        else if (target == "people" || target == "mate") {
            for (int pers_id : Person_Map[pos.y][pos.x]) {
                if (pl[p].id != pers_id) {
                    found = true;
                    if (target == "mate") {
                        if (pl[p_by_id(pers_id)].sex != pl[p].sex) {
                            found = true;
                            break;
                        }
                        else {
                            found = false;
                        }
                    }
                    break;//this assumes there's only 1 person per tile, but collision has not yet been implemented so a tile can have many people, NEED TO FIX
                }
            }
        }
        else if (target == "no campsite") {//find nearest location without a campsite
            if (Environment::Map[pos.y][pos.x].has_tent == false) {
                found = true;
            }
        }
        else if (target == "messages") {//might also serve as a generic for reacting to sounds
            for (Message m : Message_Map[pos.y][pos.x]) {//check all messages in this tile
                if (pl[p].found_messages.size() > 0) {//vector has a check empty function, should use that instead in all cases, need to fix
                    bool repeated_message = false;
                    for (Message m1 : pl[p].found_messages) {
                        if (m.messsage == m1.messsage && m.sender_id == m1.sender_id && m.reciever_id == m1.reciever_id) {//avoids copying messages that differ only in their location
                            repeated_message = true;
                            break;
                        }
                    }
                    if (!repeated_message && m.sender_id != pl[p].id) {
                        pl[p].found_messages.push_back(m);
                    }
                }
                else if (m.sender_id != pl[p].id) {
                    pl[p].found_messages.push_back(m);
                }
            }
            //messages currently don't need to return what tile they were found at, therefore return false even if found messages as the messages are stored in pl[p].found_messages
            return false;
        }
    }
    return found;
}

//can this function also be folded into the find_all function somehow to further reduce for loops searching on the map?
void People::speak(string message_text) {//currently all messages are to everyone
    //current valid messages include:
    //"food request"
    //"giving food"

    //the outward ring method might make more sense in this function to allow certain objects such as walls to block sound, might implement later but not currently
    Message m = { pl[p].id, -1, message_text };//creates message
    m.origin = pl[p].pos;
    for (int y = pl[p].pos.y - pl[p].audioline_radius; y < pl[p].pos.y + pl[p].audioline_radius; y++) {//creates copies of message for each map position it reaches then adds to global message list
        for (int x = pl[p].pos.x - pl[p].audioline_radius; x < pl[p].pos.x + pl[p].audioline_radius; x++) {
            if (valid_position({ x,y })) {
                m.pos = { x,y };
                Message_Map[y][x].push_back(m);
            }
        }
    }
}

//need to add categorization to functions to keep them better organized
void People::utility_function() {//is currently actually just a behavior tree not a utility function. Selects what action to take this tick.
    /* unsure how to prevent searching for mate in map search if not needed
    if (!start_mate_search) {
        pl[p].target_chosen[pl[p].target_index["mate"]] = false; //prevent search for mate in map search
    }
    else {
        pl[p].target_chosen[pl[p].target_index["mate"]] = true;
    }
    */

    /*part of old iteration, unsure if still has use
    //if (start_search_for_new_campsite) {//this doesn't trigger given the new numbers for when hungry triggers, is it actually necessary or was the problem just the other numbers?
        hungry = pl[p].hunger_level > 40 && (pl[p].function_record.back() != "searching_for_food" || pl[p].function_record.back() != "gathering_food" || pl[p].function_record.back() != "eating"); //allows more time searching for campsite instead of food, unless food search was already triggered in which case keep getting food to reset hunger padding}*/

    if (pl[p].clean_image) {
        pl[p].current_image = "pics/human.png";
        pl[p].clean_image = false;
    }
    //this implementation allows functions to be interrupted by higher priority ones on every update, however this means that a function may not properly reset or preserve as needed for when it gets called again later, need to fix
    //if(func()==false) go to next func(), if(func()==true) executed this func() for both in progress and done cases, use a separate flag to declare a func() as done
    bool skip_map_search = false;
    bool is_idle = true;
    //if these function triggers are true, being highest priority, skip the map search and related function triggers
    if (sleeping()) {
        is_idle = false;
        skip_map_search = true;
    }
    else if (eating()) {
        is_idle = false;
        skip_map_search = true;
    }
    if (!skip_map_search) {
        //map search
        pl[p].all_found = find_all(pl[p].potential_targets);
        //bools that require map search
        if (give_food()) {}
        else if (moving_to_bed()) {}
        else if (searching_for_food()){if (gathering_food()) {}}
        else if (search_for_new_campsite()) {if (set_up_camp()) {}}
        else if (reproduce()) {}
        else {idle();}
    }
    else if(is_idle){ idle(); }
    add_func_record();//adds current function to record
    pl[p].function_done = false;//unsure if this variable has a use, it is currently part of every function above
    pl[p].all_found.clear();
}

bool People::moving_to_bed() {
    bool tired = pl[p].tired_level > 50;
    bool start_moving_to_bed = tired && pl[p].campsite_pos.x != -1;
    if (!start_moving_to_bed) {
        return false;
    }
    pl[p].current_state = "moving to bed";
    pl[p].function_done = move_to(pl[p].campsite_pos); //go to campsite.
    return true;//done and in progress
}

bool People::give_food() {
    bool hungry = pl[p].hunger_level > 50;
    bool someone_requested_food = false;
    Message food_request;
    if (!pl[p].found_messages.empty()) {
        for (Message m : pl[p].found_messages) {
            if (m.messsage == "food request") {
                someone_requested_food = true;
                food_request = m;
            }
        }
    }
    bool start_give_food = !hungry && !pl[p].food_inventory.empty() && someone_requested_food;
    if (!start_give_food) {//function trigger
        return false;
    }
    pl[p].current_state = "give_food";
    pl[p].current_image = "pics/human_giving_food.png";
    speak("giving food");
    Person& p2 = pl[p_by_id(food_request.sender_id)];//store this person in a function object so as not to repeat search every call, fix this
    
    if (move_to(p2.pos)) {
        p2.food_inventory.push_back(1); //give food
        pl[p].food_inventory.pop_back();//remove from own inventory
        pl[p].current_image = "pics/human.png";//reset image to default
        pl[p].function_done = true;
        return true;//done
    }//move towards p2 until reached
    return true;//in progress
}

bool People::search_for_new_campsite(){ //need to bias search direction in the direction of wherever there is more food rather than waiting to randomly stumble on a site with enough food for campsite. Also need to add a system of not searching the same tile within too short a time frame.
    //check for nearby food
    if (pl[p].start_set_up_camp) {
        return true;//continue to next function in function chain
    }
    
    vector<Position> food_pos_list = pl[p].all_found[pl[p].target_index["food"]]; //gets results, assigns 1 result to food_pos
    Position food_pos;
    bool found_food = false;
    if (!food_pos_list.empty()) {
        food_pos = food_pos_list[0];
        found_food = true;
    }
    else {
        food_pos = { -1,-1 };
    }
    int dist = distance(food_pos, pl[p].campsite_pos);

    bool cond1 = !found_food || (found_food && dist > campsite_distance_search);//if no food found OR food was found but the distance is too far from the campsite
    bool cond2 = pl[p].campsite_pos.x == -1 || pl[p].hungry_time >= 3;//AND: have no campsite OR have been hungry too long
    bool cond3 = pl[p].campsite_age > 10;//AND: campsite is old enough to move again. Unsure if this might have an issue if the null campsite has an age

    bool start = cond1 && cond2 && cond3;

    //currently only creates a campsite after having been hungry 3 days. Still need to figure out when and when not to create a campsite, such as for trips away from home or extreme high mobility nomad
    //function trigger
    if (!start){
        return false;
    }
    
    pl[p].current_state = "search for new campsite";
    if (pl[p].campsite_pos.x != -1) { //if have campsite, remove. //Later add an option to just abandon a campsite without removing the house. Should only decontruct if going to carry it to new location such as a tent/sleeping bag/lean to/etc.
        if (move_to(pl[p].campsite_pos)) {//walk to campsite to remove
            Environment::Map[pl[p].campsite_pos.y][pl[p].campsite_pos.x].has_tent = false; //removes campsite
            pl[p].campsite_pos = { -1,-1 };
            pl[p].campsite_age = -1;
        }
    }
    //walk to search
    if (pl[p].fo1.pos.x == -1 || move_to(pl[p].fo1.pos)) {//initialize function object or reinitialize if reached destination
        pl[p].fo1.pos = walk_search_random_dest();
    }//the move_to function triggers in the conditional
    if (food_pos_list.size() >= 4) {//if there are 4 food items within sight, select area for campsite, else keep searching
        pl[p].start_set_up_camp = true;
        pl[p].function_done = true;
        pl[p].fo1.pos = { -1,-1 };// reset function object if ending function
        return true;//done
    }
    else if (!food_pos_list.empty()) {
        //need to add a method of investigating if any food found might have more food just out of current sightline, but this probably will require more complex modifiable pathfinding, as in have it be a detour from the current destination rather than a change in destination.
    }    
    return true;//in progress
}

bool People::set_up_camp(){
    if (!pl[p].start_set_up_camp) {
        return false;
    }
    pl[p].current_state = "set up camp";
    pl[p].campsite_age = 0; //resets campsite age
    //place tent
    vector<Position> pos_list = pl[p].all_found[pl[p].target_index["no campsite"]];
    if (!pos_list.empty()) {
        Environment::Map[pos_list[0].y][pos_list[0].x].has_tent = true; //for now simply have a tent be an attribute of the tile rather than a placed item
        pl[p].campsite_pos = pos_list[0]; //store campsite location
        pl[p].start_set_up_camp = false;
        pl[p].function_done = true;
        return true; //done. need to add an actual animation/progress to building the tent rather than immediate placement
    }
    else {
        Position dest = walk_search_random_dest();
        move_to(dest);//random movement without destination
        return true;//in progress
    }
}

People::Position People::walk_search_random_dest() {
    Position dest = {-1,-1};
    bool valid_dest = false;
    int tries = 0;//tracks how many times a destination roll has been tried, used to limit number of loops, need to find a better method
    while (!valid_dest) {//set destination by setting a direction and duration
        int max_x = Environment::map_x_max;//unsure if this is should be map_x_max or map_x_max-1 
        int min_x = 0;
        int max_y = Environment::map_y_max;
        int min_y = 0;
        if (pl[p].campsite_pos.x != -1) {
            max_x = pl[p].campsite_pos.x + campsite_distance_search;
            min_x = pl[p].campsite_pos.x - campsite_distance_search;
            max_y = pl[p].campsite_pos.y + campsite_distance_search;
            min_y = pl[p].campsite_pos.y - campsite_distance_search;
        }
        dest.x = (rand() % (max_x - min_x)) + min_x;
        dest.y = (rand() % (max_y - min_y)) + min_y;
        dest = make_position_valid(dest, max_x, min_x, max_y, min_y);
        if (!valid_position(dest)) {
            valid_dest = false;
            tries++;
        }
        else {
            valid_dest = true;
        }
    }
    return dest;
}

People::Position People::make_position_valid(Position dest, int ux, int lx, int uy, int ly) {//bounds: u==upper, l==lower
    if (dest.x < lx) {
        dest.x = lx;
    }
    else if (dest.x >= ux) {
        dest.x = ux - 1;
    }
    if (dest.y < ly) {
        dest.y = 0;
    }
    else if (dest.y >= uy) {
        dest.y = uy - 1;
    }
    return dest;
}

bool People::idle(){
    pl[p].current_state = "idle";
    //flip between idle and default image
    if (pl[p].current_image == "pics/human.png") {
        pl[p].current_image = "pics/human_idle.png"; //need to make image, just have human with raised hands
    }
    else {
        pl[p].current_image = "pics/human.png";
    }
    return true;
}

bool People::sleeping(){
    bool tired = pl[p].tired_level > 50;
    bool very_tired = pl[p].tired_level > 100;//might need to cap sleep such that a person can't ever have a tired_level over x_level as well as under y_level
    bool cond1 = tired && (pl[p].pos == pl[p].campsite_pos || pl[p].campsite_pos.x != -1);//if tired AND either at campsite or have no campsite
    if (!(!pl[p].awake || cond1 || very_tired)) {//function trigger
        return false;
    }
    pl[p].current_state = "sleeping";
    pl[p].current_image = "pics/human_sleeping.png";
    pl[p].awake = false;
    pl[p].tired_level-=11; //every call to this function reduces tired by 11, this means need 5 hours/updates to stop sleeping and sleep every 50 hours/updates. Is -11 so as to do -10 per hour and also -1 to negate the +1 tired in the regular update function
    if (pl[p].tired_level <= 0) {//fix this, need to cap at 0, also need cap for upper limit?
        pl[p].current_image = "pics/human.png";
        pl[p].awake = true;
        pl[p].function_done = true;
        return true;//done
    }
    return true;//in progress
}

bool People::eating(){
    bool hungry = pl[p].hunger_level > 50;
    if (!(hungry && pl[p].food_inventory.size() > 0)) {//function trigger
        return false;
    }
    pl[p].current_state = "eating";
    if (pl[p].fo3.progress == 0) {
        pl[p].current_image = "pics/human_eating.png";
    }
    pl[p].fo3.progress++;
    if (pl[p].fo3.progress == pl[p].fo3.progress_done) {//makes eating take more than 1 frame
        pl[p].fo3.progress = 0;//reset
        //need to delete food item in inventory from game once an item system is created
        pl[p].food_inventory.pop_back();//delete item from inventory
        pl[p].hunger_level -= 50; //reduce hungry level by 10, therefore need 2 meals a day to stay at 0 hunger_level average
        pl[p].function_done = true;
        pl[p].clean_image = true; //when this function ends, return to default image on next update
        return true;//done eating
    }
     return true;//in progress
}

//when searching_for_food() and gathering_food have been fixed, make them generics for any item
//fix this: to reduce overlap between searching_for_food() and gathering_food(), searching_for_food() should handle moving until food is found, once found it should pass the position of the food to gathering_food() which only checks that position in case it no longer contains food, this is done as a function chain like set_up_camp()
bool People::searching_for_food(){ //this function is far too similar to gathering_food() and should be consolidated or call gathering_food()
    bool hungry = pl[p].hunger_level > 50;
    if (!(hungry && pl[p].food_inventory.empty())) {//function trigger
        return false;
    }
    if (pl[p].start_gathering_food) {
        return true;//go to next function in function chain
    }
    pl[p].current_state = "searching for food";
    bool someone_is_giving_food = false;
    if (!pl[p].found_messages.empty()) {
        for (Message m : pl[p].found_messages) {
            if (m.messsage == "giving food") {
                someone_is_giving_food = true;
                break;
            }
        }
    }
    //check for nearby people
    vector<Position> nearby_people = pl[p].all_found[pl[p].target_index["people"]];
    bool people_found = false;
    if (!nearby_people.empty()) {
        people_found = true;
    }
    //check for nearby food
    vector<Position> food_pos_list = pl[p].all_found[pl[p].target_index["food"]]; //gets results
    bool found_food = false;
    Position food_pos;
    if (!food_pos_list.empty()) {
        found_food = true;
        food_pos = food_pos_list[0];
    }
    else {
        food_pos = { -1,-1 };
    }
    bool start_request_food = people_found && !found_food;
    if (start_request_food) {//spoken messages might need their own priority tree separate from the movement related actions
        if (someone_is_giving_food) {//if someone is giving food, do nothing, however this might cause problems by interrupting things like eating/sleep/etc until one recieves food
            //currently doesn't actually do-nothing due to sequence order issue of NPCs updating, therefore only does nothing every other turn. Also can't tell if food is intended for self or for another. NEED TO FIX
            //return;
        }
        speak("food request");
    }
    bool start_search_for_new_campsite = (!found_food || (found_food && distance(food_pos, pl[p].campsite_pos) > campsite_distance_search)) && (pl[p].campsite_pos.x == -1 || pl[p].hungry_time >= 3) && (pl[p].campsite_age > 10 || pl[p].campsite_age == -1); //currently only creates a campsite after having been hungry 3 days, need a trigger for creating a campsite when there is abundant food. Still need to figure out when and when not to create a campsite.
    if (start_search_for_new_campsite) {//remove campsite to be able to be unbound and get food while searching for new campsite, figure out a cleaner way to organize order between finding food to eat now and finding a new campsite
        if (pl[p].campsite_pos.x != -1) {
            if (move_to(pl[p].campsite_pos)) {//walk to campsite to remove
                Environment::Map[pl[p].campsite_pos.y][pl[p].campsite_pos.x].has_tent = false; //removes campsite
                pl[p].campsite_pos = { -1,-1 };
                pl[p].campsite_age = -1;
                return true;//in progress
            }
        }
    }
    //reset function if it was interrupted by another function
    if (pl[p].function_record.back() != "searching for food") {
        pl[p].fo2.pos = { -1,-1 }; //reset function object
    }
    if (pl[p].fo2.pos.x == -1 || move_to(pl[p].fo2.pos)) {//initialize function object or reinitialize if reached destination
            pl[p].fo2.pos = walk_search_random_dest();
            return true;//in progress
    }//the move_to function triggers in the conditional
    if (found_food) {
        if (pl[p].campsite_pos.x != -1) {
            int dist = distance(pl[p].campsite_pos, food_pos);
            if (dist > campsite_distance_search) {
                return true;//in progress
            }
        }
        pl[p].fo2.pos = { -1,-1 }; //reset function object
        pl[p].function_done = true;
        pl[p].start_gathering_food = true;
        return true;//done
    }
    return true;//in progress
}

int People::distance(Position pos1, Position pos2) {
    int xd = abs(pos1.x - pos2.x);
    int yd = abs(pos1.y - pos2.y);
    int max = -1;
    (xd > yd) ? max= xd : max= yd;
    return max;
}

bool People::gathering_food(){
    if (!pl[p].start_gathering_food) {
        return false;
    }
    pl[p].current_state = "gathering food";
    //look around self for food
    Position food_pos = { -1,-1 };
    bool found_food = false;
    vector<Position> food_pos_list = pl[p].all_found[pl[p].target_index["food"]];
    if (!food_pos_list.empty()) {
        found_food = true;
        food_pos = food_pos_list[0];
    }
    if (found_food) {
        if (pl[p].campsite_pos.x != -1) {
            int dist = distance(pl[p].campsite_pos, food_pos);
            if (dist > campsite_distance_search) {
                pl[p].function_done = true;
                pl[p].start_gathering_food = false; //resets function trigger to false when function ends
                return true; //done. keep within x distance to campsite, if outside range then end function (return true)
            }
        }
        bool reached = move_to(food_pos); //move towards food
        if (reached) {//if at food location, pick up food
            pl[p].current_image = "pics/human_gathering.png";
            //need to implement food item in the map to then pick up
            Environment::Map[food_pos.y][food_pos.x].has_food = false; //remove food from map
            pl[p].food_inventory.push_back(1); //add food to inventory
            //if there is more food very close by and inventory has less than 4 items, then gather that as well, without moving to those positions. Need to adjust so it's not instant pickup
            if (pl[p].food_inventory.size() < 4) {
                if (food_pos_list.size() > 1) {//if there are additional food found
                    for (int i = 1; i <= 3 && pl[p].food_inventory.size() <= 4 && i< food_pos_list.size(); i++) {//only gather enough to to fill inventory to 4 items
                        if (distance(food_pos_list[i], pl[p].pos) == 1) {//limit gather distance to 1 tile away from player
                            Environment::Map[food_pos_list[i].y][food_pos_list[i].x].has_food = false; //remove food from map
                            pl[p].food_inventory.push_back(1); //add food to inventory
                        }
                    }
                }
            }
            pl[p].function_done = true;
            pl[p].start_gathering_food = false; //resets function trigger to false when function ends
            return true;//done
        }
    }
    else { //food is no longer there and search function must begin again
        pl[p].function_done = true;
        pl[p].start_gathering_food = false; //resets function trigger to false when function ends
        return true;//done
    }
    return true;//in progress
}

bool People::sharing_food(){//Already handled with request and give food
    /*sharing food
    start:
    - if have food and someone else asks for food, give food, keep track of group/family members and who is hungriest, distribute food according to need such that 
      everyone's hunger level is equalized
    end:
    - if no longer have food or no one is asking for food
*/

    return false;
}
