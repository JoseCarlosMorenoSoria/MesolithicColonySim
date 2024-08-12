#include "ItemSys.hpp"
using namespace std;

//need to add exception-throws for if an image/item that doesn't exist is called, to catch what the name asked for is and find the source
map<int, ItemSys::Item*> ItemSys::item_list;
map<int, ItemSys::Item> ItemSys::misc_item_list;
map<int, ItemSys::Weapon> ItemSys::weapon_item_list;
map<int, ItemSys::Apparel> ItemSys::apparel_item_list;
map<int, ItemSys::Structure> ItemSys::structure_item_list;
map<int, ItemSys::Tool> ItemSys::tool_item_list;
map<int, ItemSys::Material> ItemSys::material_item_list;
map<int, ItemSys::Container> ItemSys::container_item_list;
int ItemSys::item_id_iterator = 0; 


void ItemSys::update_item_list() {
    for (auto i : weapon_item_list) {
        item_list[i.first] = &i.second;
    }
    for (auto i : apparel_item_list) {
        item_list[i.first] = &i.second;
    }
    for (auto i : tool_item_list) {
        item_list[i.first] = &i.second;
    }
    for (auto i : container_item_list) {
        item_list[i.first] = &i.second;
    }
    for (auto i : material_item_list) {
        item_list[i.first] = &i.second;
    }
    for (auto i : structure_item_list) {
        item_list[i.first] = &i.second;
    }
    for (auto i : misc_item_list) {
        item_list[i.first] = &i.second;
    }
}

int ItemSys::create_item(string item_name) {
    Item new_item = *presets[item_name];

    if (new_item.item_type == "weapon") {
        Weapon w = weapon_presets[item_name];
        w.item_id = new_item_id();
        weapon_item_list.insert({ w.item_id,w });
        item_list.insert({w.item_id, &weapon_item_list[w.item_id]});
        update_item_list();
        return w.item_id;
    }
    else if (new_item.item_type == "apparel") {
        Apparel w = apparel_presets[item_name];
        w.item_id = new_item_id();
        apparel_item_list.insert({ w.item_id,w });
        item_list.insert({ w.item_id, &apparel_item_list[w.item_id] });
        update_item_list();
        return w.item_id;
    }
    else if (new_item.item_type == "tool") {
        Tool w = tool_presets[item_name];
        w.item_id = new_item_id();
        tool_item_list.insert({ w.item_id,w });
        item_list.insert({ w.item_id, &tool_item_list[w.item_id] });
        update_item_list();
        return w.item_id;
    }
    else if (new_item.item_type == "material") {
        Material w = material_presets[item_name];
        w.item_id = new_item_id();
        material_item_list.insert({ w.item_id,w });
        item_list.insert({ w.item_id, &material_item_list[w.item_id] });
        update_item_list();
        return w.item_id;
    }
    else if (new_item.item_type == "container") {
        Container w = container_presets[item_name];
        w.item_id = new_item_id();
        container_item_list.insert({ w.item_id,w });
        item_list.insert({ w.item_id, &container_item_list[w.item_id] });
        update_item_list();
        return w.item_id;
    }
    else if (new_item.item_type == "structure") {
        Structure w = structure_presets[item_name];
        w.item_id = new_item_id();
        structure_item_list.insert({ w.item_id,w });
        item_list.insert({ w.item_id, &structure_item_list[w.item_id] });
        update_item_list();
        return w.item_id;
    }
    else if (new_item.item_type == "misc") {
        Item w = misc_presets[item_name];
        w.item_id = new_item_id();
        misc_item_list.insert({ w.item_id,w });
        item_list.insert({ w.item_id, &misc_item_list[w.item_id] });
        update_item_list();
        return w.item_id;
    }
    return -1;//item is invalid
}

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

int ItemSys::new_item_id() {//unsure if this function is redundant with how int++ works or if there's a better method
    item_id_iterator++;
    return item_id_iterator;
}

void ItemSys::delete_item(int id) {//to reduce need for updating master list, can store up deleted items and simply delete them from master list and then delete periodically from actual object lists and only then update master list (due to pointer corruption)
    Item it = *item_list[id];

    if (it.item_type == "weapon") {
        weapon_item_list.erase(id);
    }
    else if (it.item_type == "apparel") {
        apparel_item_list.erase(id);
    }
    else if (it.item_type == "tool") {
        tool_item_list.erase(id);
    }
    else if (it.item_type == "material") {
        material_item_list.erase(id);
    }
    else if (it.item_type == "container") {
        container_item_list.erase(id);
    }
    else if (it.item_type == "structure") {
        structure_item_list.erase(id);
    }
    else if (it.item_type == "misc") {
        misc_item_list.erase(id);
    }
    item_list.erase(id);
    update_item_list();
}

void ItemSys::ItemPresetsCSVPull() {
    vector<vector<string>> data = get_data("My Game CSVs - Misc Items.csv");
    {
        for (int i = 21; i < data.size(); i++) {
            Item it;
            int r = -1;
            it.item_name = data[i][++r];
            it.image = data[i][++r];
            for (int j = 0; j < 2; j++) {
                if (data[i][r] == "") {
                    ++r;
                    continue;
                }
                it.tags.push_back(data[i][++r]);//include tags such as "food", "tool", "building", etc
            }
            for (int j = 0; j < 3; j++) {
                if (data[i][r] == "") {
                    ++r;
                    continue;
                }
                it.ingredients.push_back(data[i][++r]);//includes tools (and later station) needed to craft item. Later can add quantity requirements for ingredients, etc
            }
            it.consumable_ingredient = (data[i][++r] == "TRUE") ? true : false;//true means item is consumed when used as an ingredient to craft something, false means it isn't
            it.can_pick_up = (data[i][++r] == "TRUE") ? true : false;
            misc_presets.insert({ it.item_name,it });
            //misc_presets[it.item_name] = it;
            presets.insert({ it.item_name,&misc_presets[it.item_name] });
        }
    }
    data = get_data("My Game CSVs - Weapons.csv");
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
            for (int j = 0; j < 3 && r<data[i].size()-1; j++) {
                if (data[i][r] == "") {
                    break;
                }
                w.ingredients.push_back(data[i][++r]);//current max is 3
            }
            weapon_presets.insert({ w.item_name,w });//store object
            presets.insert({ w.item_name, &weapon_presets[w.item_name]});//store reference in master list
        }
    }
    data = get_data("My Game CSVs - ClothingAndArmor.csv");
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
            for (int j = 0; j < 2 && r < data[i].size() - 1; j++) {
                if (data[i][r] == "") {
                    break;
                }
                a.ingredients.push_back(data[i][++r]);
            }
            apparel_presets.insert({ a.item_name,a });
            presets.insert({ a.item_name, &apparel_presets[a.item_name] });
        }
    }
    data = get_data("My Game CSVs - StorageItems.csv");
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
            presets.insert({ c.item_name, &container_presets[c.item_name] });
        }
    }
    data = get_data("My Game CSVs - Structures.csv");
    {
        for (int i = 7; i < data.size(); i++) {
            Structure s;
            s.item_id=-1;
            s.can_pick_up = false;
            s.consumable_ingredient = false;
            int r = -1;
            s.item_name=data[i][++r];
            s.image = data[i][++r];
            ++r;//material, unsure why it's in csv
            s.use = data[i][++r];
            s.ingredients.push_back(data[i][++r]);
            s.insulation_cold = stoi(data[i][++r]);
            structure_presets.insert({ s.item_name,s });
            presets.insert({ s.item_name, &structure_presets[s.item_name] });
        }
    }
    data = get_data("My Game CSVs - Materials.csv");
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
            presets.insert({ m.item_name, &material_presets[m.item_name] });
        }
    }
    data = get_data("My Game CSVs - Tools.csv");
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
            presets.insert({ t.item_name, &tool_presets[t.item_name] });
        }
    }
    data = get_data("My Game CSVs - Food.csv");    //FIX THIS, STRUCT NOT YET IMPLEMENTED
    {
        for (int i = 0; i < data.size(); i++) {

        }
    }
}