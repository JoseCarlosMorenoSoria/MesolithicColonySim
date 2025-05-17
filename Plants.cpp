#include "Plants.hpp"

map<string,Plants::SpeciesPreset> Plants::species_presets;
vector<Plants::Plant> Plants::pln;
Plants::Plant* p;//current plant being updated
Plants::SpeciesPreset* s;//the current plant's species constants
int Plants::plant_id_iterator = -1;
//should the Plants class be converted into fully replacing the Plant struct?

Plants::Plants(){}

Plants::Plants(int a) {
	fill_presets();

	new_plant("wheat", { 10,5 });
	new_plant("medicinal plant", { 30,5 });
	new_plant("cannabis plant", { 40,5 });
	new_plant("poisonous plant", { 50,5 });
	new_plant("gourd", { 70,5 });
	for (int i = 0; i < 80; i++) {
		int x, y;
		do {
			x = rand() % envi2.map_x_max;
			y = rand() % envi2.map_y_max;
		} while (!valid_plant_pos({x,y}));
		new_plant("tree", { x,y });
	}
	for (int i = 0; i < 80; i++) {
		int x, y;
		do {
			x = rand() % envi2.map_x_max;
			y = rand() % envi2.map_y_max;
		} while (!valid_plant_pos({ x,y }));
		new_plant("berry bushes", { x,y });
	}
	for (int i = 0; i < 80; i++) {
		int x, y;
		do {
			x = rand() % envi2.map_x_max;
			y = rand() % envi2.map_y_max;
		} while (!valid_plant_pos({ x,y }));
		new_plant("grass", { x,y });
	}
}

bool Plants::valid_plant_pos(Position pos) {
	if (envi2.tile(pos).terrain_name == "freshwater" || envi2.tile(pos).plant_id != -1 || !Position::valid_position(pos)) {
		return false;
	}
	return true;
}

void Plants::delete_plant(Position pos) {
	pln.erase(pln.begin() + get_by_id(envi2.tile(pos).plant_id));
	envi2.tile(pos).plant_id = -1;//remove plant from map
}

void Plants::new_plant(string species, Position pos) {//need to make sure tile doesn't have plant before calling this function
	Plant np;
	SpeciesPreset& sp = species_presets[species];
	Environment::Tile& t = envi2.tile(pos);
	np.plant_id=++plant_id_iterator;//use id generator
	np.species=species;
	np.pos=pos;
	np.light_hunger = {0, sp.light_min,100};//how long plant has been living without light
	np.current_temp = {t.temperature, sp.temp_min, sp.temp_max};
	np.current_water = {t.surface_water_level+t.underground_water_level, sp.water_min, sp.water_max};
	np.current_height = {0,0,sp.max_height};//height and radius shade out shorter plants to outcompete them (deprive them of sunlight and water)
	np.current_radius = {0,0,sp.max_radius};//how far the tree shades out shorter plants or trees of the same height, or bushes doing the same to shorter plants
	//need to set initial components, growth, production, etc (at age 0 = seed, at age 1 = root,stem,leaf, at age 3 = root,stem,leaves,fruit, at age 4 = same as 3 but higher quantities)
	//Items: fruit, log, wood, branch, leaves, roots, sap, bark, fibers, etc
	np.current_components = sp.potential_components;
	np.age = {0,0,sp.lifespan};
	np.growth_level=0;
	t.plant_id = np.plant_id;//tie to Map
	np.current_image = np.species;
	pln.push_back(np);
}

int pi = -1;//for resetting p pointer after adding or deleting a plant due to pointer corruption when vector changes size
void Plants::update_all(int hour, int gtick) {
	for (int i = 0; i < pln.size(); i++) {
		p = &pln[i];
		s = &species_presets[p->species];
		pi = i;
		update(hour, gtick);
	}
}

bool once = true;
void Plants::update(int hour, int gtick) {
	if (!p->is_alive) {
		return;
	}
	check_death();
	Environment::Tile& t = envi2.tile(p->pos);

	string soil_requirements;
	string nutrient_requirements;

	if (t.surface_water_level + t.underground_water_level > p->current_water) {
		p->current_water++;//need to add the same nonlinear change as that planned for human temperature changes
	}
	else if (t.surface_water_level + t.underground_water_level < p->current_water) {
		p->current_water--;
	}

	if (t.temperature > p->current_temp) {
		p->current_temp++;
	}
	else if (t.temperature < p->current_temp) {
		p->current_temp--;
	}

	if (p->age>0 && gtick % s->reproduction_rate == 0) {
		reproduce();
	}
	if (hour == 0) {//once a day
		p->age++;
	}
	
	//is growth_level redundant? Regardless, this formula is just a placeholder for relevant factors and needs to be rewritten
	if (p->current_height < s->max_height || p->current_radius < s->max_radius) {
		p->growth_level += s->growth_rate * p->current_temp * p->current_water * t.light_level * t.soil_fertility_level;
	}
	if (p->current_height < s->max_height) {//FIX THIS: need to add some type of either shading mechanic or exclusion marker on Map to prevent plants growing if they are near another plant that is either taller or at the same level as them, with the distance being determined by the plant's radius
		p->current_height += s->max_height * (p->growth_level / 100);//growth level serves as a percentage of max height to be added over time
	}
	if (p->current_radius < s->max_radius) {
		p->current_radius += s->max_radius * (p->growth_level / 100);
	}

	if (t.light_level < s->light_min) {
		p->light_hunger++;
	}
	else if (p->light_hunger > 0) {
		p->light_hunger--;
	}
}

void Plants::reproduce() {//for now, have reproduce() also trigger the replenishment and production of components such as leaves/fruit/etc
	vector<Position> empty_tiles;
	Position o = p->pos;//origin
	int radiusmax = s->reproduction_distance;
	for (int radius = 0; radius <= radiusmax; radius++) { //this function checks tilemap in outward rings by checking top/bottom and left/right ring boundaries
		if (radius == 0) {//avoids double checking origin
			//do nothing
			continue;
		}
		int xmin = o.x - radius;
		if (xmin < 0) { xmin = 0; }//these reduce iterations when near edges of map
		int xmax = o.x + radius;
		if (xmax > Environment::map_x_max - 1) { xmax = Environment::map_x_max - 1; }
		int ymin = o.y - radius + 1;//+1 and -1 to avoid double checking corners
		if (ymin < 0) { ymin = 0; }
		int ymax = o.y + radius - 1;
		if (ymax > Environment::map_y_max - 1) { ymax = Environment::map_y_max - 1; }
		for (int x = xmin, y = ymin; x <= xmax; x++, y++) {
			for (int sign = -1; sign <= 1; sign += 2) {//sign == -1, then sign == 1
				Position pos1 = { x,o.y + (sign * radius) };//Might be better to turn the for loops into an iterator function that returns the next position to check
				Position pos2 = { o.x + (sign * radius), y };
				if (valid_plant_pos(pos1)) {
					empty_tiles.push_back(pos1);
				}
				if (y <= ymax && pos1 != pos2) {//need to figure out why pos1 sometimes == pos2 and rewrite for loops to avoid this
					if (valid_plant_pos(pos2)) {
						empty_tiles.push_back(pos2);
					}
				}
			}
		}
	}

	if (empty_tiles.empty()) {//create a better function to avoid excessive loops. fix this
		return;//no empty tiles found for new plant
	}

	int max = empty_tiles.size();
	int choice = rand() % max;
	
	vector<string> components;//Items: fruit, log, wood, branch, leaves, roots, sap, bark, fibers, etc
	new_plant(p->species, empty_tiles[choice]);
	p = &pln[pi];//reset pointer due to resizing
}

int Plants::get_by_id(int id) {
	int low = 0;
	int high = pln.size() - 1;
	while (low <= high) {
		int mid = low + (high - low) / 2;
		if (pln[mid].plant_id == id) {
			return mid;
		}
		(pln[mid].plant_id < id) ? low = mid + 1 : high = mid - 1;
	}
	return -1;//not found
}

Plants::Plant& Plants::plant(int id) {
	return pln[get_by_id(id)];
}

void Plants::check_death() {
	bool dehydration = p->current_water.ismin();
	bool drowned = p->current_water.ismax();
	bool light_deprivation = p->light_hunger >= s->light_starvation;
	bool cold_death = p->current_temp.ismin();
	bool heat_death= p->current_temp.ismax();
	bool old_age_death = p->age.ismax();
	bool malnourished;//need to implement
	bool death = dehydration || drowned || light_deprivation || cold_death || heat_death || old_age_death;

	if (death) {
		p->is_alive = false;
		//change image to dead plant
		//decompose
		//when decompose is done, remove from map and list
	}
}

void Plants::fill_presets() {
	//fill from plants csv
	vector<vector<string>> data = get_data("My Game CSVs - Plants.csv");
	for (int i = 8; i < data.size(); i++) {
		SpeciesPreset spp;
		int r = -1;
		spp.species=data[i][++r];
		spp.plant_type = data[i][++r];//tree, flower, vine, grass, shrub, succulent, cactus, etc
		spp.image = data[i][++r];//unsure if this is necessary or if species name can be used to access correct file?
		spp.soil_requirements = data[i][++r];//soil fertility/nutrient levels necesssary to grow/live
		spp.light_min = stoi(data[i][++r]);//minimum light needed to grow and live
		spp.light_starvation = stoi(data[i][++r]);//how many max hours plant can survive without light
		spp.temp_min = stoi(data[i][++r]);
		spp.temp_ideal = stoi(data[i][++r]);//these ranges could be saved as const bints
		spp.temp_max = stoi(data[i][++r]);//temperature limits for growth, death and leaf loss
		spp.water_min = stoi(data[i][++r]);
		spp.water_ideal = stoi(data[i][++r]);
		spp.water_max = stoi(data[i][++r]);
		spp.nutrient_requirements = stoi(data[i][++r]);//minimum, this shouldn't have a max?
		spp.max_height = stoi(data[i][++r]);
		spp.max_radius = stoi(data[i][++r]);
		//Items: fruit, log, wood, branch, leaves, roots, sap, bark, fibers, etc
		for (int j = 0; j < 2; j++) {
			if (data[i][++r] == "") {
				continue;
			}
			else {
				spp.potential_components.push_back(data[i][r]);//current max is 2
			}
		}
		spp.reproduction_rate = stoi(data[i][++r]);//in days
		spp.reproduction_distance = stoi(data[i][++r]);//in tiles, distance plant spreads
		spp.lifespan = stoi(data[i][++r]);//also affects if plant is annual or perrenial, etc
		spp.growth_rate = stoi(data[i][++r]);
		spp.domesticated = (data[i][++r] == "TRUE") ? true:false;
		spp.movement_cost = stoi(data[i][++r]);//moss and short grass have the lowest cost, trees are impassable.
		spp.effects_uses = data[i][++r];//should this instead be only an attribute of plant components?
		spp.calories = stoi(data[i][++r]);//likewise should this be only attribute of components?
		species_presets.insert({ spp.species,spp });
	}
}

string Plants::render_plant(Position pos) {
	int plid = envi2.tile(pos).plant_id;
	Plant& pl = plant(plid);
	return pl.current_image;
}


