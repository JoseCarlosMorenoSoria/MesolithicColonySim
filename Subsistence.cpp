#include "People.hpp"
#include "Animal.hpp"
using namespace std;
People peep1;//to access People functions in Animal functions
//Any main functions that regard obtaining and processing food and water for People

bool Animal::eating() {
    animal& c = (a_p_flip) ? al[a] : People::pl[People::p];//controls if accessing an animal from animal list or a person from people list (derived from Animal)
    Species& sp = species[c.species];
    if (c.hungry_time > sp.DAYS_HUNGRY_MOVE_CAMP && c.campsite_pos.x != 1 && c.campsite_age > sp.NEW_CAMP_PROBATION_TIME) {//allows search for campsite to trigger but not before new camp is at least a few ticks old
        return false;
    }

    bool hungry = c.hunger_level > sp.HUNGRY_LEVEL;
    bool has_food = false;
    vector<int> food_indexes1;
    if (c.species == "human") {
        food_indexes1 = peep1.inventory_has("ready food");
        if (!food_indexes1.empty()) {
            has_food = true;
            c.eating_food_index = food_indexes1[0];
        }
        if ((hungry && !has_food) /* || food_indexes1.size() < MIN_EXTRA_FOOD_IN_INVENTORY*/) {//ensures that person has 2 food items in inventory for self or to share
            if (acquire("ready food")) {
                return true;//done
            }
            else {
                return true;
            }
        }
    }//should these return sets instead? would remove the need for converting to sets when set operations are needed. Duplicate indexes are never relevant.
    else {
        if (hungry) {//unlike for humans, an animal acquires an item by being next to it, given it doesn't have an inventory
            if (acquire("food")) {//FIX THIS, Needs to take diet specific tag
                has_food = true;//continue
            }
            else {
                return true;//in progress
            }
        }
    }

    if (!(hungry && has_food)) {//function trigger
        return false;
    }
    if (c.eating_progress.progress == 0) {
        c.current_image = c.species+"_eating.png";
    }
    if (c.age < sp.MAX_INFANT_AGE) {
        c.current_image = c.species+"_infant";
    }
    if (c.eating_progress.progress_func()) {//makes eating take more than 1 frame
        int index = c.eating_food_index;
        if (c.species == "human") {
            int food_id = People::pl[People::p].item_inventory[index];
            peep1.delete_item(food_id, { -1,-1 }, index);//delete food from game
        }
        else {
            int food_id = envi.tile(c.food_to_eat).item_id;
            peep1.delete_item(food_id, c.food_to_eat, -1);//delete food from game
        }
        c.hunger_level -= sp.HUNGER_REDUCTION_RATE; //reduce hungry level by 10, therefore need 2 meals a day to stay at 0 hunger_level average
        c.clean_image = true; //when this function ends, return to default image on next update
        return true;//done eating
    }
    return true;//in progress
}

//continue fixing/improving this
bool People::hunting(string species) {//need to add a percentage success/fail chance to all hunting
    /*DO THIS NOW
        if animal is large then must either get help to carry home, camp next to animal to eat for days, or butcher it and bring home piece by piece
        pack/group hunting surrounding prey
    */

    string hunting_method = choose_hunting_method(species);

    if (hunting_method != "angling" && pl[p].search_results.find(species) == pl[p].search_results.end()) {
        if (tracking(species)) {
            if (hunting_method != "trap") {//this is because tracking() needs a chance to set traps if there are no animals, but trap also needs a chance to check traps if there are no animals
                return false;//tracking in progress
            }
        }
        else {
            pl[p].general_search_called = true;//search for target
            return false;//animal not found. This should be handled by tracking, need to check if this is redundant
        }
    }

    //FIX THIS. search_results should already be sorted, therefore move sort function to the end of find_all to sort all results
    vector<Position> pos_list = pl[p].search_results[species];
    if (!pos_list.empty()) {//second check due to angling not requiring having found the target animal
        sort(pos_list.begin(), pos_list.end());//need to sort vector before using or else will get stuck. unsure if sort by current position or by 0,0
        pl[p].search_results[species] = pos_list;
    }

    Animal::animal a_null;
    Animal::animal& a = (!pos_list.empty()) ? al[a_by_id(envi.tile(pl[p].search_results[species][0]).animal_id)] : a_null;//the assignment of the first animal in animal list is simply because &a can't be NULL



    ///////////////////////////////
    if (hunting_method == "persistence_and_pick_up") {
        return persistence_and_pick_up(species,a);
    }
    else if (hunting_method == "ambush") {
        return ambush(species,a);
    }
    else if (hunting_method == "angling") {
        return angling(species,a);
    }
    else if (hunting_method == "trap") {
        return trap("check", species,a);//either this or the previous check one is redundant
    }
    //this one is cooperative, so need to implement cooperation first
    // if (hunting_method == "cliff_run") {//chasing prey off cliffs or into pits to kill or catch

    // }
    
}

string People::choose_hunting_method(string species) {//later, create a more complicated version that takes into context available means, costs, etc. Not just species:hunting method 1:1 pairs
    if (species == "deer") {
        return "persistence_and_pick_up";
    }
    else if (species == "rabbit") {//need to determine when to use fishing net and when to angle. Maybe according to fish type?
        return "trap";

    }
    else if (species == "fish") {
        return "angling";
    }
}

bool People::tracking(string species) {
    //tracks currently only include footprints, not feces, urine, grazing/hunt remains, sounds, etc.
    //need to first find animal to hunt, if no animal seen then search for tracks, if tracks found then follow tracks
    //need to convert this if conditional into a for loop to check all possible animal names in search results
    
    //if no animal found, check for tracks
    if (pl[p].search_results.find("animal tracks") != pl[p].search_results.end()) {

        //if tracks belong to trappable animals, set traps
        Position track_pos = pl[p].search_results["animal tracks"][0];
        if (envi.tile(track_pos).track.creature == "rabbit") {//need to make these species checks tag checks on the species instead
            Animal::animal& a = al[0];//need to pass an &a, so use this as a "NULL". "set" doesn't use &a currently
            trap("set",species,a);
        }
        //tracks should hold direction info so hunter knows what direction to follow
        Position dir = { pl[p].search_results["animal tracks"][0].x,pl[p].search_results["animal tracks"][0].y };
        if (move_to({ pl[p].pos.x + dir.x,pl[p].pos.y + dir.y }, "following tracks")) {
            //do nothing else, simply follow tracks until prey is found
        }
        return true;//tracking in progress
    }
    else {
        return false;//no animals or tracks found
    }
    
}

//Hunting Methods
bool People::persistence_and_pick_up(string species, Animal::animal& a) {
    // include clams for this method to pick up
        //when hunting, should use the means available, if a better method is known but not currently available, then need to set a flag to prepare the better method for the next hunt
        //also need to add fleeing and tiring to deer
        //FIX THIS: need to add "if have ranged weapon, use at distance" as well as the need to get the ranged weapon if knowledge of its crafting is held, unless materials to make ranged weapon are not found in x amount of time or hunger is too great to wait
        //this serves as both persistance hunting and simple gathering as it is simply "move to animal and kill animal". 
        
        if (!acquire("rock")) {
            return false;//in progress
        }
        bool reached = move_to({ a.pos.x,a.pos.y }, "to prey");
        if (reached) {
            if (a.is_alive) {
                a.is_alive = false;//kill prey
                return false;//in progress
            }
            if (!acquire("knife")) {
                return false;//in progress
            }
            envi.tile(a.pos).animal_id = -1;//remove dead animal from map. Might make more sense to have animal contain body parts and components such as feathers and bones and when all have been removed from corpse, then the corpse is removed.
            create_item(Animal::species[a.species].meat_type, { a.pos.x, a.pos.y });//add meat in its place   <- fix this, Might make more sense to have animals contain a list of items they turn into when butchered and go through the list
            al.erase(al.begin() + a_by_id(a.a_id));//erase animal from global animal list
            return true;//done
        }
}

bool People::ambush(string species, Animal::animal& a) {
    //when ranged attack is added, include throwing a net to catch animal as well as regular ranged attack
    //FIX THIS: need to add relevant changes to Animal to make this work
    //ambush hunted animals such as chickens (includes spearfishing) (stalk animal, when close enough and still unnoticed, rush to attack and catch or kill)
        if (Position::distance({ a.pos.x, a.pos.y }, pl[p].pos) == con.STEALTH_DISTANCE || move_to({ a.pos.x, a.pos.y }, "to prey - stealth")) {
            pl[p].animal_id_ambush = a.a_id;
            pl[p].speed = con.STEALTH_SPEED;
            pl[p].speed_type = "stealth";
            pl[p].in_stealth = true;
            if (Position::distance({ a.pos.x, a.pos.y }, pl[p].pos) == con.AMBUSH_DISTANCE || move_to({ a.pos.x, a.pos.y }, "to prey - ambush")) {
                if (pl[p].sprint_stamina == 0) {//0 means sprint cooldown is done
                    pl[p].speed = con.SPRINT_SPEED;//sprint once close enough to prey
                    pl[p].speed_type = "sprinting";
                    pl[p].in_stealth = false;
                    if (move_to({ a.pos.x, a.pos.y }, "to prey - ambush kill")) {
                        //kill prey
                        if (a.is_alive) {
                            a.is_alive = false;//kill prey
                            return false;//in progress
                        }
                        if (!acquire("knife")) {//should be separated in a process corpse function with the option to process in the field or carry home first, fix this

                            return false;//in progress
                        }
                        envi.tile(a.pos).animal_id = -1;//remove dead animal from map
                        create_item(Animal::species[a.species].meat_type, { a.pos.x, a.pos.y });//add meat in its place   <- fix this, Might make more sense to have animals contain a list of items they turn into when butchered and go through the list
                        al.erase(al.begin() + a_by_id(a.a_id));//erase animal from global animal list
                        return true;//done
                    }
                }
            }
        }

}

//does this ever use &a?
bool People::angling(string species, Animal::animal& a) {
    if (pl[p].active_fish_hook_pos.x != -1) {
        //get fishing rod and bait
        if (!acquire("fishing rod")) {
            return false;//don't have fishing rod, getting it
        }
        if (!acquire("fish bait")) {
            return false;//don't have fish bait, getting it
        }
        //go to water
        if (pl[p].search_results.find("water") != pl[p].search_results.end()) {
            if (Position::distance(pl[p].search_results["water"][0], pl[p].pos) == 1 || move_to(pl[p].search_results["water"][0], "to water")) {
                //reached water edge, continue angling function
            }
            return false;//moving towards water
        }
        else {
            pl[p].general_search_called = true;
            return false;//search for water
        }
        //throw fishing rod hook into water
        create_item("active fish hook", { -1,-1 });//FIX THIS: need to create fish, fish hook, fish bait, fishing rod, and active fish hook and related fish behavior
        int trap_ind = inventory_has("active fish hook")[0];
        if (!drop_item(trap_ind)) {//if no valid tile to throw hook in, if valid tile then drop hook
            pl[p].general_search_called = true;
            pl[p].active_fish_hook_pos = pl[p].dropsite;
            return false;//in progress
        }
    }

    int fish_id = envi.tile(pl[p].active_fish_hook_pos).animal_id;
    if (fish_id == -1) {
        pl[p].time_waited++;//might make more sense to use progress_state instead, fix this
        //pl[p].move_already = true;//don't move this tick, might not work as might move before reaching this line of code. fix this
        if (pl[p].time_waited >= ANGLING_WAIT_TIME) {
            //remove active fish hook
            delete_item(envi.tile(pl[p].active_fish_hook_pos).item_id, pl[p].active_fish_hook_pos, -1);
            pl[p].time_waited = 0;
            pl[p].active_fish_hook_pos = { -1,-1 };
            return false;//did not catch fish
        }
        return false;//waiting for fish to bite
    }
    else {
        Animal::animal& a2 = al[a_by_id(fish_id)];
        if (!a2.is_alive) {//something is dead on the hook, caught something. Distance doesn't matter because not moving to animal because it's "reeled in"
            //remove active hook ("reel it in")
            delete_item(envi.tile(pl[p].active_fish_hook_pos).item_id, pl[p].active_fish_hook_pos, -1);
            pl[p].time_waited = 0;
            pl[p].active_fish_hook_pos = { -1,-1 };
            envi.tile(a2.pos).animal_id = -1;//remove dead animal from map
            create_item(Animal::species[a2.species].meat_type, { a2.pos.x,a2.pos.y });//add meat in its place
            al.erase(al.begin() + a_by_id(a2.a_id));//erase animal from global animal list
            return true;//done
        }
    }
        
}

bool People::trap(string set_or_check, string species, Animal::animal& a) {
    if (set_or_check == "check") {
        //need to also add another type of trap, fishing net, and keep track separately
            //fix this, need to add age and lifetime to traps so that they are removed after a certain amount of time if they haven't caught anything. If trap contains prey, then reset trap if reusable.
        if (!pl[p].traps_set.empty()) {//if have traps, check traps
            for (trap_check& t : pl[p].traps_set) {
                if ((pday_count - t.last_day_checked) >= 3) {//check each trap once every other day since it was set
                    if (move_to(t.pos, "to trap")) {
                        t.last_day_checked = pday_count;
                    }
                    break;
                }
            }
        }//checks traps and then continues so as to react if prey is found in trap
        if (a.a_id == -1) {
            return false;//animal not found, only moved to check traps
        }
        //this serves as the trapping method, need to add a bit of variety for if prey is fish or not. FIX THIS
        //need to add fish
        //(includes fishing with nets) (go to area known to have animal, set trap and either wait for trap to spring or come back later to check traps)
    //need to handle when to remove traps (could make them one use too) or place them in new spots, especially when either not having a campsite or when moving campsites, fix this
        if (!a.is_alive) {
            if (!acquire("knife")) {//fix this, this should be already handled in acquire(), as in simply call acquire(item) and if it returns true then it's becuase already have item, else it's in process and therefore if false return false in hunting() because an action is in process

                return false;//in progress
            }
            if (move_to({ a.pos.x,a.pos.y }, "to trapped small game")) {
                envi.tile(a.pos).animal_id = -1;//remove dead animal from map
                delete_item(envi.tile(a.pos).item_id, a.pos, -1);//delete active trap
                create_item(Animal::species[a.species].meat_type, a.pos);//add meat in its place
                al.erase(al.begin() + a_by_id(a.a_id));//erase animal from global animal list
                return true;//done
            }
        }
    }


    else if (set_or_check == "set") {//called by tracking()
            if (pl[p].traps_set.size() < 4) {
                if (pl[p].search_results.find("no item") != pl[p].search_results.end()) {
                    if (!acquire("trap")) {
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
}

//Need to add alternative methods of obtaining water for humans, such as filling containers, digging wells, cisterns, reservoirs, irrigation, rain barrels, etc.
bool Animal::drinking() {
    animal& c = (a_p_flip) ? al[a] : People::pl[People::p];//controls if accessing an animal from animal list or a person from people list (derived from Animal)
    Species& sp = species[c.species];
    if (c.thirst_level < sp.THIRSTY_LEVEL && c.drinking.progress == 0) {
        return false;
    }
    if (c.search_results.find("water") != c.search_results.end()) {
        if (Position::distance(c.pos, c.search_results["water"][0]) == 1 || move_to(c.search_results["water"][0], "to water")) {
            c.thirst_level -= sp.THIRST_REDUCTION_RATE;
            c.drinking.progress_done = c.thirst_level / sp.THIRST_REDUCTION_RATE;//drink until practically no longer any thirst
            if (c.drinking.progress_func()) {//drinking delay. add animation
                return true;//done
            }
            return true;//in progress
        }
    }
    else {
        c.general_search_called = true;
    }
    return true;//in progress
}


