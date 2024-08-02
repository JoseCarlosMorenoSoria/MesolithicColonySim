#include "People.hpp"
using namespace std;

vector<People::Person> People::pl; //pl, using the name pl because of the frequency of use, used to store all Person instances
vector<People::Message> People::message_list;
vector<int> People::Message_Map[Environment::map_y_max][Environment::map_x_max];
int p = -1;//index for accessing current person. Using index to access instead of a pointer because list may change such as when a new person is born or dies which invalidates pointers to pl (people_list)
ItemSys it2;//used to access member functions and variables of ItemSys

People::People() {
    Person p1 = { new_person_id(), {50,25}, true};
    p1.age = 11;
    pl.push_back(p1);
    Environment::Map[p1.pos.y][p1.pos.x].person_id = p1.id;
    Person p2 = { new_person_id(), {51,26}, false};
    p2.age = 11;
    pl.push_back(p2);
    Environment::Map[p2.pos.y][p2.pos.x].person_id = p2.id;
}

int People::new_person_id() {//unsure if this function is redundant with how int++ works or if there's a better method
    people_id_iterator++;
    return people_id_iterator;
}

int People::new_message_id() {//unsure if this function is redundant with how int++ works or if there's a better method
    message_id_iterator++;
    return message_id_iterator;
}

int People::message_by_id(int id) {//uses binary search to find and return index to message in list
    int low = 0;
    int high = message_list.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (message_list[mid].message_id == id) {
            return mid;
        }
        (message_list[mid].message_id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
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
        if (pl[p].age < 5) {
            pl[p].current_image = "human_infant_dead";
        }

        //if have spouse, free spouse to remarry, need a more realistic way to handle this rather than instant long distance unlinking
        if (pl[p].spouse_id != -1) {
            pl[p_by_id(pl[p].spouse_id)].spouse_id = -1;//unlink from spouse
        }

        //set these to -1 to prevent others referencing them
        pl[p].hunger_level = -1;
        pl[p].hungry_time = -1;
        pl[p].reproduction_cooldown = -1;

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
            << "\nitem_inventory size: " << pl[p].item_inventory.size()
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
        pl[p].age++;
    }

    if (pl[p].age < 5) {//is infant. Currently that means it doesn't do anything except get hungry and needs to be fed
        pl[p].current_image = "human_infant";
        pl[p].hunger_level++;

        if (pl[p].being_carried) {//if being carried, then position is the position of the carrier offset by 1
            pl[p].pos = pl[p_by_id(pl[p].carried_by_id)].pos;
            pl[p].pos.x += 1;
            //cout << "am carried";
        }

        eating();
        return;
    }

    

    pl[p].hunger_level++; //hunger increases by 1 per hour, meaning it must decrease by 20 per day to stay at 0 daily average
    pl[p].tired_level++; //same for tired level
    pl[p].campsite_age++;
    pl[p].reproduction_cooldown++; //for when to find mate and create new person

    utility_function();
    pl[p].found_messages.clear();

    //check to ensure that spouses share campsite
    if (!pl[p].sex && !pl[p].adopt_spouse_campsite && pl[p].spouse_id != -1) {
        pl[p].campsite_pos = pl[p_by_id(pl[p].spouse_id)].campsite_pos;
    }
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
    Position old_pos = pl[p].pos;
    Environment::Map[pl[p].pos.y][pl[p].pos.x].person_id = -1;//remove person from Map
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
    bool reached = false;
    if (Environment::Map[pl[p].pos.y][pl[p].pos.x].person_id == -1) {//if no other person on this tile
        Environment::Map[pl[p].pos.y][pl[p].pos.x].person_id = pl[p].id;//add person back to Person_Map at new location
        reached = pl[p].pos == dest;
    }
    else {
        pl[p].pos = old_pos;//NEED TO FIX: need to handle case where someone else is blocking path or occuppying destination
        //move to a random adjacent tile. Temporary fix
        Position test_pos;
        for (int y = -1;y <= 1;y++) {
            for (int x = -1;x <= 1;x++) {
                test_pos.x = old_pos.x + x;
                test_pos.y = old_pos.y + y;
                if (valid_position(test_pos) && test_pos != pl[p].pos && Environment::Map[test_pos.y][test_pos.x].person_id == -1) {
                    pl[p].pos = test_pos;
                    break;//need to handle if no adjacent tile is empty
                }
            }
        }
    }
    
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

bool People::reproduce() {//later, add marriage ceremony/customs, options for polygamy, infidelity, premarital sex, widow status, age and family restrictions on potential mates, family size limits, divorce, etc
    if (!(pl[p].reproduction_cooldown > 100)) {//function trigger
        return false;
    }
    pl[p].current_state = "reproduce";
    vector<Position> &pos_list = pl[p].all_found[pl[p].target_index["mate"]];//note: using reference (&) reduces copying
    int p2 = -1;
    bool mate_willing = false;
    if (!pos_list.empty()) {
            p2 = p_by_id(Environment::Map[pos_list[0].y][pos_list[0].x].person_id);
            if (p2 == -1) {
                return true;//person not found, end function
            }
            if (pl[p2].reproduction_cooldown > 100 && pl[p2].sex != pl[p].sex) {
                mate_willing = true;
            }
        
        if (mate_willing && (distance(pl[p].pos, pl[p2].pos)==1 || move_to(pl[p2].pos))) {//go to tile adjacent to p2
                //create a new human, add pregnancy later, only female creates child
                if (!pl[p].sex) {
                    int sex = rand() % 2;
                    Position child_pos = pl[p].pos;
                    Position test_pos;
                    for (int y = -1;y <= 1;y++) {
                        for (int x = -1;x <= 1;x++) {
                            test_pos.x = child_pos.x + x;
                            test_pos.y = child_pos.y + y;
                            if (valid_position(test_pos) && test_pos != pl[p].pos && Environment::Map[test_pos.y][test_pos.x].person_id == -1) {
                                child_pos = test_pos;
                                break;//need to handle if no adjacent tile is empty
                            }
                        }
                    }
                    Person child = { new_person_id(), child_pos, sex };
                    Environment::Map[child.pos.y][child.pos.x].person_id = child.id;
                    pl.push_back(child);
                    pl[p].children_id.push_back(child.id);
                    pl[p2].children_id.push_back(child.id);
                    pl[p].reproduction_cooldown = 0;//reset
                    pl[p2].reproduction_cooldown = 0;//unsure if this is the best way to handle interaction between 2 people, speaking or some other function might be better to avoid 2 people not being in sync
                    if (pl[p].spouse_id == -1 && pl[p2].spouse_id == -1) {//if don't have spouse, set as spouse
                        pl[p].spouse_id = pl[p2].id;
                        pl[p2].spouse_id = pl[p].id;
                    }
                    //remove campsite and adopt male's campsite as own.
                    pl[p].adopt_spouse_campsite = true;
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

//if quantity parameter is < 0 (usually -1), then returns all results.
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
            if (tile_has("food",pos)) {
                found = true;
            }
        }
        else if (target == "no campsite") {//find location without a campsite
            if (!tile_has("tent", pos)) {
                found = true;
            }
        }
        else if (target == "rock") {
            if (tile_has("rock", pos)) {
                found = true;
            }
        }
        else if (target == "people" || target == "mate") {
            int pers_id = Environment::Map[pos.y][pos.x].person_id;
            if (pers_id != -1 && pers_id != pl[p].id) {
                found = true;
                if (target == "mate") {
                    int pid = p_by_id(pers_id);
                    if (pl[pid].sex != pl[p].sex && pl[pid].age > 10) {
                        bool is_my_child = false;
                        for (int i = 0; i < pl[p].children_id.size(); i++) {
                            if (pl[p].children_id[i] == pl[pid].id) {
                                is_my_child = true;
                                break;
                            }
                        }
                        if (!is_my_child) {
                            if (pl[pid].spouse_id == -1 && pl[p].spouse_id == -1) {//if both unmarried
                                found = true;
                            }
                            else if (pl[pid].spouse_id == pl[p].id && pl[p].spouse_id == pl[pid].id) {//if married to each other
                                found = true;
                            }
                            else {
                                found = false;//need to clean this code up to not have so many nested if-else
                            }
                        }
                        else {
                            found = false;
                        }
                    }
                    else {
                        found = false;
                    }
                }
            }
        }
        else if (target == "messages") {//might also serve as a generic for reacting to sounds
            for (int m_id : Message_Map[pos.y][pos.x]) {//check all messages in this tile
                if (!pl[p].found_messages.empty()) {
                    bool repeated_message = false;
                    for (int m1_id : pl[p].found_messages) {
                        if (m_id==m1_id) {//avoids copying messages that differ only in their location
                            repeated_message = true;
                            break;
                        }
                    }
                    if (!repeated_message && message_list[message_by_id(m_id)].sender_id != pl[p].id) {
                        pl[p].found_messages.push_back(m_id);
                    }
                }
                else if (message_list[message_by_id(m_id)].sender_id != pl[p].id) {
                    pl[p].found_messages.push_back(m_id);
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
    Message m = { new_message_id(), pl[p].id, -1, message_text };//creates message
    m.origin = pl[p].pos;
    for (int y = pl[p].pos.y - pl[p].audioline_radius; y < pl[p].pos.y + pl[p].audioline_radius; y++) {//creates copies of message for each map position it reaches then adds to global message list
        for (int x = pl[p].pos.x - pl[p].audioline_radius; x < pl[p].pos.x + pl[p].audioline_radius; x++) {
            if (valid_position({ x,y })) {
                Message_Map[y][x].push_back(m.message_id);
                message_list.push_back(m);
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
        feed_own_infants();//always returns false because it simply updates flags for other functions

        if (give_food()) {}
        else if (moving_to_bed()) {}
        else if (searching_for_food()) { if (gathering_food()) {} }
        else if (search_for_new_campsite()) {if (set_up_camp()) {}}
        else if (processing_food()) {}
        //else if (reproduce()) {} avoid execution of this function to focus on other features without worrying about population size
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
        for (int m_id : pl[p].found_messages) {
            if (message_list[message_by_id(m_id)].messsage == "food request") {
                someone_requested_food = true;
                food_request = message_list[message_by_id(m_id)];
            }
        }
    }
    bool start_give_food = !hungry && !inventory_has("food").empty() && (someone_requested_food || pl[p].child_is_hungry);
    if (!start_give_food) {//function trigger
        return false;
    }
    pl[p].current_state = "give_food";
    pl[p].current_image = "pics/human_giving_food.png";
    int p2_index = -1;
    if (pl[p].child_is_hungry) {
        p2_index = pl[p].hungry_child_index;
    }
    else if (someone_requested_food) {
        speak("giving food");
        p2_index = p_by_id(food_request.sender_id);//store this person in a function object so as not to repeat search every call, fix this
    }
    Person& p2 = pl[p2_index];
    if (distance(p2.pos,pl[p].pos)==1 || move_to(p2.pos)) {//move to adjacent tile
        int index = inventory_has("food")[0];
        p2.item_inventory.push_back(pl[p].item_inventory[index]); //give food id from inventory
        pl[p].item_inventory.erase(pl[p].item_inventory.begin() + index);//remove from own inventory
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
    if (!pl[p].sex && pl[p].spouse_id!=-1 && pl[p].campsite_pos == pl[p_by_id(pl[p].spouse_id)].campsite_pos) {
        return false;//prevent searching for a new campsite if married, only for females
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

    bool start = (cond1 && cond2 && cond3) || pl[p].adopt_spouse_campsite;

    //currently only creates a campsite after having been hungry 3 days. Still need to figure out when and when not to create a campsite, such as for trips away from home or extreme high mobility nomad
    //function trigger
    if (!start){
        return false;
    }
    
    pl[p].current_state = "search for new campsite";
    if (pl[p].campsite_pos.x != -1) { //if have campsite, remove. //Later add an option to just abandon a campsite without removing the house. Should only decontruct if going to carry it to new location such as a tent/sleeping bag/lean to/etc.
        if (move_to(pl[p].campsite_pos)) {//walk to campsite to remove
            int item_id = Environment::Map[pl[p].campsite_pos.y][pl[p].campsite_pos.x].item_id;
            delete_item(item_id, pl[p].campsite_pos);
            pl[p].campsite_pos = { -1,-1 };
            pl[p].campsite_age = -1;
            if (pl[p].adopt_spouse_campsite) {
                pl[p].campsite_pos = pl[p_by_id(pl[p].spouse_id)].campsite_pos;
                pl[p].adopt_spouse_campsite = false;//reset for if spouse dies later
                return true;//in progress
            }
        }
        else {
            return true;//in progress
        }
    }
    //if have infants, carry them
    if(!pl[p].children_id.empty()) {//if the game renderer stops running through the whole people list, this breaks because infant is no longer on map. Therefore need to create a better function for carrying and dropping both infants and general items/people. Currently also carries all infants in the same spot, so they overlap. Need to fix this.
        for (int i = 0; i < pl[p].children_id.size(); i++) {
            int kid_index = p_by_id(pl[p].children_id[i]);
            if (pl[kid_index].age < 5 && !pl[kid_index].being_carried) {
                if (distance(pl[kid_index].pos, pl[p].pos) == 1 || move_to(pl[kid_index].pos)) {
                    pl[kid_index].being_carried = true;
                    pl[kid_index].carried_by_id = pl[p].id;
                    Environment::Map[pl[kid_index].pos.y][pl[kid_index].pos.x].person_id = -1;
                    pl[kid_index].pos = { -1,-1 };
                    cout << "carrying kid";
                }
            }
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
        create_item(it2.tent, pos_list[0]);//create and place tent
        pl[p].campsite_pos = pos_list[0]; //store campsite location
        pl[p].start_set_up_camp = false;

        //if have infants, drop them
        if (!pl[p].children_id.empty()) {
            for (int i = 0; i < pl[p].children_id.size(); i++) {
                int kid_index = p_by_id(pl[p].children_id[i]);
                if (pl[kid_index].being_carried && pl[kid_index].carried_by_id==pl[p].id) {
                    Position child_pos = pl[p].pos;
                    Position test_pos;
                    for (int y = -1;y < 2;y++) {
                        for (int x = -1;x < 2;x++) {
                            test_pos.x = child_pos.x + x;
                            test_pos.y = child_pos.y + y;
                            if (valid_position(test_pos) && test_pos != pl[p].pos && Environment::Map[test_pos.y][test_pos.x].person_id == -1) {
                                child_pos = test_pos;
                                break;//need to handle if no adjacent tile is empty
                            }
                        }
                    }
                    pl[kid_index].pos = child_pos;
                    Environment::Map[child_pos.y][child_pos.x].person_id = pl[kid_index].id;
                    pl[kid_index].being_carried = false;
                    pl[kid_index].carried_by_id = -1;
                }
            }
        }

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
        else if(!pl[p].sex && pl[p].spouse_id!=-1) {//keeps near spouse if searching for new campsite, currently just has female follow male, otherwise both don't go far because they pull in different directions
            int spouse_distance_limit = 10;
            int spouse_index = p_by_id(pl[p].spouse_id);
            max_x = pl[spouse_index].pos.x + spouse_distance_limit;
            min_x = pl[spouse_index].pos.x - spouse_distance_limit;
            max_y = pl[spouse_index].pos.y + spouse_distance_limit;
            min_y = pl[spouse_index].pos.y - spouse_distance_limit;
        }
        dest.x = (rand() % (max_x - min_x)) + min_x;
        dest.y = (rand() % (max_y - min_y)) + min_y;
        dest = make_position_valid(dest, max_x, min_x, max_y, min_y);
        if (!valid_position(dest)) {
            valid_dest = false;
            tries++;//might no longer be necessary, the destination should be valid on the first try
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
    bool cond1 = tired && (pl[p].pos == pl[p].campsite_pos || pl[p].campsite_pos.x == -1);//if tired AND either at campsite or have no campsite
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
    bool cond1 = false;
    vector<int> food_indexes1 = inventory_has("food");
    vector<int> food_indexes2 = inventory_has("needs processing");
    if (!food_indexes1.empty()) {//fix this. This should be part of the inventory_has() by passing a vector<string> instead of a single string
        if (food_indexes2.empty()) {
            cond1 = true;
            pl[p].eating_food_index = food_indexes1[0];
        }
        else {
            bool found = true;
            for (int i = 0; i < food_indexes1.size(); i++) {
                found = true;
                for (int j = 0; j < food_indexes2.size(); j++) {
                    if (food_indexes1[i] == food_indexes2[j]) {
                        found = false;
                        break;
                    }
                }
                if (found) {
                    cond1 = true;
                    pl[p].eating_food_index = food_indexes1[0];
                    break;
                }
            }
        }
    }

    if (!(hungry && cond1)) {//function trigger
        return false;
    }
    pl[p].current_state = "eating";
    if (pl[p].eating_progress.progress == 0) {
        pl[p].current_image = "pics/human_eating.png";
    }
    if (pl[p].age < 5) {
        pl[p].current_image = "human_infant";
    }
    if (pl[p].eating_progress.progress_func()) {//makes eating take more than 1 frame
        int index = pl[p].eating_food_index;
        int food_id = pl[p].item_inventory[index];
        pl[p].item_inventory.erase(pl[p].item_inventory.begin() + index);//delete item from inventory
        delete_item(food_id, {-1,-1});//delete food from game
        pl[p].hunger_level -= 50; //reduce hungry level by 10, therefore need 2 meals a day to stay at 0 hunger_level average
        pl[p].function_done = true;
        pl[p].clean_image = true; //when this function ends, return to default image on next update
        return true;//done eating
    }
     return true;//in progress
}

//fix this: make this and gathering_food() generics for any item
bool People::searching_for_food(){
    bool hungry = pl[p].hunger_level > 50;
    if (!((hungry || pl[p].child_is_hungry) && inventory_has("food").empty())) {//function trigger
        return false;
    }
    if (pl[p].start_gathering_food) {
        return true;//go to next function in function chain
    }
    pl[p].current_state = "searching for food";

    bool someone_is_giving_food = false;
    if (!pl[p].found_messages.empty()) {
        for (int m_id : pl[p].found_messages) {
            if (message_list[message_by_id(m_id)].messsage == "giving food") {
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
    int dist = distance(food_pos, pl[p].campsite_pos);
    bool cond1 = !found_food || (found_food && dist > campsite_distance_search);//if no food found OR food was found but the distance is too far from the campsite
    bool cond2 = pl[p].campsite_pos.x == -1 || pl[p].hungry_time >= 3;//AND: have no campsite OR have been hungry too long
    bool cond3 = pl[p].campsite_age > 10;//AND: campsite is old enough to move again. Unsure if this might have an issue if the null campsite has an age
    bool start_search_for_new_campsite = cond1 && cond2 && cond3;//fix this, unsure if this is still necessary, this should only be handled by the relevant function
    if (start_search_for_new_campsite) {//remove campsite to be able to be unbound and get food while searching for new campsite, figure out a cleaner way to organize order between finding food to eat now and finding a new campsite
        if (pl[p].campsite_pos.x != -1) {
            if (move_to(pl[p].campsite_pos)) {//walk to campsite to remove
                int item_id = Environment::Map[pl[p].campsite_pos.y][pl[p].campsite_pos.x].item_id;
                delete_item(item_id, pl[p].campsite_pos);//delete campsite
                pl[p].campsite_pos = { -1,-1 };
                pl[p].campsite_age = -1;
                return true;//in progress
            }
            return true; //in progress
        }
    }

    if (pl[p].campsite_pos.x == -1) {
        //if have infants, carry them
        if (!pl[p].children_id.empty()) {//if the game renderer stops running through the whole people list, this breaks because infant is no longer on map. Therefore need to create a better function for carrying and dropping both infants and general items/people. Currently also carries all infants in the same spot, so they overlap. Need to fix this.
            for (int i = 0; i < pl[p].children_id.size(); i++) {
                int kid_index = p_by_id(pl[p].children_id[i]);
                if (pl[kid_index].age < 5 && !pl[kid_index].being_carried) {
                    if (distance(pl[kid_index].pos, pl[p].pos) == 1 || move_to(pl[kid_index].pos)) {
                        pl[kid_index].being_carried = true;
                        pl[kid_index].carried_by_id = pl[p].id;
                        Environment::Map[pl[kid_index].pos.y][pl[kid_index].pos.x].person_id = -1;
                        pl[kid_index].pos = { -1,-1 };
                    }
                }
            }
        }
    }
    
    //reset function if it was interrupted by another function
    if (!pl[p].function_record.empty() && pl[p].function_record.back() != "searching for food") {
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
        //pl[p].fo2.pos = { -1,-1 }; //reset function object    old code, still unsure if keeping replacement
        pl[p].fo2.pos = food_pos;//replacement: store position of food that was found to be picked up by gathering_food() function, this variable is reset by that function
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
    if (!tile_has("food", pl[p].fo2.pos)) {
        pl[p].function_done = true;
        pl[p].start_gathering_food = false;
        pl[p].fo2.pos = { -1,-1 };//belongs to searching_for_food()
        pl[p].clean_image = true;
        return true;//done. food that had been found by searching_for_food() is no longer there, restart search
    }
    pl[p].current_state = "gathering food";
    bool reached = move_to(pl[p].fo2.pos); //move towards food
    if (reached) {//if at food location, pick up food
        pl[p].current_image = "pics/human_gathering.png";
        int item_id = Environment::Map[pl[p].fo2.pos.y][pl[p].fo2.pos.x].item_id;
        pick_up_item(item_id, pl[p].fo2.pos);//place food in inventory and remove it from map
        //if there is more food very close by and inventory has less than 4 items, then gather that as well, without moving to those positions. Need to adjust so it's not instant pickup
        int inventory_max = 4;
        if (inventory_has("food").size() < inventory_max) {
            vector<Position> food_pos_list = pl[p].all_found[pl[p].target_index["food"]];
            if (food_pos_list.size() > 1) {//if there are additional food found
                for (int i = 0; inventory_has("food").size() < inventory_max && i< food_pos_list.size(); i++) {//only gather enough to to fill inventory to 4 items
                    if (distance(food_pos_list[i], pl[p].pos) == 1) {//limit gather distance to 1 tile away from player
                        if (Environment::Map[food_pos_list[i].y][food_pos_list[i].x].item_id != -1) {
                            pl[p].item_inventory.push_back(Environment::Map[food_pos_list[i].y][food_pos_list[i].x].item_id); //add food's item_id to inventory
                            Environment::Map[food_pos_list[i].y][food_pos_list[i].x].item_id = -1; //remove food from map
                        }
                    }
                }
            }
        }
        pl[p].fo2.pos = { -1,-1 };//belongs to searching_for_food()
        pl[p].function_done = true;
        pl[p].start_gathering_food = false; //resets function trigger to false when function ends
        pl[p].clean_image = true;
        return true;//done
    }
    return true;//in progress
}

bool People::tile_has(string target, Position pos) {//should this be moved to the Environment class? Only for checking Item
    if (Environment::Map[pos.y][pos.x].item_id == -1) {
        return false;
    }
    int item_index = ItemSys::item_by_id(Environment::Map[pos.y][pos.x].item_id);
    if (ItemSys::item_list[item_index].item_name == target) {
        return true;
    }
    for (int i = 0; i < ItemSys::item_list[item_index].tags.size(); i++) {
        if (ItemSys::item_list[item_index].tags[i] == target) {
            return true;
        }
    }
    return false;
}

vector<int> People::inventory_has(string target) {//return list of indexes of matching items in inventory if found, empty list if not found
    vector<int> indexes;
    if (pl[p].item_inventory.empty()) {
        return indexes;
    }
    for (int i = 0; i < pl[p].item_inventory.size(); i++) {
        int item_index = ItemSys::item_by_id(pl[p].item_inventory[i]);
        if (ItemSys::item_list[item_index].item_name == target) {
            indexes.push_back(i);
        }
        for (int j = 0; j < ItemSys::item_list[item_index].tags.size(); j++) {
            if (ItemSys::item_list[item_index].tags[j] == target) {
                indexes.push_back(i);
                break;
            }
        }
    }
    return indexes;
}

void People::create_item(ItemSys::Item item_type, Position pos) {
    ItemSys::Item new_item = item_type;
    new_item.item_id = ItemSys::new_item_id();
    ItemSys::item_list.push_back(new_item);
    if (pos.x == -1) {
        pl[p].item_inventory.push_back(new_item.item_id);
    }
    else {
        Environment::Map[pos.y][pos.x].item_id = new_item.item_id;//create item, then place on map and global item list
    }
}

void People::pick_up_item(int item_id, Position pos) {
    if (Environment::Map[pos.y][pos.x].item_id == -1) {
        cout << "pick up item error";
    }
    pl[p].item_inventory.push_back(Environment::Map[pos.y][pos.x].item_id); //add item's item_id to inventory
    Environment::Map[pos.y][pos.x].item_id = -1; //remove item from map
}

void People::delete_item(int item_id, Position pos) {
    if (item_id == -1) {//don't know what's causing this issue but need this check to work
        return;
    }
    int item_index = ItemSys::item_by_id(item_id);
    ItemSys::item_list.erase(ItemSys::item_list.begin() + item_index);//remove item from global item_list
    if (pos.x != -1) {//if pos.x == -1, then the item was not on the map and was probably in a Person's inventory from which it was deleted separately
        Environment::Map[pos.y][pos.x].item_id = -1; //removes item from map
    }
}

bool People::feed_own_infants() {//fix this: need a better version of this function because it currently just instantly notifies parents about hungry kids regardless of distance. But otherwise would need to include a scheduled check on infant to feed them if hungry, otherwise may never be close enough often enough to notice when hungry
    //vector<int> feed_order;//feed hungriest first. Unsure if this should be implemented
    //might make more sense to use the infant to speak to ask for food
    bool start = false;
    int hungry_child_index = -1;
    for (int i = 0; i < pl[p].children_id.size(); i++) {
        if (pl[p_by_id(pl[p].children_id[i])].hunger_level > 50 && pl[p_by_id(pl[p].children_id[i])].age<5) {
            start = true;
            hungry_child_index = p_by_id(pl[p].children_id[i]);
            break;
        }
    }
    if (!start) {//function trigger
        pl[p].child_is_hungry = false;
        pl[p].hungry_child_index = -1;
        return false;
    }
    pl[p].child_is_hungry = true;
    pl[p].hungry_child_index = hungry_child_index;
    return false;//done, flag triggers searching_for_food, gathering_food, and give_food. returns false because it only sets child_is_hungry flag, not a full function in its own right
}


bool People::processing_food() {//could instead be used as a crafting generic, fix this
    bool start = !inventory_has("needs processing").empty(); 

    if (!start) {//function trigger
        return false;
    }

    
    if (inventory_has("mortar_pestle").empty()) {//should this be in utility function or should function chains be internal to relevant functions?
        craft_mortar_pestle();//this should be turned into a generic, meaning the crafted item must have its ingredients included in its struct, fix this
        //need to have option to search instead of craft in case already made one and dropped it near camp
        return true;//in progress
    }
    

    pl[p].current_state = "processing food";
    if (pl[p].processing_food_prog.progress == 0) {
        pl[p].current_image = "human_crafting";
    }
    if (pl[p].processing_food_prog.progress_func()) {//currently if function is interrupted, the progress is saved and coninued/finished when the function is called again. This may be an issue if ingredient is no longer in inventory. fix this
        int index = inventory_has("needs processing")[0];
        int food_ingredient_id = pl[p].item_inventory[index];
        pl[p].item_inventory.erase(pl[p].item_inventory.begin() + index);//delete ingredient from inventory
        delete_item(food_ingredient_id, { -1,-1 });//delete ingredient from game
        Position new_pos = empty_adjacent_tile();
        create_item(it2.bread, new_pos);//creates bread item. Currently places on map to see resulting item. Can change later to inserting directly to inventory by using {-1,-1} instead of new_pos
        pl[p].clean_image = true; //when this function ends, return to default image on next update
        return true;//done
    }
    return true;//in progress
}

People::Position People::empty_adjacent_tile() {//should add option between if tile has no item or no person?
    Position test_pos;
    for (int y = -1;y < 2;y++) {
        for (int x = -1;x < 2;x++) {
            test_pos.x = pl[p].pos.x + x;
            test_pos.y = pl[p].pos.y + y;
            if (valid_position(test_pos) && test_pos != pl[p].pos && Environment::Map[test_pos.y][test_pos.x].item_id == -1) {
                return test_pos;//need to handle if no adjacent tile is empty
            }
        }
    }
}

bool People::craft_mortar_pestle() {//sometimes works fine, sometimes causes person to be stuck either not moving or moving to the same spot unless move_to_bed and sleep take priority, don't know why, need to fix
    if (!inventory_has("rock").empty()) {
        pl[p].current_state = "crafting mortar and pestle";
        if (pl[p].crafting.progress == 0) {
            pl[p].current_image = "human_crafting";
        }
        if (pl[p].crafting.progress_func()) {//currently if function is interrupted, the progress is saved and coninued/finished when the function is called again. This may be an issue if ingredient is no longer in inventory. fix this
            int index = inventory_has("rock")[0];
            int ingredient_id = pl[p].item_inventory[index];
            pl[p].item_inventory.erase(pl[p].item_inventory.begin() + index);//delete ingredient from inventory
            delete_item(ingredient_id, { -1,-1 });//delete ingredient from game
            Position new_pos = empty_adjacent_tile();
            create_item(it2.mortar_pestle, {-1,-1});//creates bread item. Currently places on map to see resulting item. Can change later to inserting directly to inventory by using {-1,-1} instead of new_pos
            pl[p].clean_image = true; //when this function ends, return to default image on next update
            return true;//done
        }
        return true;//in progress
    }
    else {
        pl[p].current_state = "searching for rock";
        //walk to search
        if (pl[p].rock_search.x == -1 || move_to(pl[p].rock_search)) {//initialize function object or reinitialize if reached destination
            pl[p].rock_search = walk_search_random_dest();
            return true;//in progress
        }//the move_to function triggers in the conditional
        //look around for rock
        if (!pl[p].all_found[pl[p].target_index["rock"]].empty()) {
            Position rpos = pl[p].all_found[pl[p].target_index["rock"]][0];
            if (move_to(rpos)) {//if found, move to item, once reached, place in inventory
                int item_id = Environment::Map[rpos.y][rpos.x].item_id;
                pick_up_item(item_id, rpos);
                return true;//subtask done
            }
            return true;//in progress
        }
    }
    return false;//?
}