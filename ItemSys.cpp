#include "ItemSys.hpp"
using namespace std;

//need to add exception-throws for if an image/item that doesn't exist is called, to catch what the name asked for is and find the source

vector<ItemSys::Item> ItemSys::misc_item_list;
vector<ItemSys::Weapon> ItemSys::weapon_item_list;
vector<ItemSys::Apparel> ItemSys::apparel_item_list;
vector<ItemSys::Structure> ItemSys::structure_item_list;
vector<ItemSys::Tool> ItemSys::tool_item_list;
vector<ItemSys::Material> ItemSys::material_item_list;
vector<ItemSys::Container> ItemSys::container_item_list;
int ItemSys::item_id_iterator = 0; 

map<string, ItemSys::Item> ItemSys::misc_presets;
map<string, ItemSys::Weapon> ItemSys::weapon_presets;
map<string, ItemSys::Apparel> ItemSys::apparel_presets;
map<string, ItemSys::Tool> ItemSys::tool_presets;
map<string, ItemSys::Material> ItemSys::material_presets;
map<string, ItemSys::Structure> ItemSys::structure_presets;
map<string, ItemSys::Container> ItemSys::container_presets;

ItemSys::ItemSys() {
    ItemPresetsCSVPull();
    //no longer relevant given changes
    //fill_tag_lookup();
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


void ItemSys::fill_ingredients_lookup() {
    for (auto const& i : weapon_presets) {
        for (string t : i.second.ingredients) {
            if (ingredients.find(t) != ingredients.end()) {//if ingredient is in list
                ingredients[t].push_back(i.first);//add item name to ingredients list
            }
            else {
                ingredients.insert({ t,{i.first} });//else add ingredient to list with item name as its first element
            }
        }
    }
    for (auto const& i : apparel_presets) {
        for (string t : i.second.ingredients) {
            if (ingredients.find(t) != ingredients.end()) {//if ingredient is in list
                ingredients[t].push_back(i.first);//add item name to ingredients list
            }
            else {
                ingredients.insert({ t,{i.first} });//else add ingredient to list with item name as its first element
            }
        }
    }
    for (auto const& i : tool_presets) {
        for (string t : i.second.ingredients) {
            if (ingredients.find(t) != ingredients.end()) {//if ingredient is in list
                ingredients[t].push_back(i.first);//add item name to ingredients list
            }
            else {
                ingredients.insert({ t,{i.first} });//else add ingredient to list with item name as its first element
            }
        }
    }

    //material has no ingredients

    for (auto const& i : container_presets) {
        for (string t : i.second.ingredients) {
            if (ingredients.find(t) != ingredients.end()) {//if ingredient is in list
                ingredients[t].push_back(i.first);//add item name to ingredients list
            }
            else {
                ingredients.insert({ t,{i.first} });//else add ingredient to list with item name as its first element
            }
        }
    }
    for (auto const& i : misc_presets) {
        for (string t : i.second.ingredients) {
            if (ingredients.find(t) != ingredients.end()) {//if ingredient is in list
                ingredients[t].push_back(i.first);//add item name to ingredients list
            }
            else {
                ingredients.insert({ t,{i.first} });//else add ingredient to list with item name as its first element
            }
        }
    }
    for (auto const& i : structure_presets) {
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

/* This isn't working, use separate functions for each child struct for now
ItemSys::Item* ItemSys::item_by_id(int id) {//uses binary search to find and return pointer to item from item lists
    vector<int> size = {
        (int)misc_item_list.size(),
        (int)weapon_item_list.size(),
        (int)apparel_item_list.size(),
        (int)container_item_list.size(),
        (int)structure_item_list.size(),
        (int)material_item_list.size(),
        (int)tool_item_list.size()
    };
    for (int i = 0; i < size.size(); i++) {
        int low = 0;
        int high = size[i] - 1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            Item *it;
            if (i == 0) {
                it = &misc_item_list[mid];
            }
            else if (i == 1) {
                it = &weapon_item_list[mid];
            }
            else if (i == 2) {
                it = &apparel_item_list[mid];
            }
            else if (i == 3) {
                it = &container_item_list[mid];
            }
            else if (i == 4) {
                it = &structure_item_list[mid];
            }
            else if (i == 5) {
                it = &material_item_list[mid];
            }
            else if (i == 6) {
                it = &tool_item_list[mid];
            }

            if (it->item_id == id) {
                return it;
            }
            (it->item_id < id) ? low = mid + 1 : high = mid - 1;
        }
    }

    return nullptr;//not found
}
*/


int ItemSys::misc_item_by_id(int id) {
    int low = 0;
    int high = misc_item_list.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (misc_item_list[mid].item_id == id) {
            return mid;
        }
        (misc_item_list[mid].item_id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}
int ItemSys::weapon_by_id(int id) {
    int low = 0;
    int high = weapon_item_list.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (weapon_item_list[mid].item_id == id) {
            return mid;
        }
        (weapon_item_list[mid].item_id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}
int ItemSys::apparel_by_id(int id) {
    int low = 0;
    int high = apparel_item_list.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (apparel_item_list[mid].item_id == id) {
            return mid;
        }
        (apparel_item_list[mid].item_id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}
int ItemSys::container_by_id(int id) {
    int low = 0;
    int high = container_item_list.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (container_item_list[mid].item_id == id) {
            return mid;
        }
        (container_item_list[mid].item_id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}
int ItemSys::tool_by_id(int id) {
    int low = 0;
    int high = tool_item_list.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (tool_item_list[mid].item_id == id) {
            return mid;
        }
        (tool_item_list[mid].item_id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}
int ItemSys::material_by_id(int id) {
    int low = 0;
    int high = material_item_list.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (material_item_list[mid].item_id == id) {
            return mid;
        }
        (material_item_list[mid].item_id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}
int ItemSys::structure_by_id(int id) {
    int low = 0;
    int high = structure_item_list.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (structure_item_list[mid].item_id == id) {
            return mid;
        }
        (structure_item_list[mid].item_id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}
ItemSys::Item ItemSys::as_item_by_id(int id) {//returns truncated copy of item which contains base struct properties only
    if (weapon_by_id(id) != -1) {
        Item it = weapon_item_list[weapon_by_id(id)];
        return it;
    }
    if (apparel_by_id(id) != -1) {
        Item it = apparel_item_list[apparel_by_id(id)];
        return it;
    }
    if (container_by_id(id) != -1) {
        Item it = container_item_list[container_by_id(id)];
        return it;
    }
    if (tool_by_id(id) != -1) {
        Item it = tool_item_list[tool_by_id(id)];
        return it;
    }
    if (material_by_id(id) != -1) {
        Item it = material_item_list[material_by_id(id)];
        return it;
    }
    if (structure_by_id(id) != -1) {
        Item it = structure_item_list[structure_by_id(id)];
        return it;
    }
    if (misc_item_by_id(id) != -1) {
        Item it = misc_item_list[misc_item_by_id(id)];
        return it;
    }
    Item null_item;
    return null_item;
}
ItemSys::Item ItemSys::as_item_preset_by_name(string name) {
    if (misc_presets.find(name) != misc_presets.end()) {
        return misc_presets[name];
    }
    if (weapon_presets.find(name) != weapon_presets.end()) {
        return weapon_presets[name];
    }
    if (apparel_presets.find(name) != apparel_presets.end()) {
        return apparel_presets[name];
    }
    if (container_presets.find(name) != container_presets.end()) {
        return container_presets[name];
    }
    if (tool_presets.find(name) != tool_presets.end()) {
        return tool_presets[name];
    }
    if (structure_presets.find(name) != structure_presets.end()) {
        return structure_presets[name];
    }
    if (material_presets.find(name) != material_presets.end()) {
        return material_presets[name];
    }
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
            weapon_presets.insert({ w.item_name,w });
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
            apparel_presets.insert({ a.item_name,a });
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
            container_presets.insert({ c.item_name,c });
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
            structure_presets.insert({ s.item_name,s });
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
            material_presets.insert({ m.item_name,m });
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
            tool_presets.insert({ t.item_name,t });
        }
    }
    data = get_data("food");    //FIX THIS, STRUCT NOT YET IMPLEMENTED
    {
        for (int i = 0; i < data.size(); i++) {

        }
    }
}