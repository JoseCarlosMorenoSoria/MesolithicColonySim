#include "Plants.hpp"

map<string,Plants::SpeciesPreset> Plants::species_presets;
vector<Plants::Plant> Plants::pln;
Plants::Plant* p;//current plant being updated
Plants::SpeciesPreset* s;//the current plant's species constants
//should the Plants class be converted into fully replacing the Plant struct?

Plants::Plants(int a) {

}

Plants::Plant Plants::new_plant(string species, Position pos) {
	Plant np;
	SpeciesPreset& sp = species_presets[species];
	Environment::Tile& t = Environment::Map[pos.y][pos.x];
	np.plant_id;//use id generator
	np.species=species;
	np.pos=pos;
	np.light_hunger = {0, sp.light_min,100};//how long plant has been living without light
	np.current_temp = {t.temperature, sp.temp_min, sp.temp_max};
	np.current_water = {t.surface_water_level+t.underground_water_level, sp.water_min, sp.water_max};
	np.current_height = {0,0,sp.max_height};//height and radius shade out shorter plants to outcompete them (deprive them of sunlight and water)
	np.current_radius = {0,0,sp.max_radius};//how far the tree shades out shorter plants or trees of the same height, or bushes doing the same to shorter plants
	//need to set initial components
	//vector<string> current_components;//Items: fruit, log, wood, branch, leaves, roots, sap, bark, fibers, etc
	np.age = {0,0,sp.lifespan};
	np.growth_level=0;

	//need to tie to Map
	//

	return np;
}

void Plants::update_all(int hour) {
	for (int i = 0; i < pln.size(); i++) {
		//pli = i;
		p = &pln[i];
		s = &species_presets[p->species];
		update(hour);
	}
}

void Plants::update(int hour) {
	if (!p->is_alive) {
		return;
	}
	check_death();
	Environment::Tile& t = Environment::Map[p->pos.y][p->pos.x];

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

	if (hour == 0) {//once a day
		p->age++;
	}
	if (p->age>0 && p->age % s->reproduction_rate == 0) {
		reproduce();
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
		p->light_hunger--;//might be useful to create a bounded int replacement such that the variable can never increase beyond a max or below a min
	}//a bounded int could be more cleanly/efficiently done by using AND to bitmask the int to be within bounds
}

void Plants::reproduce() {//for now, have reproduce() also trigger the replenishment and production of components such as leaves/fruit/etc
	int reproduction_distance;//in tiles, distance plant spreads
	vector<string> components;//Items: fruit, log, wood, branch, leaves, roots, sap, bark, fibers, etc

	int nx = rand() % (s->reproduction_distance * 2) - s->reproduction_distance;
	int ny = rand() % (s->reproduction_distance * 2) - s->reproduction_distance;
	nx = p->pos.x + nx;
	ny = p->pos.y + ny;
	if (nx < 0) { nx = 0; }
	if (nx >= Environment::map_x_max) { nx = Environment::map_x_max - 1; }
	if (ny < 0) { ny = 0; }
	if (ny >= Environment::map_y_max) { ny = Environment::map_y_max - 1; }
	//FIX THIS: need to add a plant id generator and a get_by_id function like those of People()
	Plant np = new_plant(p->species,{nx,ny}); //need to use preset to initialize a new plant
	pln.push_back(np);
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
	}
}
void Plants::fill_presets() {
	//fill from plants csv
}


