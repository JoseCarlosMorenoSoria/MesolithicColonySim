#include "ItemSys.hpp"
using namespace std;

//need to add exception-throws for if an image/item that doesn't exist is called, to catch what the name asked for is and find the source

vector<ItemSys::Item> ItemSys::item_list;
int ItemSys::item_id_iterator = 0; 

ItemSys::ItemSys() {
    ItemPresetsCSVPull();
    fill_tag_lookup();
    fill_ingredients_lookup();
    /*
    cout << "\n\n\n\n\n";
    for (auto const& i : tags) {
        cout << i.first << ": ";
        for (string s : i.second) {
            cout << s << ", ";
        }
        cout << "\n";
    }
    */

}

//Note: "active trap" is a version of trap that avoids getting picked up, must create a special pick up or deactivate function for it

void ItemSys::fill_tag_lookup() {
    for(auto const& i : presets){
        for (string t : i.second.tags) {
            if (tags.find(t) != tags.end()) {//if tag is in list
                tags[t].push_back(i.first);//add item name to tag list
            }
            else {
                tags.insert({ t,{i.first} });//else add tag to list with item name as its first element
            }
        }
    }
}

void ItemSys::fill_ingredients_lookup() {
    for (auto const& i : presets) {
        for (string t : i.second.ingredients) {
            if (ingredients.find(t) != ingredients.end()) {//if ingredient is in list
                ingredients[t].push_back(i.first);//add item name to ingredients list
            }
            else {
                ingredients.insert({ t,{i.first} });//else add ingredient to list with item name as its first element
            }
        }
    }
}

int ItemSys::item_by_id(int id) {//uses binary search to find and return index to item in item list
    int low = 0;
    int high = item_list.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (item_list[mid].item_id == id) {
            return mid;
        }
        (item_list[mid].item_id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}

int ItemSys::new_item_id() {//unsure if this function is redundant with how int++ works or if there's a better method
    item_id_iterator++;
    return item_id_iterator;
}

void ItemSys::ItemPresetsCSVPull() {
    vector<vector<string>> data = get_data("misc item csv");    //FIX THIS, DATA NOT YET IMPLEMENTED
    {
        int item_id = -1;
        string item_name = "";
        string image = "";
        vector<string> tags;//include tags such as "food", "tool", "building", etc
        vector<string> ingredients;//includes tools (and later station) needed to craft item. Later can add quantity requirements for ingredients, etc
        bool consumable_ingredient = true;//true means item is consumed when used as an ingredient to craft something, false means it isn't
        bool can_pick_up = true;
    }
    data = get_data("weapons csv");
    {
        for (int i = 21; i < data.size(); i++) {//starts at row 22 (i==21) because the above rows are comments/labels/notes
            Weapon w;
            w.item_type = "weapon";
            w.consumable_ingredient = false;
            w.can_pick_up = true;
            w.item_id=-1;
            int r = -1;//row number, allows rearranging column order without having to rewrite indices for each row
            w.item_name=data[i][++r];
            w.image = data[i][++r];
            w.tags.push_back(data[i][++r]);//currently weapons only have 1 tag
            w.cut_damage = stoi(data[i][++r]);
            w.slash_damage = stoi(data[i][++r]);
            w.piercing_damage = stoi(data[i][++r]);
            w.blunt_damage = stoi(data[i][++r]);
            w.burn_damage = stoi(data[i][++r]);
            w.stagger = stoi(data[i][++r]);
            w.range = stoi(data[i][++r]);
            w.mass = stoi(data[i][++r]);
            w.speed = stoi(data[i][++r]);
            for (int j = 0; j < 3; j++) {
                if (data[i][r] == "") {
                    break;
                }
                w.ingredients.push_back(data[i][++r]);//current max is 3
            }
            presets.insert({ w.item_name,w });
        }
    }
    data = get_data("clothing and armor csv");
    {
        for (int i = 14; i < data.size(); i++) {
            Apparel a;
            a.item_type = "apparel";
            a.consumable_ingredient = false;
            a.can_pick_up = true;
            a.item_id = -1;
            int r = -1;
            a.item_name = data[i][++r];
            a.image = data[i][++r];
            a.cut_defense = stoi(data[i][++r]);
            a.slash_defense = stoi(data[i][++r]);
            a.piercing_defense = stoi(data[i][++r]);
            a.blunt_defense = stoi(data[i][++r]);
            a.burn_defense = stoi(data[i][++r]);
            a.weight = stoi(data[i][++r]);//should this be mass for consistency?
            a.body_part = data[i][++r];
            a.insulation_cold = stoi(data[i][++r]);
            a.beauty = stoi(data[i][++r]);
            for (int j = 0; j < 2; j++) {
                if (data[i][r] == "") {
                    break;
                }
                a.ingredients.push_back(data[i][++r]);
            }
            presets.insert({ a.item_name,a });
        }
    }
    data = get_data("storage item csv");
    {
        for (int i = 6; i < data.size(); i++) {
            Container c;
            c.item_type = "container";
            c.item_id = -1;
            c.consumable_ingredient = false;
            int r = -1;
            c.item_name = data[i][++r];
            c.image = data[i][++r];
            for (int j = 0; j < 2; j++) {
                if (data[i][r] == "") {
                    ++r;
                    continue;
                }
                c.tags.push_back(data[i][++r]);
            }
            c.capacity=stoi(data[i][++r]);
            c.holds_liquid = (data[i][++r] == "TRUE") ? true:false;
            c.airtight = (data[i][++r] == "TRUE") ? true : false;
            c.can_pick_up = (data[i][++r] == "TRUE") ? true : false;
            for (int j = 0; j < 3; j++) {
                if (data[i][r] == "") {
                    break;
                }
                c.ingredients.push_back(data[i][++r]);
            }
            presets.insert({ c.item_name,c });
        }
    }
    data = get_data("structures csv");
    {
        for (int i = 7; i < data.size(); i++) {
            Structure s;
            s.item_id=-1;
            s.can_pick_up = false;
            s.consumable_ingredient = false;
            int r = -1;
            s.item_name=data[i][++r];
            s.use = data[i][++r];
            s.ingredients.push_back(data[i][++r]);
            s.insulation_cold = stoi(data[i][++r]);
            presets.insert({ s.item_name,s });
        }
    }
    data = get_data("materials");
    {
        for (int i = 4; i < data.size(); i++) {
            Material m;
            m.can_pick_up = true;
            m.consumable_ingredient = true;
            m.item_id = -1;
            int r = -1;
            m.item_name=data[i][++r];
            m.image = data[i][++r];
            m.source = data[i][++r];
            m.mass = stoi(data[i][++r]);
            m.sharpness = stoi(data[i][++r]);
            m.beauty = stoi(data[i][++r]);
            m.crafting_time = stoi(data[i][++r]);
            m.insulation = stoi(data[i][++r]);
            presets.insert({ m.item_name,m });
        }
    }
    data = get_data("tools");
    {
        for (int i = 6; i < data.size(); i++) {
            Tool t;
            t.item_id = -1;
            t.can_pick_up = true;
            t.consumable_ingredient = false;
            int r = -1;
            t.item_name=data[i][++r];
            t.image = data[i][++r];
            t.use_case = data[i][++r];
            t.crafting_process = data[i][++r];
            t.crafting_time = stoi(data[i][++r]);
            for (int j = 0; j < 3; j++) {
                if (data[i][r] == "") {
                    break;
                }
                t.ingredients.push_back(data[i][++r]);//current max is 3
            }
            presets.insert({ t.item_name,t });
        }
    }
    data = get_data("food");    //FIX THIS, STRUCT NOT YET IMPLEMENTED
    {
        for (int i = 0; i < data.size(); i++) {

        }
    }
}