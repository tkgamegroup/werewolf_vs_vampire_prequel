#include "game.h"

#include <flame/foundation/sheet.h>
#include <flame/foundation/system.h>
#include <flame/graphics/canvas.h>

enum TileType
{
	TileField,
	TileCity,
	TileResourceField,

	TileTypeCount
};

enum GameState
{
	GameInit,
	GameDay,
	GameNight,
	GameBattle
};

enum ResourceType
{
	ResourceWood,
	ResourceClay,
	ResourceIron,
	ResourceCrop,
	ResourceGold,

	ResourceTypeCount
};

enum BuildingType
{
	BuildingTownCenter,
	BuildingHouse,
	BuildingBarracks,
	BuildingPark,
	BuildingTower,
	BuildingWall,

	BuildingTypeCount,
	BuildingInTownBegin = BuildingTownCenter,
	BuildingInTownEnd = BuildingPark,
};

const wchar_t* get_building_name(BuildingType type)
{
	switch (type)
	{
	case BuildingTownCenter: return L"Town Center";
	case BuildingHouse: return L"House";
	case BuildingBarracks: return L"Barracks";
	case BuildingPark: return L"Park";
	case BuildingTower: return L"Tower";
	case BuildingWall: return L"Wall";
	}
	return L"";
}

enum PokemonType
{
	PokemonNone,
	PokemonNormal,
	PokemonFire,
	PokemonWater,
	PokemonElectric,
	PokemonGrass,
	PokemonIce,
	PokemonFighting,
	PokemonPoison,
	PokemonGround,
	PokemonFlying,
	PokemonPsychic,
	PokemonBug,
	PokemonRock,
	PokemonGhost,
	PokemonDragon,
	PokemonDark,
	PokemonSteel,
	PokemonFairy
};

const wchar_t* get_pokemon_type_name(PokemonType type)
{
	switch (type)
	{
	case PokemonNone: return L"None";
	case PokemonNormal: return L"Normal";
	case PokemonFire: return L"Fire";
	case PokemonWater: return L"Water";
	case PokemonElectric: return L"Electric";
	case PokemonGrass: return L"Grass";
	case PokemonIce: return L"Ice";
	case PokemonFighting: return L"Fighting";
	case PokemonPoison: return L"Poison";
	case PokemonGround: return L"Ground";
	case PokemonFlying: return L"Flying";
	case PokemonPsychic: return L"Psychic";
	case PokemonBug: return L"Bug";
	case PokemonRock: return L"Rock";
	case PokemonGhost: return L"Ghost";
	case PokemonDragon: return L"Dragon";
	case PokemonDark: return L"Dark";
	case PokemonSteel: return L"Steel";
	case PokemonFairy: return L"Fairy";
	}
	return L"";
}

cCameraPtr camera;

graphics::CanvasPtr canvas;

graphics::ImagePtr img_be_hit;

graphics::ImagePtr img_tile_frame;
graphics::ImagePtr img_tile_grass;
graphics::ImagePtr img_tile_castle;

graphics::ImagePtr img_city;
graphics::ImagePtr img_wall;
graphics::ImagePtr img_ground;
graphics::ImagePtr img_town_center;
graphics::ImagePtr img_house;
graphics::ImagePtr img_barracks;
graphics::ImagePtr img_park;
graphics::ImagePtr img_tower;

graphics::ImagePtr img_resources[ResourceTypeCount] = {};
graphics::ImagePtr img_population;

graphics::SamplerPtr sp_repeat;

void draw_image(graphics::ImagePtr img, const vec2& pos, const vec2& sz, const vec2& pivot = vec2(0.f), const cvec4& tint = cvec4(255))
{
	auto p = pos;
	p -= pivot * sz;
	canvas->draw_image(img->get_view(), p, p + sz, vec4(0.f, 0.f, 1.f, 1.f), tint);
}

void draw_text(std::wstring_view text, uint font_size, const vec2& pos, const vec2& pivot = vec2(0.f), const cvec4& color = cvec4(255), const vec2& shadow_offset = vec2(0.f), const cvec4& shadow_color = cvec4(255))
{
	auto p = pos;
	if (pivot.x != 0.f || pivot.y != 0.f)
	{
		auto sz = canvas->calc_text_size(nullptr, font_size, text);
		p -= pivot * sz;
	}
	canvas->draw_text(nullptr, font_size, p, text, color);
	if (shadow_offset.x != 0.f || shadow_offset.y != 0.f)
		canvas->draw_text(nullptr, font_size, p + shadow_offset, text, shadow_color);
}

auto tile_cx = 24U;
auto tile_cy = 9U;
auto tile_sz = 50.f;
auto tile_sz_y = tile_sz * 0.5f * 1.732050807569;

struct Tile
{
	uint id;
	uint x, y;
	vec2 pos;
	TileType type;
	int lord_id;

	int tile_lt = -1;
	int tile_t = -1;
	int tile_rt = -1;
	int tile_lb = -1;
	int tile_b = -1;
	int tile_rb = -1;
};
std::vector<Tile> tiles;

int selected_tile = -1;

std::vector<uint> find_path(uint start_id, uint end_id)
{
	std::vector<uint> ret;
	std::vector<bool> marks;
	marks.resize(tiles.size());
	std::vector<std::pair<uint, uint>> candidates;
	candidates.push_back({ start_id, start_id });
	marks[start_id] = true;
	auto idx = 0;
	while (idx < candidates.size())
	{
		auto [id, parent] = candidates[idx];

		if (id == end_id)
		{
			ret.push_back(end_id);
			id = candidates[parent].first;
			parent = candidates[parent].second;
			while (id != start_id)
			{
				ret.push_back(id);
				id = candidates[parent].first;
				parent = candidates[parent].second;
			}
			ret.push_back(start_id);
			break;
		}

		auto& tile = tiles[id];
		if (tile.tile_lt != -1 && !marks[tile.tile_lt])
		{
			candidates.push_back({ tile.tile_lt, idx });
			marks[tile.tile_lt] = true;
		}
		if (tile.tile_t != -1 && !marks[tile.tile_t])
		{
			candidates.push_back({ tile.tile_t, idx });
			marks[tile.tile_t] = true;
		}
		if (tile.tile_rt != -1 && !marks[tile.tile_rt])
		{
			candidates.push_back({ tile.tile_rt, idx });
			marks[tile.tile_rt] = true;
		}
		if (tile.tile_lb != -1 && !marks[tile.tile_lb])
		{
			candidates.push_back({ tile.tile_lb, idx });
			marks[tile.tile_lb] = true;
		}
		if (tile.tile_b != -1 && !marks[tile.tile_b])
		{
			candidates.push_back({ tile.tile_b, idx });
			marks[tile.tile_b] = true;
		}
		if (tile.tile_rb != -1 && !marks[tile.tile_rb])
		{
			candidates.push_back({ tile.tile_rb, idx });
			marks[tile.tile_rb] = true;
		}

		idx++;
	}
	std::reverse(ret.begin(), ret.end());
	return ret;
}

struct UnitData
{
	std::wstring name;
	uint cost_gold;
	uint cost_population;
	uint HP;
	uint ATK;
	uint DEF;
	uint SA;
	uint SD;
	uint SP;
	graphics::ImagePtr icon = nullptr;
};
std::vector<UnitData> unit_datas;

struct BuildingBaseData
{
	uint cost_wood;
	uint cost_clay;
	uint cost_iron;
	uint cost_crop;
	uint cost_gold;
	uint cost_population;

	void read(Sheet::Row& row, SheetPtr sht)
	{
		cost_wood = sht->get_as<uint>(row, "cost_wood"_h);
		cost_clay = sht->get_as<uint>(row, "cost_clay"_h);
		cost_iron = sht->get_as<uint>(row, "cost_iron"_h);
		cost_crop = sht->get_as<uint>(row, "cost_crop"_h);
		cost_gold = sht->get_as<uint>(row, "cost_gold"_h);
		cost_population = sht->get_as<uint>(row, "cost_population"_h);
	}
};

struct TownCenterData : BuildingBaseData
{
};
std::vector<TownCenterData> town_center_datas;

struct HouseData : BuildingBaseData
{
	uint provide_population;
};
std::vector<HouseData> house_datas;

struct BarracksData : BuildingBaseData
{
};
std::vector<BarracksData> barracks_datas;

struct ParkData : BuildingBaseData
{
	std::vector<uint> encounter_list;
	uint capture_num;
};
std::vector<ParkData> park_datas;

struct TowerData : BuildingBaseData
{
};
std::vector<TowerData> tower_datas;

struct WallData : BuildingBaseData
{
};
std::vector<WallData> wall_datas;

BuildingBaseData* get_building_base_data(BuildingType type, uint lv)
{
	BuildingBaseData* ret = nullptr;
	switch (type)
	{
	case BuildingTownCenter: 
		if (town_center_datas.size() > lv)
			ret = &town_center_datas[lv];
		break;
	case BuildingHouse:
		if (house_datas.size() > lv)
			ret = &house_datas[lv];
		break;
	case BuildingBarracks:
		if (park_datas.size() > lv)
			ret = &park_datas[lv];
		break;
	case BuildingPark:
		if (barracks_datas.size() > lv)
			ret = &barracks_datas[lv];
		break;
	case BuildingTower:
		if (tower_datas.size() > lv)
			ret = &tower_datas[lv];
		break;
	case BuildingWall:
		if (wall_datas.size() > lv)
			ret = &wall_datas[lv];
		break;
	}
	return ret;
}

struct Building
{
	uint slot;
	BuildingType type;
	uint lv = 0;
};

struct Unit
{
	uint id;
	int HP;
	int HP_MAX;
	int ATK;
	int DEF;
	int SA;
	int SD;
	int SP;
};

struct City
{
	uint id;
	uint tile_id;
	uint lord_id;
	uint loyalty;
	std::vector<Building> buildings;
	std::vector<uint> captures;
	std::vector<Unit> units;

	int get_building_lv(BuildingType type, int slot = -1)
	{
		for (auto& building : buildings)
		{
			if (building.type == type && (slot == -1 || building.slot == slot))
				return building.lv;
		}
		return -1;
	}
};

struct Troop
{
	uint lord_id;
	uint city_id;
	std::vector<uint> unit_ids;
	std::vector<uint> path;
	uint idx = 0;

	Unit& get_unit(uint idx);
};

struct BuildingSlot
{
	vec2 pos;
	float radius;
	BuildingType type;
};
std::vector<BuildingSlot> building_slots;

int selected_building_slot = -1;

const wchar_t* get_resource_field_name(ResourceType type)
{
	switch (type)
	{
	case ResourceWood: return L"Woodcutter";
	case ResourceClay: return L"Clay Pit";
	case ResourceIron: return L"Iron Mine";
	case ResourceCrop: return L"Crop Field";
	}
	return L"";
}

struct ResourceField
{
	uint tile_id;
	uint lv;
	ResourceType type;
	uint production;
};

struct ResourceFieldData
{
	uint cost_wood;
	uint cost_clay;
	uint cost_iron;
	uint cost_crop;
	uint cost_gold;
	uint cost_population;
	uint production;

	void read(Sheet::Row& row, SheetPtr sht)
	{
		cost_wood = sht->get_as<uint>(row, "cost_wood"_h);
		cost_clay = sht->get_as<uint>(row, "cost_clay"_h);
		cost_iron = sht->get_as<uint>(row, "cost_iron"_h);
		cost_crop = sht->get_as<uint>(row, "cost_crop"_h);
		cost_gold = sht->get_as<uint>(row, "cost_gold"_h);
		cost_population = sht->get_as<uint>(row, "cost_population"_h);
		production = sht->get_as<uint>(row, "production"_h);
	}
};

std::vector<ResourceFieldData> resource_field_datas[ResourceTypeCount];

struct BattleTroop
{
	struct UnitDisplay
	{
		uint unit_id;
		int HP;
		int HP_MAX;
		vec3 init_pos;
		vec2 pos; float ang; vec2 scl; float alpha;
		uint damage = 0;
		float damage_label_remain = 0.f;
		std::wstring state_text;
		float state_text_remain = 0.f;

		void take_damage(uint _damage)
		{
			damage += _damage;
			HP -= damage;
			damage_label_remain = 0.3f;

			if (HP <= 0)
			{
				add_event([this]() {
					auto id = game.tween->begin_2d_targets(1);
					game.tween->setup_2d_target(id, 0, nullptr, nullptr, nullptr, &alpha);
					game.tween->alpha_to(id, 0.f, 0.5f);
					game.tween->set_ease(id, EaseInCubic);
					game.tween->end(id);
					return false;
				});
			}
		}

		void get_state(const std::wstring& text)
		{
			state_text = text;
			state_text_remain = 0.5f;
		}
	};

	uint side;
	Troop* troop;
	std::vector<UnitDisplay> unit_displays;
	uint action_idx;

	void refresh_display()
	{
		unit_displays.resize(troop->unit_ids.size());
		for (auto i = 0; i < troop->unit_ids.size(); i++)
		{
			auto& unit = troop->get_unit(i);
			auto& display = unit_displays[i];
			display.unit_id = unit.id;
			display.HP = unit.HP;
			display.HP_MAX = unit.HP_MAX;
			display.init_pos = vec3(132.f + i * 75.f, 100.f + 200.f * (1 - side), 0.f);
			display.pos = display.init_pos;
			display.scl = vec3(1.f);
			display.alpha = 1.f;
		}
	}
};

struct Lord
{
	uint id;
	bool dead = false;

	uint resources[ResourceTypeCount];
	uint provide_population;
	uint consume_population;

	std::vector<City> cities;
	std::vector<uint> territories;
	std::vector<ResourceField> resource_fields;

	std::vector<Troop> troops;

	void update_territories()
	{
		territories.clear();
		for (auto id = 0; id < tiles.size(); id++)
		{
			auto tile_pos = tiles[id].pos;
			auto ok = false;
			for (auto& city : cities)
			{
				if (distance(tiles[city.tile_id].pos, tile_pos) < tile_sz * 1.5f)
				{
					ok = true;
					break;
				}
			}
			if (ok)
				territories.push_back(id);
		}
	}

	bool has_territory(uint tile_id)
	{
		for (auto& t : territories)
		{
			if (t == tile_id)
				return true;
		}
		return false;
	}

	int find_resource_field(uint tile_id)
	{
		for (auto i = 0; i < resource_fields.size(); i++)
		{
			if (resource_fields[i].tile_id == tile_id)
				return i;
		}
		return -1;
	}

	int find_city(uint tile_id)
	{
		for (auto i = 0; i < cities.size(); i++)
		{
			if (cities[i].tile_id == tile_id)
				return i;
		}
		return -1;
	}

	bool build_resource_field(uint tile_id, ResourceType type, bool free = false)
	{
		auto& tile = tiles[tile_id];
		if (tile.type != TileField)
			return false;

		if (resource_field_datas[type].empty())
			return false;

		auto& first_level = resource_field_datas[type].front();

		if (!free)
		{
			if (resources[ResourceWood] < first_level.cost_wood ||
				resources[ResourceClay] < first_level.cost_clay ||
				resources[ResourceIron] < first_level.cost_iron ||
				resources[ResourceCrop] < first_level.cost_crop ||
				resources[ResourceGold] < first_level.cost_gold ||
				provide_population < consume_population + first_level.cost_population)
				return false;

			resources[ResourceWood] -= first_level.cost_wood;
			resources[ResourceClay] -= first_level.cost_clay;
			resources[ResourceIron] -= first_level.cost_iron;
			resources[ResourceCrop] -= first_level.cost_crop;
			resources[ResourceGold] -= first_level.cost_gold;
			consume_population += first_level.cost_population;
		}

		ResourceField resource_field;
		resource_field.tile_id = tile_id;
		resource_field.lv = 1;
		resource_field.type = type;
		resource_field.production = first_level.production;
		resource_fields.push_back(resource_field);

		tile.type = TileResourceField;
		tile.lord_id = id;

		return true;
	}

	bool build_city(uint tile_id)
	{
		City city;
		city.id = cities.size();
		city.tile_id = tile_id;
		city.lord_id = id;
		city.loyalty = 100;
		city.buildings.resize(building_slots.size());
		for (auto i = 0; i < building_slots.size(); i++)
		{
			auto type = building_slots[i].type;
			auto& building = city.buildings[i];
			building.slot = i;
			building.type = type;
			if (type == BuildingTownCenter)
				upgrade_building(city, i, true);
		}
		city.captures = { 1, 4, 7 };

		auto& tile = tiles[tile_id];
		tile.type = TileCity;
		tile.lord_id = id;

		cities.push_back(city);
		update_territories();
		return true;
	}

	bool upgrade_resource_field(ResourceField& resource_field, bool free = false)
	{
		if (resource_field_datas[resource_field.type].size() <= resource_field.lv)
			return false;

		auto& next_level = resource_field_datas[resource_field.type][resource_field.lv];

		if (!free)
		{
			if (resources[ResourceWood] < next_level.cost_wood ||
				resources[ResourceClay] < next_level.cost_clay ||
				resources[ResourceIron] < next_level.cost_iron ||
				resources[ResourceCrop] < next_level.cost_crop ||
				resources[ResourceGold] < next_level.cost_gold ||
				provide_population < consume_population + next_level.cost_population)
				return false;

			resources[ResourceWood] -= next_level.cost_wood;
			resources[ResourceClay] -= next_level.cost_clay;
			resources[ResourceIron] -= next_level.cost_iron;
			resources[ResourceCrop] -= next_level.cost_crop;
			resources[ResourceGold] -= next_level.cost_gold;
			consume_population += next_level.cost_population;
		}

		resource_field.lv++;
		resource_field.production = next_level.production;

		return true;
	}

	bool upgrade_building(City& city, uint slot, bool free = false)
	{
		auto& building = city.buildings[slot];
		auto type = building.type;

		if (!free)
		{
			if (auto base_data = get_building_base_data(type, building.lv); base_data)
			{
				if (resources[ResourceWood] < base_data->cost_wood ||
					resources[ResourceClay] < base_data->cost_clay ||
					resources[ResourceIron] < base_data->cost_iron ||
					resources[ResourceCrop] < base_data->cost_crop ||
					resources[ResourceGold] < base_data->cost_gold ||
					provide_population < consume_population + base_data->cost_population)
					return false;

				resources[ResourceWood] -= base_data->cost_wood;
				resources[ResourceClay] -= base_data->cost_clay;
				resources[ResourceIron] -= base_data->cost_iron;
				resources[ResourceCrop] -= base_data->cost_crop;
				resources[ResourceGold] -= base_data->cost_gold;
				consume_population += base_data->cost_population;
			}
		}

		switch (building.type)
		{
		case BuildingHouse:
		{
			auto& data = house_datas[building.lv];
			if (building.lv > 0)
				provide_population -= house_datas[building.lv - 1].provide_population;
			provide_population += data.provide_population;
		}
			break;
		}
		building.lv++;

		return true;
	}

	bool buy_unit(City& city, uint unit_id)
	{
		auto& unit_data = unit_datas[unit_id];
		if (resources[ResourceGold] < unit_data.cost_gold ||
			provide_population < consume_population + unit_data.cost_population)
			return false;

		resources[ResourceGold] -= unit_data.cost_gold;
		consume_population += unit_data.cost_population;

		Unit unit;
		unit.id = unit_id;
		unit.HP = unit_data.HP;
		unit.HP_MAX = unit.HP;
		unit.HP = unit_data.HP;
		unit.ATK = unit_data.ATK;
		unit.DEF = unit_data.DEF;
		unit.SA = unit_data.SA;
		unit.SD = unit_data.SD;
		unit.SP = unit_data.SP;
		city.units.push_back(unit);
	}

	uint get_production(ResourceType type)
	{
		uint ret = 0;
		for (auto& field : resource_fields)
		{
			if (field.type == type)
				ret += field.production;
		}
		return ret;
	}
};
std::vector<Lord> lords;

int add_lord(uint tile_id)
{
	Lord lord;
	lord.id = lords.size();

	lord.resources[ResourceWood] = 800;
	lord.resources[ResourceClay] = 800;
	lord.resources[ResourceIron] = 800;
	lord.resources[ResourceCrop] = 800;
	lord.resources[ResourceGold] = 5000;
	lord.provide_population = 10;
	lord.consume_population = 0;

	lord.build_city(tile_id);

	auto id = lords.size();
	lords.push_back(lord);
	return id;
}

int search_lord_location()
{
	std::vector<uint> candidates;
	for (auto i = 0; i < tiles.size(); i++)
	{
		auto& tile = tiles[i];
		if (tile.type == TileField)
		{
			if (tile.x > 0 && tile.y > 0 && tile.x < tile_cx - 1 && tile.y < tile_cy - 1)
			{
				auto ok = true;
				for (auto& lord : lords)
				{
					for (auto& city : lord.cities)
					{
						if (distance(tiles[city.tile_id].pos, tile.pos) < tile_sz * 4.5f)
						{
							ok = false;
							break;
						}
					}
				}
				if (ok)
					candidates.push_back(i);
			}
		}
	}
	if (candidates.empty())
		return -1;
	auto idx = linearRand(0, (int)candidates.size() - 1);
	return candidates[idx];
}

City* search_random_hostile_city(uint lord_id)
{
	if (lords.size() < 2)
		return nullptr;

	std::vector<uint> candidates;
	for (auto i = 0; i < lords.size(); i++)
	{
		if (i != lord_id)
			candidates.push_back(i);
	}
	auto& target_lord = lords[candidates[linearRand(0, (int)candidates.size() - 1)]];
	if (target_lord.cities.empty())
		return nullptr;
	return &target_lord.cities[linearRand(0, (int)target_lord.cities.size() - 1)];
}

Unit& Troop::get_unit(uint idx)
{
	auto& lord = lords[lord_id];
	auto& city = lord.cities[city_id];
	return city.units[unit_ids[idx]];
}


cvec4 hsv(float h, float s, float v, float a)
{
	return cvec4(vec4(rgbColor(vec3(h, s, v)), a) * 255.f);
}

GameState state = GameInit;
BattleTroop battle_troops[2];
uint battle_action_side = 0;
float troop_move_time = 0.f;
float last_troop_moved_time = 0.f;
float troop_anim_time = 0.f;
float battle_time = 0.f;
float anim_remain = 0;

bool recruit_window_opened = false;
void show_recruit_window(Lord& main_player, sRendererPtr renderer, const vec2& screen_size)
{
	if (selected_tile == -1)
	{
		recruit_window_opened = false;
		return;
	}
	auto& tile = tiles[selected_tile];
	if (tile.type != TileCity || tile.lord_id != main_player.id)
	{
		recruit_window_opened = false;
		return;
	}

	renderer->hud_begin(vec2(0.f, 24.f), screen_size - vec2(0.f, 24.f), cvec4(0, 0, 0, 127));
	auto& lord = lords[tile.lord_id];
	auto& city = lord.cities[lord.find_city(selected_tile)];
	renderer->hud_begin_layout(HudHorizontal);
	for (auto i = 0; i < city.captures.size(); i++)
	{
		auto& unit_data = unit_datas[city.captures[i]];
		if (unit_data.icon)
		{
			renderer->hud_image_button(vec2(128.f), unit_data.icon);
			if (renderer->hud_item_hovered())
				;
		}
	}
	renderer->hud_end_layout();
	renderer->hud_end();
}

void new_day()
{
	if (state == GameDay)
		return;
	state = GameDay;

	for (auto& lord : lords)
	{
		lord.troops.clear();

		for (auto& city : lord.cities)
		{
			for (auto& building : city.buildings)
			{
				switch (building.type)
				{
				case BuildingPark:
				{
					if (building.lv > 0)
					{
						auto& park_data = park_datas[building.lv - 1];
						auto list_sz = (int)park_data.encounter_list.size();
						for (auto i = 0; i < park_data.capture_num; i++)
							city.captures.push_back(park_data.encounter_list[linearRand(0, list_sz - 1)]);
					}
				}
					break;
				}
			}

			if (auto target_city = search_random_hostile_city(lord.id); target_city)
			{
				Troop troop;
				troop.lord_id = lord.id;
				troop.city_id = city.id;
				troop.path = find_path(city.tile_id, target_city->tile_id);
				if (!troop.path.empty())
				{
					for (auto i = 0; i < city.units.size(); i++)
						troop.unit_ids.push_back(i);
					if (!troop.unit_ids.empty())
						lord.troops.push_back(troop);
				}
			}
		}
	}
}

void start_battle()
{
	if (state == GameNight)
		return;
	state = GameNight;
	troop_move_time = 0.f;
	last_troop_moved_time = 0.f;

	for (auto& lord : lords)
	{
		for (auto& city : lord.cities)
			city.captures.clear();
	}
}

void step_troop_moving()
{
	auto no_troops = true;
	for (auto& lord : lords)
	{
		if (!lord.troops.empty())
		{
			no_troops = false;
			break;
		}
	}
	if (no_troops)
	{
		new_day();
		return;
	}

	for (auto& lord : lords)
	{
		for (auto& troop : lord.troops)
		{
			if (troop.idx < troop.path.size() - 1)
				troop.idx++;
		}
	}

	for (auto i = 0; i < lords.size(); i++)
	{
		auto& lord = lords[i];
		for (auto j = 0; j < lord.troops.size(); j++)
		{
			auto& troop = lord.troops[j];
			for (auto ii = i + 1; ii < lords.size(); ii++)
			{
				auto& _lord = lords[ii];
				for (auto jj = 0; jj < _lord.troops.size(); jj++)
				{
					auto& _troop = _lord.troops[jj];
					if (troop.path[troop.idx] == _troop.path[_troop.idx])
					{
						state = GameBattle;
						{
							auto& battle_troop = battle_troops[0];
							battle_troop.side = 0;
							battle_troop.troop = &troop;
							battle_troop.action_idx = 0;
							battle_troop.refresh_display();
						}
						{
							auto& battle_troop = battle_troops[1];
							battle_troop.side = 1;
							battle_troop.troop = &_troop;
							battle_troop.action_idx = 0;
							battle_troop.refresh_display();
						}
						battle_action_side = 0;
						return;
					}
				}
			}
		}
	}

	for (auto i = 0; i < lords.size(); i++)
	{
		auto& lord = lords[i];
		for (auto j = 0; j < lord.troops.size(); j++)
		{
			auto& troop = lord.troops[j];
			for (auto ii = i + 1; ii < lords.size(); ii++)
			{
				auto& _lord = lords[ii];
				for (auto jj = 0; jj < _lord.troops.size(); jj++)
				{
					auto& _troop = _lord.troops[jj];
					if (troop.path[troop.idx] == _troop.path[_troop.idx < _troop.path.size() - 1 ? _troop.idx + 1 : _troop.idx] &&
						_troop.path[_troop.idx] == troop.path[troop.idx < troop.path.size() - 1 ? troop.idx + 1 : troop.idx])
					{
						state = GameBattle;
						{
							auto& battle_troop = battle_troops[0];
							battle_troop.side = 0;
							battle_troop.troop = &troop;
							battle_troop.action_idx = 0;
							battle_troop.refresh_display();
						}
						{
							auto& battle_troop = battle_troops[1];
							battle_troop.side = 1;
							battle_troop.troop = &_troop;
							battle_troop.action_idx = 0;
							battle_troop.refresh_display();
						}
						battle_action_side = 0;
						return;
					}
				}
			}
		}
	}

	for (auto& lord : lords)
	{
		for (auto it = lord.troops.begin(); it != lord.troops.end(); )
		{
			if (it->idx == it->path.size() - 1)
			{
				auto tile_id = it->path.back();
				for (auto& _lord : lords)
				{
					if (auto id = _lord.find_city(tile_id); id != -1)
					{
						auto& city = _lord.cities[id];
						auto damage = 0;
						for (auto& unit : it->unit_ids)
							damage += 1;
						if (city.loyalty > damage)
							city.loyalty -= damage;
						else
							city.loyalty = 0;
						break;
					}
				}
				it = lord.troops.erase(it);
			}
			else
				it++;
		}
	}

	for (auto& lord : lords)
	{
		for (auto it = lord.cities.begin(); it != lord.cities.end(); )
		{
			if (it->loyalty == 0)
			{
				auto& tile = tiles[it->tile_id];
				tile.type = TileField;
				tile.lord_id = -1;
				for (auto it2 = lord.resource_fields.begin(); it2 != lord.resource_fields.end(); )
				{
					auto no_nearby_cities = true;
					auto& tile = tiles[it2->tile_id];
					if (no_nearby_cities && tile.tile_lt != -1)
					{
						auto& _tile = tiles[tile.tile_lt];
						if (_tile.type == TileCity && _tile.lord_id == lord.id)
							no_nearby_cities = false;
					}
					if (no_nearby_cities && tile.tile_t != -1)
					{
						auto& _tile = tiles[tile.tile_t];
						if (_tile.type == TileCity && _tile.lord_id == lord.id)
							no_nearby_cities = false;
					}
					if (no_nearby_cities && tile.tile_rt != -1)
					{
						auto& _tile = tiles[tile.tile_rt];
						if (_tile.type == TileCity && _tile.lord_id == lord.id)
							no_nearby_cities = false;
					}
					if (no_nearby_cities && tile.tile_lb != -1)
					{
						auto& _tile = tiles[tile.tile_lb];
						if (_tile.type == TileCity && _tile.lord_id == lord.id)
							no_nearby_cities = false;
					}
					if (no_nearby_cities && tile.tile_b != -1)
					{
						auto& _tile = tiles[tile.tile_b];
						if (_tile.type == TileCity && _tile.lord_id == lord.id)
							no_nearby_cities = false;
					}
					if (no_nearby_cities && tile.tile_rb != -1)
					{
						auto& _tile = tiles[tile.tile_rb];
						if (_tile.type == TileCity && _tile.lord_id == lord.id)
							no_nearby_cities = false;
					}
					if (no_nearby_cities)
					{
						tile.type = TileField;
						tile.lord_id = -1;
					}
				}
				it = lord.cities.erase(it);
			}
			else
				it++;
		}
	}

}

void step_battle()
{
	if (anim_remain > 0.f)
		return;
	anim_remain = 0.5f;

	if (battle_troops[0].troop->unit_ids.empty() || battle_troops[1].troop->unit_ids.empty())
	{
		for (auto i = 0; i < 2; i++)
		{
			if (battle_troops[i].troop->unit_ids.empty())
			{
				auto& lord = lords[battle_troops[i].troop->lord_id];
				for (auto it = lord.troops.begin(); it != lord.troops.end(); it++)
				{
					if (&(*it) == battle_troops[i].troop)
					{
						lord.troops.erase(it);
						break;
					}
				}
			}
		}
		state = GameNight;
		battle_troops[0].troop = battle_troops[1].troop = nullptr;
		return;
	}

	for (auto i = 0; i < 2; i++)
		battle_troops[i].refresh_display();

	auto unit_take_damage = [&](Unit& caster, Unit& target, uint damage) {
		if (target.HP > damage)
			target.HP -= damage;
		else
			target.HP = 0;
	};

	auto& action_troop = battle_troops[battle_action_side];
	auto& other_troop = battle_troops[1 - battle_action_side];
	auto& caster = action_troop.troop->get_unit(action_troop.action_idx);
	auto target_idx = linearRand(0, (int)other_troop.troop->unit_ids.size() - 1);
	auto& target = other_troop.troop->get_unit(target_idx);
	auto& cast_unit_display = action_troop.unit_displays[action_troop.action_idx];
	auto& target_unit_display = other_troop.unit_displays[target_idx];

	auto double_attacked = false;
	auto target_damage = caster.ATK;
	unit_take_damage(caster, target, target_damage);

	{
		auto id = game.tween->begin_2d_targets(1);
		game.tween->setup_2d_target(id, 0, &cast_unit_display.pos, nullptr, &cast_unit_display.scl, nullptr);
		auto target_pos = mix(cast_unit_display.pos, target_unit_display.pos, 0.9f);
		game.tween->scale_to(id, vec2(1.5f), 0.3f);
		game.tween->move_to(id, target_pos, 0.3f);
		game.tween->set_ease(id, EaseInCubic);
		game.tween->set_callback(id, [&, target_damage]() {
			target_unit_display.take_damage(target_damage);
		});
		game.tween->move_to(id, cast_unit_display.pos, 0.3f);
		game.tween->newline(id);
		game.tween->scale_to(id, vec2(1.f), 0.3f);
		game.tween->end(id);
		anim_remain = 1.3f;
	}

	action_troop.action_idx++;
	if (action_troop.action_idx >= action_troop.troop->unit_ids.size())
		action_troop.action_idx = 0;
	battle_action_side = 1 - battle_action_side;

	for (auto i = 0; i < 2; i++)
	{
		auto& battle_troop = battle_troops[i];
		for (auto j = 0; j < battle_troop.troop->unit_ids.size(); j++)
		{
			if (battle_troop.troop->get_unit(j).HP <= 0)
			{
				if (j < battle_troop.action_idx)
					battle_troop.action_idx--;
				battle_troop.troop->unit_ids.erase(battle_troop.troop->unit_ids.begin() + j);
				j--;
			}
		}
	}
}

void Game::init()
{
	create("Werewolf VS Vampire", uvec2(1280, 720), WindowStyleFrame, false, true, 
		{ {"mesh_shader"_h, 0} });

	auto root = world->root.get();
	root->add_component<cNode>();
	camera = root->add_component<cCamera>();

	renderer->add_render_task(RenderModeShaded, camera, main_window, {}, graphics::ImageLayoutPresent);
	canvas = renderer->render_tasks.front()->canvas;

	Path::set_root(L"assets", L"assets");

	img_be_hit = graphics::Image::get(L"assets/be_hit.png");
	img_tile_frame = graphics::Image::get(L"assets/HexTilesetv3.png_slices/00.png");
	img_tile_grass = graphics::Image::get(L"assets/HexTilesetv3.png_slices/01.png");
	img_tile_castle = graphics::Image::get(L"assets/HexTilesetv3.png_slices/216.png");
	img_city = graphics::Image::get(L"assets/city.png");
	img_wall = graphics::Image::get(L"assets/wall.png");
	img_ground = graphics::Image::get(L"assets/ground.png");
	img_town_center = graphics::Image::get(L"assets/town_center.png");
	img_house = graphics::Image::get(L"assets/house.png");
	img_barracks = graphics::Image::get(L"assets/barracks.png");
	img_park = graphics::Image::get(L"assets/park.png");
	img_tower = graphics::Image::get(L"assets/tower.png");
	img_resources[ResourceWood] = graphics::Image::get(L"assets/icons/wood.png");
	img_resources[ResourceClay] = graphics::Image::get(L"assets/icons/clay.png");
	img_resources[ResourceIron] = graphics::Image::get(L"assets/icons/iron.png");
	img_resources[ResourceCrop] = graphics::Image::get(L"assets/icons/crop.png");
	img_resources[ResourceGold] = graphics::Image::get(L"assets/icons/gold.png");
	img_population = graphics::Image::get(L"assets/icons/population.png");

	sp_repeat = graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressRepeat);

	tiles.resize(tile_cx * tile_cy);
	for (auto y = 0; y < tile_cy; y++)
	{
		for (auto x = 0; x < tile_cx; x++)
		{
			auto id = y * tile_cx + x;
			auto& tile = tiles[id];
			tile.id = id;
			tile.x = x; tile.y = y;
			tile.pos = vec2(x * tile_sz * 0.75f, y * tile_sz_y) + vec2(0.f, 36.f);
			tile.type = TileField;
			if (x % 2 == 1)
				tile.pos.y += tile_sz_y * 0.5f;
			
			if (x % 2 == 0)
			{
				if (x > 0 && y > 0)
					tile.tile_lt = id - tile_cx - 1;
				if (x < tile_cx - 1 && y > 0)
					tile.tile_rt = id - tile_cx + 1;
				if (x > 0 && y < tile_cy - 1)
					tile.tile_lb = id - 1;
				if (x < tile_cx - 1 && y < tile_cy - 1)
					tile.tile_rb = id + 1;
			}
			else
			{
				tile.tile_lt = id - 1;
				if (x < tile_cx - 1)
					tile.tile_rt = id + 1;
				if (y < tile_cy - 1)
					tile.tile_lb = id + tile_cx - 1;
				if (x < tile_cx - 1 && y < tile_cy - 1)
					tile.tile_rb = id + tile_cx + 1;
			}

			if (y > 0)
				tile.tile_t = id - tile_cx;
			if (y < tile_cy - 1)
				tile.tile_b = id + tile_cx;
		}
	}

	//if (auto sht = Sheet::get(L"assets/units.sht"); sht)
	//{
	//	for (auto i = 0; i < sht->rows.size(); i++)
	//	{
	//		UnitData data;
	//		auto& row = sht->rows[i];
	//		data.name = sht->get_as_wstr(row, "name"_h);
	//		data.cost_gold = sht->get_as<uint>(row, "cost_gold"_h);
	//		data.cost_population = sht->get_as<uint>(row, "cost_population"_h);
	//		data.HP = sht->get_as<uint>(row, "HP"_h);
	//		data.ATK = sht->get_as<uint>(row, "ATK"_h);
	//		data.DEF = sht->get_as<uint>(row, "DEF"_h);
	//		data.SA = sht->get_as<uint>(row, "SA"_h);
	//		data.SD = sht->get_as<uint>(row, "SD"_h);
	//		data.SP = sht->get_as<uint>(row, "SP"_h);
	//		data.icon = graphics::Image::get(L"assets/icons/troop/" + data.name + L".png");
	//		unit_datas.push_back(data);
	//	}
	//}
	if (auto sht = Sheet::get(L"assets/pokemon.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			UnitData data;
			auto& row = sht->rows[i];
			data.name = sht->get_as_wstr(row, "name"_h);
			data.cost_gold = sht->get_as<uint>(row, "cost_gold"_h);
			data.cost_population = sht->get_as<uint>(row, "cost_population"_h);
			data.HP = sht->get_as<uint>(row, "HP"_h);
			data.ATK = sht->get_as<uint>(row, "ATK"_h);
			data.DEF = sht->get_as<uint>(row, "DEF"_h);
			data.SA = sht->get_as<uint>(row, "SA"_h);
			data.SD = sht->get_as<uint>(row, "SD"_h);
			data.SP = sht->get_as<uint>(row, "SP"_h);
			{
				wchar_t buf[32];
				swprintf(buf, L"%03d", i + 1);
				data.icon = graphics::Image::get(L"assets/pokemon/" + std::wstring(buf) + L".png");
			}
			unit_datas.push_back(data);
		}
	}
	if (auto sht = Sheet::get(L"assets/building_slots.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			BuildingSlot slot;
			auto& row = sht->rows[i];
			slot.pos = sht->get_as<vec2>(row, "pos"_h);
			slot.radius = sht->get_as<float>(row, "radius"_h);
			auto building_name = sht->get_as_wstr(row, "building"_h);
			for (auto j = 0; j < BuildingTypeCount; j++)
			{
				if (building_name == get_building_name((BuildingType)j))
				{
					slot.type = (BuildingType)j;
					break;
				}
			}
			building_slots.push_back(slot);
		}
	}
	building_slots.push_back({ .type = BuildingTower });
	building_slots.push_back({ .type = BuildingWall });
	if (auto sht = Sheet::get(L"assets/town_center.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			TownCenterData data;
			auto& row = sht->rows[i];
			data.read(row, sht);
			town_center_datas.push_back(data);
		}
	}
	if (auto sht = Sheet::get(L"assets/house.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			HouseData data;
			auto& row = sht->rows[i];
			data.read(row, sht);
			data.provide_population = sht->get_as<uint>(row, "provide_population"_h);
			house_datas.push_back(data);
		}
	}
	if (auto sht = Sheet::get(L"assets/barracks.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			BarracksData data;
			auto& row = sht->rows[i];
			data.read(row, sht);
			barracks_datas.push_back(data);
		}
	}
	if (auto sht = Sheet::get(L"assets/park.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			ParkData data;
			auto& row = sht->rows[i];
			data.read(row, sht);
			data.encounter_list = sht->get_as<std::vector<uint>>(row, "encounter_list"_h);
			data.capture_num = sht->get_as<uint>(row, "capture_num"_h);

			park_datas.push_back(data);
		}
	}
	if (auto sht = Sheet::get(L"assets/tower.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			TowerData data;
			auto& row = sht->rows[i];
			data.read(row, sht);
			tower_datas.push_back(data);
		}
	}
	if (auto sht = Sheet::get(L"assets/wall.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			WallData data;
			auto& row = sht->rows[i];
			data.read(row, sht);
			wall_datas.push_back(data);
		}
	}
	if (auto sht = Sheet::get(L"assets/woodcutter.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			ResourceFieldData data;
			auto& row = sht->rows[i];
			data.read(row, sht);
			resource_field_datas[ResourceWood].push_back(data);
		}
	}
	if (auto sht = Sheet::get(L"assets/clay_pit.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			ResourceFieldData data;
			auto& row = sht->rows[i];
			data.read(row, sht);
			resource_field_datas[ResourceClay].push_back(data);
		}
	}
	if (auto sht = Sheet::get(L"assets/iron_mine.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			ResourceFieldData data;
			auto& row = sht->rows[i];
			data.read(row, sht);
			resource_field_datas[ResourceIron].push_back(data);
		}
	}
	if (auto sht = Sheet::get(L"assets/crop_field.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			ResourceFieldData data;
			auto& row = sht->rows[i];
			data.read(row, sht);
			resource_field_datas[ResourceCrop].push_back(data);
		}
	}

	// give a unit for testing
	if (auto tile_id = search_lord_location(); tile_id != -1)
	{
		if (auto id = add_lord(tile_id); id != -1)
		{
			auto& lord = lords[id];
			auto& city = lord.cities.front();
			lord.buy_unit(city, 0);
		}
	}
	if (auto tile_id = search_lord_location(); tile_id != -1)
	{
		if (auto id = add_lord(tile_id); id != -1)
		{
			auto& lord = lords[id];
			auto& city = lord.cities.front();
			lord.buy_unit(city, 0);
		}
	}

	new_day();
}

void Game::on_render()
{
	if (lords.empty())
		return;

	auto& main_player = lords.front();
	auto screen_size = canvas->size;
	auto mpos = input->mpos;

	if (anim_remain > 0.f)
		anim_remain -= delta_time;
	troop_anim_time += delta_time;

	switch (state)
	{
	case GameNight:
		troop_move_time += delta_time;
		if (troop_move_time - last_troop_moved_time >= 0.5f)
		{
			last_troop_moved_time += 0.5f;
			step_troop_moving();
		}
		break;
	case GameBattle:
		step_battle();
		break;
	}

	if (state == GameBattle)
	{
		canvas->draw_rect_filled(vec2(100.f, 236.f), vec2(500.f, 336.f), hsv(battle_troops[0].troop->lord_id * 60.f, 0.5f, 0.5f, 1.f));
		canvas->draw_rect_filled(vec2(100.f, 36.f), vec2(500.f, 136.f), hsv(battle_troops[1].troop->lord_id * 60.f, 0.5f, 0.5f, 1.f));

		for (auto i = 0; i < 2; i++)
		{
			auto& battle_troop = battle_troops[i];
			auto& lord = lords[battle_troop.troop->lord_id];
			for (auto& display : battle_troop.unit_displays)
			{
				auto& unit_data = unit_datas[display.unit_id];
				auto pos = vec2(display.pos);
				if (unit_data.icon)
					draw_image(unit_data.icon, pos, vec2(64.f) * display.scl.x, vec2(0.5f, 1.f), cvec4(255, 255, 255, 255 * display.alpha));
				//draw_text(std::format(L"{}", display.ATK), 24, pos - vec2(21.f, 0.f), vec2(0.5f, 1.f), cvec4(255, 255, 255, 255 * display.alpha));
				draw_text(std::format(L"{}", display.HP), 24, pos + vec2(21.f, 0.f), vec2(0.5f, 1.f), display.HP > 0 ? cvec4(255, 255, 255, 255 * display.alpha) : cvec4(255, 0, 0, 255 * display.alpha));
				if (display.damage_label_remain > 0.f)
				{
					draw_image(img_be_hit, pos - vec2(0.f, 32.f), vec2(50.f), vec2(0.5f, 0.5f));
					draw_text(std::format(L"-{}", display.damage), 40, pos - vec2(0.f, 32.f), vec2(0.5f, 0.5f), cvec4(0, 0, 0, 255), -vec2(2.f), cvec4(255));
					display.damage_label_remain -= delta_time;
					if (display.damage_label_remain <= 0.f)
						display.damage = 0;
				}
				if (display.state_text_remain > 0.f)
				{
					draw_text(display.state_text, 20, vec2(display.init_pos) + vec2(0.f, 5.f), vec2(0.5f, 0.f), cvec4(0, 0, 0, 255), -vec2(1.f), cvec4(255));
					display.state_text_remain -= delta_time;
					if (display.state_text_remain <= 0.f)
						display.state_text = L"";
				}
			}
		}
	}
	else
	{
		if (!recruit_window_opened)
		{
			for (auto& tile : tiles)
			{
				draw_image(img_tile_grass, tile.pos, vec2(tile_sz, tile_sz_y));
				//canvas->draw_text(nullptr, 16, tile.pos + vec2(10.f), wstr(tile.id), cvec4(255));
			}
			for (auto& lord : lords)
			{
				for (auto& city : lord.cities)
				{
					auto& tile = tiles[city.tile_id];
					draw_image(img_city, tile.pos, vec2(tile_sz, tile_sz_y));
					draw_text(wstr(city.loyalty), 20, tile.pos + vec2(tile_sz, tile_sz_y) * 0.5f + vec2(0.f, -10.f), vec2(0.5f), cvec4(0, 255, 0, 255));
				}
				for (auto& field : lord.resource_fields)
				{
					auto pos = tiles[field.tile_id].pos + vec2(tile_sz, tile_sz_y) * 0.5f;
					draw_image(img_resources[field.type], pos, vec2(36.f, 24.f), vec2(0.5f));
				}
				{
					std::vector<vec2> strips;
					for (auto id : lord.territories)
					{
						auto& tile = tiles[id];
						vec2 pos[6];
						for (auto i = 0; i < 6; i++)
							pos[i] = arc_point(tile.pos + vec2(tile_sz, tile_sz_y) * 0.5f, i * 60.f, tile_sz * 0.5f);
						if (tile.tile_rb == -1 || !lord.has_territory(tile.tile_rb))
							make_line_strips<2>(pos[0], pos[1], strips);
						if (tile.tile_b == -1 || !lord.has_territory(tile.tile_b))
							make_line_strips<2>(pos[1], pos[2], strips);
						if (tile.tile_lb == -1 || !lord.has_territory(tile.tile_lb))
							make_line_strips<2>(pos[2], pos[3], strips);
						if (tile.tile_lt == -1 || !lord.has_territory(tile.tile_lt))
							make_line_strips<2>(pos[3], pos[4], strips);
						if (tile.tile_t == -1 || !lord.has_territory(tile.tile_t))
							make_line_strips<2>(pos[4], pos[5], strips);
						if (tile.tile_rt == -1 || !lord.has_territory(tile.tile_rt))
							make_line_strips<2>(pos[5], pos[0], strips);
					}
					canvas->path = strips;
					canvas->stroke(2.f, hsv(lord.id * 60.f, 0.5f, 1.f, 0.5f), false);
				}
				for (auto& troop : lord.troops)
				{
					vec2 end_point;
					for (auto i = 0; i <= troop.idx; i++)
					{
						auto id = troop.path[i];
						canvas->path.push_back(tiles[id].pos + vec2(tile_sz) * 0.5f);
					}
					end_point = canvas->path.back();
					if (auto idx = troop.idx + 1; idx < troop.path.size())
					{
						auto t = fract(troop_move_time * 2.f);
						end_point = mix(end_point, tiles[troop.path[idx]].pos + vec2(tile_sz) * 0.5f, t);
						canvas->path.push_back(end_point);
					}
					canvas->stroke(4.f, hsv(lord.id * 60.f, 0.5f, 0.8f, 1.f), false);
					for (auto i = 0; i < troop.path.size() - 1; i++)
					{
						for (auto j = 0; j < 4; j++)
						{
							if (abs(int(i * 4 + j - troop_anim_time * 12.f) % 20) < 4)
							{
								auto a = tiles[troop.path[i]].pos;
								auto b = tiles[troop.path[i + 1]].pos;
								canvas->draw_circle_filled(mix(a, b, j / 4.f) + vec2(tile_sz) * 0.5f, 3.f, hsv(lord.id * 60.f, 0.5f, 0.8f, 0.8f));
							}
						}
					}
					canvas->draw_circle_filled(end_point, 6.f, hsv(lord.id * 60.f, 0.5f, 1.f, 1.f));
				}
			}

			if (input->mpressed(Mouse_Left))
			{
				for (auto i = 0; i < tiles.size(); i++)
				{
					auto& tile = tiles[i];
					if (distance(tile.pos + tile_sz * 0.5f, mpos) < tile_sz * 0.5f)
					{
						selected_tile = i;
						break;
					}
				}
			}
		}

		renderer->hud_begin(vec2(0.f, 0.f), vec2(screen_size.x, 20.f), cvec4(0, 0, 0, 255));
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(16.f, 0.f));
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_resources[ResourceWood]);
		renderer->hud_text(std::format(L"{} +{}", main_player.resources[ResourceWood], main_player.get_production(ResourceWood)), 24);
		renderer->hud_end_layout();
		if (renderer->hud_item_hovered())
		{
			renderer->hud_begin(mpos + vec2(0.f, 10.f));
			renderer->hud_text(std::format(L"Wood: {}\nProduction: {}", main_player.resources[ResourceWood], main_player.get_production(ResourceWood)));
			renderer->hud_end();
		}
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_resources[ResourceClay]);
		renderer->hud_text(std::format(L"{} +{}", main_player.resources[ResourceClay], main_player.get_production(ResourceClay)), 24);
		renderer->hud_end_layout();
		if (renderer->hud_item_hovered())
		{
			renderer->hud_begin(mpos + vec2(0.f, 10.f));
			renderer->hud_text(std::format(L"Clay: {}\nProduction: {}", main_player.resources[ResourceClay], main_player.get_production(ResourceClay)));
			renderer->hud_end();
		}
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_resources[ResourceIron]);
		renderer->hud_text(std::format(L"{} +{}", main_player.resources[ResourceIron], main_player.get_production(ResourceIron)), 24);
		renderer->hud_end_layout();
		if (renderer->hud_item_hovered())
		{
			renderer->hud_begin(mpos + vec2(0.f, 10.f));
			renderer->hud_text(std::format(L"Iron: {}\nProduction: {}", main_player.resources[ResourceIron], main_player.get_production(ResourceIron)));
			renderer->hud_end();
		}
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_resources[ResourceCrop]);
		renderer->hud_text(std::format(L"{} +{}", main_player.resources[ResourceCrop], main_player.get_production(ResourceCrop)), 24);
		renderer->hud_end_layout();
		if (renderer->hud_item_hovered())
		{
			renderer->hud_begin(mpos + vec2(0.f, 10.f));
			renderer->hud_text(std::format(L"Crop: {}\nProduction: {}", main_player.resources[ResourceCrop], main_player.get_production(ResourceCrop)));
			renderer->hud_end();
		}
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_resources[ResourceGold]);
		renderer->hud_text(std::format(L"{}", main_player.resources[ResourceGold]), 24);
		renderer->hud_end_layout();
		if (renderer->hud_item_hovered())
		{
			renderer->hud_begin(mpos + vec2(0.f, 10.f));
			renderer->hud_text(std::format(L"Gold: {}", main_player.resources[ResourceGold]));
			renderer->hud_end();
		}
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_population);
		renderer->hud_text(std::format(L"{}/{}", main_player.consume_population, main_player.provide_population), 24);
		renderer->hud_end_layout();
		if (renderer->hud_item_hovered())
		{
			renderer->hud_begin(mpos + vec2(0.f, 10.f));
			renderer->hud_text(std::format(L"Population\nProvide: {}\nConsume: {}", main_player.provide_population, main_player.consume_population));
			renderer->hud_end();
		}
		renderer->hud_end_layout();
		renderer->hud_end();

		if (!recruit_window_opened)
		{
			auto bottom_pannel_height = 250.f;
			renderer->hud_begin(vec2(0.f, screen_size.y - bottom_pannel_height), vec2(screen_size.x, bottom_pannel_height), cvec4(0, 0, 0, 255));
			auto rect = renderer->hud_hud_rect();
			if (selected_tile != -1)
			{
				auto& tile = tiles[selected_tile];
				switch (tile.type)
				{
				case TileField:
					if (main_player.has_territory(selected_tile))
					{
						renderer->hud_begin_layout(HudHorizontal);

						auto show_build_resource_field = [&](ResourceType type) {
							renderer->hud_begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 8.f));
							renderer->hud_text(get_resource_field_name(type));
							if (!resource_field_datas[type].empty())
							{
								auto& first_level = resource_field_datas[type].front();

								renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
								renderer->hud_text(L"Production: ", 22);
								renderer->hud_image(vec2(20.f, 13.f), img_resources[type]);
								renderer->hud_text(std::format(L"{}", first_level.production), 22);
								renderer->hud_end_layout();

								renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceWood]);
								renderer->hud_text(std::format(L"{}", first_level.cost_wood), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceClay]);
								renderer->hud_text(std::format(L"{}", first_level.cost_clay), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceIron]);
								renderer->hud_text(std::format(L"{}", first_level.cost_iron), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
								renderer->hud_text(std::format(L"{}", first_level.cost_crop), 16);
								renderer->hud_end_layout();
								renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceGold]);
								renderer->hud_text(std::format(L"{}", first_level.cost_gold), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_population);
								renderer->hud_text(std::format(L"{}", first_level.cost_population), 16);
								renderer->hud_end_layout();

								if (renderer->hud_button(L"Build", 18))
									main_player.build_resource_field(selected_tile, type);
							}
							renderer->hud_end_layout();
							renderer->hud_stroke_item();
						};

						show_build_resource_field(ResourceWood);
						show_build_resource_field(ResourceClay);
						show_build_resource_field(ResourceIron);
						show_build_resource_field(ResourceCrop);

						renderer->hud_end_layout();
					}
					break;
				case TileCity:
				{
					auto circle_sz = 120.f;
					auto c = rect.a + vec2(circle_sz + 20.f, circle_sz);
					vec2 corner_pos[6];
					vec2 wall_rect[4 * 6];
					for (auto i = 0; i < 6; i++)
						corner_pos[i] = arc_point(c, i * 60.f, circle_sz);
					for (auto i = 0; i < 6; i++)
					{
						auto a = corner_pos[i];
						auto b = corner_pos[i + 1 == 6 ? 0 : i + 1];
						auto d = normalize(b - a);
						auto n = vec2(d.y, -d.x);
						wall_rect[4 * i + 0] = a - n * 4.f;
						wall_rect[4 * i + 1] = a + n * 4.f;
						wall_rect[4 * i + 2] = b + n * 4.f;
						wall_rect[4 * i + 3] = b - n * 4.f;
					}

					auto hovering_slot = -1;
					auto& lord = lords[tile.lord_id];
					auto& city = lord.cities[lord.find_city(selected_tile)];

					{
						auto ok = false;
						for (auto i = 0; i < building_slots.size(); i++)
						{
							auto& slot = building_slots[i];
							if (slot.type > BuildingInTownEnd)
								break;
							if (distance(mpos, c + slot.pos) < slot.radius)
							{
								hovering_slot = i;
								ok = true;
								break;
							}
						}
						if (!ok)
						{
							for (auto i = 0; i < 6; i++)
							{
								if (distance(mpos, corner_pos[i]) < 8.f)
								{
									hovering_slot = building_slots.size() - 2;
									ok = true;
									break;
								}
							}
						}
						if (!ok)
						{
							for (auto i = 0; i < 6; i++)
							{
								if (convex_contains(mpos, { &wall_rect[4 * i], &wall_rect[4 * i + 4] }))
									hovering_slot = building_slots.size() - 1;
							}
						}
					}

					canvas->draw_text(nullptr, 24, rect.a + vec2(0.5f, -16.f), lord.id == main_player.id ? L"Your City" : L"Enemy's City");

					{
						std::vector<vec2> pts;
						std::vector<vec2> uvs;
						pts.resize(6);
						uvs.resize(6);
						for (auto i = 0; i < 6; i++)
						{
							pts[i] = corner_pos[i];
							uvs[i] = (corner_pos[i] - c + vec2(circle_sz)) / circle_sz * 3.f;
						}
						canvas->draw_image_polygon(img_ground->get_view(), pts, uvs, cvec4(255), sp_repeat);
					}

					{
						auto col = cvec4(255);
						if (hovering_slot == building_slots.size() - 1)
							col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
						if (city.get_building_lv(BuildingWall) == 0)
							col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
						for (auto i = 0; i < 6; i++)
						{
							canvas->draw_image_polygon(img_wall->get_view(), { wall_rect[4 * i + 0], wall_rect[4 * i + 3], wall_rect[4 * i + 2], wall_rect[4 * i + 1] },
								{ vec2(0.f, 1.f), vec2(2.f, 1.f), vec2(2.f, 0.f), vec2(0.f, 0.f) }, col, sp_repeat);
						}
					}

					renderer->hud_begin_layout(HudHorizontal);
					renderer->hud_rect(vec2(circle_sz * 2.f + 28.f, bottom_pannel_height - 8.f), cvec4(0));

					{
						auto col = cvec4(255);
						if (hovering_slot == building_slots.size() - 2)
							col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
						if (city.get_building_lv(BuildingTower) == 0)
							col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
						for (auto i = 0; i < 6; i++)
							draw_image(img_tower, corner_pos[i], vec2(img_tower->extent) * 0.5f, vec2(0.5f, 0.8f), col);
					}

					for (auto i = 0; i < building_slots.size(); i++)
					{
						auto& slot = building_slots[i];
						if (slot.type > BuildingInTownEnd)
							break;
						auto col = cvec4(255);
						if (hovering_slot == i)
							col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
						if (city.get_building_lv(slot.type, i) == 0)
							col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
						switch (slot.type)
						{
						case BuildingTownCenter:
							draw_image(img_town_center, c + slot.pos, vec2(img_town_center->extent) * 0.5f, vec2(0.5f, 0.8f), col);
							break;
						case BuildingHouse:
							draw_image(img_house, c + slot.pos, vec2(img_house->extent) * 0.5f, vec2(0.5f, 0.8f), col);
							break;
						case BuildingBarracks:
							draw_image(img_barracks, c + slot.pos, vec2(img_barracks->extent) * 0.5f, vec2(0.5f, 0.8f), col);
							break;
						case BuildingPark:
							draw_image(img_park, c + slot.pos, vec2(img_park->extent) * 0.5f, vec2(0.5f, 0.8f), col);
							break;
						}
					}

					if (lord.id == main_player.id)
					{
						if (input->mpressed(Mouse_Left))
						{
							if (hovering_slot != -1)
								selected_building_slot = hovering_slot;
						}

						if (selected_building_slot != -1)
						{
							auto& building = city.buildings[selected_building_slot];
							auto type = building_slots[selected_building_slot].type;
							auto show_upgrade_building = [&]() {
								if (auto next_level = get_building_base_data(type, building.lv); next_level)
								{
									renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
									renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceWood]);
									renderer->hud_text(std::format(L"{}", next_level->cost_wood), 16, main_player.resources[ResourceWood] >= next_level->cost_wood ? cvec4(255) : cvec4(255, 0, 0, 255));
									renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceClay]);
									renderer->hud_text(std::format(L"{}", next_level->cost_clay), 16, main_player.resources[ResourceClay] >= next_level->cost_clay ? cvec4(255) : cvec4(255, 0, 0, 255));
									renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceIron]);
									renderer->hud_text(std::format(L"{}", next_level->cost_iron), 16, main_player.resources[ResourceIron] >= next_level->cost_iron ? cvec4(255) : cvec4(255, 0, 0, 255));
									renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
									renderer->hud_text(std::format(L"{}", next_level->cost_crop), 16, main_player.resources[ResourceCrop] >= next_level->cost_crop ? cvec4(255) : cvec4(255, 0, 0, 255));
									renderer->hud_end_layout();
									renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
									renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceGold]);
									renderer->hud_text(std::format(L"{}", next_level->cost_gold), 16, main_player.resources[ResourceGold] >= next_level->cost_gold ? cvec4(255) : cvec4(255, 0, 0, 255));
									renderer->hud_image(vec2(18.f, 12.f), img_population);
									renderer->hud_text(std::format(L"{}", next_level->cost_population), 16, (int)main_player.provide_population - (int)main_player.consume_population >= next_level->cost_population ? cvec4(255) : cvec4(255, 0, 0, 255));
									renderer->hud_end_layout();

									if (renderer->hud_button(building.lv == 0 ? L"Build" : L"Upgrade", 18))
										main_player.upgrade_building(city, selected_building_slot);
								}
							};
							switch (type)
							{
							case BuildingTownCenter:
								renderer->hud_begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 8.f));
								renderer->hud_text(std::format(L"Town Center LV: {}", building.lv));
								show_upgrade_building();
								renderer->hud_end_layout();
								renderer->hud_stroke_item();
								break;
							case BuildingHouse:
							{
								renderer->hud_begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 8.f));
								renderer->hud_text(std::format(L"House LV: {}", building.lv));
								if (building.lv > 0)
								{
									auto& data = house_datas[building.lv - 1];
									renderer->hud_text(std::format(L"Current Provide Population: {}", data.provide_population), 22);
								}
								auto next_level = house_datas[building.lv];
								renderer->hud_text(std::format(L"Level {} Provide Population: {}", building.lv + 1, next_level.provide_population), 22);
								show_upgrade_building();
								renderer->hud_end_layout();
								renderer->hud_stroke_item();
							}
								break;
							case BuildingBarracks:
							{
								renderer->hud_begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 8.f));
								renderer->hud_text(std::format(L"Barracks LV: {}", building.lv));
								show_upgrade_building();
								renderer->hud_end_layout();
								renderer->hud_stroke_item();
							}
								break;
							case BuildingPark:
							{
								renderer->hud_begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 8.f));
								renderer->hud_text(std::format(L"Park LV: {}", building.lv));
								show_upgrade_building();
								renderer->hud_end_layout();
								renderer->hud_stroke_item();
							}
								break;
							case BuildingTower:
								renderer->hud_begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 8.f));
								renderer->hud_text(std::format(L"Tower LV: {}", building.lv));
								show_upgrade_building();
								renderer->hud_end_layout();
								renderer->hud_stroke_item();
								break;
							case BuildingWall:
								renderer->hud_begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 8.f));
								renderer->hud_text(std::format(L"Wall LV: {}", building.lv));
								show_upgrade_building();
								renderer->hud_end_layout();
								renderer->hud_stroke_item();
								break;
							}
						}
					}

					renderer->hud_end_layout();
				}
					break;
				case TileResourceField:
					if (auto id = main_player.find_resource_field(selected_tile); id != -1)
					{
						auto& resource_field = main_player.resource_fields[id];
						if (!resource_field_datas[resource_field.type].empty())
						{
							renderer->hud_begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 8.f));
							renderer->hud_text(std::format(L"{} LV: {}", get_resource_field_name(resource_field.type), resource_field.lv));
							renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
							renderer->hud_text(L"Current Production: ", 22);
							renderer->hud_image(vec2(20.f, 13.f), img_resources[resource_field.type]);
							renderer->hud_text(std::format(L"{}", resource_field.production), 22);
							renderer->hud_end_layout();
							if (resource_field.lv < resource_field_datas[resource_field.type].size())
							{
								auto& next_level = resource_field_datas[resource_field.type][resource_field.lv];

								renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
								renderer->hud_text(std::format(L"Level {} Production: ", resource_field.lv + 1), 22);
								renderer->hud_image(vec2(20.f, 13.f), img_resources[resource_field.type]);
								renderer->hud_text(std::format(L"{}", next_level.production), 22);
								renderer->hud_end_layout();

								renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceWood]);
								renderer->hud_text(std::format(L"{}", next_level.cost_wood), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceClay]);
								renderer->hud_text(std::format(L"{}", next_level.cost_clay), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceIron]);
								renderer->hud_text(std::format(L"{}", next_level.cost_iron), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
								renderer->hud_text(std::format(L"{}", next_level.cost_crop), 16);
								renderer->hud_end_layout();
								renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceGold]);
								renderer->hud_text(std::format(L"{}", next_level.cost_gold), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_population);
								renderer->hud_text(std::format(L"{}", next_level.cost_population), 16);
								renderer->hud_end_layout();

								if (renderer->hud_button(L"Upgrade", 18))
									main_player.upgrade_resource_field(resource_field);
							}
							renderer->hud_end_layout();
							renderer->hud_stroke_item();
						}
					}
					break;
				}
			}
			renderer->hud_end();
		}
		else
			show_recruit_window(main_player, renderer, screen_size);

		renderer->hud_begin(vec2(screen_size.x - 100.f, screen_size.y - 300.f), vec2(160.f, 300.f), cvec4(0, 0, 0, 255));
		if (state == GameDay)
		{
			if (recruit_window_opened)
			{
				if (renderer->hud_button(L"Back"))
					recruit_window_opened = false;
			}
			else
			{
				if (renderer->hud_button(L"Recruit"))
					recruit_window_opened = true;
				if (renderer->hud_button(L"Start Battle"))
					start_battle();
			}
		}
		renderer->hud_end();
	}

	UniverseApplication::on_render();
}

void Game::on_gui()
{

}

Game game;

int entry(int argc, char** args)
{
	game.init();
	game.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
