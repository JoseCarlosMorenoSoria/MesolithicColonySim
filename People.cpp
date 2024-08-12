#include "People.hpp"
using namespace std;

vector<People::Person> People::pl; //pl, using the name pl because of the frequency of use, used to store all Person instances
int People::p = -1;//index for accessing current person. Using index to access instead of a pointer because list may change such as when a new person is born or dies which invalidates pointers to pl (people_list)
int People::people_id_iterator = 0;
vector<int> People::people_in_stealth;//unsure if need a separate people and animal list of these, also need to implement both animal and people not seeing or reduced seeing probability of those in stealth
//Need to unit test every function and also test the frequency that each function executes, average value for all variables, and different scenarios (resource/people density, size, environment, etc)

//Currently, an npc wins the game on average in 750 ticks, nearly 8 days. Might be less such as 600 or more such as 950. Only ran 3 tests to check tick amount.

//note: readability might be increased by creating null constants for things like int (-1) and Position (-1,-1), etc.
//could also maybe just overload and use the operator! as a return true or false if null function


//FIX THIS: fix find_all according to changes in item, plants, etc


People::People(){}

People::People(int init) {
    
    Person p1;
    p1.p_id = new_person_id();
    p1.pos = { 50,25 };
    p1.sex = true;
    p1.age = 11;
    pl.push_back(p1);
    envi.tile(p1.pos).person_id = p1.p_id;

    
    Person p2;
    p2.p_id = new_person_id();
    p2.pos = { 51,26 };
    p2.sex = false;
    p2.age = 11;
    pl.push_back(p2);
    envi.tile(p2.pos).person_id = p2.p_id;
    
   
    
}

int ticks = 0;
void People::update_all(int day_count, int hour_count, int hours_in_day) {
    a_p_flip = false;
    //if (game_won) {//for debugging, checking if game is winnable
    //    cout << "Number of ticks: " << ticks << "\n";
    //    return;
   // }

    //ticks++;

    pday_count = day_count;
    phour_count = hour_count;

    //updates these constants if zoom level changes
    //WALK_SPEED = TILE_PIXEL_SIZE / 4;
    //STEALTH_SPEED = TILE_PIXEL_SIZE / 8;
    //SPRINT_SPEED = TILE_PIXEL_SIZE / 2;

    people_in_stealth.clear();//clears every tick for simpler implementation. This needs to be rewritten in a better method instead of using Environment to communicate with Animal

    int top_auth = 0;
    for(int i = 1; i < pl.size(); i++) {//i starts at 1 because for now, the first Person in the list is reserved for player control
        p = i;
        ox = pl[p].pos.x;
        oy = pl[p].pos.y;
        update(day_count, hour_count, hours_in_day);
        if (pl[p].in_stealth) {
            people_in_stealth.push_back(pl[p].p_id);
        }
        if (pl[p].authority > top_auth) {
            top_auth = pl[p].authority;
        }
    }
    //cout << top_auth << "\n";
    
    //for debugging
    if (day_count == -1) {//kill all people. set to -1 to prevent execution
        for (Person& pip : pl) {
            pip.is_alive = false;
        }
    }
    /*
    if (day_count % 5 == 0 && hour_count==0) {
        for (int i = 0; i < pl.size(); i++) {
            cout << "id: " << pl[i].id << " -- \n";
            for (auto const& d : pl[i].dispositions) {
                cout << "pers: " << d.first << " disp:" << d.second << "\n";
            }
            cout << "----------------------------\n\n\n";
        }
    }
    */
}

void People::update(int day_count, int hour_count, int hours_in_day) {
    if (check_death()) {
        return;
    }

    if (pl[p].speed_type == "walking") {//adjusts in case tile size changed
        pl[p].speed = con.WALK_SPEED;
    }
    else if (pl[p].speed_type == "sprinting") {
        pl[p].speed = con.SPRINT_SPEED;
    }
    else if (pl[p].speed_type == "stealth") {
        pl[p].speed = con.STEALTH_SPEED;
    }


	if (hour_count == 0) { //once a day check
		if (pl[p].hunger_level > con.HUNGRY_LEVEL) { //tracks for continuous hungry days at the start of every day
			pl[p].hungry_time++;
		}
		else {
			pl[p].hungry_time = 0;
		}
        pl[p].age++;
    }
    if (pl[p].age < con.MAX_INFANT_AGE) {//is infant. Currently that means it doesn't do anything except get hungry and needs to be fed
        pl[p].current_image = "human_infant";
        pl[p].hunger_level++;
        if (pl[p].hunger_level > con.HUNGRY_LEVEL) {
            speak("requesting food",-1);
        }
        if (pl[p].being_carried) {//if being carried, then position is the position of the carrier offset by 1
            pl[p].pos = person(pl[p].carried_by_id).pos;
            pl[p].pos.x += 1;
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
            ItemSys::Apparel* ap = static_cast<ItemSys::Apparel*>(it2.item_list[i.second]);
            insulation_cold += ap->insulation_cold;//FIX THIS: Need to ignore case where equipped item is not apparel
        }
    }
    if (envi.tile(pl[p].pos).temperature > pl[p].my_temperature) {//like other needs, having this update every tick is not ideal and should be changed.
        pl[p].my_temperature++;//fix this, need to make increase/decrease a percent of the difference between my_temp and tile_temp. Such that a large difference causes a large change in my_temp but a small one does not. Do the same for the clothing insulation below.
        
    }
    else if (envi.tile(pl[p].pos).temperature < pl[p].my_temperature) {
        if (insulation_cold == 0) {
            pl[p].my_temperature--; //for now, insulation is essentially a bool that prevents getting colder than already are. Need to implement a more thought out system of person temp change and insulation. Fix.
        }
    }
    



    if (pl[p].clean_image) {
        pl[p].current_image = "pics/human.png";
        pl[p].clean_image = false;
    }

    find_all();//gets all items, people, etc from within sight/earshot to then react to or inform next action

    utility_function();
    if (!pl[p].move_already && pl[p].general_search_called) {
        general_search_walk("");//ensures this function only executes once per update and also only after all other move_to's have been considered. This prioritizes intentional movement (moving to a target) rather than random movement
    }

    pl[p].found_messages.clear();
    pl[p].search_results.clear();

    //check to ensure that spouses share campsite
    if (!pl[p].sex && !pl[p].adopt_spouse_campsite && pl[p].spouse_id != -1) {
        pl[p].campsite_pos = person(pl[p].spouse_id).campsite_pos;
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

void People::utility_function() {//is currently actually just a behavior tree not a utility function. Selects what action to take this tick.
    //this implementation allows functions to be interrupted by higher priority ones on every update, however this means that a function may not properly reset or preserve as needed for when it gets called again later, need to fix
    //if(func()==false) go to next func(), if(func()==true) executed this func() for both in progress and done cases
    if (pl[p].mov) {
        move_to(pl[p].dest, "continue");
        return;
    }
    /*
    Every goal has to have a method to satisfy alone or cooperatively or worrying about another person having their goals satisfied such as children.
    safety - need to add need and function, related to fight function
    social - other than seeking social status and mood hits from loneliness, this might be unnecessary given other functions, related to chat and idle.socialize and reproduce and child_birth and need to feed infants
    social status - need to add need and function, do this last as it's more complex. Isn't this also a bit redundant with social need?
    health - improve this, need triggers for illness and injury
    */

    //initializing function locks so that if a function starts execution, it is allowed to run for at least a few ticks before getting preempted
    
    

    if (pl[p].animal_id_ambush != -1) {
        Animal::animal& a = Animal::al[a_by_id(pl[p].animal_id_ambush)];
        if (Position::distance({a.pos.x, a.pos.y},pl[p].pos) > pl[p].sightline_radius) {//if lost sight of prey being ambushed, return to regular speed and no stealth
            pl[p].animal_id_ambush = -1;
            pl[p].speed = con.WALK_SPEED;
            pl[p].speed_type = "walking";
            pl[p].in_stealth = false;
        }
        else {
            hunting(a.species);//continue to hunt this type of animal. Need to change to specifically this target, also need to allow interruption by high priority functions such as being attacked/injured/etc
            return;
        }
    }
    
    if (pl[p].dumping_not_done) {
        inventory_dump();
        return;
    }

    if (pl[p].progress_states.find("utility preemption protector") == pl[p].progress_states.end()) {
        pl[p].progress_states.insert({ "utility preemption protector",{15} });//protects a function for at most 5 ticks
    }
    
    chat();//chance to chat every update
    
    if (pl[p].protected_func!=-1) {
        if (pl[p].progress_states["utility preemption protector"].progress_func()) {//not sure if it actually helped
            pl[p].protected_func = -1;//release
        }
    }
    if (need_light() || pl[p].protected_func == 0) { pl[p].protected_func = 0; }
    else if (child_birth() || pl[p].protected_func == 1) { pl[p].protected_func = 1; }//pregnancy advancement should be moved to update function, not child_birth()
    else if (fight() || pl[p].protected_func == 2) { pl[p].protected_func = 2; }
    else if (health() || pl[p].protected_func == 3) { pl[p].protected_func = 3; }
    else if (exposure() || pl[p].protected_func == 4) { pl[p].protected_func == 4; }
    else if (sleeping() || pl[p].protected_func == 5) { pl[p].protected_func = 5; }//need to move collapsing from sleep to update function instead of sleeping()
    else if (drinking() || pl[p].protected_func == 6) { pl[p].protected_func = 6; }
    else if (eating() || pl[p].protected_func == 7) { pl[p].protected_func = 7; }//if don't have food, searches for food. Therefore the structure of utility_function is focused on which needs to satsify first (sleep, hunger, campsite, reproduction, etc)
    else if (search_for_new_campsite() || pl[p].protected_func == 8) { pl[p].protected_func = 8; }
    //Commented out until carry infants is fixed due to changes in Renderer
    else if (reproduce() || pl[p].protected_func == 9) { pl[p].protected_func =9; } //avoid execution of this function to focus on other features without worrying about population size
    else if (answer_item_request() || pl[p].protected_func == 10) { pl[p].protected_func = 10; }
    else if (hygiene() || pl[p].protected_func == 11) { pl[p].protected_func = 11; }
    else if (recreation() || pl[p].protected_func == 12) { pl[p].protected_func = 12; }
    else if (beauty() || pl[p].protected_func == 13) { pl[p].protected_func = 13; }
    else {idle(); }
    //DO THIS: (this (authority pursuit AI) might be too complex for this version, maybe organic leaders is better and add behavior that makes it likelier for some npcs to get to victory condition?) need to add authority as a need/goal to be pursued. Which means starting and winning fights with new people to increase number of submissives, and gaining favor with more people and increasing favor with existing friends/allies
}

bool People::reproduce() {//later, add marriage ceremony/customs, options for polygamy, infidelity, premarital sex, widow status, age and family restrictions on potential mates, family size limits, divorce, etc
    if (!(pl[p].reproduction_cooldown > con.REPRODUCTION_TRIGGER)) {//function trigger
        return false;
    }
    vector<Position>& pos_list1 = pl[p].search_results["people"];//note: using reference (&) reduces copying
    Person* p2=nullptr;
    bool mate_found = false;
    for (int i = 0; i < pos_list1.size(); i++) {//filter out valid mates from people found list
        int pers_id = envi.tile(pos_list1[i]).person_id;
        if (pers_id == -2) {
            cout << "error: pid==-2\n";//don't know why this is happening, already checked find_all() but it's the only place it could be inserted
            return true;//try again
        }
        p2 = &person(pers_id);
        if (p2->sex != pl[p].sex && p2->age > con.MIN_ADULT_AGE && p2->is_alive) {
            bool is_my_child = false;
            for (int i = 0; i < pl[p].children_id.size(); i++) {
                if (pl[p].children_id[i] == p2->p_id) {
                    is_my_child = true;
                    break;
                }
            }
            if (!is_my_child && ((p2->spouse_id == -1 && pl[p].spouse_id == -1) || (p2->spouse_id == pl[p].p_id && pl[p].spouse_id == p2->p_id))) {//if not my child AND both unmarried or if married to each other
                mate_found = true;//mate found
                break;
            }
        }
    }
    bool mate_willing = false;
    if (mate_found && p2!=nullptr) {
        if (p2->reproduction_cooldown > con.REPRODUCTION_TRIGGER && p2->sex != pl[p].sex) {
            mate_willing = true;
        }
        if (mate_willing && (Position::distance(pl[p].pos, p2->pos) == 1 || move_to(p2->pos, "going to mate"))) {//go to tile adjacent to p2
            //create a new human, add pregnancy later, only female creates child
            if (!pl[p].sex) {//if female
                pl[p].pregnancy.progress = 1;//am now pregnant
                pl[p].reproduction_cooldown = 0;//reset
                p2->reproduction_cooldown = 0;//unsure if this is the best way to handle interaction between 2 people, speaking or some other function might be better to avoid 2 people not being in sync
                if (pl[p].spouse_id == -1 && p2->spouse_id == -1) {//if don't have spouse, set as spouse
                    pl[p].spouse_id = p2->p_id;
                    p2->spouse_id = pl[p].p_id;
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

//fix this, this function has magic numbers but they are random percents, need to decide how to handle them
bool People::idle() {
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
            else if (pl[p].current_image == "pics/human_idle.png") {
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
            }//Add later: one way to determine who to socialize with and who to avoid is to when executing social/avoid function, create a map and assign each tile a desirability score, such that each person known gives the tile they're on a positive/negative value with a small radius they affect, then the person chooses the highest value tile nearest to them.
            if (p2id != -1) {
                if (move_to(person(p2id).pos, "to socialize - idle")) {//to search out liked people to encourage clique congregation and disposition sharing
                    pl[p].current_image = "human_socializing";
                    if (pl[p].progress_states.find("socialize") != pl[p].progress_states.end()) {
                        if (pl[p].progress_states["socialize"].progress_func()) {
                            pl[p].clean_image = true;
                        }
                    }
                    else {
                        progress_state state = { 5 };
                        pl[p].progress_states.insert({ "socialize", state });
                    }
                }
            }
            else {
                have_no_friends = true;//causes execution of other idle option
                option = 51;
            }
        }
    } while (have_no_friends);
    return true;
}

//Cooking is just a variation of crafting involving butchering, milling, mixing, brewing, boiling, etc. Smithing, tailoring, etc are also just variations on crafting.
bool People::craft(string product) {//later add station requirements such as campfire/stove/oven/furnace
    //if inventory has product.ingredients then craft product (consumes non tool ingredients) and place in inventory
    ItemSys::Item it = *it2.presets[product];
    vector<string> ingredients = it.ingredients;
    int num_of_ingredients = ingredients.size();
    vector<string> missing_ingredients;//fix this, this should be a single string given that every update rechecks if it can craft the item and only 1 ingredient is sought per tick. Unless a more detailed method is implemented such as checking if any missing ingredient is craftable rather than doing them strictly in order.
    vector<int> temp;
    for (int i = 0; i < num_of_ingredients; i++) {
        temp = inventory_has(ingredients[i]);
        if (temp.empty()) {
            missing_ingredients.push_back(ingredients[i]);
        }
    }
    if (missing_ingredients.empty()) {//have all items, therefore craft product
        pl[p].current_image = "human_crafting";
        //pl[p].immobile = true;//prevents moving while crafting?? doesn't work?
        if (pl[p].crafting.find(product) == pl[p].crafting.end()) {//key not found
            pl[p].crafting.insert({ product,{4} });//4 is the number of ticks/frames crafting image/animation lasts
        }
        if (pl[p].crafting[product].progress_func()) {//animation/time delay    currently, progress is saved if interrupted which might not make sense in some contexts
            for (int i = 0; i < num_of_ingredients; i++) {//later implement tool degradation here as well
                ItemSys::Item it_c = *it2.presets[it.ingredients[i]];
                if (it_c.consumable_ingredient) {
                    int consume_index = inventory_has(it.ingredients[i])[0];
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
    int service = -1;
    for (int i = request_messages.size() - 1; i > -1; i--) {
        int sid = request_messages[i].sender_id;
        change_disposition(sid, 0, "check exists");
        if (pl[p].submissive_to.find(sid) == pl[p].submissive_to.end()) { pl[p].submissive_to.insert({ sid,{0,false} }); }//should have a first contact function to populate these lists instead of checking if exists every time the list is used. Fix this
        if (pl[p].dispositions[sid] < DISLIKE_THRESHOLD && !pl[p].submissive_to[sid].submissive_to) {//don't answer requests from disliked people unless you are submissive to them
            request_messages.erase(request_messages.begin() + i);
            items_requested.erase(items_requested.begin() + i);
        }
        if (pl[p].dispositions[sid] > LOVED_THRESHOLD || pl[p].submissive_to[sid].submissive_to) {
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
    pl[p].current_image = "pics/human_giving_food.png";
    speak("answering request for " + target, receiver_id);
    //move to requester's position, adjacent
    Person& p2 = person(receiver_id);
    if (Position::distance(pl[p].pos, p2.pos) == 1 || move_to(p2.pos, "answering request")) {
        //once reached, place requested item in their inventory
        int index = inventory_has(target)[0];
        p2.item_inventory.push_back(pl[p].item_inventory[index]); //give item id from inventory
        pl[p].item_inventory.erase(pl[p].item_inventory.begin() + index);//remove from own inventory
        pl[p].clean_image = true;
        int op = p;//temp reassign main p
        p = p_by_id(receiver_id);
        change_disposition(pl[op].p_id, 10, "recieved item requested");
        p = op;
        return true;//done
    }
    return true;//in process
}


//FIX THIS      //need to cache items being actively sought somehow
bool People::acquire(string target) {//target_type: animal/plant/pickup/adjaceny/craftable/person/information/permission
//First, determine the type of target to be acquired
    string target_type;
    string source_type;
    //if item can be picked up: target_type = "pickup"
    //else if item can be carried = "carry"
    //else if item is a source = "source" / "adjacency"     (terrain/plant/animal)
    //else if item is a station = "station" / "adjacency"   (campfire)
    //else if item is a building = "building"
        
    //check if target is an item name or item tag
    set<string> item_categories = { "weapon","apparel","container","material" };//structure is handled differently, tool and misc not included, food needs to be included
    ItemSys::Item it = *it2.presets[target];
    if (it.item_name!="") {//target is an item name
        if (it.item_type=="structure") {
            target_type = "building";
        }
        else if (!it.can_pick_up) {
            target_type = "carry";
        }
        else {
            target_type = "pickup";
        }
    }
    else if (species.find(target) != species.end()) {//target is an animal name
        if (hunting(target)) {//call hunting function
            return true;//done
        }
        return false;//in progress
    }
    else if (Plants::species_presets.find(target) != Plants::species_presets.end()) {//target is a plant name
        target_type = "source";
        source_type = "plant";
    }
    /*
    else if (it2.tags.find(target) != it2.tags.end()) {//target is tag name
        for (string item_name : it2.tags[target]) {//for every item with this tag, attempt to acquire item, if one is acquired then tag is acquired therefore return true
            if (acquire(item_name)) {
                return true;//done
            }
        }
        return false;//in progress
    }
    */
    else if (item_categories.find(target) != item_categories.end()) {//target general item category name
        if (target == "weapon") {
            for (auto w : ItemSys::weapon_presets) {//for every item in this category, attempt to acquire item, if one is acquired then category is acquired therefore return true
                if (acquire(w.first)) {
                    return true;//done
                }
            }
            return false;//in progress
        }
        else if (target == "apparel") {
            for (auto w : ItemSys::apparel_presets) {//for every item in this category, attempt to acquire item, if one is acquired then category is acquired therefore return true
                if (acquire(w.first)) {
                    return true;//done
                }
            }
            return false;//in progress
        }
        else if (target == "container") {
            for (auto w : ItemSys::container_presets) {//for every item in this category, attempt to acquire item, if one is acquired then category is acquired therefore return true
                if (acquire(w.first)) {
                    return true;//done
                }
            }
            return false;//in progress
        }
        else if (target == "material") {
            for (auto w : ItemSys::material_presets) {//for every item in this category, attempt to acquire item, if one is acquired then category is acquired therefore return true
                if (acquire(w.first)) {
                    return true;//done
                }
            }
            return false;//in progress
        }
        
    }
    else if (target == "animal") {
        for (auto w : species) {//for every item with this tag, attempt to acquire item, if one is acquired then tag is acquired therefore return true
            if (acquire(w.first)) {
                return true;//done
            }
        }
        return false;//in progress
    }
    else if (target == "plant") {
        for (auto w : Plants::species_presets) {//for every item with this tag, attempt to acquire item, if one is acquired then tag is acquired therefore return true
            if (acquire(w.first)) {
                return true;//done
            }
        }
        return false;//in progress
    }
    else if (Environment::terrains.find(target) != Environment::terrains.end()) {//target is a terrain type
        target_type = "source";
        source_type = "terrain";
    }
    else {
        throw invalid_argument{"target is invalid"};
        return false;
    }

//Second, determine if already have the target or if it is nearby
    
    if (target_type == "pickup") {//an item that can be picked up and placed in one's inventory
        if (!inventory_has(target).empty()) {//this only applies to items that can be picked up, not those that can only be carried or cannot be moved.
            return true;
        }
    }

    //if item has prereqs for acquiring, such as if item is a source (tree) and requires an axe to obtain, then fulfill prereq first (acquire(axe)) before continuing. Same for animals/plants, acquire spear/sickle

    //look around self for item
    //if found, move to item (or adjacent according to target_type) (or hunting action if living animal)

    if (pl[p].search_results.find(target) != pl[p].search_results.end()) {//key found, if key exists then at least 1 was found
        Position pos = pl[p].search_results[target][0];
        int item_id = envi.tile(pos).item_id;
        if (Position::distance(pl[p].pos, pos)==1 || move_to(pos, "to found item " + target)) {//if item is found, move to it and pick it up
            if (target_type == "pickup" && move_to(pos, "to found item " + target)) {
                pick_up_item(item_id, pos);
                return true;//target acquired
            }
            else {
                return false;//still moving to dest
            }
            
            if (target_type == "carry") {
                //carry function
                return true;//target acquired
            }
            else if(target_type=="source") {
                adjacency_acquire_handler(target, source_type, pos);//extract/deconstruct
                return true;//target acquired
            }
            else if (target_type == "building") {
                //enter building function (find entrance and walk inside)???
                return true;//target acquired
            }
        }
        return false;//if still moving towards item, continue to next tick
    }

    //if not found in immediate vicinity, then according to target_type:

    //hunting and well/tuber digging have special actions if the target isn't found (tracking,setting traps,digging) so need to execute those if target not found


    //if item is craftable, craft it but if in the process of crafting, the item is found, abort crafting the item
    if (it.ingredients.empty()) {//if has ingredients, then is craftable
        if (craft(target)) {
            return true;//crafting item was successful
        }
    }

    //if item is buildable, build it

    //else if item has a source (wood comes from trees, rock comes from stone terrain, water is from water terrain, bones from animals, etc
    //then acquire source
    if (it.item_type == "material") {
        ItemSys::Material m = ItemSys::material_presets[it.item_name];
        acquire(m.source);
    }

    //if item not found, and people nearby, request item
    if (pl[p].search_results.find("people") != pl[p].search_results.end()) {

        bool request_answered = false;
        int answerer_id = -1;
        if (!pl[p].found_messages.empty()) {
            for (int m_id : pl[p].found_messages) {
                Message& m = message_list[message_by_id(m_id)];
                if (m.reciever_id == pl[p].p_id && m.messsage == "answering request for " + target) {
                    request_answered = true;
                    answerer_id = m.sender_id;
                    break;
                }
            }
        }
        //if request answered, stop requesting and move toward answerer
        if (request_answered) {//Due to sequence ordering issues of NPC updates, need to remember message for a bit to avoid missing messages from NPCs that update after self.
            Position pos = person(answerer_id).pos;
            if (Position::distance(pos, pl[p].pos) == 1 || move_to(pos, "recieving item")) {//move to adjacent to answerer
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

//continue improving this
bool People::health() {
    if (!pl[p].am_injured && !pl[p].am_sick) {//sickness and injury need triggers
        return false;
    }

    //if sick, get medicine and rest/sleep
    if (pl[p].sick_time > 100) {//very sick, go to bed and be bedridden
        pl[p].tired_level = con.SLEEP_TRIGGER;
        sleeping();
        return true;//bedridden or going to bed
    }
    else if (pl[p].sick_time > 20) {//slightly sick, get medicine
        if (!acquire("medicine")) {
            return true;//getting medicine
        }
        else {
            //if have medicine, heal self and consume medicine
            delete_item(inventory_has("medicine")[0], { -1,-1 }, 0);
            pl[p].am_sick = false;
            pl[p].sick_time = 0;
        }
    }

    //if injured, get bandage and medicine and rest/sleep
    if (pl[p].injured_time > 100) {//very injured, go to bed and be bedridden
        pl[p].tired_level = con.SLEEP_TRIGGER;
        sleeping();
        return true;//bedridden or going to bed
    }
    else if (pl[p].injured_time > 20) {//slightly injured, get bandage
        if (!acquire("bandage")) {
            return true;//getting bandage
        }
        else {
            //if have bandage, heal self and consume bandage
            delete_item(inventory_has("bandage")[0], { -1,-1 }, 0);
            pl[p].am_injured = false;
            pl[p].injured_time = 0;
        }
    }
    //if can't help self, request help

    //need triggers for illness and injury. Both contagious and not. Each symptom or illness should have a different medicine.
}

//DO THIS later
bool People::need_light() {
    return false;
    //if too dark, either move to light or create or get light source such as candle/torch/campfire
}

bool People::exposure() {//protection from heat/sun and cold/wind/rain. Only temperature protection for now.
    if (pl[p].my_temperature == pl[p].my_preffered_temperature) {//fix this, needs to be a distance from ideal temp, not exactly ideal temp
        return false;//temperature is fine
    }
    bool better_temp_found = false;
    if (pl[p].my_temperature > pl[p].my_preffered_temperature) {//need to add impact on thirst by heat in update function
        //seek cold, am too hot
        for (auto const& i : pl[p].search_results) {
            if (stoi(i.first) < pl[p].my_temperature) {//the only keys that are numbers will be tile temperatures, so find the first tile that has a lower temperature and go there
                move_to(i.second[0], "towards cold");
                better_temp_found = true;
                return true;
            }
        }
        if (!better_temp_found) {
            //take off clothes //what about sunburn? also add natural tanning mechanic
            for (auto& i : pl[p].equipped.equipment) {
                int item_id = pl[p].equipped.unequip(i.first);//remove one item of clothing
                if (item_id != -1) {
                    continue;
                }
                else {
                    pl[p].item_inventory.push_back(item_id);//place unequipped item in inventory
                    return true;
                }
            }
            //implement these 3 later
            //seek darkness/shade
            //go in water
            //move less, be lazy
        }
        //if no method to improve temp, search
        pl[p].general_search_called = true;
    }
    else if (pl[p].my_temperature < pl[p].my_preffered_temperature) {
        //seek heat, am too cold
        for (auto const& i : pl[p].search_results) {
            if (stoi(i.first) > pl[p].my_temperature) {//inverse of "towards cold"
                move_to(i.second[0], "towards heat");
                better_temp_found = true;
            }
        }
        if (!better_temp_found) {
            //put on clothes and blankets
            if (!inventory_has("clothes").empty()) {
                //add one item of clothing. 
                for (auto const& i : pl[p].equipped.equipment) {
                    if (i.second == -1) {
                        if (!inventory_has(i.first).empty()) {//if inventory has item with a tag saying it can be worn in this slot
                            pl[p].equipped.equip(inventory_has(i.first)[0]);
                            return true;
                        }
                    }
                }
            }
            //start or find fire
            if (pl[p].search_results.find("campfire") == pl[p].search_results.end()) {//if there is a campfire, it will warm the area and so is handled by "move to heat"
                //if there is no campfire nearby
                if (!acquire("campfire")) {
                    pl[p].general_search_called= true;//searching for fire
                }
                else {
                    drop_item(inventory_has("campfire")[0]);//place campfire
                }
            }

            //implement these 2 later
            //seek light
            //move more, be active
        }
        //if no method to improve temp, search
        pl[p].general_search_called = true;
    }
    //if am wet or in rain or in strong wind, seek shelter. Need to implement these. Fix this
    return false;//?
}

bool People::hygiene() {
    if (pl[p].dirtiness < HYGIENE_TRIGGER) {
        return false;
    }
    if (pl[p].search_results.find("water") != pl[p].search_results.end()) {//go to water source
        if (move_to(pl[p].search_results["water"][0], "to water")) {//run bathing animation
            pl[p].current_image = "bathing";
            if (pl[p].bathing.progress_func()) {
                pl[p].dirtiness = 0;
                pl[p].clean_image = true;
                return true;//done
            }
        }
    }
    else {
        pl[p].general_search_called = true;
    }
    return true;//in progress
}

bool People::beauty() {
    if (pl[p].beauty_need_satisfied) {
        return false;
    }
    //decorate self and surroundings

    //FIX THIS: need to implement decorative items and bring them to camp or jewelry and craft it

    //temporary implementation, like recreation. Both of these functions are more about polish and content rather than mechanics so do last
    if (acquire("necklace")) {
        pl[p].beauty_need_satisfied = true;
    }
}

//games/dance/stories/song/instruments/sports/competitions/socializing/exploring/swimming/painting rock walls
bool People::recreation() {
    if (!(pl[p].recreation_level > RECREATION_TRIGGER)) {//unsure if this needs a trigger or simply replace idle? and have negative mood if not fulfilled?
        return false;
    }
    //need to add method of choosing what to do to recreate, should desire variety as well
    string option = "play trumpet";
    if (option == "play trumpet") {//need to first get instrument, fix this. Also need to create instrument item
        if (acquire("trumpet")) {
            pl[p].current_image = "playing trumpet";
            pl[p].recreation_level -= 50;//each recreation type should have its own reduction value?
            pl[p].clean_image = true;
        }
    }
}

//need to simplify this function.
bool People::search_for_new_campsite(){ //need to bias search direction in the direction of wherever there is more food rather than waiting to randomly stumble on a site with enough food for campsite. Also need to add a system of not searching the same tile within too short a time frame.
    if (!pl[p].sex && pl[p].spouse_id!=-1 && pl[p].campsite_pos == person(pl[p].spouse_id).campsite_pos) {
        return false;//prevent searching for a new campsite if married, only for females
    }
    
    bool cond2 = pl[p].campsite_pos.x == -1 || pl[p].hungry_time >= con.DAYS_HUNGRY_MOVE_CAMP;//AND: have no campsite OR have been hungry too long
    bool cond3 = pl[p].campsite_age > con.NEW_CAMP_PROBATION_TIME || pl[p].campsite_age==-1;//AND: campsite is old enough to move again. Unsure if this might have an issue if the null campsite has an age
    bool start = (cond2 && cond3) || pl[p].adopt_spouse_campsite;
    //currently only creates a campsite after having been hungry 3 days. Still need to figure out when and when not to create a campsite, such as for trips away from home or extreme high mobility nomad
    //function trigger
    if (!start){
        return false;
    }


    if (pl[p].campsite_pos.x != -1) { //if have campsite, remove. //Later add an option to just abandon a campsite without removing the house. Should only decontruct if going to carry it to new location such as a tent/sleeping bag/lean to/etc.
        if (move_to(pl[p].campsite_pos, "to campsite")) {//walk to campsite to remove
            int item_id = envi.tile(pl[p].campsite_pos).item_id;
            delete_item(item_id, pl[p].campsite_pos, -1);
            pl[p].campsite_pos = { -1,-1 };
            pl[p].campsite_age = -1;
            pl[p].friend_camp_check = true;
            if (pl[p].adopt_spouse_campsite) {
                pl[p].campsite_pos = person(pl[p].spouse_id).campsite_pos;
                pl[p].adopt_spouse_campsite = false;//reset for if spouse dies later
                return true;//in progress
            }
        }
        else {
            return true;//in progress
        }
    }
    //if have infants, carry them
    pickup_infants();

    vector<Position> food_pos_list = filter_search_results("food"); //gets results, assigns 1 result to food_pos
    Position food_pos = { -1,-1 };
    bool found_food = false;
    if (!food_pos_list.empty()) {
        food_pos = food_pos_list[0];
        found_food = true;
    }

    if (pl[p].friend_camp_check) {//encourages campsite congregation between people who like each other (forgot to check if other person likes self, fix this)
        for (auto const& i : pl[p].dispositions) {
            if (i.second > LOVED_THRESHOLD) {
                if (person(i.first).campsite_pos.x != -1) {//if have a friend with a campsite and am finding a new campsite, move to within 10 tiles of friend to search for campsite location there
                    if (Position::distance(pl[p].pos, person(i.first).campsite_pos) < NEW_CAMP_CLOSE_TO_FRIEND || move_to(person(i.first).campsite_pos, "to friend's campsite")) {
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
        pl[p].campsite_age = 0; //resets campsite age
        //place tent
        vector<Position> pos_list = pl[p].search_results["no item"];
        if (!pos_list.empty()) {
            create_item("tent", pos_list[0]);//create and place tent
            pl[p].campsite_pos = pos_list[0]; //store campsite location
            
            //if have infants, drop them
            if (!drop_infants()) {
                return true;//in progress
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

//NOTE: for implementing cooperation, conduct through speak() requests and answers. Person 1 proposes joint action, Person 2 decides whether to agree or not. If a 3rd person or more are involved, then need to set a meeting location and time to conduct the proposition -> up/down vote and an option to continue action with those who said yes only. Later add option to be able to coerce those who said no into complying.

//same magic number issue of percent chances, fix this
void People::chat() {//if make this a speak() action, can affect dispositions of more than 1 person with 1 comment, fix this?
    if (pl[p].search_results.find("people") != pl[p].search_results.end()) {
    int topic = rand() % 100;
    int p2_ind = p_by_id(envi.tile(pl[p].search_results["people"][0]).person_id);
    int p2_id = pl[p2_ind].p_id;
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
            change_disposition(pl[op].p_id, comment, valence);//positive is compliment, negative is insult
            p = op;

        }
        else {
            share_disposition(p2_ind);
        }
    }
}

//has a magic number but unsure if it needs to be pulled out as it's only relevant here and might not need to be balanced against other numbers.
void People::authority_calc() {
    int num_people_liked_by = 0; 
    int amount_liked = 0;
    int num_submissives = 0;
    bool am_sovereign = true;
    for (int i = 0; i < pl.size(); i++) {
        if (pl[i].dispositions.find(pl[p].p_id) != pl[i].dispositions.end()) {
            if (pl[i].dispositions[pl[p].p_id] > 0) {
                num_people_liked_by++;
                amount_liked += pl[i].dispositions[pl[p].p_id];
            }
        }
        if (pl[i].submissive_to.find(pl[p].p_id) != pl[i].submissive_to.end()) {
            if (pl[i].submissive_to[pl[p].p_id].submissive_to) {
                num_submissives++;
            }
        }
    }
    for (auto const& i : pl[p].submissive_to) {//if one is submissive to anyone, one is not sovereign. Fix this. DO THIS: Need to implement a rebellion function so that when one's authority increases a certain amount or relative to a dominator, can cast off submissive status and become sovereign
        if (i.second.submissive_to) {
            am_sovereign = false;
        }
    }
    
    bool corrected1 = false;
    bool corrected2 = false;
    bool corrected3 = false;
    bool corrected4 = false;
    if (pl[p].num_fights_won == 0) { pl[p].num_fights_won = 1; corrected1 = true; }
    if (num_people_liked_by == 0) { num_people_liked_by=1; corrected2 = true;}//to avoid multiplying by 0
    if (amount_liked == 0) { amount_liked = 1; corrected3 = true;}
    if (num_submissives == 0) { num_submissives = 1; corrected4 = true;}

    pl[p].authority = num_people_liked_by * amount_liked * num_submissives * pl[p].num_fights_won;
    
    bool c = corrected1 || corrected2 || corrected3 || corrected4;
    if (c && pl[p].authority == 1) { pl[p].authority = 0; }//to correct for factors being initialized as 1, not sure if this is the best approach

    if (corrected1) { pl[p].num_fights_won = 0; }//to correct for these being initialized as 1
    if (corrected2) { num_people_liked_by = 0; }
    if (corrected3) { amount_liked = 0; }
    if (corrected4) { num_submissives = 0; }
    pl[p].num_people_liked_by = num_people_liked_by;
    pl[p].amount_liked = amount_liked;
    pl[p].num_submissives = num_submissives;
    
    //10000 = 10*10*10*10 = 1*100*1*1  <-- this may be a problem, rethink formula
    if (pl[p].authority > 10000 && am_sovereign) {
        pl[p].monument_unlocked = true;
        game_won = true;
    }
    else {
        pl[p].monument_unlocked = false;//access to monument building can be lost if authority is lost
    }
}

//note: maybe a mechanic to desire to be liked, such that if someone dislikes one, one tries to improve the other's opinion of onself, would help prevent excessive social isolation on a population level
//but this should be either random or personality determined so that some seek out more or less to be liked. 

//DO THIS
bool People::give_tribute() {
    vector<int> superiors;
    int sum = 0;
    for (auto const& i : pl[p].submissive_to) {
        if (i.second.submissive_to) {
            superiors.push_back(i.first);
            sum += person(i.first).authority;
        }
    }
    int give_to = rand() % sum;
    sum = 0;
    int tribute_to = -1;
    for (int i : superiors) {
        sum += person(i).authority;
        if (sum > give_to) {
            tribute_to = i;
        }
    }
    //choose gift to give
        //food, tool, clothing, art, etc

    //go to give it
    if (move_to(person(tribute_to).pos,"to give tribute")) {
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
            if (person(i.first).authority <= pl[p].authority || person(i.first).num_fights_won<=pl[p].num_fights_won) {
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
    if (!pl[p].monument_unlocked) {//instead of a hard lock gated by a level, might be better to have this simply be prohibitively expensive and difficult to build unless the player has accumulated the resources and labor to do so and can spare the resources and labor to build it. This would create a more organic win condition that would vary game to game and player to player.
        return;
    }
    //not sure yet how to implement this. If built, win game. If choose to continue playing, then grants huge authority boost
    //temp implementation
    create_item("monument", { -1,-1 });
    drop_item(inventory_has("monument")[0]);
    //cout << pl[p].p_id << " HAS WON THE GAME\n";
}

//this needs to trigger the attempt to acquire storage containers and inventory expanders (baskets and bags)
bool People::inventory_dump() {//later, inventory capacity should also consider if the items are tiny/small/medium/large/huge. Huge items should only be able to be pushed/pulled/carried and may require multiple people or draft animals
    if (pl[p].item_inventory.size() < 6 || pl[p].campsite_pos.x==-1) {
        return false;//don't dump
    }//later, if don't have campsite, then need an alternative to either store items (stockpile near self/choose what to keep and discard/bury or hide items/trade for smaller, lighter items of equal value)
    //if inventory is full and have campsite, go to campsite and dump items
    if (!pl[p].dumping_not_done && move_to(pl[p].campsite_pos, "to camp to dump inventory")) {
        pl[p].dumping_not_done = true;
    }
    if (pl[p].dumping_not_done) {
        for (int i : pl[p].item_inventory) {
            if (!drop_item(i)) {
                pl[p].general_search_called = true;//no empty spot found, search for empty tile
                return false;
            }
        }
        pl[p].dumping_not_done = false;
    }
}




//new funcs



bool People::extinguish_fire() {
    //Firefighting: NPCs should stomp or hit with blankets on burning items, use buckets to throw soil/water on fire, create firebreaks, flee from fire or bug out (collect family and belongings before fleeing) either far from fire or into water
    
    //identify fire (from search_results)
    //identify if fire is uncontrolled (is not campfire/hearth/candle/torch/bonfire)
    //identify size of fire (look at surrounding tiles)
    //if it is 1 tile, attempt to stomp out, if it is 2 tile radius, hit it with blankets/clothing if have any, if 3 tile radius, if have containers, attempt to fill them with soil/sand or water and throw on tiles on fire, if 4 tile radius, do 3 tile response and also create fire breaks.
    //need to add cooperative action such as spreading out to extinguish fire and bucket chains for water/soil
    //if fire is 5 tile radius, flee. If fire is near home/belongings/family then rescue them before fleeing.
    return false;
}

bool People::carry() {//should also have a push/pull action for larger/heavier items such as a large stone block for a monument
    //receive whether it is an item or person
    //recieve its id and store it
    //set current image to "carrying" and the current image and state of target person if a person to "carried", tie the position of the carried item/person to the tile in front of the carrier but also offset into the tile of the carrier so the images overlap
    return false;
}

bool People::drop() {
    //if carrying something/someone, remove the pixel offset of the carried item/person and reset images of carrier and carried, remove id of carried from carrier and reset the state of carried to false
    return false;
}

Plants plant_ac;
//also serves mining stone, obtaining water and milking cows, picking berries off bushes instead of consuming whole bush
bool People::adjacency_acquire_handler(string target, string type, Position pos) {//for cutting down trees, mining rock, digging out dirt, collecting water, etc
    //accept target from acquire()
    //acquire should only call this func if person is next to target source

    //if tree, if have axe, chop tree animation
    if (type == "plant") {//for now, just converts plant into its components and stores components in inventory
        Plants::Plant& plant = plant_ac.pln[plant_ac.get_by_id(envi.tile(pos).plant_id)];//need to simplify all these methods of accessing an object, maybe by overloading or using a wrapper?
        for (string c : plant.current_components) {
            create_item(c, { -1,-1 });//create component items from plant and insert into inventory
        }
        plant_ac.pln.erase(plant_ac.pln.begin() + plant_ac.get_by_id(envi.tile(pos).plant_id));
        envi.tile(pos).plant_id = -1;//remove plant from map
        return true;//done
    }
    //if stone, if have pickaxe, mining animation
    if (type == "terrain") {
        create_item(target, { -1,-1 });//create item of the same name as terrain and insert in inventory
        return true;//done
    }
    //if it's a station passed as an ingredient/requisite to craft something, then "acquiring" the station counts as either being next to it or building it first
    //ex: campfire for cooking

    //Construction is a variation on crafting but done by emplacing something on a tile with the resources adjacent to that tile or self rather than in one's inventory.
    return false;
}



bool People::coerce() {
    //Coerce: intimidate, punish, threaten, etc a person into compliance with a request, is used for managing slaves, tributaries, subordinates, bullying, parenting, governance, etc. 
    return false;
}



//need to implement a chance of no longer submitting to someone if their authority level falls too low or below one's own
//need to either figure out a way to handle order execution priority between getting food and removing campsite or create a function called remove_campsite to encapsulate its code and call before attempting to get food

//note: need to make parent check on kid periodically, and give priority to giving food to kids rather than others. fix this //feed hungriest first? Unsure if this should be implemented
//note: spoken messages might need their own priority tree (or even .cpp file) separate from the movement related actions

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
* add disposition effects - later
* finish rebel - later
* handle picking up traps or moving them - later
* handle better for carrying and dropping infants - later
*/

//if average family is kept at 2 parents, 2 grandparents of male, and 2 kids/infants, then family is size 6, then soft cap total pop size at around 60 people or 10 families

//need to conduct unit tests on all functions to ensure they work properly and for all edge cases

//need to write out in english list of all conditionals, scenarios and behaviors and then check that the code actually does these. 
//write out exactly what each function does in english to see if it is actually a desirable behavior

//Compare current version of my game to rimworld and list all differences so as to reach action/item parity where it makes sense. 
//Where an item or action does not make sense to add to my game, then add something else such that there is numerical parity of features at least. 
//Do the same with other similar games to ensure my quality is worth an equal pricing of game.