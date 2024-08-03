#include "People.hpp"
using namespace std;

vector<People::Person> People::pl; //pl, using the name pl because of the frequency of use, used to store all Person instances
vector<People::Message> People::message_list;
vector<int> People::Message_Map[Environment::map_y_max][Environment::map_x_max];
int People::p = -1;//index for accessing current person. Using index to access instead of a pointer because list may change such as when a new person is born or dies which invalidates pointers to pl (people_list)
ItemSys it2;//used to access member functions and variables of ItemSys
int People::ox = -1;
int People::oy = -1;
int People::pday_count;
int People::phour_count;
int People::people_id_iterator = 0;

People::People(){}

People::People(int initint) {
    
    Person p1 = { new_person_id(), {50,25}, true};
    p1.age = 11;
    pl.push_back(p1);
    Environment::Map[p1.pos.y][p1.pos.x].person_id = p1.id;

    /*
    Person p2 = { new_person_id(), {51,26}, false};
    p2.age = 11;
    pl.push_back(p2);
    Environment::Map[p2.pos.y][p2.pos.x].person_id = p2.id;

    Person p3 = { new_person_id(), {50,24}, true };
    p3.age = 11;
    pl.push_back(p3);
    Environment::Map[p3.pos.y][p3.pos.x].person_id = p3.id;
    Person p4 = { new_person_id(), {52,23}, true };
    p4.age = 11;
    pl.push_back(p4);
    Environment::Map[p4.pos.y][p4.pos.x].person_id = p4.id;
    Person p5 = { new_person_id(), {53,22}, false };
    p5.age = 11;
    pl.push_back(p5);
    Environment::Map[p5.pos.y][p5.pos.x].person_id = p5.id;
    Person p6 = { new_person_id(), {54,21}, false };
    p6.age = 11;
    pl.push_back(p6);
    Environment::Map[p6.pos.y][p6.pos.x].person_id = p6.id;
    */
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

    for(int i = 1; i < pl.size(); i++) {//i starts at 1 because for now, the first Person in the list is reserved for player control
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

    if (day_count % 5 == 0 && hour_count==0) {
        for (int i = 0; i < pl.size(); i++) {
            cout << "id: " << pl[i].id << " -- \n";
            for (auto const& d : pl[i].dispositions) {
                cout << "pers: " << d.first << " disp:" << d.second << "\n";
            }
            cout << "----------------------------\n\n\n";
        }
    }
}

bool People::check_death() {
    bool starvation = pl[p].hunger_level > 1000;
    bool dehydration = pl[p].thirst_level > 1000;
    bool old_age = pl[p].age > 50;
    bool death = !pl[p].is_alive || starvation || dehydration || old_age;
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
        pl[p].hunger_level = -1;
        pl[p].thirst_level = -1;
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
        if (pl[p].hunger_level > 50) {
            speak("requesting food",-1);
        }
        if (pl[p].being_carried) {//if being carried, then position is the position of the carrier offset by 1
            pl[p].pos = pl[p_by_id(pl[p].carried_by_id)].pos;
            pl[p].pos.x += 1;
            //cout << "am carried";
        }
        pl[p].immobile = true;
        eating();
        return;
    }
    else {
        pl[p].immobile = false;
        pl[p].clean_image = true;
    }

    

    pl[p].hunger_level++; //hunger increases by 1 per hour, meaning it must decrease by 20 per day to stay at 0 daily average
    pl[p].tired_level++; //same for tired level
    pl[p].campsite_age++;
    pl[p].reproduction_cooldown++; //for when to find mate and create new person
    pl[p].thirst_level++;

    if (pl[p].clean_image) {
        pl[p].current_image = "pics/human.png";
        pl[p].clean_image = false;
    }

    find_all();//gets all items, people, etc from within sight/earshot to then react to or inform next action

    utility_function();
    if (!pl[p].move_already && pl[p].general_search_called) {
        general_search_walk("");//ensures this function only executes once per update and also only after all other move_to's have been considered. This prioritizes intentional movement (moving to a target) rather than random movement
    }

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

    pl[p].move_already = false;
    pl[p].general_search_called = false;

    authority_calc();
    build_monument();
    if (hour_count == 0) {//once a day
        for (auto& i : pl[p].dispositions) {//dispositions tend toward returning to neutral over time if no other factor involved
            if (i.second > 0) {
                change_disposition(i.first, -1, "neutral drift");
            }
            else if (i.second < 0) {
                change_disposition(i.first, 1, "neutral drift");
            }
        }
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

bool People::move_to(Position dest, string caller) {//need to add speed of moving from one tile to another and how many tiles at a time. Also need to add a check to prevent it being called more than once per person per update.
    bool reached = false;
    Position old_pos;
    Position new_pos;
    if (!pl[p].mov) {
        if (pl[p].general_search_called && caller != "general searching") {
            pl[p].general_search_called = false;
        }

        if (pl[p].downed || pl[p].immobile || pl[p].move_already || pl[p].age < 5) {//the image check shouldn't be necessary but I don't know why it still moves while having crafting image
            if (pl[p].pos == dest) {
                return true;
            }
            return false;
        }
        //cout << pl[p].id << " - " << caller << "\n";
        old_pos = pl[p].pos;
        new_pos = pl[p].pos;
        if (new_pos.x < dest.x) {//for future optimization, see: https://stackoverflow.com/questions/14579920/fast-sign-of-integer-in-c
            new_pos.x++;
        }
        else if (new_pos.x > dest.x) {
            new_pos.x--;
        }

        if (new_pos.y < dest.y) {
            new_pos.y++;
        }
        else if (new_pos.y > dest.y) {
            new_pos.y--;
        }

        if (!(Environment::Map[new_pos.y][new_pos.x].person_id == -1)) {//if a person is blocking path, move to first empty tile in a clockwise direction
            pl[p].current_direction = { old_pos.x - new_pos.x, old_pos.y - new_pos.y };
            Position test_pos;
            bool next = false;
            for (Position pos : pl[p].rotations) {//still just a temp fix, need actual pathfinding algo
                if (next) {
                    test_pos = { old_pos.x + pos.x, old_pos.y + pos.y };
                    if (valid_position(test_pos) && test_pos != pl[p].pos && Environment::Map[test_pos.y][test_pos.x].person_id == -1) {
                        new_pos = test_pos;
                        break;
                    }
                }
                if (pl[p].current_direction == pos) {
                    if (next) {
                        new_pos = old_pos;//no free adjacent tile, don't move
                        break;
                    }
                    next = true;
                }
            }
        }

        pl[p].move_already = true;
        pl[p].dest = new_pos;
    }

    
    old_pos = pl[p].pos;
    new_pos = pl[p].dest;
    int sqdim1 = 17;
    int spd = 4;
    pl[p].current_direction = { old_pos.x - new_pos.x, old_pos.y - new_pos.y };

    if (old_pos.x<new_pos.x) {//fix this, later need to modify to tie to new tile when it hits the midway point between 2 tiles
        pl[p].px_x += 1 * spd;
        
        pl[p].mov = true;
        if (pl[p].px_x == 16) {
            pl[p].mov = false;
            pl[p].px_x += 1;
            pl[p].pos.x++;
            pl[p].px_x = 0;
            //pl[p].px_x *= -1;
        }
    }
    else if (old_pos.x > new_pos.x) {
        pl[p].px_x -= 1 * spd;
        
        pl[p].mov = true;
        if (pl[p].px_x == -16) {
            pl[p].mov = false;
            pl[p].px_x -= 1;
            pl[p].pos.x--;
            pl[p].px_x = 0;
            //pl[p].px_x *= -1;
        }
    }
    if (old_pos.y > new_pos.y) {
        pl[p].px_y -= 1 * spd;
        
        pl[p].mov = true;
        if (pl[p].px_y == -16) {
            pl[p].mov = false;
            pl[p].pos.y--;
            pl[p].px_y -= 1;
            pl[p].px_y = 0;
            //pl[p].px_y *= -1;
        }
    }
    else if (old_pos.y < new_pos.y) {
        pl[p].px_y += 1 * spd;
       
        pl[p].mov = true;
        if (pl[p].px_y == 16) {
            pl[p].mov = false;
            pl[p].pos.y++;
            pl[p].px_y += 1;
            pl[p].px_y = 0;
            //pl[p].px_y *= -1;
        }
    }

    if (pl[p].pos == new_pos) {
        Environment::Map[old_pos.y][old_pos.x].person_id = -1;//remove person from Map
        Environment::Map[pl[p].pos.y][pl[p].pos.x].person_id = pl[p].id;//add person back to Map at new location
    }
    



    //pl[p].pos = new_pos;
   // Environment::Map[pl[p].pos.y][pl[p].pos.x].person_id = -1;//remove person from Map
    //Environment::Map[new_pos.y][new_pos.x].person_id = pl[p].id;//add person back to Map at new location
    reached = pl[p].pos == dest;
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

bool People::child_birth() {
    if (pl[p].sex) {
        return false;//am male
    }
    if (pl[p].pregnancy.progress == 0) {
        return false;//not pregnant
    }
    if (pl[p].pregnancy.progress_func()) {//advance pregnancy until done, if done create child
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
        pl[p_by_id(pl[p].spouse_id)].children_id.push_back(child.id);
        return true;//done
    }
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
                p2 = p_by_id(pers_id);//mate found
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
            if (!pl[p].sex) {//if female
                pl[p].pregnancy.progress = 1;//am now pregnant
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
//for cleaner code
void People::find_all_helper(Position pos, string type) {
    if (!valid_position(pos)) {return;}
    string key;
    if (type == "people") {
        if (pl[p].lastpos == pos) {
            throw std::invalid_argument("my error");
        }
        pl[p].lastpos = pos;
        if (Environment::Map[pos.y][pos.x].person_id != -1) {key = "people";}
        else {key = "no people";}}
    else if (type == "item") {
        if (Environment::Map[pos.y][pos.x].item_id != -1) {key = ItemSys::item_list[ItemSys::item_by_id(Environment::Map[pos.y][pos.x].item_id)].item_name;}
        else {key = "no item";}}
    else if (type == "animal") {
        if (Environment::Map[pos.y][pos.x].animal_id != -1) {key = Animal::al[anim1.a_by_id(Environment::Map[pos.y][pos.x].animal_id)].species;}
        else {key = "no animal";}}
    else if (type == "terrain") {key = Environment::Map[pos.y][pos.x].terrain;}
    
    if (pl[p].search_results.find(key) != pl[p].search_results.end()) {//check if key exists
        pl[p].search_results[key].push_back(pos);}//key found
    else {pl[p].search_results.insert({ key,{pos} });}//key not found
}

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
    Position o = pl[p].pos;//origin
    vector<int> target_quantity_current;
    for (int radius = 0; radius <= pl[p].radiusmax; radius++) { //this function checks tilemap in outward rings by checking top/bottom and left/right ring boundaries
        if (radius == 0) {//avoids double checking origin
            if (radius <= pl[p].sightline_radius) {
                vector<string> types = { "item","animal","terrain" };
                for (string s : types) {
                    find_all_helper(o, s);
                }
            }
            if (radius <= pl[p].audioline_radius) {
                //check for messages
                check_tile_messages(o);
            }
            continue;
        }
        int xmin = o.x - radius;
        if (xmin < 0) { xmin = 0; }//these reduce iterations when near edges of map
        int xmax = o.x + radius;
        if (xmax > Environment::map_x_max-1) { xmax = Environment::map_x_max-1; }
        int ymin = o.y - radius + 1;//+1 and -1 to avoid double checking corners
        if (ymin < 0) { ymin = 0; }
        int ymax = o.y + radius - 1;
        if (ymax > Environment::map_y_max - 1) { ymax = Environment::map_y_max - 1; }
        for (int x = xmin, y = ymin; x <= xmax; x++, y++) {
            for (int sign = -1; sign <= 1; sign += 2) {//sign == -1, then sign == 1
                Position pos1 = { x,o.y + (sign * radius) };
                Position pos2 = { o.x + (sign * radius), y };

                //if (pos1==pos2) {
                //        throw std::invalid_argument("my error2");
                 //   }

                if (radius <= pl[p].sightline_radius) {
                    vector<string> types = { "people","item","animal","terrain" };
                    for (string s : types) {
                        find_all_helper(pos1, s);
                        if (y <= ymax && pos1!=pos2) {//need to figure out why pos1 sometimes == pos2 and rewrite for loops to avoid this
                            find_all_helper(pos2, s);
                        }
                    }
                }
                if (radius <= pl[p].audioline_radius) {
                    //check for messages
                    if (valid_position(pos1)) {
                        check_tile_messages(pos1);
                    }
                    if (y <= ymax && pos1 != pos2) {
                        if (valid_position(pos2)) {
                            check_tile_messages(pos2);
                        }
                    }
                }
            }
        }
    }
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
    int option = rand() % 100;
    if (pl[p].dispositions.empty()) {
        option = 51;
    }
    bool have_no_friends = false;
    do {
        if (option > 50) {
            //flip between idle and default image
            if (pl[p].current_image == "pics/human.png") {
                pl[p].current_image = "pics/human_idle.png"; //need to make image, just have human with raised hands
            }
            else {
                pl[p].current_image = "pics/human.png";
            }
            have_no_friends = false;
        }
        else {//should this instead be a social need or is it good here in idle?
            int sum = 0;
            for (auto const& i : pl[p].dispositions) {
                if (i.second > 0) {
                    sum += i.second;
                }
            }
            int dice = (rand() % (sum + 1)) - 1;//+1,-1 is to avoid division by 0
            sum = 0;
            int p2id = -1;
            for (auto const& i : pl[p].dispositions) {//selects a person one likes with higher chance the more someone is liked, to go to socialize with
                if (i.second > 0) {
                    sum += i.second;
                    if (sum > dice) {
                        p2id = i.first;
                        break;
                    }
                }
            }
            if (p2id != -1) {
                move_to(pl[p_by_id(p2id)].pos, "to socialize - idle");//to search out liked people to encourage clique congregation and disposition sharing
            }
            else {
                have_no_friends = true;//causes execution of other idle option
                option = 51;
            }
        }
    } while (have_no_friends);
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
        //pl[p].immobile = true;//prevents moving while crafting?? doesn't work?
        if (pl[p].crafting.find(product) == pl[p].crafting.end()) {//key not found
            pl[p].crafting.insert({ product,{4} });//4 is the number of ticks/frames crafting image/animation lasts
        }
        if (pl[p].crafting[product].progress_func()) {//animation/time delay    currently, progress is saved if interrupted which might not make sense in some contexts
            //cout << "made: " << product << "\n";
            for (int i = 0; i < num_of_ingredients; i++) {//later implement tool degradation here as well
                if (it2.presets[it2.presets[product].ingredients[i]].consumable_ingredient) {
                    int consume_index = inventory_has(it2.presets[product].ingredients[i])[0];
                    delete_item(pl[p].item_inventory[consume_index], { -1,-1 }, consume_index);
                }
            }
            create_item(product, { -1,-1 });
            pl[p].crafting.erase(product);
            pl[p].clean_image = true; //when this function ends, return to default image on next update
           // pl[p].immobile = false;
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

bool People::answer_item_request() {
    if (pl[p].found_messages.empty()) {
        return false;
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
        return false;
    }
    //choose message to answer  //answering item requests. if answered and whether to answer. If like person or they have authority or I submit to them, then chance of if they request something I don't have, I acquire item in order to give it to them
    bool acquire_to_give = false;
    int service=-1;
    for (int i = request_messages.size() - 1; i > -1; i--) {
        int sid = request_messages[i].sender_id;
        change_disposition(sid, 0, "check exists");
        if (pl[p].submissive_to.find(sid) == pl[p].submissive_to.end()) { pl[p].submissive_to.insert({ sid,{0,false} }); }//should have a first contact function to populate these lists instead of checking if exists every time the list is used. Fix this
        if (pl[p].dispositions[sid] < -25 && !pl[p].submissive_to[sid].submissive_to) {//don't answer requests from disliked people unless you are submissive to them
            request_messages.erase(request_messages.begin() + i);
            items_requested.erase(items_requested.begin() + i);
        }
        if (pl[p].dispositions[sid] > 75 || pl[p].submissive_to[sid].submissive_to) {
            acquire_to_give = true;
            service = i;
            break;//force answer this request
        }
    }
    if (items_requested.empty()) {//requests were from disliked and non dominant people
        return false;
    }
    if (!acquire_to_give) {
        for (int i = items_requested.size() - 1; i > -1; i--) {
            if (inventory_has(items_requested[i]).empty()) {//if don't have item requested, remove from list of items requested
                items_requested.erase(items_requested.begin() + i);
                request_messages.erase(request_messages.begin() + i);
            }
        }
        if (items_requested.empty()) {//don't have any of the items requested
            return false;
        }
    }

    //conditional checks such as don't give food if self is hungry or give to kids before others if kids are hungry. Not implemented, fix this
    /*
    * bool hungry = pl[p].hunger_level > 50;//condition on willingness to answer request
      bool start_give_food = !hungry && !pl[p].child_is_hungry;
    */
    


    //send answer
    int m_ind = 0;
    if (acquire_to_give) {
        m_ind = service;
        if (acquire(items_requested[service])) {
            //continue
        }
        else {
            return true;//in process
        }
    }
    string target = items_requested[m_ind];//currently simply selects the first item request in list to answer. Fix this, no condition on when or when not to answer has been implemented
    int receiver_id = request_messages[m_ind].sender_id;//id of person who requested the item
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
        int op = p;//temp reassign main p
        p = p_by_id(receiver_id);
        change_disposition(pl[op].id, 10, "recieved item requested");
        p = op;
        return true;//done
    }
    return true;//in process
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
        if (target == "wood") {
            if (cut_down_tree()) {//do/fix this: might make sense to make tree a consumable station, an activity that must be performed at a specific spot (next to a tree) but the spot is consumed at the end (tree turned into wood)
                return true;//done
            }
            return false;//in progress
        }
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
        if (move_to(pos,"found item"+target)) {//if item is found, move to it and pick it up
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

void People::utility_function() {//is currently actually just a behavior tree not a utility function. Selects what action to take this tick.
    //this implementation allows functions to be interrupted by higher priority ones on every update, however this means that a function may not properly reset or preserve as needed for when it gets called again later, need to fix
    //if(func()==false) go to next func(), if(func()==true) executed this func() for both in progress and done cases
    chat();//chance to chat every update
    if (pl[p].mov) {
        move_to(pl[p].dest, "continue");
        return;
    }
    if(child_birth()){}
    else if(fight()){}
    else if (sleeping()) {}
    else if(drinking()){}
    else if (eating()) {}//if don't have food, searches for food. Therefore the structure of utility_function is focused on which needs to satsify first (sleep, hunger, campsite, reproduction, etc)
    else if (search_for_new_campsite()) {}
    else if (reproduce()) {} //avoid execution of this function to focus on other features without worrying about population size
    else if(answer_item_request()){}
    else {idle();}
    //DO THIS: (this (authority pursuit AI) might be too complex for this version, maybe organic leaders is better and add behavior that makes it likelier for some npcs to get to victory condition?) need to add authority as a need/goal to be pursued. Which means starting and winning fights with new people to increase number of submissives, and gaining favor with more people and increasing favor with existing friends/allies
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
            pl[p].friend_camp_check = true;
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
                    for (int j = 0; j < pl[p].search_results["people"].size(); j++) {//ensure integrity of search_results (due to possible vector out of bounds error source)
                        if (pl[p].search_results["people"][j] == pl[kid_index].pos) {
                            pl[p].search_results["people"].erase(pl[p].search_results["people"].begin() + j);
                        }
                    }
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

    if (pl[p].friend_camp_check) {//encourages campsite congregation between people who like each other (forgot to check if other person likes self, fix this)
        for (auto const& i : pl[p].dispositions) {
            if (i.second > 75) {
                if (pl[p_by_id(i.first)].campsite_pos.x != -1) {//if have a friend with a campsite and am finding a new campsite, move to within 10 tiles of friend to search for campsite location there
                    if (Position::distance(pl[p].pos, pl[p_by_id(i.first)].campsite_pos) < 10 || move_to(pl[p_by_id(i.first)].campsite_pos, "to friend's campsite")) {
                        pl[p].friend_camp_check = false;
                    }
                    else {
                        return true;//in progress
                    }
                }
            }
        }
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
    bool start_moving_to_bed = tired && pl[p].campsite_pos.x != -1 && pl[p].hunger_level<50 && pl[p].thirst_level<50;
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
    if (pl[p].hungry_time > 3 && pl[p].campsite_pos.x != 1 && pl[p].campsite_age>15) {//allows search for campsite to trigger but not before new camp is at least a few ticks old
        return false;
    }

    bool hungry = pl[p].hunger_level > 50;
    bool has_food = false;
    vector<int> food_indexes1 = inventory_has("ready food");//should these return sets instead? would remove the need for converting to sets when set operations are needed. Duplicate indexes are never relevant.
    if (!food_indexes1.empty()) {
        has_food = true;
        //pl[p].eating_food_index = *food_indexes.begin();
        pl[p].eating_food_index = food_indexes1[0];
    }

    if ((hungry && !has_food) || food_indexes1.size() < 2) {//ensures that person has 2 food items in inventory for self or to share
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

bool People::drinking() {
    if (pl[p].thirst_level < 50) {
        return false;
    }
    if (pl[p].search_results.find("water") != pl[p].search_results.end()) {
        if (move_to(pl[p].search_results["water"][0],"to water")) {
            pl[p].thirst_level -= 25;
            return true;//done
        }
    }
    else {
        pl[p].general_search_called = true;
    }
    return true;//in progress
}

bool People::cut_down_tree() {
    if (!inventory_has("wood").empty()) {
        cout << "have wood";
        return true;
    }
    if (pl[p].search_results.find("tree") != pl[p].search_results.end()) {
        if (Position::distance(pl[p].pos, pl[p].search_results["tree"][0])==1 || move_to(pl[p].search_results["tree"][0], "to tree")) {
            Position pos = pl[p].search_results["tree"][0];
            int tree_id = Environment::Map[pos.y][pos.x].item_id;
            delete_item(tree_id, pos, -1);
            create_item("wood", pos);
            int wood_id = Environment::Map[pos.y][pos.x].item_id;
            pick_up_item(wood_id, pos);
            return true;//done
        }
        return false;//in progress
    }
    else {
        pl[p].general_search_called = true;
        return false;//searching
    }
}

void People::change_disposition(int p_id, int amount, string reason) {
    if (p_id == 0) {
        throw std::invalid_argument("pid is 0");
    }
    if (pl[p].id == p_id) {
        return;
    }
    if (pl[p].dispositions.find(p_id) != pl[p].dispositions.end()) {
        pl[p].dispositions[p_id] = pl[p].dispositions[p_id] + amount;
        if (pl[p].dispositions[p_id] < -100) {//caps dispostion at -100 and 100
            pl[p].dispositions[p_id] = -100;
        }
        else if (pl[p].dispositions[p_id] > 100) {
            pl[p].dispositions[p_id] = 100;
        }
        //cout << "change\n";
    }
    else {
        pl[p].dispositions.insert({ p_id,amount });
    }
}

void People::chat() {//if make this a speak() action, can affect dispositions of more than 1 person with 1 comment, fix this?
    if (pl[p].search_results.find("people") != pl[p].search_results.end()) {
    int topic = rand() % 100;
    int p2_ind = p_by_id(Environment::Map[pl[p].search_results["people"][0].y][pl[p].search_results["people"][0].x].person_id);
    int p2_id = pl[p2_ind].id;
    if (topic < 50) {//compliment or insult
            int comment = (rand() % 30) - 15;
            if (pl[p].dispositions.find(p2_id) == pl[p].dispositions.end()) {//fix this every instance, these are unnecessary as it is more readable to just call change_disposition with an amount of 0 to do the same (insert key if key not found)
                pl[p].dispositions.insert({ p2_id,0 });
            }
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
            string valence;//valence should be the int and comment should be the string, switch, fix this, just for clarity

            (comment < 0) ? valence = "insult" : valence = "compliment";
            change_disposition(pl[op].id, comment, valence);//positive is compliment, negative is insult
            p = op;
            //cout << "chat\n";

        }
        else {
            share_disposition(p2_ind);
        }
    }
}

void People::authority_calc() {
    int num_people_liked_by = 0;
    int amount_liked = 0;
    int num_submissives = 0;
    bool am_sovereign = true;
    for (int i = 0; i < pl.size(); i++) {
        if (pl[i].dispositions.find(pl[p].id) != pl[i].dispositions.end()) {
            if (pl[i].dispositions[pl[p].id] > 0) {
                num_people_liked_by++;
                amount_liked += pl[i].dispositions[pl[p].id];
            }
        }
        if (pl[i].submissive_to.find(pl[p].id) != pl[i].submissive_to.end()) {
            if (pl[i].submissive_to[pl[p].id].submissive_to) {
                num_submissives++;
            }
        }
    }
    for (auto const& i : pl[p].submissive_to) {//if one is submissive to anyone, one is not sovereign. Fix this. DO THIS: Need to implement a rebellion function so that when one's authority increases a certain amount or relative to a dominator, can cast off submissive status and become sovereign
        if (i.second.submissive_to) {
            am_sovereign = false;
        }
    }
    pl[p].authority = num_people_liked_by * amount_liked * num_submissives * pl[p].num_fights_won;
    //10000 = 10*10*10*10 = 1*100*1*1  <-- this may be a problem, rethink formula
    if (pl[p].authority > 10000 && am_sovereign) {
        pl[p].monument_unlocked = true;
    }
    else {
        pl[p].monument_unlocked = false;//access to monument building can be lost if authority is lost
    }
}

bool People::hostile_detection() {
    bool found_hostiles = false;
    if (pl[p].search_results.find("people") != pl[p].search_results.end()) {
        for (Position pos : pl[p].search_results["people"]) {
            Person& p2 = pl[p_by_id(Environment::Map[pos.y][pos.x].person_id)];
            for (int i : p2.active_hostile_towards) {
                if (i == pl[p].id) {//if a person nearby is actively hostile to self
                    pl[p].active_hostile_towards.push_back(p2.id);
                    pl[p].hostile_towards.push_back(p2.id);
                    found_hostiles = true;
                }
            }
        }
    }
    if (found_hostiles) {
        return true;
    }
    else {
        return false;
    }
}

vector<int> People::remove_dup(vector<int> v) {
    vector<int> v2;
    bool dupe = false;
    for (int i : v) {
        for (int j : v2) {
            if (i == j) {
                dupe = true;
                break;
            }
        }
        if (!dupe) {
            v2.push_back(i);
        }
        dupe = false;
    }
    return v2;
}


//need to implement being downed if lost a personal fight and how to recover, as well as in what cases might be killed or can kill (maybe a percent chance?)
//need to add weapons and armor that affect dice rolls, and triggers for acquiring them
bool People::fight() {//Order of execution for people here might necessitate having an extra tick to resolve results of combat dice rolls, otherwise people who update first will always attack and the latter always defend?
    hostile_detection();//gets active enemies
    pl[p].active_hostile_towards = remove_dup(pl[p].active_hostile_towards);
    pl[p].hostile_towards = remove_dup(pl[p].hostile_towards);
    int fight_choice = -1;
    if (pl[p].hostile_towards.empty()) {//if initiating fight
        //go through every person have disposition towards. If hate someone, chance to trigger fight. //not yet implemented: If fight triggered, chance to invite friends and family to join fight. 
        vector<int> hated;
        for (auto const& i : pl[p].dispositions) {
            if (i.second < -75) {
                hated.push_back(i.first);//get id's
            }
        }
        if (!hated.empty()) {
            fight_choice = rand() % hated.size();//get index
            fight_choice = hated[fight_choice];//get id of enemy
        }
        int chance_to_fight = rand() % 100;

        if (!(chance_to_fight < 5 && fight_choice != -1)) {//function trigger
            return false;//no fight
        }

        //if initiating fight, become hostile towards target
        pl[p].active_hostile_towards.push_back(fight_choice);
        pl[p].hostile_towards.push_back(fight_choice);
    }
    else {
        if (!pl[p].active_hostile_towards.empty()) {
            fight_choice = pl[p].active_hostile_towards[0];//if didn't initiate fight, focus on fighting first active hostile found
        }
    }

    if (!pl[p].active_hostile_towards.empty()) {//active hostiles means fight is active
        bool won = false;//for individual fights and personal glory
        //DO THIS, later: check combat allies, if a third have been downed or killed, surrender and submit. Currently fights are only 1v1, no combat allies. Multiple enemies or combat allies might appear simply due to hate triggering 1v1 fights but is unintended.
        for (int i = 0; i < pl[p].active_hostile_towards.size(); i++) {//for every enemy combatant in the fight
            int p2 = p_by_id(fight_choice);
            bool tie = true;
            
            //combat
            while (tie && (Position::distance(pl[p].pos, pl[p2].pos) == 1 || move_to(pl[p2].pos, "to enemy combatant"))) {//go to target
                if (pl[p].dice == -1) {
                    pl[p].dice = rand() % 20;//when next to target, progress delay and dice roll, currently no progress delay implemented
                    return true;//in progress
                }
                if (pl[p].dice > pl[p2].dice) {//if my dice roll is larger than theirs, I win, else I lose. Tie means keep fighting
                    won = true;
                    tie = false;
                    for (int i = 0; i < pl[p].active_hostile_towards.size(); i++) {//win or lose, need to move myself or other from active_hostile to hostile list to no longer be targeted in fight
                        if (pl[p].active_hostile_towards[i] == pl[p2].id) {
                            pl[p].active_hostile_towards.erase(pl[p].active_hostile_towards.begin() + i);
                            break;
                        }
                    }
                    for (int i = 0; i < pl[p2].active_hostile_towards.size(); i++) {
                        if (pl[p2].active_hostile_towards[i] == pl[p].id) {
                            pl[p2].active_hostile_towards.erase(pl[p2].active_hostile_towards.begin() + i);
                            break;
                        }
                    }
                }
                else if (pl[p].dice < pl[p2].dice) {
                    pl[p].downed = true;
                    won = false;
                    tie = false;
                    for (int i = 0; i < pl[p].active_hostile_towards.size(); i++) {
                        if (pl[p].active_hostile_towards[i] == pl[p2].id) {
                            pl[p].active_hostile_towards.erase(pl[p].active_hostile_towards.begin() + i);
                            break;
                        }
                    }
                    for (int i = 0; i < pl[p2].active_hostile_towards.size(); i++) {
                        if (pl[p2].active_hostile_towards[i] == pl[p].id) {
                            pl[p2].active_hostile_towards.erase(pl[p2].active_hostile_towards.begin() + i);
                            break;
                        }
                    }
                }
                else {
                    pl[p].dice = -1;
                }
            }
            //can only fight against 1 person at a time meaning being outnumbered personally at the moment is immediate loss of fight
            if (won) {
                pl[p].num_fights_won++;

            }
            else {
                pl[p].num_fights_won--;

            }
        }
    }
    
    if (pl[p].active_hostile_towards.empty() && !pl[p].hostile_towards.empty()) {//unsure if this check is redundant. Post fight/battle handling.
        bool battle_won = false;
        if (!pl[p].combat_allies.empty()) {
            //check if battle won or lost   not yet implemented
            //do something
            //if in group and group wins fight, get a bit of reflected glory (increase fights won by 1 even if lost personal fights)
        }
        if (pl[p].downed = false) {//for now, battle is won if not downed when no active hostiles left
            battle_won = true;
        }
        else {
            battle_won = false;
        }
        if (battle_won) {
            pl[p].num_fights_won++;
            for (int i = 0; i < pl[p].hostile_towards.size(); i++) {//enemies submit. If I lose then when enemy updates they will modify my own list to submit to them
                if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.find(pl[p].id) != pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.end()) {
                    pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].fights_lost++;
                    if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].fights_lost >= 3) {
                        pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].submissive_to = true;
                    }
                }
                else {
                    pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.insert({ pl[p].id,{false,1} });
                }
            }
        }
        else {
            pl[p].num_fights_won--;
            for (int i = 0; i < pl[p].hostile_towards.size(); i++) {//reduces enemy submission level to me
                if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.find(pl[p].id) != pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.end()) {
                    pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].fights_lost--;
                    if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].fights_lost < 3) {
                        pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].submissive_to = false;
                    }
                }
                else {
                    pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.insert({ pl[p].id,{false,-1} });
                }
            }
        }

        for (int i = 0; i < pl[p].hostile_towards.size(); i++) {//for every person I'm hostile to, remove me from their hostile list. Clean up this code to make more readable
            for (int j = 0; j < pl[p_by_id(pl[p].hostile_towards[i])].hostile_towards.size(); j++) {
                if (pl[p_by_id(pl[p].hostile_towards[i])].hostile_towards[j] == pl[p].id) {
                    pl[p_by_id(pl[p].hostile_towards[i])].hostile_towards.erase(pl[p_by_id(pl[p].hostile_towards[i])].hostile_towards.begin() + j);
                    break;
                }
            }
        }
        pl[p].hostile_towards.clear();
        pl[p].downed = false;//when fight ends, no longer downed.
        return true;//fight over
    }
    return false;//no fight. is this check redundant?
}

void People::share_disposition(int p2_ind) {
    if (pl[p].dispositions.empty()) {
        return;
    }
    //random person for now, but should adjust to move all dispositions closer (or further) between the 2 people chatting
    int subjectp_ind = rand() % pl[p].dispositions.size();
    vector<int> pids;
    for (auto const& i : pl[p].dispositions) {//convert to vector because can't access random element from map
        pids.push_back(i.first);
    }
    int subjectp_id = pids[subjectp_ind];
    int disp = pl[p].dispositions[subjectp_id];

    int their_opinion_of_me;
    if (pl[p2_ind].dispositions.find(pl[p].id) != pl[p2_ind].dispositions.end()) {
        their_opinion_of_me = pl[p2_ind].dispositions[pl[p].id];
    }
    else{
        pl[p2_ind].dispositions.insert({pl[p].id,0});
        their_opinion_of_me = 0;
    }

    if (pl[p2_ind].dispositions.find(subjectp_id) == pl[p2_ind].dispositions.end()) {
        pl[p2_ind].dispositions.insert({ subjectp_id,0 });
    }

    if (their_opinion_of_me >= 0) {
        int op = p;
        p = p2_ind;
        disp = (their_opinion_of_me/10)*(disp / 10);//if like the person, disposition towards 3rd party goes up/down by a fraction, greater change proportional to how much one likes the sharer
        change_disposition(subjectp_id, disp, "shared disposition");
        if ((pl[p2_ind].dispositions[subjectp_id] >= 0 && pl[op].dispositions[subjectp_id] >= 0)||(pl[p2_ind].dispositions[subjectp_id] <= 0 && pl[op].dispositions[subjectp_id] <= 0)) {
            change_disposition(pl[op].id, 1, "shared friend/enemy");
        }
        else {
            change_disposition(pl[op].id, -1, "different friend/enemy");
        }
        p = op;
    }
    else {
        int op = p;
        p = p2_ind;
        disp = -1*(disp / 10)* (their_opinion_of_me / 10);//if dislike the person, same as above but inverted
        change_disposition(subjectp_id, disp, "shared disposition");
        if ((pl[p2_ind].dispositions[subjectp_id] >= 0 && pl[op].dispositions[subjectp_id] >= 0) || (pl[p2_ind].dispositions[subjectp_id] <= 0 && pl[op].dispositions[subjectp_id] <= 0)) {
            change_disposition(pl[op].id, 1, "shared friend/enemy");
        }
        else {
            change_disposition(pl[op].id, -1, "different friend/enemy");
        }
        p = op;
    }
}

//DO THIS
bool People::give_tribute() {
    vector<int> superiors;
    int sum = 0;
    for (auto const& i : pl[p].submissive_to) {
        if (i.second.submissive_to) {
            superiors.push_back(i.first);
            sum += pl[p_by_id(i.first)].authority;
        }
    }
    int give_to = rand() % sum;
    sum = 0;
    int tribute_to = -1;
    for (int i : superiors) {
        sum += pl[p_by_id(i)].authority;
        if (sum > give_to) {
            tribute_to = i;
        }
    }
    //choose gift to give
        //food, tool, clothing, art, etc

    //go to give it
    if (move_to(pl[p_by_id(tribute_to)].pos,"to give tribute")) {
        //give tribute
        //dispositions affected
        return true;//done????
    }
    return false;//in progress???? how to signal don't run this function?
}

//DO THIS
bool People::rebel() {
    vector<int> rebel_targets;
    for (auto const& i : pl[p].submissive_to) {
        if (i.second.submissive_to) {
            if (pl[p_by_id(i.first)].authority <= pl[p].authority || pl[p_by_id(i.first)].num_fights_won<=pl[p].num_fights_won) {
                rebel_targets.push_back(i.first);
            }
        }
    }
    //DO THIS
    //trigger fight against one of the targets, preferrably the one with lowest authority and/or fights won
    return false;
}

//DO THIS
void People::build_monument() {
    if (!pl[p].monument_unlocked) {
        return;
    }
    //not sure yet how to implement this. If built, win game. If choose to continue playing, then grants huge authority boost
    //temp implementation
    create_item("monument", { -1,-1 });
    drop_item(inventory_has("monument")[0]);
    cout << pl[p].id << " HAS WON THE GAME\n";
}
//need to implement a chance of no longer submitting to someone if their authority level falls too low or below one's own
//need to either figure out a way to handle order execution priority between getting food and removing campsite or create a function called remove_campsite to encapsulate its code and call before attempting to get food

//note: need to make parent check on kid periodically, and give priority to giving food to kids rather than others. fix this //feed hungriest first? Unsure if this should be implemented
//note: spoken messages might need their own priority tree separate from the movement related actions

//note: DO THIS add a small limit to inventory size to encourage the use of bags, carts/travois, multiple trips, etc. Makes more interesting logistics and behavior and the dropping of items or use of storage containers creates a nicer aesthetic

//disposition should be affected by or affect:
//item requests. don't request from disliked people. If I have authority or they submit to me, chance of requesting items even if I don't need them. Need to add requests for other actions such as to marry, to marry someone's daughter or to marry my kids to theirs, to start a fight with an enemy, to move campsite somewhere else (near mine or somewhere specific), etc
//chatting. likelihood of chatting with someone
//marriage. if married and whether to marry
//family. like children, parents and siblings more?
//movement. keep larger distance from enemies, closer to liked people
//fight. if fought on the same side as someone or opposite sides, and if made to submit to someone
//need more social activities, maybe song, dance, ritual. Cooperative actions such as group hunts, group gathers, building, crafting, etc. Smoking, etc.
//need more sources of drama such as infidelity, jealousy (of wealth measured in average hunger level, items, kids, being liked, etc), emotions such as sadness/grief if someone dies, theft, specific revenge, asking for forgiveness, asking for 3rd party help in resolving a problem, etc.

/* TO DO:
* fix bugs
* add disposition effects - later
* finish rebel - later
* handle picking up traps or moving them - later
* handle better for carrying and dropping infants - later
* 
* then implement player and do a test run to see if can reach victory condition
* then polish graphics and animations
* then add more variety both visual, item, misc small features/mechanics, etc
* then add sounds/music, menu, tutorial, etc
*/

//if average family is kept at 2 parents, 2 grandparents of male, and 2 kids/infants, then family is size 6, then soft cap total pop size at around 60 people or 10 families

//need to conduct unit tests on all functions to ensure they work properly and for all edge cases

//need to write out in english list of all conditionals, scenarios and behaviors and then check that the code actually does these. 
//write out exactly what each function does in english to see if it is actually a desirable behavior

//Compare current version of my game to rimworld and list all differences so as to reach action/item parity where it makes sense. 
//Where an item or action does not make sense to add to my game, then add something else such that there is numerical parity of features at least. 
//Do the same with other similar games to ensure my quality is worth an equal pricing of game.