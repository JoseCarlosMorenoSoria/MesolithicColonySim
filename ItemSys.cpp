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
    //read from csv into a vector of structs
    fstream fin;// File pointer 
    fin.open("Item Presets - Sheet1.csv", ios::in);// Open an existing file 
    vector<string> row;// Read the Data from the file as String Vector 
    string line, word;
    bool firstrowdone = false;
    bool start_count_tags = false;//for counting the max number of tag columns
    int count_tags = 0;
    bool start_count_ingredients = false;//for counting the max number of ingredient columns
    int count_ingredients = 0;
    while (getline(fin, line)) {// read an entire row and store it in a string variable 'line' 
        row.clear();
        stringstream s(line);// used for breaking words 
        while (getline(s, word, ',')) {// read every column data of a row and store it in a string variable, 'word' 
            row.push_back(word);// add all the column data of a row to a vector 
        }
        if (firstrowdone) {
            Item it;
            it.item_id = -1;
            it.item_name = row[0];
            it.image = row[1];
            int tag_end = 2 + count_tags;
            for (int i = 2; i < tag_end; i++) {
                if (row[i] == "") {
                    break;
                }
                it.tags.push_back(row[i]);
            }
            int ingr_end = tag_end + count_ingredients;
            for (int i = tag_end; i < ingr_end; i++) {
                if (row[i] == "") {
                    break;
                }
                it.ingredients.push_back(row[i]);
            }
            it.consumable_ingredient = (row[ingr_end]=="TRUE")?true:false;
            it.can_pick_up = (row[ingr_end+1] == "TRUE") ? true : false;
            
            if (row.size()==ingr_end+3) {//need to check size for last element because if the element is blank for any row, the row will be shorter and therefore result in index out of range if it is empty
                it.insulation_from_cold = stoi(row[ingr_end + 2]);
            }
            presets.insert({it.item_name, it });
        }
        else {
            for (int i = 2; i < row.size()-3; i++) {
                if (row[i] == "Tags") { start_count_tags = true; }
                else if (row[i] == "Ingredients") { start_count_tags = false; }
                if (start_count_tags) { count_tags++; }
            }
            for (int i = 2+count_tags; i < row.size()-3; i++) {
                if (row[i] == "Ingredients") { start_count_ingredients = true; }
                else if (row[i] == "Consumable Ingredient") { start_count_ingredients = false; }
                if (start_count_ingredients) { count_ingredients++; }
            }
            firstrowdone = true;
        }
    }
}