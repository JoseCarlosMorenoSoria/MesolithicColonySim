#include "People.hpp"
using namespace std;

vector<People::Person> People::pl; //pl, using the name pl because of the frequency of use, used to store all Person instances
vector<People::Message> People::message_list;
vector<int> People::Message_Map[Environment::map_y_max][Environment::map_x_max];
int p = -1;//index for accessing current person. Using index to access instead of a pointer because list may change such as when a new person is born or dies which invalidates pointers to pl (people_list)
ItemSys it2;//used to access member functions and variables of ItemSys
int People::ox = -1;
int People::oy = -1;
int People::pday_count;
int People::phour_count;

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
    pday_count = day_count;
    phour_count = hour_count;

    for(int i = 0; i < pl.size(); i++) {
        p = i;
        ox = pl[p].pos.x;
        oy = pl[p].pos.y;
        update(day_count, hour_count, hours_in_day);
    }
    //clear global message list every other update. One issue with npcs cooperating or communicating is sequence order, 
    //as in if npc1 updates before npc2, anything npc2 says or does won't be witnessed unless it is still there on the next update, NEED TO FIX
    if (message_clear_flag) {
        for (int y = 0; y < Environment::map_y_max; y++) {//unsure if this is the most efficient way to clear Message_Map
            for (int x = 0; x < Environment::map_x_max; x++) {
                Message_Map[y][x].clear();
            }
        }
    }
    message_clear_flag = !message_clear_flag;
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
        add_func_record("dead");
        pl[p].current_image = "pics/human_dead.png";
        if (pl[p].age < 5) {
            pl[p].current_image = "human_infant_dead";
        }
        //if have spouse, free spouse to remarry, need a more realistic way to handle this rather than instant long distance unlinking
        if (pl[p].spouse_id != -1) {
            pl[p_by_id(pl[p].spouse_id)].spouse_id = -1;//unlink from spouse
        }
        //set these to -1 to prevent others referencing them
        //pl[p].hunger_level = -1;
        //pl[p].hungry_time = -1;
        //pl[p].reproduction_cooldown = -1;
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
        if (pl[p].hunger_level > 50) {
            speak("requesting food",-1);
        }
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

    if (pl[p].clean_image) {
        pl[p].current_image = "pics/human.png";
        pl[p].clean_image = false;
    }

    find_all();//gets all items, people, etc from within sight/earshot to then react to or inform next action
    utility_function();

    if (pl[p].search_results.find("rabbit") != pl[p].search_results.end()) {
        pl[p].saw_rabbit_recently = true;
        pl[p].day_I_saw_rabbit = day_count;
    }
    if (day_count - pl[p].day_I_saw_rabbit > 3) {
        pl[p].saw_rabbit_recently = false;
    }

    pl[p].found_messages.clear();
    pl[p].search_results.clear();

    //check to ensure that spouses share campsite
    if (!pl[p].sex && !pl[p].adopt_spouse_campsite && pl[p].spouse_id != -1) {
        pl[p].campsite_pos = pl[p_by_id(pl[p].spouse_id)].campsite_pos;
    }


    

    if (!pl[p].move_already && pl[p].general_search_called) {
        general_search_walk("");//ensures this function only executes once per update and also only after all other move_to's have been considered. This prioritizes intentional movement (moving to a target) rather than random movement
    }
    pl[p].move_already = false;
    pl[p].general_search_called = false;
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
bool People::move_to(Position dest, string caller) {//need to add speed of moving from one tile to another and how many tiles at a time. Also need to add a check to prevent it being called more than once per person per update.
    //if (pl[p].search_active) {//prevents function getting called more than once per update
    //    return false;
    //}
    if (!valid_position(dest)) { //for debugging, kill npc if it tries to go off map or is asked to move to the spot it is already at
        pl[p].is_alive = false;
        pl[p].current_image = "pics/debug.png";
        return true;
    }
    if (pl[p].general_search_called && caller != "general searching") {
        pl[p].general_search_called = false;
    }
    //cout << caller << ":";
    //if (pl[p].move_already || pl[p].current_image != "pics/human.png" || pl[p].age < 5) {//the image check shouldn't be necessary but I don't know why it still moves while having crafting image
    //    if (pl[p].pos == dest) {
    //        return true;
    //    }
     //   return false;
    //}

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
        //potential fix would be to have a direction attribute for person and a function to handle turning right or left to then simply turn perpendicular to the obstacle until no obstacle is in the way.
        Position test_pos;
        for (int y = -1;y <= 1;y++) {
            for (int x = -1;x <= 1;x++) {
                test_pos.x = old_pos.x + x;
                test_pos.y = old_pos.y + y;
                if (valid_position(test_pos) && test_pos != pl[p].pos && Environment::Map[test_pos.y][test_pos.x].person_id == -1) {
                    pl[p].pos = test_pos;
                    break;//need to handle if no adjacent tile is empty, fix this
                }
            }
        }
    }
    pl[p].move_already = true;
    return reached;
}

void People::add_func_record(string s) {
    if (pl[p].function_record.empty()) {
        pl[p].function_record.push_back(s);
        pl[p].function_record_nums.push_back(1);
    }
    else {
        if (pl[p].function_record.back() == s) {
            pl[p].function_record_nums.back()++;
        }
        else {
            pl[p].function_record.push_back(s);
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
    add_func_record("reproduce");
    vector<Position> &pos_list1 = pl[p].search_results["people"];//note: using reference (&) reduces copying
    int p2 = -1;
    for (int i = 0; i < pos_list1.size(); i++) {//filter out valid mates from people found list
        int pers_id = Environment::Map[pos_list1[i].y][pos_list1[i].x].person_id;
        int pid = p_by_id(pers_id);
        if (pl[pid].sex != pl[p].sex && pl[pid].age > 10) {
            bool is_my_child = false;
            for (int i = 0; i < pl[p].children_id.size(); i++) {
                if (pl[p].children_id[i] == pl[pid].id) {
                    is_my_child = true;
                    break;
                }
            }
            if (!is_my_child && ((pl[pid].spouse_id == -1 && pl[p].spouse_id == -1) || (pl[pid].spouse_id == pl[p].id && pl[p].spouse_id == pl[pid].id))) {//if not my child AND both unmarried or if married to each other
                p2 = pers_id;//mate found
                break;
            }
        }
    }
    bool mate_willing = false;
    if (p2!=-1) {
        if (pl[p2].reproduction_cooldown > 100 && pl[p2].sex != pl[p].sex) {
            mate_willing = true;
        }
        if (mate_willing && (Position::distance(pl[p].pos, pl[p2].pos)==1 || move_to(pl[p2].pos,"going to mate"))) {//go to tile adjacent to p2
            //create a new human, add pregnancy later, only female creates child
            if (!pl[p].sex) {
                int sex = rand() % 2;
                Position child_pos;
                if (pl[p].search_results.find("no people") != pl[p].search_results.end()) {
                    if (Position::distance(pl[p].search_results["no people"][0], pl[p].pos) == 1) {//if empty adjacent tile
                        child_pos = pl[p].search_results["no people"][0];
                    }
                }
                if (child_pos.x == -1) {//if no empty adjacent tile found
                    pl[p].general_search_called = true;
                    return true;//in progress
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
        pl[p].general_search_called = true;
    }
    return true;//in progress
}

Animal anim1;
void People::find_all() {//returns all things (items, people, messages, etc) found, sorted according into Position lists for each thing type
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
    map<string, vector<Position>> search_results;
    Position o = pl[p].pos;//origin
    vector<int> target_quantity_current;
    for (int radius = 0; radius <= pl[p].radiusmax; radius++) { //this function checks tilemap in outward rings by checking top/bottom and left/right ring boundaries
        if (radius == 0) {//avoids double checking origin
            if (radius <= pl[p].sightline_radius) {
                if (Environment::Map[o.y][o.x].item_id != -1) {
                    ItemSys::Item& item = ItemSys::item_list[ItemSys::item_by_id(Environment::Map[o.y][o.x].item_id)];
                    search_results.insert({ item.item_name,{o}});
                }
                else {//creates list of tiles without any item, for use when placing an item on the map
                    search_results.insert({ "no item",{o} });
                }
                //don't check if person is on origin tile, because that person is self
                if (Environment::Map[o.y][o.x].animal_id != -1) {
                    string species = Animal::al[anim1.a_by_id(Environment::Map[o.y][o.x].animal_id)].species;
                    search_results.insert({ species,{o} });
                }
                else {//creates list of tiles without any item, for use when placing an item on the map
                    search_results.insert({ "no animal",{o} });
                }
            }
            if (radius <= pl[p].audioline_radius) {
                //check for messages
                check_tile_messages(o);
            }
        }
        int xmin = o.x - radius;
        int xmax = o.x + radius;
        int ymin = o.y - radius + 1;//+1 and -1 to avoid double checking corners
        int ymax = o.y + radius - 1;
        for (int x = xmin, y = ymin; x <= xmax; x++, y++) {
            for (int sign = -1; sign<=1; sign+=2) {//sign == -1, then sign == 1
                Position pos1 = { x,o.y + (sign * radius) };
                Position pos2 = { o.x + (sign * radius), y };
                //check for people
                if (valid_position(pos1)) {
                    if (Environment::Map[pos1.y][pos1.x].person_id != -1) {
                        if (search_results.find("people") != search_results.end()) {//check if key exists
                            //key found
                            search_results["people"].push_back(pos1);
                        }
                        else {
                            //key not found
                            search_results.insert({ "people",{pos1} });
                        }
                    }
                    else {//creates list of tiles without any people, for use when placing an item on the map
                        if (search_results.find("no people") != search_results.end()) {//check if key exists
                            //key found
                            search_results["no people"].push_back({ pos1 });
                        }
                        else {
                            //key not found
                            search_results.insert({ "no people",{pos1} });
                        }
                    }
                }
                if (y <= ymax) {
                    if (valid_position(pos2)) {
                        if (Environment::Map[pos2.y][pos2.x].person_id != -1) {
                            if (search_results.find("people") != search_results.end()) {//check if key exists
                                //key found
                                search_results["people"].push_back(pos2);
                            }
                            else {
                                //key not found
                                search_results.insert({ "people",{pos2} });
                            }
                        }
                        else {//creates list of tiles without any people, for use when placing an item on the map
                            if (search_results.find("no people") != search_results.end()) {//check if key exists
                                //key found
                                search_results["no people"].push_back({ pos2 });
                            }
                            else {
                                //key not found
                                search_results.insert({ "no people",{pos2} });
                            }
                        }
                    }
                }
                //check for items
                if (valid_position(pos1)) {
                    if (Environment::Map[pos1.y][pos1.x].item_id != -1) {
                        string item_name = ItemSys::item_list[ItemSys::item_by_id(Environment::Map[pos1.y][pos1.x].item_id)].item_name;
                        if (search_results.find(item_name) != search_results.end()) {//check if key exists
                            //key found
                            search_results[item_name].push_back({ pos1 });
                        }
                        else {
                            //key not found
                            search_results.insert({ item_name,{pos1} });
                        }
                    }
                    else {//creates list of tiles without any item, for use when placing an item on the map
                        if (search_results.find("no item") != search_results.end()) {//check if key exists
                            //key found
                            search_results["no item"].push_back({ pos1 });
                        }
                        else {
                            //key not found
                            search_results.insert({ "no item",{pos1} });
                        }
                    }
                }
                if (y <= ymax) {
                    if (valid_position(pos2)) {
                        if (Environment::Map[pos2.y][pos2.x].item_id != -1) {
                            string item_name = ItemSys::item_list[ItemSys::item_by_id(Environment::Map[pos2.y][pos2.x].item_id)].item_name;
                            if (search_results.find(item_name) != search_results.end()) {//check if key exists
                                //key found
                                search_results[item_name].push_back({ pos2 });
                            }
                            else {
                                //key not found
                                search_results.insert({ item_name,{pos2} });
                            }
                        }
                        else {//creates list of tiles without any item, for use when placing an item on the map
                            if (search_results.find("no item") != search_results.end()) {//check if key exists
                                //key found
                                search_results["no item"].push_back({ pos2 });
                            }
                            else {
                                //key not found
                                search_results.insert({ "no item",{pos2} });
                            }
                        }
                    }
                }
                if (valid_position(pos1)) {
                    if (Environment::Map[pos1.y][pos1.x].animal_id != -1) {
                        string species = Animal::al[anim1.a_by_id(Environment::Map[pos1.y][pos1.x].animal_id)].species;
                        if (search_results.find(species) != search_results.end()) {//check if key exists
                            //key found
                            search_results[species].push_back({ pos1 });
                        }
                        else {
                            //key not found
                            search_results.insert({ species,{pos1} });
                        }
                    }
                    else {//creates list of tiles without any item, for use when placing an item on the map
                        if (search_results.find("no animal") != search_results.end()) {//check if key exists
                            //key found
                            search_results["no animal"].push_back({pos1});
                        }
                        else {
                            //key not found
                            search_results.insert({ "no animal",{pos1}});
                        }
                    }
                }
                if (y <= ymax) {
                    if (valid_position(pos2)) {
                        if (Environment::Map[pos2.y][pos2.x].animal_id != -1) {
                            string species = Animal::al[anim1.a_by_id(Environment::Map[pos2.y][pos2.x].animal_id)].species;
                            if (search_results.find(species) != search_results.end()) {//check if key exists
                                //key found
                                search_results[species].push_back({ pos2 });
                            }
                            else {
                                //key not found
                                search_results.insert({ species,{pos2} });
                            }
                        }
                        else {//creates list of tiles without any item, for use when placing an item on the map
                            if (search_results.find("no animal") != search_results.end()) {//check if key exists
                                //key found
                                search_results["no animal"].push_back({ pos2 });
                            }
                            else {
                                //key not found
                                search_results.insert({ "no animal",{pos2} });
                            }
                        }
                    }
                }

                //check for messages
                if (valid_position(pos1)) {
                    check_tile_messages(pos1);
                }
                if (y <= ymax) {
                    if (valid_position(pos2)) {
                        check_tile_messages(pos2);
                    }
                }
            }
        }
    }
    pl[p].search_results=search_results;
}

void People::check_tile_messages(Position pos) {
    //might also serve as a generic for reacting to sounds
    for (int m_id : Message_Map[pos.y][pos.x]) {//check all messages in this tile
        if (!pl[p].found_messages.empty()) {
            bool repeated_message = false;
            for (int m1_id : pl[p].found_messages) {
                if (m_id == m1_id) {//avoids copying messages that differ only in their location
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
}

//can this function also be folded into the find_all function somehow to further reduce for loops searching on the map?
//might also need to add some restriction so that each person doesn't blast out too many messages at once? Or maybe that's ok?
void People::speak(string message_text, int receiver_id) {//if receiver_id == -1, then the message is for everyone
    //current valid messages include: need to list valid messages here
    //the outward ring method might make more sense in this function to allow certain objects such as walls to block sound, might implement later but not currently
    Message m = { new_message_id(), pl[p].id, receiver_id, message_text, pl[p].pos };//creates message
    for (int y = pl[p].pos.y - pl[p].audioline_radius; y < pl[p].pos.y + pl[p].audioline_radius; y++) {//creates copies of message for each map position it reaches then adds to global message list
        for (int x = pl[p].pos.x - pl[p].audioline_radius; x < pl[p].pos.x + pl[p].audioline_radius; x++) {
            if (valid_position({ x,y })) {
                Message_Map[y][x].push_back(m.message_id);
                message_list.push_back(m);
            }
        }
    }
}

People::Position People::walk_search_random_dest() {
    Position dest = { -1,-1 };
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
        else if (!pl[p].sex && pl[p].spouse_id != -1) {//keeps near spouse if searching for new campsite, currently just has female follow male, otherwise both don't go far because they pull in different directions
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

bool People::idle() {
    add_func_record("idle");
    //flip between idle and default image
    if (pl[p].current_image == "pics/human.png") {
        pl[p].current_image = "pics/human_idle.png"; //need to make image, just have human with raised hands
    }
    else {
        pl[p].current_image = "pics/human.png";
    }
    return true;
}

int People::Position::distance(Position pos1, Position pos2) {
    int xd = abs(pos1.x - pos2.x);
    int yd = abs(pos1.y - pos2.y);
    int max = -1;
    (xd > yd) ? max = xd : max = yd;
    return max;
}

vector<int> People::inventory_has(string target) {//return list of indexes of matching items in inventory if found, empty list if not found
    vector<int> indexes;
    if (pl[p].item_inventory.empty()) {
        return indexes;
    }
    for (int i = 0; i < pl[p].item_inventory.size(); i++) {
        int item_index = ItemSys::item_by_id(pl[p].item_inventory[i]);
        if (item_index == -1) {//this shouldn't be happening, need to figure out why, temp fix. fix this
            return indexes;
        }
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

void People::create_item(string item_type, Position pos) {
    ItemSys::Item new_item = it2.presets[item_type];
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
    pl[p].current_image = "pics/human_gathering.png";
    pl[p].clean_image = true;
    pl[p].item_inventory.push_back(Environment::Map[pos.y][pos.x].item_id); //add item's item_id to inventory
    Environment::Map[pos.y][pos.x].item_id = -1; //remove item from map
}

void People::delete_item(int item_id, Position pos, int index) {
    if (item_id == -1) {//don't know what's causing this issue but need this check to work
        return;
    }
    int item_index = ItemSys::item_by_id(item_id);
    ItemSys::item_list.erase(ItemSys::item_list.begin() + item_index);//remove item from global item_list
    if (pos.x != -1) {//if pos.x == -1, then the item was not on the map and was probably in a Person's inventory from which it was deleted separately
        Environment::Map[pos.y][pos.x].item_id = -1; //removes item from map
    }
    if (index != -1) {
        pl[p].item_inventory.erase(pl[p].item_inventory.begin() + index);//delete item from inventory
    }
}

bool People::craft(string product) {//later add station requirements such as campfire/stove/oven/furnace
    //if inventory has product.ingredients then craft product (consumes non tool ingredients) and place in inventory
    int num_of_ingredients = it2.presets[product].ingredients.size();
    vector<string> missing_ingredients;//fix this, this should be a single string given that every update rechecks if it can craft the item and only 1 ingredient is sought per tick. Unless a more detailed method is implemented such as checking if any missing ingredient is craftable rather than doing them strictly in order.
    vector<int> temp;
    for (int i = 0; i < num_of_ingredients; i++) {
        temp = inventory_has(it2.presets[product].ingredients[i]);
        if (temp.empty()) {
            missing_ingredients.push_back(it2.presets[product].ingredients[i]);
        }
    }
    if (missing_ingredients.empty()) {//have all items, therefore craft product
        add_func_record("crafting " + product);
        pl[p].current_image = "human_crafting";
        pl[p].move_already = true;//prevents moving while crafting
        pl[p].crafting.insert({ product,{4} });//4 is the number of ticks/frames crafting image/animation lasts
        if (pl[p].crafting[product].progress_func()) {//animation/time delay    currently, progress is saved if interrupted which might not make sense in some contexts
            for (int i = 0; i < num_of_ingredients; i++) {//later implement tool degradation here as well
                if (it2.presets[it2.presets[product].ingredients[i]].consumable_ingredient) {
                    int consume_index = inventory_has(it2.presets[product].ingredients[i])[0];
                    delete_item(pl[p].item_inventory[consume_index], { -1,-1 }, consume_index);
                }
            }
            create_item(product, { -1,-1 });
            pl[p].crafting.erase(product);
            pl[p].clean_image = true; //when this function ends, return to default image on next update
            return true;//done
        }
        return false;//in progress
    }
    else {
        if (acquire(missing_ingredients[0])) {
            return false;//successfully acquired ingredient, try crafting again next update
        }
        else {
            return false;//acquisition of ingredient in progress, continue to next tick
        }
    }
}

void People::general_search_walk(string target) {
    //walk to search
    if (pl[p].general_search_dest.x == -1 || move_to(pl[p].general_search_dest, "general searching")) {//initialize function object or reinitialize if reached destination
        pl[p].general_search_dest = walk_search_random_dest();
    }//the move_to function triggers in the conditional
}

void People::answer_item_request() {
    if (pl[p].found_messages.empty()) {
        return;
    }
    //if request for an item found, and have item, and willing to give item (don't give food if hungry or only to kids if kids are hungry, etc)
    vector<Message> request_messages;
    vector<string> items_requested;
    for (int m_id : pl[p].found_messages) {
        if (message_list[message_by_id(m_id)].messsage.find("requesting ") != -1) {//checks if messages contains substring "requesting". -1 == not found
            request_messages.push_back(message_list[message_by_id(m_id)]);
            string item = message_list[message_by_id(m_id)].messsage;
            string sub_s = "requesting ";
            item.erase(item.find(sub_s), sub_s.length());//erases substring from message, leaving only the item name
            items_requested.push_back(item);
        }
    }
    if (items_requested.empty()) {//none of the messages were item requests
        return;
    }
    for (int i = items_requested.size() - 1; i > -1; i--) {
        if (inventory_has(items_requested[i]).empty()) {//if don't have item requested, remove from list of items requested
            items_requested.erase(items_requested.begin() + i);
            request_messages.erase(request_messages.begin() + i);
        }
    }
    if (items_requested.empty()) {//don't have any of the items requested
        return;
    }

    //conditional checks such as don't give food if self is hungry or give to kids before others if kids are hungry. Not implemented, fix this
    /*
    * bool hungry = pl[p].hunger_level > 50;//condition on willingness to answer request
      bool start_give_food = !hungry && !pl[p].child_is_hungry;
    */

    //send answer
    string target = items_requested[0];//currently simply selects the first item request in list to answer. Fix this, no condition on when or when not to answer has been implemented
    int receiver_id = request_messages[0].sender_id;//id of person who requested the item
    add_func_record("answering request for " + target);
    pl[p].current_image = "pics/human_giving_food.png";
    speak("answering request for " + target, receiver_id);
    //move to requester's position, adjacent
    Person& p2 = pl[p_by_id(receiver_id)];
    if (Position::distance(pl[p].pos, p2.pos) == 1 || move_to(p2.pos, "answering request")) {
        //once reached, place requested item in their inventory
        int index = inventory_has(target)[0];
        p2.item_inventory.push_back(pl[p].item_inventory[index]); //give item id from inventory
        pl[p].item_inventory.erase(pl[p].item_inventory.begin() + index);//remove from own inventory
        pl[p].clean_image = true;
        return;
    }
    return;
}

bool People::drop_item(int index) {
    //if dropping item use search_results["no item"] to find an empty tile
    Position dropsite;
    if (pl[p].search_results.find("no item") != pl[p].search_results.end()) {
        dropsite = pl[p].search_results["no item"][0];
        if (Position::distance(dropsite, pl[p].pos) > 1) {//only drop on adjacent tile or one's own tile
            return false;
        }
        Environment::Map[dropsite.y][dropsite.x].item_id = pl[p].item_inventory[index];//place on map
        pl[p].item_inventory.erase(pl[p].item_inventory.begin() + index);//remove from inventory
        pl[p].dropsite = dropsite;
        return true;
    }
    else {
        return false;
    }
    //if dropping an infant, use "no people"    Not yet implemented, fix this
    /*
         Position child_pos;
            if (pl[p].search_results.find("no people") != pl[p].search_results.end()) {
                if (Position::distance(pl[p].search_results["no people"][0], pl[p].pos) == 1) {//if empty adjacent tile
                    child_pos = pl[p].search_results["no people"][0];
                }
            }
            if (child_pos.x == -1) {//if no empty adjacent tile found
                move_to(walk_search_random_dest());//move to a random adjacent tile
                return true;//in progress
            }
    */

}
//for filtering search results by tag such as food
vector<People::Position> People::filter_search_results(string target) {     
    set<Position> unique_results;//used to store unique positions only
    for (string i : it2.tags[target]) {
        if (pl[p].search_results.find(i) != pl[p].search_results.end()) {
            for (Position pos : pl[p].search_results[i]) {
                unique_results.insert(pos);
            }
        }
    }
    vector<Position> final_results;//unsure if it's worth converting to a vector or whether it should return a set
    for (Position pos : unique_results) {
        final_results.push_back(pos);
    }
    return final_results;
}

bool People::acquire(string target) {
    //add_func_record("acquiring " + target);
    //check if target is an item name or item tag
    if (it2.presets.find(target) != it2.presets.end()) {
        //target is an item name
    }
    else if (anim1.species_names.find(target) != anim1.species_names.end()) {
        //target is an animal name
        if (hunting(target)) {//call hunting function
            return true;//done
        }
        return false;//in progress
    }
    else if (it2.tags.find(target) != it2.tags.end()) {
        //target is a tag name
        for (string item_name : it2.tags[target]) {//for every item with this tag, attempt to acquire item, if one is acquired then tag is acquired therefore return true
            //cout << item_name;
            if (acquire(item_name)) {
                return true;//done
            }
        }
        return false;//in progress
    }
    else {
        return false;//target isn't valid
    }

    //if item is craftable, craft it but if in the process of crafting, the item is found, abort crafting the item
    if (!it2.presets[target].ingredients.empty()) {//if has ingredients, then is craftable
        if (craft(target)) {
            return true;//crafting item was successful
        }
    }
    //if crafting item not yet successful or item is not craftable, look around self for item
    if (pl[p].search_results.find(target) != pl[p].search_results.end()) {//key found, if key exists then at least 1 was found
        Position pos = pl[p].search_results[target][0];
        int item_id = Environment::Map[pos.y][pos.x].item_id;
        if (move_to(pos,"found item")) {//if item is found, move to it and pick it up
            pick_up_item(item_id, pos);
            return true;//item picked up
        }
        return false;//if still moving towards item, continue to next tick
    }
    //if item not found, and people nearby, request item
    if (pl[p].search_results.find("people") != pl[p].search_results.end()) {

        bool request_answered = false;
        int answerer_id = -1;
        if (!pl[p].found_messages.empty()) {
            for (int m_id : pl[p].found_messages) {
                Message& m = message_list[message_by_id(m_id)];
                if (m.reciever_id == pl[p].id && m.messsage == "answering request for " + target) {
                    request_answered = true;
                    answerer_id = m.sender_id;
                    break;
                }
            }
        }
        //if request answered, stop requesting and move toward answerer
        if (request_answered) {//Due to sequence ordering issues of NPC updates, need to remember message for a bit to avoid missing messages from NPCs that update after self.
            Position pos = pl[p_by_id(answerer_id)].pos;
            if (Position::distance(pos, pl[p].pos) == 1 || move_to(pos,"recieving item")) {//move to adjacent to answerer
                return false;//wait for answerer to place item in one's inventory (acquire() won't be called next update if was given requested item)
            }
            else {
                return false;//moving toward answerer, in progress
            }
        }
        else {//broadcast request for item
            speak("requesting " + target, -1);
        }
    }
    //if all fails, move in search pattern. Search pattern is shared, to reduce erratic movement from various instances of search patterns
    pl[p].general_search_called = true;
    return false;//searching
}



/////////////////////////above is verified, check below

//need to restructure this function completely given the new changes to find_all and acquire, etc
void People::utility_function() {//is currently actually just a behavior tree not a utility function. Selects what action to take this tick.
    //this implementation allows functions to be interrupted by higher priority ones on every update, however this means that a function may not properly reset or preserve as needed for when it gets called again later, need to fix
    //if(func()==false) go to next func(), if(func()==true) executed this func() for both in progress and done cases
    if (sleeping()) {}
    else if (eating()) {}//if don't have food, searches for food. Therefore the structure of utility_function is focused on which needs to satsify first (sleep, hunger, campsite, reproduction, etc)
    else if (search_for_new_campsite()) {}
    //else if (reproduce()) {} avoid execution of this function to focus on other features without worrying about population size
    else {idle();}
}


//fix later
//much of this function is taken up by carrying or dropping infants, need to create separate functions for that, fix this. Also need to otherwise simplify this function.
bool People::search_for_new_campsite(){ //need to bias search direction in the direction of wherever there is more food rather than waiting to randomly stumble on a site with enough food for campsite. Also need to add a system of not searching the same tile within too short a time frame.
    if (!pl[p].sex && pl[p].spouse_id!=-1 && pl[p].campsite_pos == pl[p_by_id(pl[p].spouse_id)].campsite_pos) {
        return false;//prevent searching for a new campsite if married, only for females
    }
    
    bool cond2 = pl[p].campsite_pos.x == -1 || pl[p].hungry_time >= 3;//AND: have no campsite OR have been hungry too long
    bool cond3 = pl[p].campsite_age > 10;//AND: campsite is old enough to move again. Unsure if this might have an issue if the null campsite has an age
    bool start = (cond2 && cond3) || pl[p].adopt_spouse_campsite;
    //currently only creates a campsite after having been hungry 3 days. Still need to figure out when and when not to create a campsite, such as for trips away from home or extreme high mobility nomad
    //function trigger
    if (!start){
        return false;
    }


    if (pl[p].campsite_pos.x != -1) { //if have campsite, remove. //Later add an option to just abandon a campsite without removing the house. Should only decontruct if going to carry it to new location such as a tent/sleeping bag/lean to/etc.
        if (move_to(pl[p].campsite_pos, "to campsite")) {//walk to campsite to remove
            int item_id = Environment::Map[pl[p].campsite_pos.y][pl[p].campsite_pos.x].item_id;
            delete_item(item_id, pl[p].campsite_pos, -1);
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
    if (!pl[p].children_id.empty()) {//if the game renderer stops running through the whole people list, this breaks because infant is no longer on map. Therefore need to create a better function for carrying and dropping both infants and general items/people. Currently also carries all infants in the same spot, so they overlap. Need to fix this.
        for (int i = 0; i < pl[p].children_id.size(); i++) {
            int kid_index = p_by_id(pl[p].children_id[i]);
            if (pl[kid_index].age < 5 && !pl[kid_index].being_carried) {
                if (Position::distance(pl[kid_index].pos, pl[p].pos) == 1 || move_to(pl[kid_index].pos, "to infant")) {
                    pl[kid_index].being_carried = true;
                    pl[kid_index].carried_by_id = pl[p].id;
                    Environment::Map[pl[kid_index].pos.y][pl[kid_index].pos.x].person_id = -1;
                    pl[kid_index].pos = { -1,-1 };
                    cout << "carrying kid";
                }
            }
        }
    }


    vector<Position> food_pos_list = filter_search_results("food"); //gets results, assigns 1 result to food_pos
    Position food_pos = { -1,-1 };
    bool found_food = false;
    if (!food_pos_list.empty()) {
        food_pos = food_pos_list[0];
        found_food = true;
    }

    if (food_pos_list.size() >= 4) {//if there are 4 food items within sight, select area for campsite, else keep searching
        add_func_record("set up camp");
        pl[p].campsite_age = 0; //resets campsite age
        //place tent
        vector<Position> pos_list = pl[p].search_results["no item"];
        if (!pos_list.empty()) {
            create_item("tent", pos_list[0]);//create and place tent
            pl[p].campsite_pos = pos_list[0]; //store campsite location
            //if have infants, drop them
            if (!pl[p].children_id.empty()) {
                for (int i = 0; i < pl[p].children_id.size(); i++) {
                    int kid_index = p_by_id(pl[p].children_id[i]);
                    if (pl[kid_index].being_carried && pl[kid_index].carried_by_id == pl[p].id) {

                        Position child_pos;
                        if (pl[p].search_results.find("no people") != pl[p].search_results.end()) {
                            if (Position::distance(pl[p].search_results["no people"][0], pl[p].pos) == 1) {//if empty adjacent tile
                                child_pos = pl[p].search_results["no people"][0];
                            }
                        }
                        if (child_pos.x == -1) {//if no empty adjacent tile found
                            pl[p].general_search_called = true;
                            return true;//in progress
                        }

                        pl[kid_index].pos = child_pos;
                        Environment::Map[child_pos.y][child_pos.x].person_id = pl[kid_index].id;
                        pl[kid_index].being_carried = false;
                        pl[kid_index].carried_by_id = -1;
                    }
                }
            }
            return true; //done. need to add an actual animation/progress to building the tent rather than immediate placement
        }
        else {
            pl[p].general_search_called = true;
            return true;//in progress
        }
        return true;//done
    }
    else if (!food_pos_list.empty()) {
        //need to add a method of investigating if any food found might have more food just out of current sightline, but this probably will require more complex modifiable pathfinding, as in have it be a detour from the current destination rather than a change in destination.
    }

    pl[p].general_search_called = true;
    return true;//in progress
}

bool People::sleeping(){
    bool tired = pl[p].tired_level > 50;
    bool start_moving_to_bed = tired && pl[p].campsite_pos.x != -1 && pl[p].hunger_level<50;
    if (start_moving_to_bed) {
        add_func_record("moving to bed");
        if(move_to(pl[p].campsite_pos,"to bed")) { //go to campsite.
            //go to sleep, continue
        }
        else {
            return true;//done and in progress
        }
    }
    bool very_tired = pl[p].tired_level > 100;//might need to cap sleep such that a person can't ever have a tired_level over x_level as well as under y_level
    bool cond1 = tired && (pl[p].pos == pl[p].campsite_pos || pl[p].campsite_pos.x == -1);//if tired AND either at campsite or have no campsite
    if (!(!pl[p].awake || cond1 || very_tired)) {//function trigger
        return false;
    }
    add_func_record("sleeping");
    pl[p].current_image = "pics/human_sleeping.png";
    pl[p].awake = false;
    pl[p].tired_level-=11; //every call to this function reduces tired by 11, this means need 5 hours/updates to stop sleeping and sleep every 50 hours/updates. Is -11 so as to do -10 per hour and also -1 to negate the +1 tired in the regular update function
    if (pl[p].tired_level <= 0) {//fix this, need to cap at 0, also need cap for upper limit?
        pl[p].current_image = "pics/human.png";
        pl[p].awake = true;
        return true;//done
    }
    return true;//in progress
}

bool People::eating(){
    bool hungry = pl[p].hunger_level > 50;
    bool has_food = false;
    vector<int> food_indexes1 = inventory_has("ready food");//should these return sets instead? would remove the need for converting to sets when set operations are needed. Duplicate indexes are never relevant.
    /*
    vector<int> food_indexes2 = inventory_has("needs processing");

    set<int> f_ind_1;
    set<int> f_ind_2;
    for (int i : food_indexes1) {//fix this? This should be part of the inventory_has() by passing a vector<string> instead of a single string
        f_ind_1.insert(i);
    }
    for (int i : food_indexes2) {
        f_ind_2.insert(i);
    }
    set<int> food_indexes;
    set_difference(f_ind_1.begin(), f_ind_1.end(), f_ind_2.begin(), f_ind_2.end(), inserter(food_indexes,food_indexes.end()));//for legibility, might be better to overload operator- so that set3=set1-set2
    */
    if (!food_indexes1.empty()) {
        has_food = true;
        //pl[p].eating_food_index = *food_indexes.begin();
        pl[p].eating_food_index = food_indexes1[0];
    }

    if (hungry && !has_food) {
        if (acquire("ready food")) {
            add_func_record("acquired ready food");
            //done
            return true;
        }
        else {
            return true;
        }
    }

    if (!(hungry && has_food)) {//function trigger
        return false;
    }
    add_func_record("eating");
    if (pl[p].eating_progress.progress == 0) {
        pl[p].current_image = "pics/human_eating.png";
    }
    if (pl[p].age < 5) {
        pl[p].current_image = "human_infant";
    }
    if (pl[p].eating_progress.progress_func()) {//makes eating take more than 1 frame
        int index = pl[p].eating_food_index;
        int food_id = pl[p].item_inventory[index];
        delete_item(food_id, {-1,-1}, index);//delete food from game
        pl[p].hunger_level -= 50; //reduce hungry level by 10, therefore need 2 meals a day to stay at 0 hunger_level average
        pl[p].clean_image = true; //when this function ends, return to default image on next update
        return true;//done eating
    }
     return true;//in progress
}

bool People::hunting(string species) {
    if (species == "deer") {
        if (pl[p].search_results.find("deer") != pl[p].search_results.end()) {
            Animal::animal& a = anim1.al[anim1.a_by_id(Environment::Map[pl[p].search_results["deer"][0].y][pl[p].search_results["deer"][0].x].animal_id)];
            if (inventory_has("rock").empty()) {
                acquire("rock");
                return false;//in progress
            }
            //don't know why it takes a while for the move_to to execute even after search_results has found deer, fix later, leave for now given it eventually does move to hunt the deer
            bool reached = move_to({ a.pos.x,a.pos.y }, "to deer");
            if (reached) {
                if (a.is_alive) {
                    a.is_alive = false;//kill deer
                    return false;//in progress
                }
                if (inventory_has("knife").empty()) {
                    acquire("knife");
                   return false;//in progress
                }
                Environment::Map[a.pos.y][a.pos.x].animal_id = -1;//remove dead animal from map
                create_item("deer_meat", {a.pos.x, a.pos.y});//add deer meat in its place
                anim1.al.erase(anim1.al.begin() + anim1.a_by_id(a.id));//erase animal from global animal list
                return true;//done
            }
        }
        else {
            pl[p].general_search_called = true;
            return false;//search for target
        }
    }
    else if (species == "rabbit") {//need to handle when to remove traps or place them in new spots, especially when either not having a campsite or when moving campsites, fix this
        if (pl[p].search_results.find("rabbit") != pl[p].search_results.end()) {
            vector<Position> pos_list = pl[p].search_results["rabbit"];
            sort(pos_list.begin(), pos_list.end());//need to sort vector before using or else will get stuck. unsure if sort by current position or by 0,0
            Animal::animal& a = anim1.al[anim1.a_by_id(Environment::Map[pos_list[0].y][pos_list[0].x].animal_id)]; 
            if (!a.is_alive) {
                if (inventory_has("knife").empty()) {
                    acquire("knife");
                    return false;//in progress
                }
                if (move_to(pos_list[0],"to rabbit")) {
                    Environment::Map[a.pos.y][a.pos.x].animal_id = -1;//remove dead animal from map
                    delete_item(Environment::Map[a.pos.y][a.pos.x].item_id, { a.pos.x,a.pos.y }, -1);//delete active trap
                    create_item("rabbit_meat", { a.pos.x,a.pos.y });//add rabbit meat in its place
                    anim1.al.erase(anim1.al.begin() + anim1.a_by_id(a.id));//erase animal from global animal list
                    return true;//done
                }
            }
        }
        if (pl[p].saw_rabbit_recently) {
            if (pl[p].traps_set.size() < 4) {
                if (pl[p].search_results.find("no item") != pl[p].search_results.end()) {
                    if (inventory_has("trap").empty()) {
                        acquire("trap");
                        return false; //in progress
                    }
                    int trap_ind = inventory_has("trap")[0];
                    delete_item(pl[p].item_inventory[trap_ind], { -1,-1 }, trap_ind);
                    create_item("active trap", { -1,-1 });
                    trap_ind = inventory_has("active trap")[0];
                    if (!drop_item(trap_ind)) {
                        pl[p].general_search_called = true;
                        return false;//in progress
                    }
                    pl[p].traps_set.push_back({ pl[p].dropsite, pday_count });
                    pl[p].dropsite = { -1,-1 };
                    return false;//in progress
                }
            }
        }
        if (!pl[p].traps_set.empty()) {//if have traps, check traps
            bool all_traps_checked = true;
            for (trap_check& t : pl[p].traps_set) {
                if ((pday_count - t.last_day_checked) >= 3) {//check each trap once every other day since it was set
                    all_traps_checked = false;
                    if (move_to(t.pos, "to trap")) {
                        t.last_day_checked = pday_count;
                    }
                    break;
                }
            }
        }
     }
    pl[p].general_search_called = true;
    return false;//searching
}




//need to either figure out a way to handle order execution priority between getting food and removing campsite or create a function called remove_campsite to encapsulate its code and call before attempting to get food

//note: need to make parent check on kid periodically, and give priority to giving food to kids rather than others. fix this //feed hungriest first? Unsure if this should be implemented
//note: when gathering food, always gather a bit more than needed to satisfy hunger to have some in inventory either for self later or to share or feed infants
//note: spoken messages might need their own priority tree separate from the movement related actions