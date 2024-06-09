#include "game.h"

#include <flame/xml.h>
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

BuildingType get_building_type_from_name(std::wstring_view name)
{
	for (auto i = 0; i < BuildingTypeCount; i++)
	{
		if (name == get_building_name((BuildingType)i))
			return (BuildingType)i;
	}
	return BuildingTypeCount;
}

enum PokemonType
{
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
	PokemonFairy,

	PokemonTypeCount
};

const wchar_t* get_pokemon_type_name(PokemonType type)
{
	switch (type)
	{
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

PokemonType get_pokemon_type_from_name(std::wstring_view name)
{
	for (auto i = 0; i < PokemonTypeCount; i++)
	{
		if (name == get_pokemon_type_name((PokemonType)i))
			return (PokemonType)i;
	}
	return PokemonTypeCount;
}

cvec4 get_pokemon_type_color(PokemonType type)
{
	switch (type)
	{
	case PokemonNormal: return cvec4(168, 168, 120, 255);
	case PokemonFire: return cvec4(240, 128, 48, 255);
	case PokemonWater: return cvec4(104, 144, 240, 255);
	case PokemonElectric: return cvec4(248, 208, 48, 255);
	case PokemonGrass: return cvec4(120, 200, 80, 255);
	case PokemonIce: return cvec4(152, 216, 216, 255);
	case PokemonFighting: return cvec4(192, 48, 40, 255);
	case PokemonPoison: return cvec4(160, 64, 160, 255);
	case PokemonGround: return cvec4(224, 192, 104, 255);
	case PokemonFlying: return cvec4(168, 144, 240, 255);
	case PokemonPsychic: return cvec4(248, 88, 136, 255);
	case PokemonBug: return cvec4(168, 184, 32, 255);
	case PokemonRock: return cvec4(184, 160, 56, 255);
	case PokemonGhost: return cvec4(112, 88, 152, 255);
	case PokemonDragon: return cvec4(112, 56, 248, 255);
	case PokemonDark: return cvec4(112, 88, 72, 255);
	case PokemonSteel: return cvec4(184, 184, 208, 255);
	case PokemonFairy: return cvec4(240, 182, 188, 255);
	}
	return cvec4(0, 0, 0, 255);
}

float pokemon_type_effectiveness[PokemonTypeCount][PokemonTypeCount]; // attacker, defender

enum SkillCategory
{
	SkillPhysical,
	SkillSpecial,
	SkillStatus,

	SkillCategoryCount
};

const wchar_t* get_skill_category_name(SkillCategory type)
{
	switch (type)
	{
	case SkillPhysical: return L"Physical";
	case SkillSpecial: return L"Special";
	case SkillStatus: return L"Status";
	}
	return L"";
}

SkillCategory get_skill_category_from_name(std::wstring_view name)
{
	for (auto i = 0; i < SkillCategoryCount; i++)
	{
		if (name == get_skill_category_name((SkillCategory)i))
			return (SkillCategory)i;
	}
	return SkillPhysical;
}

cCameraPtr camera;

graphics::CanvasPtr canvas;

graphics::ImagePtr img_target;

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

graphics::ImagePtr img_be_hit;

graphics::SamplerPtr sp_repeat;

void draw_rect(const vec2& pos, const vec2& sz, const vec2& pivot = vec2(0.f), const cvec4& color = cvec4(255))
{
	auto p = pos;
	p -= pivot * sz;
	canvas->draw_rect_filled(p, p + sz, color);
}

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
	if (start_id == end_id)
		return { start_id };
	std::vector<uint> ret;
	std::vector<bool> marks;
	marks.resize(tiles.size());
	std::vector<std::pair<uint, uint>> candidates;
	candidates.push_back({ start_id, 0 });
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

struct SkillData
{
	std::wstring name;
	PokemonType type;
	SkillCategory category;
	uint power;
	uint acc;
	uint pp;
	std::wstring effect;
};
std::vector<SkillData> skill_datas;

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
	PokemonType type1 = PokemonTypeCount;
	PokemonType type2 = PokemonTypeCount;
	std::vector<ivec2> skillset;
	graphics::ImagePtr icon = nullptr;
};
std::vector<UnitData> unit_datas;

struct Unit
{
	uint id;
	uint lv;
	uint exp;
	int skills[4] = { -1, -1, -1, -1 };
};

struct UnitInstance
{
	Unit* original;
	uint id;
	uint lv;
	int HP_MAX;
	int HP;
	int ATK;
	int DEF;
	int SA;
	int SD;
	int SP;
	PokemonType type1;
	PokemonType type2;
	int skills[4] = { -1, -1, -1, -1 };

	int choose_attack_skill()
	{
		auto n = 0;
		uint cands[4];
		for (auto i = 0; i < 4; i++)
		{
			auto skill_id = skills[i];
			if (skill_id != -1)
			{
				auto& skill_data = skill_datas[skill_id];
				if (skill_data.category != SkillStatus)
				{
					cands[n] = skill_id;
					n++;
				}
			}
		}
		if (n == 0)
			return -1;
		return cands[linearRand(0, n - 1)];
	}
};

const uint IV_VAL = 31;
const uint BP_VAL = 252;

uint calc_hp_stat(uint base, uint lv)
{
	return uint((base * 2 + IV_VAL + BP_VAL / 4) * lv / 100.f) + lv + 10;
}

uint calc_stat(uint base, uint lv)
{
	return uint((base * 2 + IV_VAL + BP_VAL / 4) * lv / 100.f) + 5;
}

uint calc_damage(UnitInstance& attacker, UnitInstance& defender, uint skill_id)
{
	auto& skill_data = skill_datas[skill_id];
	if (skill_data.category == SkillStatus)
		return 0;
	auto effectiveness1 = 1.f;
	if (defender.type1 != PokemonTypeCount)
		effectiveness1 = pokemon_type_effectiveness[skill_data.type][defender.type1];
	auto effectiveness2 = 1.f;
	if (defender.type2 != PokemonTypeCount)
		effectiveness2 = pokemon_type_effectiveness[skill_data.type][defender.type2];
	if (effectiveness1 * effectiveness2 == 0.f)
		return 0;
	auto effectiveness = effectiveness1 * effectiveness2;
	if (attacker.type1 == skill_data.type || attacker.type2 == skill_data.type)
		effectiveness *= 1.5f;
	auto A = skill_data.category == SkillPhysical ? attacker.ATK : attacker.SA;
	auto D = skill_data.category == SkillPhysical ? defender.DEF : defender.SD;
	return ((2.f * attacker.lv + 10.f) / 250.f * ((float)A / (float)D) * skill_data.power + 2.f) * effectiveness;
}

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

struct Troop
{
	std::vector<uint>	units;		// indices
	uint				target = 0;	// tile id
	std::vector<uint>	path;
};

struct PokemonCapture
{
	uint unit_id;
	uint exclusive_id;
	uint lv;
	int skills[4] = { -1, -1, -1, -1 };
};

struct City
{
	uint id;
	uint tile_id;
	uint lord_id;
	uint loyalty;
	std::vector<Building> buildings;
	std::vector<PokemonCapture> captures;
	std::vector<Unit> units;
	std::vector<Troop> troops;

	int get_building_lv(BuildingType type, int slot = -1)
	{
		for (auto& building : buildings)
		{
			if (building.type == type && (slot == -1 || building.slot == slot))
				return building.lv;
		}
		return -1;
	}

	void add_capture_ll(uint unit_id, uint exclusive_id, uint lv)
	{
		auto& unit_data = unit_datas[unit_id];
		PokemonCapture capture;
		capture.unit_id = unit_id;
		capture.exclusive_id = exclusive_id;
		capture.lv = lv;

		auto n = 0;
		for (auto it = unit_data.skillset.rbegin(); it != unit_data.skillset.rend(); it++)
		{
			if (it->x <= lv)
			{
				capture.skills[n] = it->y;
				n++;
				if (n >= 4)
					break;
			}
		}

		captures.push_back(capture);
	}

	void add_capture(uint unit_id, uint lv)
	{
		add_capture_ll(unit_id, 0, lv);
	}

	void add_exclusive_captures(const std::vector<uint>& unit_ids, uint exclusive_id, uint lv)
	{
		for (auto unit_id : unit_ids)
			add_capture_ll(unit_id, exclusive_id, lv);
	}

	void add_unit(uint unit_id, uint lv, int* skills)
	{
		auto& unit_data = unit_datas[unit_id];

		Unit unit;
		unit.id = unit_id;
		unit.lv = lv;
		unit.exp = 0;
		if (skills)
			memcpy(unit.skills, skills, sizeof(unit.skills));
		units.push_back(unit);

		troops.front().units.push_back(units.size() - 1);
	}

	void set_troop_target(Troop& troop, uint target)
	{
		troop.target = target;
		troop.path = find_path(tile_id, target);
	}
};

struct TroopInstance
{
	uint lord_id;
	std::vector<UnitInstance> units;
	std::vector<uint> path;
	uint idx = 0;
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
		vec2 init_pos;
		vec2 pos; float ang; vec2 scl; float alpha;
		uint HP;
		std::wstring label;
		vec2 label_pos; float label_ang; vec2 label_scl; float label_alpha;
	};

	uint side;
	TroopInstance* troop;
	City* city = nullptr; // for damage calc
	std::vector<UnitDisplay> unit_displays;
	int action_idx;

	void refresh_display()
	{
		if (!troop)
			return;
		unit_displays.resize(troop->units.size());
		for (auto i = 0; i < troop->units.size(); i++)
		{
			auto& unit = troop->units[i];
			auto& display = unit_displays[i];
			display.unit_id = unit.id;
			display.init_pos = vec3(136.f + i * 75.f, 240.f + 100.f * (1 - side), 0.f);
			display.HP = unit.HP;
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

	std::vector<TroopInstance> troops;

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
		city.add_exclusive_captures({ 0, 3, 6 }, "yusanjia"_h, 5);
		city.troops.emplace_back().target = tile_id;
		city.troops.emplace_back();

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

	bool buy_unit(City& city, uint capture_id)
	{
		auto& capture = city.captures[capture_id];
		auto& unit_data = unit_datas[capture.unit_id];
		if (resources[ResourceGold] < unit_data.cost_gold ||
			provide_population < consume_population + unit_data.cost_population)
			return false;

		resources[ResourceGold] -= unit_data.cost_gold;
		consume_population += unit_data.cost_population;

		city.add_unit(capture.unit_id, capture.lv, capture.skills);

		auto exclusive_id = capture.exclusive_id;
		if (exclusive_id != 0)
		{
			for (auto it = city.captures.begin(); it != city.captures.end();)
			{
				if (it->exclusive_id == exclusive_id)
					it = city.captures.erase(it);
				else
					it++;
			}
		}
		else
			city.captures.erase(city.captures.begin() + capture_id);
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

cvec4 hsv(float h, float s, float v, float a)
{
	return cvec4(vec4(rgbColor(vec3(h, s, v)), a) * 255.f);
}

GameState state = GameInit;
BattleTroop battle_troops[2];
uint battle_action_side = 0;
uint city_damge = 0;
float troop_anim_time = 0.f;
float battle_time = 0.f;
float anim_remain = 0;

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
							city.add_capture(park_data.encounter_list[linearRand(0, list_sz - 1)], 1);
					}
				}
					break;
				}
			}
		}
	}

	// do ai for all computer players
	for (auto i = 1; i < lords.size(); i++)
	{
		auto& lord = lords[i];

		for (auto& city : lord.cities)
		{
			if (!city.captures.empty())
				lord.buy_unit(city, linearRand(0, (int)city.captures.size() - 1));

			if (auto target_city = search_random_hostile_city(lord.id); target_city)
			{
				auto& troop = city.troops[1];
				troop.units.clear();
				for (auto i = 0; i < city.units.size(); i++)
					troop.units.push_back(i);
				city.set_troop_target(troop, target_city->tile_id);
			}
		}
	}
}

void start_battle()
{
	if (state == GameNight)
		return;
	state = GameNight;

	for (auto& lord : lords)
	{
		for (auto& city : lord.cities)
		{
			city.captures.clear();
			for (auto i = 1; i < city.troops.size(); i++)
			{
				auto& troop = city.troops[i];
				if (troop.path.empty() || troop.units.empty())
					continue;
				TroopInstance troop_ins;
				troop_ins.lord_id = lord.id;
				troop_ins.path = troop.path;
				for (auto idx : troop.units)
				{
					auto& unit = city.units[idx];
					auto& unit_data = unit_datas[unit.id];
					UnitInstance unit_ins;
					unit_ins.original = &unit;
					unit_ins.id = unit.id;
					unit_ins.lv = unit.lv;
					unit_ins.HP_MAX = calc_hp_stat(unit_data.HP, unit.lv);
					unit_ins.HP = unit_ins.HP_MAX;
					unit_ins.ATK = calc_stat(unit_data.ATK, unit.lv);
					unit_ins.DEF = calc_stat(unit_data.DEF, unit.lv);
					unit_ins.SA = calc_stat(unit_data.SA, unit.lv);
					unit_ins.SD = calc_stat(unit_data.SD, unit.lv);
					unit_ins.SP = calc_stat(unit_data.SP, unit.lv);
					unit_ins.type1 = unit_data.type1;
					unit_ins.type2 = unit_data.type2;
					memcpy(unit_ins.skills, unit.skills, sizeof(unit_ins.skills));
					troop_ins.units.push_back(unit_ins);
				}
				lord.troops.push_back(troop_ins);
			}
		}
	}
}

void step_troop_moving()
{
	if (anim_remain > 0.f)
		return;
	anim_remain = 0.5f;

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
		for (auto& troop : lord.troops)
		{
			if (troop.idx == troop.path.size() - 1)
			{
				auto tile_id = troop.path.back();
				state = GameBattle;

				for (auto& _lord : lords)
				{
					if (auto id = _lord.find_city(tile_id); id != -1)
					{
						auto& city = _lord.cities[id];
						{
							auto& battle_troop = battle_troops[0];
							battle_troop.side = 0;
							battle_troop.troop = nullptr;
							battle_troop.city = &city;
							battle_troop.action_idx = 0;
							battle_troop.unit_displays.clear();
						}
						{
							auto& battle_troop = battle_troops[1];
							battle_troop.side = 1;
							battle_troop.troop = &troop;
							battle_troop.action_idx = 0;
							battle_troop.refresh_display();
						}
						break;
					}
				}

				return;
			}
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

	if (battle_troops[0].troop && battle_troops[1].troop)
	{
		for (auto i = 0; i < 2; i++)
		{
			auto& battle_troop = battle_troops[i];
			for (auto j = 0; j < battle_troop.troop->units.size(); j++)
			{
				if (battle_troop.troop->units[j].HP <= 0)
				{
					if (j <= battle_troop.action_idx && battle_troop.action_idx > 0)
						battle_troop.action_idx--;
					battle_troop.troop->units.erase(battle_troop.troop->units.begin() + j);
					j--;
				}
			}
		}

		if (battle_troops[0].troop->units.empty() || battle_troops[1].troop->units.empty())
		{
			for (auto i = 0; i < 2; i++)
			{
				if (battle_troops[i].troop->units.empty())
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

		auto& action_troop = battle_troops[battle_action_side];
		auto& other_troop = battle_troops[1 - battle_action_side];
		auto& caster = action_troop.troop->units[action_troop.action_idx];
		auto target_idx = linearRand(0, (int)other_troop.troop->units.size() - 1);
		auto& target = other_troop.troop->units[target_idx];
		auto& cast_unit_display = action_troop.unit_displays[action_troop.action_idx];
		auto& target_unit_display = other_troop.unit_displays[target_idx];

		auto cast_skill = caster.choose_attack_skill();
		auto target_damage = cast_skill == -1 ? 0 : calc_damage(caster, target, cast_skill);
		target.HP = max(0, target.HP - (int)target_damage);

		{
			auto id = game.tween->begin_2d_targets();
			game.tween->add_2d_target(id, &cast_unit_display.pos, nullptr, &cast_unit_display.scl, nullptr);
			game.tween->add_2d_target(id, nullptr, nullptr, nullptr, &target_unit_display.alpha);
			game.tween->add_2d_target(id, &target_unit_display.label_pos, nullptr, nullptr, nullptr);
			game.tween->add_int_target(id, (int*)&target_unit_display.HP);
			auto target_pos = mix(cast_unit_display.pos, target_unit_display.pos, 0.9f);
			game.tween->scale_to(id, vec2(1.5f), 0.3f);
			game.tween->move_to(id, target_pos, 0.3f);
			game.tween->set_ease(id, EaseInCubic);
			auto time_attacked = game.tween->get_time(id);

			game.tween->set_target(id, 2);
			game.tween->set_channel(id, 2, time_attacked);
			game.tween->set_callback(id, [&, target_damage]() {
				target_unit_display.label = wstr(target_damage);
				target_unit_display.label_pos = target_unit_display.init_pos + vec2(0.f, -45.f);
			});
			game.tween->move_to(id, target_unit_display.init_pos + vec2(0.f, -60.f), 0.4f);
			game.tween->set_callback(id, [&, target_damage]() {
				target_unit_display.label = L"";
			});

			if (target.HP <= 0)
			{
				game.tween->set_target(id, 1);
				game.tween->alpha_to(id, 0.f, 0.4f);
				game.tween->set_ease(id, EaseInCubic);
			}

			game.tween->set_target(id, 0U);
			game.tween->set_channel(id, 0, time_attacked);
			game.tween->move_to(id, cast_unit_display.pos, 0.3f);
			game.tween->set_channel(id, 1, time_attacked);
			game.tween->scale_to(id, vec2(1.f), 0.3f);
			game.tween->end(id);
			anim_remain = 1.5f;
		}

		action_troop.action_idx++;
		if (action_troop.action_idx >= action_troop.troop->units.size())
			action_troop.action_idx = 0;
		battle_action_side = 1 - battle_action_side;
	}
	else
	{
		auto& action_troop = battle_troops[1];

		if (action_troop.action_idx == -1)
		{
			{
				auto troop = battle_troops[1].troop;
				auto& lord = lords[troop->lord_id];
				for (auto it = lord.troops.begin(); it != lord.troops.end(); it++)
				{
					if (&*it == troop)
					{
						lord.troops.erase(it);
						break;
					}
				}
			}

			auto& city = *battle_troops[0].city;
			if (city.loyalty > city_damge)
				city.loyalty -= city_damge;
			else
				city.loyalty = 0;
			city_damge = 0;

			state = GameNight;
			battle_troops[0].troop = battle_troops[1].troop = nullptr;
			battle_troops[0].city = nullptr;
			return;
		}

		if (action_troop.action_idx >= action_troop.troop->units.size())
		{
			action_troop.action_idx = -1;
			anim_remain = 1.5f;
			return;
		}

		auto& caster = action_troop.troop->units[action_troop.action_idx];
		auto& cast_unit_display = action_troop.unit_displays[action_troop.action_idx];

		{
			auto id = game.tween->begin_2d_targets();
			game.tween->add_2d_target(id, &cast_unit_display.pos, nullptr, &cast_unit_display.scl, nullptr);
			game.tween->add_2d_target(id, &cast_unit_display.label_pos, nullptr, nullptr, nullptr);
			game.tween->scale_to(id, vec2(1.1f), 0.2f);
			auto damage = max(1U, caster.lv / 10);
			game.tween->set_callback(id, [&, damage]() {
				cast_unit_display.label = wstr(damage);
				cast_unit_display.label_pos = cast_unit_display.init_pos + vec2(0.f, 5.f);
			});
			game.tween->scale_to(id, vec2(1.f), 0.2f);
			game.tween->set_target(id, 1);
			game.tween->move_to(id, vec2(450.f, 300.f), 0.5f);
			game.tween->set_callback(id, [&, damage]() {
				cast_unit_display.label = L"";
				city_damge += damage;
			});
			game.tween->end(id);
		}

		action_troop.action_idx++;
	}
}

void Game::init()
{
	srand(time(0));

	create("Werewolf VS Vampire", uvec2(1280, 720), WindowStyleFrame, false, true, 
		{ {"mesh_shader"_h, 0} });

	auto root = world->root.get();
	root->add_component<cNode>();
	camera = root->add_component<cCamera>();

	renderer->add_render_task(RenderModeShaded, camera, main_window, {}, graphics::ImageLayoutPresent);
	canvas = renderer->render_tasks.front()->canvas;
	hud->canvas = canvas;

	Path::set_root(L"assets", L"assets");

	img_target = graphics::Image::get(L"assets/icons/target.png");
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
	img_be_hit = graphics::Image::get(L"assets/be_hit.png");

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

	{
		auto effectiveness = pokemon_type_effectiveness[PokemonNormal];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		1.f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		1.f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		1.f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		1.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		0.5f;
		effectiveness[PokemonGhost] =		0.f;
		effectiveness[PokemonDragon] =		1.f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		0.5f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonFire];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		0.5f;
		effectiveness[PokemonWater] =		0.5f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		2.f;
		effectiveness[PokemonIce] =			2.f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		1.f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		1.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			2.f;
		effectiveness[PokemonRock] =		0.5f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		0.5f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		2.f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonWater];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		2.f;
		effectiveness[PokemonWater] =		0.5f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		0.5f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		1.f;
		effectiveness[PokemonGround] =		2.f;
		effectiveness[PokemonFlying] =		1.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		2.f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		0.5f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		1.f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonElectric];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		1.f;
		effectiveness[PokemonWater] =		2.f;
		effectiveness[PokemonElectric] =	0.5f;
		effectiveness[PokemonGrass] =		0.5f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		1.f;
		effectiveness[PokemonGround] =		0.f;
		effectiveness[PokemonFlying] =		2.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		1.f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		0.5f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		1.f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonGrass];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		0.5f;
		effectiveness[PokemonWater] =		2.f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		0.5f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		0.5f;
		effectiveness[PokemonGround] =		2.f;
		effectiveness[PokemonFlying] =		0.5f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			0.5f;
		effectiveness[PokemonRock] =		2.f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		0.5f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		0.5f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonIce];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		0.5f;
		effectiveness[PokemonWater] =		0.5f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		2.f;
		effectiveness[PokemonIce] =			0.5f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		1.f;
		effectiveness[PokemonGround] =		2.f;
		effectiveness[PokemonFlying] =		2.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		1.f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		2.f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		0.5f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonFighting];
		effectiveness[PokemonNormal] =		2.f;
		effectiveness[PokemonFire] =		1.f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		1.f;
		effectiveness[PokemonIce] =			2.f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		0.5f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		0.5f;
		effectiveness[PokemonPsychic] =		0.5f;
		effectiveness[PokemonBug] =			0.5f;
		effectiveness[PokemonRock] =		2.f;
		effectiveness[PokemonGhost] =		0.f;
		effectiveness[PokemonDragon] =		1.f;
		effectiveness[PokemonDark] =		2.f;
		effectiveness[PokemonSteel] =		2.f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonPoison];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		1.f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		2.f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		0.5f;
		effectiveness[PokemonGround] =		0.5f;
		effectiveness[PokemonFlying] =		1.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		0.5f;
		effectiveness[PokemonGhost] =		0.5f;
		effectiveness[PokemonDragon] =		1.f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		0.f;
		effectiveness[PokemonFairy] =		2.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonGround];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		2.f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	2.f;
		effectiveness[PokemonGrass] =		0.5f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		2.f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		0.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			0.5f;
		effectiveness[PokemonRock] =		2.f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		1.f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		2.f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonFlying];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		1.f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	0.5f;
		effectiveness[PokemonGrass] =		2.f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	2.f;
		effectiveness[PokemonPoison] =		1.f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		1.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			2.f;
		effectiveness[PokemonRock] =		0.5f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		1.f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		0.5f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonPsychic];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		1.f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		1.f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	2.f;
		effectiveness[PokemonPoison] =		2.f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		1.f;
		effectiveness[PokemonPsychic] =		0.5f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		1.f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		1.f;
		effectiveness[PokemonDark] =		0.f;
		effectiveness[PokemonSteel] =		0.5f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonBug];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		0.5f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		2.f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	0.5f;
		effectiveness[PokemonPoison] =		0.5f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		0.5f;
		effectiveness[PokemonPsychic] =		2.f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		1.f;
		effectiveness[PokemonGhost] =		0.5f;
		effectiveness[PokemonDragon] =		1.f;
		effectiveness[PokemonDark] =		2.f;
		effectiveness[PokemonSteel] =		0.5f;
		effectiveness[PokemonFairy] =		0.5f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonRock];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		2.f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		1.f;
		effectiveness[PokemonIce] =			2.f;
		effectiveness[PokemonFighting] =	0.5f;
		effectiveness[PokemonPoison] =		1.f;
		effectiveness[PokemonGround] =		0.5f;
		effectiveness[PokemonFlying] =		2.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			2.f;
		effectiveness[PokemonRock] =		1.f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		1.f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		0.5f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonGhost];
		effectiveness[PokemonNormal] =		0.f;
		effectiveness[PokemonFire] =		1.f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		1.f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		1.f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		1.f;
		effectiveness[PokemonPsychic] =		2.f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		1.f;
		effectiveness[PokemonGhost] =		2.f;
		effectiveness[PokemonDragon] =		1.f;
		effectiveness[PokemonDark] =		0.5f;
		effectiveness[PokemonSteel] =		1.f;
		effectiveness[PokemonFairy] =		1.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonDragon];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		1.f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		1.f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		1.f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		1.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		1.f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		2.f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		0.5f;
		effectiveness[PokemonFairy] =		0.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonDark];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		1.f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		1.f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	0.5f;
		effectiveness[PokemonPoison] =		1.f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		1.f;
		effectiveness[PokemonPsychic] =		2.f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		1.f;
		effectiveness[PokemonGhost] =		2.f;
		effectiveness[PokemonDragon] =		1.f;
		effectiveness[PokemonDark] =		0.5f;
		effectiveness[PokemonSteel] =		1.f;
		effectiveness[PokemonFairy] =		0.5f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonSteel];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		0.5f;
		effectiveness[PokemonWater] =		0.5f;
		effectiveness[PokemonElectric] =	0.5f;
		effectiveness[PokemonGrass] =		1.f;
		effectiveness[PokemonIce] =			2.f;
		effectiveness[PokemonFighting] =	1.f;
		effectiveness[PokemonPoison] =		1.f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		1.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		2.f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		1.f;
		effectiveness[PokemonDark] =		1.f;
		effectiveness[PokemonSteel] =		0.5f;
		effectiveness[PokemonFairy] =		2.f;
	}
	{
		auto effectiveness = pokemon_type_effectiveness[PokemonFairy];
		effectiveness[PokemonNormal] =		1.f;
		effectiveness[PokemonFire] =		0.5f;
		effectiveness[PokemonWater] =		1.f;
		effectiveness[PokemonElectric] =	1.f;
		effectiveness[PokemonGrass] =		1.f;
		effectiveness[PokemonIce] =			1.f;
		effectiveness[PokemonFighting] =	2.f;
		effectiveness[PokemonPoison] =		0.5f;
		effectiveness[PokemonGround] =		1.f;
		effectiveness[PokemonFlying] =		1.f;
		effectiveness[PokemonPsychic] =		1.f;
		effectiveness[PokemonBug] =			1.f;
		effectiveness[PokemonRock] =		1.f;
		effectiveness[PokemonGhost] =		1.f;
		effectiveness[PokemonDragon] =		2.f;
		effectiveness[PokemonDark] =		2.f;
		effectiveness[PokemonSteel] =		0.5f;
		effectiveness[PokemonFairy] =		1.f;
	}

	if (auto sht = Sheet::get(L"assets/skill.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			SkillData data;
			auto& row = sht->rows[i];
			data.name = sht->get_as_wstr(row, "name"_h);
			data.type = get_pokemon_type_from_name(sht->get_as_wstr(row, "type"_h));
			data.category = get_skill_category_from_name(sht->get_as_wstr(row, "category"_h));
			data.power = sht->get_as<uint>(row, "power"_h);
			data.acc = sht->get_as<uint>(row, "acc"_h);
			data.pp = sht->get_as<uint>(row, "pp"_h);
			data.effect = sht->get_as_wstr(row, "effect"_h);
			skill_datas.push_back(data);
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
			data.type1 = get_pokemon_type_from_name(sht->get_as_wstr(row, "type1"_h));
			data.type2 = get_pokemon_type_from_name(sht->get_as_wstr(row, "type2"_h));
			{
				auto str = sht->get_as_wstr(row, "skillset"_h);
				for (auto t : SUW::split(str, ','))
				{
					auto sp = SUW::split(t, ':');
					if (sp.size() == 2)
					{
						auto lv = s2t<uint>(std::wstring(sp[0]));
						auto skill_name = std::wstring(sp[1]);
						auto skill_id = -1;
						for (auto i = 0; i < skill_datas.size(); i++)
						{
							if (skill_datas[i].name == skill_name)
							{
								skill_id = i;
								break;
							}
						}
						if (skill_id != -1)
							data.skillset.push_back(ivec2(lv, skill_id));
					}
				}
			}
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
			slot.type = get_building_type_from_name(sht->get_as_wstr(row, "building"_h));
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

	if (auto tile_id = search_lord_location(); tile_id != -1)
	{
		if (auto id = add_lord(tile_id); id != -1)
		{
			auto& lord = lords[id];
			auto& city = lord.cities.front();
			//city.add_unit(9); // for testing
			//city.add_unit(12); // for testing
		}
	}
	if (auto tile_id = search_lord_location(); tile_id != -1)
	{
		if (auto id = add_lord(tile_id); id != -1)
		{
			auto& lord = lords[id];
			auto& city = lord.cities.front();
			//city.add_unit(9); // for testing
			//city.add_unit(12); // for testing
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
		step_troop_moving();
		break;
	case GameBattle:
		step_battle();
		break;
	}

	for (auto& tile : tiles)
	{
		draw_image(img_tile_grass, tile.pos, vec2(tile_sz, tile_sz_y), vec2(0.f), distance(tile.pos + tile_sz * 0.5f, mpos) < tile_sz * 0.5f ? cvec4(255) : cvec4(230, 230, 230, 255));
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
		auto draw_troop_path = [&](const std::vector<uint>& path, int move_idx) {
			vec2 end_point;
			if (move_idx != -1)
			{
				for (auto i = 0; i <= move_idx; i++)
				{
					auto id = path[i];
					canvas->path.push_back(tiles[id].pos + vec2(tile_sz) * 0.5f);
				}
				end_point = canvas->path.back();
				if (auto idx = move_idx + 1; idx < path.size())
				{
					auto t = state == GameBattle ? 0.5f : fract(1.f - anim_remain * 1.9f);
					end_point = mix(end_point, tiles[path[idx]].pos + vec2(tile_sz) * 0.5f, t);
					canvas->path.push_back(end_point);
				}
			}
			canvas->stroke(4.f, hsv(lord.id * 60.f, 0.5f, 0.8f, 1.f), false);
			for (auto i = 0; i < path.size() - 1; i++)
			{
				for (auto j = 0; j < 4; j++)
				{
					if (abs(int(i * 4 + j - troop_anim_time * 12.f) % 20) < 4)
					{
						auto a = tiles[path[i]].pos;
						auto b = tiles[path[i + 1]].pos;
						canvas->draw_circle_filled(mix(a, b, j / 4.f) + vec2(tile_sz) * 0.5f, 3.f, hsv(lord.id * 60.f, 0.5f, 0.8f, 0.8f));
					}
				}
			}
			if (move_idx != -1)
				canvas->draw_circle_filled(end_point, 6.f, hsv(lord.id * 60.f, 0.5f, 1.f, 1.f));
		};
		for (auto& city : lord.cities)
		{
			for (auto& troop : city.troops)
			{
				if (troop.path.empty() || troop.units.empty())
					continue;
				draw_troop_path(troop.path, -1);
			}
		}
		for (auto& troop : lord.troops)
			draw_troop_path(troop.path, troop.idx);
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

	auto show_unit_detail = [&](const vec2& mpos, uint id, uint lv, uint HP, uint HP_MAX, uint ATK, uint DEF, uint SA, uint SD, uint SP, PokemonType type1, PokemonType type2, int* skills) {
		auto& unit_data = unit_datas[id];
		hud->begin(mpos + vec2(10.f, 4.f), vec2(0.f), cvec4(50, 50, 50, 255), vec2(0.f, mpos.y > 540.f ? 1.f : 0.f));
		hud->begin_layout(HudHorizontal, vec2(0.f), vec2(4.f, 0.f));
		hud->begin_layout(HudVertical);
		hud->text(std::format(L"{} LV {}", unit_data.name, lv));
		hud->begin_layout(HudHorizontal);
		if (unit_data.type1 != PokemonTypeCount)
			hud->text(get_pokemon_type_name(unit_data.type1), 20, get_pokemon_type_color(unit_data.type1));
		if (unit_data.type2 != PokemonTypeCount)
			hud->text(get_pokemon_type_name(unit_data.type2), 20, get_pokemon_type_color(unit_data.type2));
		hud->end_layout();
		if (HP_MAX == 0)
			hud->text(std::format(L"HP {}\nATK {}\nDEF {}\nSA {}\nSD {}\nSP {}", HP, ATK, DEF, SA, SD, SP), 20);
		else
			hud->text(std::format(L"HP {}/{}\nATK {}\nDEF {}\nSA {}\nSD {}\nSP {}", HP, HP_MAX, ATK, DEF, SA, SD, SP), 20);
		hud->end_layout();
		for (auto i = 0; i < 4; i++)
		{
			auto skill_id = skills[i];
			if (skill_id != -1)
			{
				auto& skill_data = skill_datas[skill_id];
				hud->begin_layout(HudVertical);
				hud->text(skill_data.name);
				hud->text(get_pokemon_type_name(skill_data.type), 20, get_pokemon_type_color(skill_data.type));
				if (skill_data.power > 0)
					hud->text(std::format(L"Power {}", skill_data.power));
				hud->text(skill_data.effect);
				hud->end_layout();
				hud->stroke_item();
			}
		}
		hud->end_layout();
		hud->end();
	};

	auto bottom_pannel_height = 275.f;
	hud->begin(vec2(0.f, screen_size.y - bottom_pannel_height), vec2(screen_size.x, bottom_pannel_height), cvec4(0, 0, 0, 255));
	auto rect = hud->wnd_rect();
	if (selected_tile != -1)
	{
		auto& tile = tiles[selected_tile];
		switch (tile.type)
		{
		case TileField:
			if (main_player.has_territory(selected_tile))
			{
				hud->begin_layout(HudHorizontal);

				auto show_build_resource_field = [&](ResourceType type) {
					hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
					hud->text(get_resource_field_name(type));
					if (!resource_field_datas[type].empty())
					{
						auto& first_level = resource_field_datas[type].front();

						hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
						hud->text(L"Production: ", 22);
						hud->image(vec2(20.f, 13.f), img_resources[type]);
						hud->text(std::format(L"{}", first_level.production), 22);
						hud->end_layout();

						hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
						hud->image(vec2(18.f, 12.f), img_resources[ResourceWood]);
						hud->text(std::format(L"{}", first_level.cost_wood), 16);
						hud->image(vec2(18.f, 12.f), img_resources[ResourceClay]);
						hud->text(std::format(L"{}", first_level.cost_clay), 16);
						hud->image(vec2(18.f, 12.f), img_resources[ResourceIron]);
						hud->text(std::format(L"{}", first_level.cost_iron), 16);
						hud->image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
						hud->text(std::format(L"{}", first_level.cost_crop), 16);
						hud->end_layout();
						hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
						hud->image(vec2(18.f, 12.f), img_resources[ResourceGold]);
						hud->text(std::format(L"{}", first_level.cost_gold), 16);
						hud->image(vec2(18.f, 12.f), img_population);
						hud->text(std::format(L"{}", first_level.cost_population), 16);
						hud->end_layout();

						if (hud->button(L"Build", 18))
							main_player.build_resource_field(selected_tile, type);
					}
					hud->end_layout();
					hud->stroke_item();
				};

				show_build_resource_field(ResourceWood);
				show_build_resource_field(ResourceClay);
				show_build_resource_field(ResourceIron);
				show_build_resource_field(ResourceCrop);

				hud->end_layout();
			}
			break;
		case TileCity:
		{
			enum Tab
			{
				TabBuildings,
				TabRecruit,
				TabTroops
			};
			static Tab tab = TabBuildings;

			auto& lord = lords[tile.lord_id];
			auto& city = lord.cities[lord.find_city(selected_tile)];

			hud->begin_layout(HudVertical, vec2(0.f), vec2(0.f, 8.f));
			hud->begin_layout(HudHorizontal);
			hud->text(lord.id == main_player.id ? L"Your City" : L"Enemy's City");
			if (lord.id == main_player.id)
			{
				if (tab != TabBuildings)
				{
					hud->push_style_color(HudStyleColorButton, cvec4(127, 127, 127, 255));
					if (hud->button(L"Buildings"))
						tab = TabBuildings;
					hud->pop_style_color(HudStyleColorButton);
				}
				else
				{
					if (hud->button(L"Buildings"))
						tab = TabBuildings;
				}
				if (tab != TabRecruit)
				{
					hud->push_style_color(HudStyleColorButton, cvec4(127, 127, 127, 255));
					if (hud->button(L"Recruit"))
						tab = TabRecruit;
					hud->pop_style_color(HudStyleColorButton);
				}
				else
				{
					if (hud->button(L"Recruit"))
						tab = TabRecruit;
				}
				if (tab != TabTroops)
				{
					hud->push_style_color(HudStyleColorButton, cvec4(127, 127, 127, 255));
					if (hud->button(L"Troops"))
						tab = TabTroops;
					hud->pop_style_color(HudStyleColorButton);
				}
				else
				{
					if (hud->button(L"Troops"))
						tab = TabTroops;
				}
			}
			hud->end_layout();
			if (lord.id == main_player.id)
			{
				switch (tab)
				{
				case TabBuildings:
				{
					auto circle_sz = 100.f;
					hud->begin_layout(HudHorizontal);
					hud->rect(vec2(circle_sz * 2.f + 28.f, bottom_pannel_height - 48.f), cvec4(0));

					auto c = hud->item_rect().a + vec2(circle_sz + 10.f, circle_sz + 10.f);
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
								hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
								hud->image(vec2(18.f, 12.f), img_resources[ResourceWood]);
								hud->text(std::format(L"{}", next_level->cost_wood), 16, main_player.resources[ResourceWood] >= next_level->cost_wood ? cvec4(255) : cvec4(255, 0, 0, 255));
								hud->image(vec2(18.f, 12.f), img_resources[ResourceClay]);
								hud->text(std::format(L"{}", next_level->cost_clay), 16, main_player.resources[ResourceClay] >= next_level->cost_clay ? cvec4(255) : cvec4(255, 0, 0, 255));
								hud->image(vec2(18.f, 12.f), img_resources[ResourceIron]);
								hud->text(std::format(L"{}", next_level->cost_iron), 16, main_player.resources[ResourceIron] >= next_level->cost_iron ? cvec4(255) : cvec4(255, 0, 0, 255));
								hud->image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
								hud->text(std::format(L"{}", next_level->cost_crop), 16, main_player.resources[ResourceCrop] >= next_level->cost_crop ? cvec4(255) : cvec4(255, 0, 0, 255));
								hud->end_layout();
								hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
								hud->image(vec2(18.f, 12.f), img_resources[ResourceGold]);
								hud->text(std::format(L"{}", next_level->cost_gold), 16, main_player.resources[ResourceGold] >= next_level->cost_gold ? cvec4(255) : cvec4(255, 0, 0, 255));
								hud->image(vec2(18.f, 12.f), img_population);
								hud->text(std::format(L"{}", next_level->cost_population), 16, (int)main_player.provide_population - (int)main_player.consume_population >= next_level->cost_population ? cvec4(255) : cvec4(255, 0, 0, 255));
								hud->end_layout();

								if (hud->button(building.lv == 0 ? L"Build" : L"Upgrade", 18))
									main_player.upgrade_building(city, selected_building_slot);
							}
						};
						switch (type)
						{
						case BuildingTownCenter:
							hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
							hud->text(std::format(L"Town Center LV: {}", building.lv));
							show_upgrade_building();
							hud->end_layout();
							hud->stroke_item();
							break;
						case BuildingHouse:
						{
							hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
							hud->text(std::format(L"House LV: {}", building.lv));
							if (building.lv > 0)
							{
								auto& data = house_datas[building.lv - 1];
								hud->text(std::format(L"Current Provide Population: {}", data.provide_population), 22);
							}
							auto next_level = house_datas[building.lv];
							hud->text(std::format(L"Level {} Provide Population: {}", building.lv + 1, next_level.provide_population), 22);
							show_upgrade_building();
							hud->end_layout();
							hud->stroke_item();
						}
							break;
						case BuildingBarracks:
						{
							hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
							hud->text(std::format(L"Barracks LV: {}", building.lv));
							show_upgrade_building();
							hud->end_layout();
							hud->stroke_item();
						}
							break;
						case BuildingPark:
						{
							hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
							hud->text(std::format(L"Park LV: {}", building.lv));
							show_upgrade_building();
							hud->end_layout();
							hud->stroke_item();
						}
							break;
						case BuildingTower:
							hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
							hud->text(std::format(L"Tower LV: {}", building.lv));
							show_upgrade_building();
							hud->end_layout();
							hud->stroke_item();
							break;
						case BuildingWall:
							hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
							hud->text(std::format(L"Wall LV: {}", building.lv));
							show_upgrade_building();
							hud->end_layout();
							hud->stroke_item();
							break;
						}
					}

					hud->end_layout();
				}
					break;
				case TabRecruit:
				{
					auto hovered_unit = -1;
					hud->begin_layout(HudHorizontal);
					for (auto i = 0; i < city.captures.size(); i++)
					{
						auto& unit_data = unit_datas[city.captures[i].unit_id];
						if (unit_data.icon)
						{
							hud->begin_layout(HudVertical);
							hud->image_button(vec2(64.f), unit_data.icon);
							if (hud->item_hovered())
								hovered_unit = i;
							const auto scl = 0.7f;
							hud->begin_layout(HudHorizontal);
							hud->image(vec2(27.f, 18.f) * scl, img_resources[ResourceGold]);
							hud->text(std::format(L"{}", unit_data.cost_gold), 24 * scl);
							hud->end_layout();
							hud->begin_layout(HudHorizontal);
							hud->image(vec2(27.f, 18.f) * scl, img_population);
							hud->text(std::format(L"{}", unit_data.cost_population), 24 * scl);
							hud->end_layout();
							hud->end_layout();
						}
					}
					hud->end_layout();

					if (hovered_unit != -1)
					{
						auto& capture = city.captures[hovered_unit];
						auto& unit_data = unit_datas[capture.unit_id];
						show_unit_detail(mpos, capture.unit_id, capture.lv, calc_hp_stat(unit_data.HP, capture.lv), 0,
							calc_stat(unit_data.ATK, capture.lv), calc_stat(unit_data.DEF, capture.lv), calc_stat(unit_data.SA, capture.lv),
							calc_stat(unit_data.SD, capture.lv), calc_stat(unit_data.SP, capture.lv), unit_data.type1, unit_data.type2, capture.skills);
						if (input->mpressed(Mouse_Left))
							lord.buy_unit(city, hovered_unit);
					}
				}
					break;
				case TabTroops:
				{
					static int dragging_unit = -1;
					static int dragging_target = -1;
					auto hovered_unit = -1;
					auto show_troop = [&](uint tidx) {
						auto& troop = city.troops[tidx];
						const float size = 64.f;
						const uint max_num = 7;
						auto pos = hud->get_cursor();
						if (dragging_unit != -1)
						{
							if (mpos.y > pos.y && mpos.y < pos.y + size && mpos.x > pos.x + troop.units.size() * size)
							{
								if (dragging_unit != -1 && input->mreleased(Mouse_Left))
								{
									auto ok = false;
									for (auto& _troop : city.troops)
									{
										for (auto j = 0; j < _troop.units.size(); j++)
										{
											if (_troop.units[j] == dragging_unit)
											{
												_troop.units.erase(_troop.units.begin() + j);
												troop.units.push_back(dragging_unit);
												ok = true;
												break;
											}
										}
										if (ok)
											break;
									}
									hovered_unit = -1;
								}
								canvas->draw_rect_filled(pos, pos + vec2(size * max_num, size), cvec4(100, 100, 100, 255));
							}
						}
						canvas->draw_rect(pos, pos + vec2(size * max_num, size), 1.f, cvec4(255));
						hud->begin_layout(HudHorizontal);
						hud->begin_layout(HudHorizontal, vec2(size * max_num, size));
						if (troop.units.empty())
							hud->rect(vec2(size), cvec4(0));
						for (auto i = 0; i < troop.units.size(); i++)
						{
							auto idx = troop.units[i];
							auto& unit = city.units[idx];
							auto& unit_data = unit_datas[unit.id];
							if (unit_data.icon)
							{
								if (hud->image_button(vec2(size), unit_data.icon))
									dragging_unit = idx;
								if (hud->item_hovered())
								{
									hovered_unit = tidx * 100 + i;
									if (dragging_unit != -1 && input->mreleased(Mouse_Left))
									{
										auto ok = false;
										for (auto& _troop : city.troops)
										{
											for (auto j = 0; j < _troop.units.size(); j++)
											{
												if (_troop.units[j] == dragging_unit)
												{
													_troop.units[j] = idx;
													troop.units[i] = dragging_unit;
													ok = true;
													break;
												}
											}
											if (ok)
												break;
										}
										hovered_unit = -1;
										dragging_unit = -1;
									}
								}
							}
						}
						hud->end_layout();
						if (tidx > 0)
						{
							hud->set_cursor(hud->get_cursor() + vec2(0.f, 16.f));
							if (hud->image_button(vec2(32.f), img_target))
								dragging_target = tidx;
							if (hud->item_hovered())
								draw_image(img_target, tiles[troop.target].pos + vec2(tile_sz) * 0.5f, vec2(32.f), vec2(0.5f, 0.5f), cvec4(255, 255, 255, 200));
						}
						hud->end_layout();
					};

					hud->text(L"In City:");
						show_troop(0);
					hud->text(L"Troops:");
					for (auto i = 1; i < city.troops.size(); i++)
						show_troop(i);
					if (hud->button(L"New"))
					{
						auto& troop = city.troops.emplace_back();
						troop.target = 0;
					}

					if (hovered_unit != -1)
					{
						auto i = hovered_unit / 100;
						auto j = hovered_unit % 100;
						auto& troop = city.troops[i];
						auto& unit = city.units[troop.units[j]];
						auto& unit_data = unit_datas[unit.id];
						show_unit_detail(mpos, unit.id, unit.lv, calc_hp_stat(unit_data.HP, unit.lv), 0,
							calc_stat(unit_data.ATK, unit.lv), calc_stat(unit_data.DEF, unit.lv), calc_stat(unit_data.SA, unit.lv), 
							calc_stat(unit_data.SD, unit.lv), calc_stat(unit_data.SP, unit.lv), unit_data.type1, unit_data.type2, unit.skills);
					}

					if (!input->mbtn[Mouse_Left])
					{
						dragging_unit = -1;
						if (dragging_target != -1)
						{
							for (auto i = 0; i < tiles.size(); i++)
							{
								auto& tile = tiles[i];
								if (distance(tile.pos + tile_sz * 0.5f, mpos) < tile_sz * 0.5f)
								{
									city.set_troop_target(city.troops[dragging_target], i);
									break;
								}
							}
						}
						dragging_target = -1;
					}
					if (dragging_unit != -1)
					{
						auto& unit = city.units[dragging_unit];
						auto& unit_data = unit_datas[unit.id];
						if (unit_data.icon)
							draw_image(unit_data.icon, input->mpos, vec2(64.f), vec2(0.5f, 0.5f), cvec4(255, 255, 255, 127));
					}
					if (dragging_target != -1)
						draw_image(img_target, input->mpos, vec2(32.f), vec2(0.5f, 0.5f), cvec4(255, 255, 255, 127));
				}
					break;
				}
			}
			hud->end_layout();

		}
			break;
		case TileResourceField:
			if (auto id = main_player.find_resource_field(selected_tile); id != -1)
			{
				auto& resource_field = main_player.resource_fields[id];
				if (!resource_field_datas[resource_field.type].empty())
				{
					hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
					hud->text(std::format(L"{} LV: {}", get_resource_field_name(resource_field.type), resource_field.lv));
					hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
					hud->text(L"Current Production: ", 22);
					hud->image(vec2(20.f, 13.f), img_resources[resource_field.type]);
					hud->text(std::format(L"{}", resource_field.production), 22);
					hud->end_layout();
					if (resource_field.lv < resource_field_datas[resource_field.type].size())
					{
						auto& next_level = resource_field_datas[resource_field.type][resource_field.lv];

						hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
						hud->text(std::format(L"Level {} Production: ", resource_field.lv + 1), 22);
						hud->image(vec2(20.f, 13.f), img_resources[resource_field.type]);
						hud->text(std::format(L"{}", next_level.production), 22);
						hud->end_layout();

						hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
						hud->image(vec2(18.f, 12.f), img_resources[ResourceWood]);
						hud->text(std::format(L"{}", next_level.cost_wood), 16);
						hud->image(vec2(18.f, 12.f), img_resources[ResourceClay]);
						hud->text(std::format(L"{}", next_level.cost_clay), 16);
						hud->image(vec2(18.f, 12.f), img_resources[ResourceIron]);
						hud->text(std::format(L"{}", next_level.cost_iron), 16);
						hud->image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
						hud->text(std::format(L"{}", next_level.cost_crop), 16);
						hud->end_layout();
						hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
						hud->image(vec2(18.f, 12.f), img_resources[ResourceGold]);
						hud->text(std::format(L"{}", next_level.cost_gold), 16);
						hud->image(vec2(18.f, 12.f), img_population);
						hud->text(std::format(L"{}", next_level.cost_population), 16);
						hud->end_layout();

						if (hud->button(L"Upgrade", 18))
							main_player.upgrade_resource_field(resource_field);
					}
					hud->end_layout();
					hud->stroke_item();
				}
			}
			break;
		}
	}
	hud->end();

	hud->begin(vec2(screen_size.x - 100.f, screen_size.y - 300.f), vec2(160.f, 300.f), cvec4(0, 0, 0, 255));
	if (state == GameDay)
	{
		if (hud->button(L"Start Battle"))
			start_battle();
	}
	hud->end();

	if (state == GameBattle)
	{
		auto get_lord_id = [](BattleTroop& battle_troop) {
			if (battle_troop.troop)
				return battle_troop.troop->lord_id;
			return battle_troop.city->lord_id;
		};
		canvas->draw_rect_filled(vec2(100.f, 250.f), vec2(800.f, 350.f), hsv(get_lord_id(battle_troops[0]) * 60.f, 0.5f, 0.5f, 1.f));
		canvas->draw_rect_filled(vec2(100.f, 150.f), vec2(800.f, 250.f), hsv(get_lord_id(battle_troops[1]) * 60.f, 0.5f, 0.5f, 1.f));

		auto hovered_unit = -1;

		for (auto i = 0; i < 2; i++)
		{
			auto& battle_troop = battle_troops[i];
			auto& lord = lords[get_lord_id(battle_troop)];
			for (auto j = 0; j < battle_troop.unit_displays.size(); j++)
			{
				auto& unit = battle_troop.troop->units[j];
				auto& display = battle_troop.unit_displays[j];
				auto& unit_data = unit_datas[display.unit_id];
				auto sz = vec2(64.f) * display.scl.x;
				if (unit_data.icon)
					draw_image(unit_data.icon, display.pos, sz, vec2(0.5f, 1.f), cvec4(255, 255, 255, 255 * display.alpha));
				Rect rect;
				rect.a = display.pos - sz * vec2(0.5f, 1.f);
				rect.b = rect.a + sz;
				if (rect.contains(mpos))
					hovered_unit = i * 100 + j;
				draw_rect(display.init_pos + vec2(-20.f, 5.f), vec2(40.f, 5.f), vec2(0.f), cvec4(150, 150, 150, 255));
				draw_rect(display.init_pos + vec2(-20.f, 5.f), vec2(40.f * ((float)display.HP / (float)unit.HP_MAX), 5.f), vec2(0.f), cvec4(0, 255, 0, 255));
				draw_text(std::format(L"{}/{}", display.HP, unit.HP_MAX), 20, display.init_pos + vec2(0.f, 8.f), vec2(0.5f, 1.f));
				if (!display.label.empty())
					draw_text(display.label, 20, display.label_pos, vec2(0.5f, 0.f), cvec4(0, 0, 0, 255), -vec2(1.f), cvec4(255));
			}
		}

		if (city_damge > 0)
			draw_text(wstr(city_damge), 20, vec2(450.f, 300.f), vec2(0.5f, 0.f), cvec4(0, 0, 0, 255), -vec2(1.f), cvec4(255));

		if (hovered_unit != -1)
		{
			auto i = hovered_unit / 100;
			auto j = hovered_unit % 100;

			auto& battle_troop = battle_troops[i];
			auto& unit = battle_troop.troop->units[j];
			auto& display = battle_troop.unit_displays[j];
			auto& unit_data = unit_datas[display.unit_id];
			show_unit_detail(mpos, display.unit_id, unit.lv, unit.HP, unit.HP_MAX, unit.ATK, unit.DEF, unit.SA, unit.SD, unit.SP, unit.type1, unit.type2, unit.skills);
		}
	}

	hud->begin(vec2(0.f, 0.f), vec2(screen_size.x, 20.f), cvec4(0, 0, 0, 255));
	hud->begin_layout(HudHorizontal, vec2(0.f), vec2(16.f, 0.f));
	hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	hud->image(vec2(27.f, 18.f), img_resources[ResourceWood]);
	hud->text(std::format(L"{} +{}", main_player.resources[ResourceWood], main_player.get_production(ResourceWood)), 24);
	hud->end_layout();
	if (hud->item_hovered())
	{
		hud->begin(mpos + vec2(0.f, 10.f));
		hud->text(std::format(L"Wood: {}\nProduction: {}", main_player.resources[ResourceWood], main_player.get_production(ResourceWood)));
		hud->end();
	}
	hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	hud->image(vec2(27.f, 18.f), img_resources[ResourceClay]);
	hud->text(std::format(L"{} +{}", main_player.resources[ResourceClay], main_player.get_production(ResourceClay)), 24);
	hud->end_layout();
	if (hud->item_hovered())
	{
		hud->begin(mpos + vec2(0.f, 10.f));
		hud->text(std::format(L"Clay: {}\nProduction: {}", main_player.resources[ResourceClay], main_player.get_production(ResourceClay)));
		hud->end();
	}
	hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	hud->image(vec2(27.f, 18.f), img_resources[ResourceIron]);
	hud->text(std::format(L"{} +{}", main_player.resources[ResourceIron], main_player.get_production(ResourceIron)), 24);
	hud->end_layout();
	if (hud->item_hovered())
	{
		hud->begin(mpos + vec2(0.f, 10.f));
		hud->text(std::format(L"Iron: {}\nProduction: {}", main_player.resources[ResourceIron], main_player.get_production(ResourceIron)));
		hud->end();
	}
	hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	hud->image(vec2(27.f, 18.f), img_resources[ResourceCrop]);
	hud->text(std::format(L"{} +{}", main_player.resources[ResourceCrop], main_player.get_production(ResourceCrop)), 24);
	hud->end_layout();
	if (hud->item_hovered())
	{
		hud->begin(mpos + vec2(0.f, 10.f));
		hud->text(std::format(L"Crop: {}\nProduction: {}", main_player.resources[ResourceCrop], main_player.get_production(ResourceCrop)));
		hud->end();
	}
	hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	hud->image(vec2(27.f, 18.f), img_resources[ResourceGold]);
	hud->text(std::format(L"{}", main_player.resources[ResourceGold]), 24);
	hud->end_layout();
	if (hud->item_hovered())
	{
		hud->begin(mpos + vec2(0.f, 10.f));
		hud->text(std::format(L"Gold: {}", main_player.resources[ResourceGold]));
		hud->end();
	}
	hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	hud->image(vec2(27.f, 18.f), img_population);
	hud->text(std::format(L"{}/{}", main_player.consume_population, main_player.provide_population), 24);
	hud->end_layout();
	if (hud->item_hovered())
	{
		hud->begin(mpos + vec2(0.f, 10.f));
		hud->text(std::format(L"Population\nProvide: {}\nConsume: {}", main_player.provide_population, main_player.consume_population));
		hud->end();
	}
	hud->end_layout();
	hud->end();

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
