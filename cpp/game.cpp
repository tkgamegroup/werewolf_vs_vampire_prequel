#include "game.h"

#include <flame/xml.h>
#include <flame/foundation/sheet.h>
#include <flame/foundation/system.h>
#include <flame/graphics/canvas.h>

template <class T>
bool has(const std::vector<T>& list, T v)
{
	for (auto _v : list)
	{
		if (_v == v)
			return true;
	}
	return false;
}

enum TileType
{
	TileField,
	TileCity,
	TileNeutralCamp,
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
	//BuildingBarracks,
	BuildingPark,
	BuildingTrainingMachine,
	BuildingTower,
	BuildingWall,

	BuildingTypeCount,
	BuildingInTownBegin = BuildingTownCenter,
	BuildingInTownEnd = BuildingTrainingMachine,
};

const wchar_t* get_building_name(BuildingType type)
{
	switch (type)
	{
	case BuildingTownCenter: return L"Town Center";
	case BuildingHouse: return L"House";
	//case BuildingBarracks: return L"Barracks";
	case BuildingPark: return L"Park";
	case BuildingTrainingMachine: return L"Training Machine";
	case BuildingTower: return L"Tower";
	case BuildingWall: return L"Wall";
	}
	return L"";
}

const wchar_t* get_building_description(BuildingType type)
{
	switch (type)
	{
	case BuildingTownCenter: return L"";
	case BuildingHouse: return L"Increase Gold Production";
	//case BuildingBarracks: return L"";
	case BuildingPark: return L"Captures Pokemons Every Turn";
	case BuildingTrainingMachine: return L"Training Pokemons\nEach Machine will train one \npokemon in the city with the \norder of from highest to lowest \nlevel of the machines";
	case BuildingTower: return L"";
	case BuildingWall: return L"";
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

enum Stat
{
	StatHP,
	StatATK,
	StatDEF,
	StatSA,
	StatSD,
	StatSP,
	StatACC,
	StatEVA,

	StatCount
};

const wchar_t* get_stat_name(Stat stat)
{
	switch (stat)
	{
	case StatHP: return L"HP";
	case StatATK: return L"ATK";
	case StatDEF: return L"DEF";
	case StatSA: return L"SA";
	case StatSD: return L"SD";
	case StatSP: return L"SP";
	case StatACC: return L"ACC";
	case StatEVA: return L"EVA";
	}
	return L"";
}

Stat get_stat_from_name(std::wstring_view name)
{
	for (auto i = 0; i < StatCount; i++)
	{
		if (name == get_stat_name((Stat)i))
			return (Stat)i;
	}
	return StatCount;
}

enum SkillCategory
{
	SkillCatePhysical,
	SkillCateSpecial,
	SkillCateStatus,

	SkillCategoryCount
};

const wchar_t* get_skill_category_name(SkillCategory cate)
{
	switch (cate)
	{
	case SkillCatePhysical: return L"Physical";
	case SkillCateSpecial: return L"Special";
	case SkillCateStatus: return L"Status";
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
	return SkillCategoryCount;
}

enum EffectType
{
	EffectUserStat,
	EffectOpponentStat,
	EffectStatus,
	EffectRecover,
	EffectLifeSteal,
	EffectMultipleHits,
	EffectDamageDebuff,

	EffectTypeCount
};

const wchar_t* get_effect_type_name(EffectType effect)
{
	switch (effect)
	{
	case EffectUserStat: return L"UserStat";
	case EffectOpponentStat: return L"OpponentStat";
	case EffectStatus: return L"Status";
	}
	return L"";
}

EffectType get_effect_type_from_name(std::wstring_view name)
{
	for (auto i = 0; i < EffectTypeCount; i++)
	{
		if (name == get_effect_type_name((EffectType)i))
			return (EffectType)i;
	}
	return EffectTypeCount;
}

enum AbnormalStatus
{
	AbnormalStatusNone,
	AbnormalStatusBurn,
	AbnormalStatusFreeze,
	AbnormalStatusParalysis,
	AbnormalStatusPoison,
	AbnormalStatusSleep,

	AbnormalStatusCount
};

const wchar_t* get_status_name(AbnormalStatus status)
{
	switch (status)
	{
	case AbnormalStatusBurn: return L"Burn";
	case AbnormalStatusFreeze: return L"Freeze";
	case AbnormalStatusParalysis: return L"Paralysis";
	case AbnormalStatusPoison: return L"Poison";
	case AbnormalStatusSleep: return L"Sleep";
	}
	return L"";
}

AbnormalStatus get_status_from_name(std::wstring_view name)
{
	for (auto i = 0; i < AbnormalStatusCount; i++)
	{
		if (name == get_status_name((AbnormalStatus)i))
			return (AbnormalStatus)i;
	}
	return AbnormalStatusCount;
}

struct SkillEffect
{
	EffectType type;
	union
	{
		struct
		{
			Stat id;
			int state;
			float prob;
		}stat;
		struct
		{
			AbnormalStatus id;
			float prob;
		}status;
	}data;
};

const uint MAX_TROOP_UNITS = 12;
const uint IV_VAL = 31;
const uint BP_VAL = 252;
const uint BASE_EXP = 100;

uint calc_hp_stat(uint base, uint lv)
{
	return uint((base * 2 + IV_VAL + BP_VAL / 4) * lv / 100.f) + lv + 10;
}

uint calc_stat(uint base, uint lv)
{
	return uint((base * 2 + IV_VAL + BP_VAL / 4) * lv / 100.f) + 5;
}

uint calc_exp(uint lv)
{
	return (lv * lv * lv * 4) / 5;
}

uint calc_gain_exp(uint lv)
{
	return (BASE_EXP * lv) / 7;
}

enum TargetType
{
	TargetEnemy,
	TargetSelf
};

struct SkillData
{
	std::wstring name;
	PokemonType type;
	SkillCategory category;
	uint power;
	uint acc;
	uint pp;
	std::wstring effect_text;
	std::vector<SkillEffect> effects;
	TargetType target_type = TargetEnemy;
};
std::vector<SkillData> skill_datas;

struct UnitData
{
	std::wstring name;
	uint cost_gold;
	uint cost_population;
	uint evolution_lv = 0;
	uint stats[StatCount];
	PokemonType type1 = PokemonTypeCount;
	PokemonType type2 = PokemonTypeCount;
	std::vector<std::pair<uint, uint>> skillset;
	graphics::ImagePtr icon = nullptr;
};
std::vector<UnitData> unit_datas;

struct Unit
{
	uint id;
	uint lv;
	uint exp;
	int skills[4] = { -1, -1, -1, -1 };
	std::vector<uint> learnt_skills;
	uint gain_exp = 0;

	void learn_skills()
	{
		auto& unit_data = unit_datas[id];
		for (auto& s : unit_data.skillset)
		{
			if (s.first <= lv)
			{
				if (!has(learnt_skills, s.second))
					learnt_skills.push_back(s.second);
			}
		}
	}
};

struct UnitInstance
{
	Unit* original;
	uint id;
	uint lv;
	uint HP_MAX;
	uint stats[StatCount];
	PokemonType type1;
	PokemonType type2;
	int skills[4] = { -1, -1, -1, -1 };
	int stat_stage[StatCount] = { 0, 0, 0, 0, 0, 0 };
	AbnormalStatus abnormal_status = AbnormalStatusNone;

	void init(uint _id, uint _lv, const int* const _skills)
	{
		id = _id;
		lv = _lv;
		auto& unit_data = unit_datas[id];
		original = nullptr;
		HP_MAX = calc_hp_stat(unit_data.stats[StatHP], lv);
		stats[StatHP] = HP_MAX;
		for (auto i = (int)StatATK; i < StatCount; i++)
			stats[i] = calc_stat(unit_data.stats[i], lv);
		type1 = unit_data.type1;
		type2 = unit_data.type2;
		memcpy(skills, _skills, sizeof(skills));
	}

	void init(Unit& unit)
	{
		init(unit.id, unit.lv, unit.skills);
		original = &unit;
	}

	int choose_skill(UnitInstance& target)
	{
		std::vector<std::pair<uint, uint>> cands;
		for (auto i = 0; i < 4; i++)
		{
			if (auto skill_id = skills[i]; skill_id != -1)
			{
				auto& skill_data = skill_datas[skill_id];
				auto weight = 100;
				if (skill_data.category == SkillCateStatus)
				{
					weight = 0;
					for (auto& e : skill_data.effects)
					{
						if (e.type == EffectOpponentStat)
						{
							auto old_state = target.stat_stage[e.data.stat.id];
							auto new_state = clamp(old_state + e.data.stat.state, -6, +6);
							if (old_state != new_state)
							{
								for (auto i = abs(old_state) + 1; i <= abs(new_state); i++)
									weight += 100 * (pow(0.8f, i) * e.data.stat.prob);
							}
						}
					}
				}
				cands.emplace_back(skill_id, weight);
			}
		}
		if (cands.empty())
			return -1;
		return weighted_random(cands);
	}
};

float get_stage_modifier(int stage)
{
	switch (stage)
	{
	case -6: return  25.f / 100.f;
	case -5: return  28.f / 100.f;
	case -4: return  33.f / 100.f;
	case -3: return  40.f / 100.f;
	case -2: return  50.f / 100.f;
	case -1: return  66.f / 100.f;
	case +1: return 150.f / 100.f;
	case +2: return 200.f / 100.f;
	case +3: return 250.f / 100.f;
	case +4: return 300.f / 100.f;
	case +5: return 350.f / 100.f;
	case +6: return 400.f / 100.f;
	}
	return 1.f;
}

float get_effectineness(PokemonType skill_type, PokemonType caster_type1, PokemonType caster_type2, PokemonType target_type1, PokemonType target_type2)
{
	auto effectiveness1 = target_type1 != PokemonTypeCount ? pokemon_type_effectiveness[skill_type][target_type1] : 1.f;
	auto effectiveness2 = target_type2 != PokemonTypeCount ? pokemon_type_effectiveness[skill_type][target_type2] : 1.f;
	auto effectiveness = effectiveness1 * effectiveness2;
	if (caster_type1 == skill_type || caster_type2 == skill_type)
		effectiveness *= 1.5f;
	return effectiveness;
}

enum SkillResult
{
	SkillHit,
	SkillMiss,
	SkillNoEffect
};

struct StatChange
{
	bool changed = false;
	int stage[StatCount] = { 0, 0, 0, 0, 0, 0 };
};

SkillResult cast_skill(UnitInstance& caster, UnitInstance& target, uint skill_id, uint& damage, StatChange& caster_stat_changed, StatChange& target_stat_changed)
{
	auto& skill_data = skill_datas[skill_id];
	auto effectiveness = get_effectineness(skill_data.type, caster.type1, caster.type2, target.type1, target.type2);
	if (effectiveness == 0.f)
		return SkillNoEffect;

	{
		auto B = skill_data.acc / 100.f;
		auto C = get_stage_modifier(caster.stat_stage[StatACC]);
		auto D = get_stage_modifier(target.stat_stage[StatEVA]);
		auto A = B * C / D;
		if (linearRand(0.f, 1.f) >= A)
			return SkillMiss;
	}

	if (skill_data.power > 0)
	{
		auto A = skill_data.category == SkillCatePhysical ? caster.stats[StatATK] : caster.stats[StatSA];
		auto D = skill_data.category == SkillCatePhysical ? target.stats[StatDEF] : target.stats[StatSD];
		damage = ((2.f * caster.lv + 10.f) / 250.f * ((float)A / (float)D) * skill_data.power + 2.f) * effectiveness;
	}

	for (auto& effect : skill_data.effects)
	{
		switch (effect.type)
		{
		case EffectUserStat:
			if (linearRand(0.f, 1.f) <= effect.data.stat.prob)
			{
				caster_stat_changed.changed = true;
				caster_stat_changed.stage[effect.data.stat.id] += effect.data.stat.state;
			}
			break;
		case EffectOpponentStat:
			if (linearRand(0.f, 1.f) <= effect.data.stat.prob)
			{
				target_stat_changed.changed = true;
				target_stat_changed.stage[effect.data.stat.id] += effect.data.stat.state;
			}
			break;
		}
	}

	return SkillHit;
}

cCameraPtr camera;

graphics::CanvasPtr canvas;

graphics::ImagePtr img_target;

graphics::ImagePtr img_tile_frame;
graphics::ImagePtr img_tile_grass;
graphics::ImagePtr img_tile_castle;

graphics::ImagePtr img_city;
graphics::ImagePtr img_camp;
graphics::ImagePtr img_ground;
graphics::ImagePtr imgs_building[BuildingTypeCount];

graphics::ImagePtr img_resources[ResourceTypeCount] = {};
graphics::ImagePtr img_population;
graphics::ImagePtr img_production;

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
	if (shadow_offset.x != 0.f || shadow_offset.y != 0.f)
		canvas->draw_text(nullptr, font_size, p + shadow_offset, text, shadow_color);
	canvas->draw_text(nullptr, font_size, p, text, color);
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
	int idx1;
	int idx2;

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
	auto path_idx = 0;
	while (path_idx < candidates.size())
	{
		auto [id, parent] = candidates[path_idx];

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
			candidates.push_back({ tile.tile_lt, path_idx });
			marks[tile.tile_lt] = true;
		}
		if (tile.tile_t != -1 && !marks[tile.tile_t])
		{
			candidates.push_back({ tile.tile_t, path_idx });
			marks[tile.tile_t] = true;
		}
		if (tile.tile_rt != -1 && !marks[tile.tile_rt])
		{
			candidates.push_back({ tile.tile_rt, path_idx });
			marks[tile.tile_rt] = true;
		}
		if (tile.tile_lb != -1 && !marks[tile.tile_lb])
		{
			candidates.push_back({ tile.tile_lb, path_idx });
			marks[tile.tile_lb] = true;
		}
		if (tile.tile_b != -1 && !marks[tile.tile_b])
		{
			candidates.push_back({ tile.tile_b, path_idx });
			marks[tile.tile_b] = true;
		}
		if (tile.tile_rb != -1 && !marks[tile.tile_rb])
		{
			candidates.push_back({ tile.tile_rb, path_idx });
			marks[tile.tile_rb] = true;
		}

		path_idx++;
	}
	std::reverse(ret.begin(), ret.end());
	return ret;
}

struct BuildingBaseData
{
	uint cost_wood;
	uint cost_clay;
	uint cost_iron;
	uint cost_crop;
	uint cost_gold;
	uint cost_population;
	uint cost_production;

	void read(Sheet::Row& row, SheetPtr sht)
	{
		cost_wood = sht->get_as<uint>(row, "cost_wood"_h);
		cost_clay = sht->get_as<uint>(row, "cost_clay"_h);
		cost_iron = sht->get_as<uint>(row, "cost_iron"_h);
		cost_crop = sht->get_as<uint>(row, "cost_crop"_h);
		cost_gold = sht->get_as<uint>(row, "cost_gold"_h);
		cost_population = sht->get_as<uint>(row, "cost_population"_h);
		cost_production = sht->get_as<uint>(row, "cost_production"_h);
	}
};

struct TownCenterData : BuildingBaseData
{
};
std::vector<TownCenterData> town_center_datas;

struct HouseData : BuildingBaseData
{
	uint gold_production;
	uint provide_population;
};
std::vector<HouseData> house_datas;

struct BarracksData : BuildingBaseData
{
};
std::vector<BarracksData> barracks_datas;

struct ParkData : BuildingBaseData
{
	std::vector<std::pair<uint, uint>> encounter_list;
	uint capture_num;
};
std::vector<ParkData> park_datas;

struct TrainingMachineData : BuildingBaseData
{
	uint exp;
};
std::vector<TrainingMachineData> training_machine_datas;

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
	//case BuildingBarracks:
	//	if (barracks_datas.size() > lv)
	//		ret = &barracks_datas[lv];
	//	break;
	case BuildingPark:
		if (park_datas.size() > lv)
			ret = &park_datas[lv];
		break;
	case BuildingTrainingMachine:
		if (training_machine_datas.size() > lv)
			ret = &training_machine_datas[lv];
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
	uint				target = 0;	// tile id
	std::vector<uint>	units;		// indices
	std::vector<uint>	path;
};

struct PokemonCapture
{
	uint unit_id;
	uint exclusive_id;
	uint lv;
	uint cost_gold;
};

struct City
{
	uint id;
	uint tile_id;
	uint lord_id;
	uint loyalty;
	uint production;
	std::vector<Building> buildings;
	std::vector<PokemonCapture> captures;
	std::vector<Unit> units;
	std::vector<Troop> troops;

	int get_building_lv(BuildingType type, int slot = -1)
	{
		if (slot != -1)
			return buildings[slot].lv;
		for (auto& building : buildings)
		{
			if (building.type == type)
				return building.lv;
		}
		return -1;
	}

	void add_capture_ll(uint unit_id, uint exclusive_id, uint lv, uint cost_gold)
	{
		auto& unit_data = unit_datas[unit_id];
		PokemonCapture capture;
		capture.unit_id = unit_id;
		capture.exclusive_id = exclusive_id;
		capture.lv = lv;
		capture.cost_gold = cost_gold;

		captures.push_back(capture);
	}

	void add_capture(uint unit_id, uint lv, uint cost_gold)
	{
		add_capture_ll(unit_id, 0, lv, cost_gold);
	}

	void add_exclusive_captures(const std::vector<uint>& unit_ids, uint exclusive_id, uint lv, uint cost_gold)
	{
		for (auto unit_id : unit_ids)
			add_capture_ll(unit_id, exclusive_id, lv, cost_gold);
	}

	void add_unit(uint unit_id, uint lv)
	{
		auto& unit_data = unit_datas[unit_id];

		Unit unit;
		unit.id = unit_id;
		unit.lv = lv;
		unit.exp = calc_exp(lv);
		unit.learn_skills();
		{
			auto n = 0;
			for (auto it = unit.learnt_skills.rbegin(); it != unit.learnt_skills.rend(); it++)
			{
				if (n >= 4)
					break;
				unit.skills[n] = *it;
				n++;
			}
		}

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
	uint city_id;
	uint id;
	std::vector<UnitInstance> units;
	std::vector<uint> path;
	uint path_idx = 0;
	uint defeat_gain_exp = 0;
};

struct BuildingSlot
{
	vec2 pos;
	float radius;
	BuildingType type;
};
std::vector<BuildingSlot> building_slots;

enum ChestType
{
	ChestGold,
	ChestProduction
};

struct Chest
{
	ChestType type;
	uint value;
};

struct NeutralCamp
{
	uint tile_id;
	std::vector<UnitInstance> units;
	Chest chest;
	uint defeat_gain_exp;
};
std::vector<NeutralCamp> neutral_camps;

struct NeutralUnit
{
	uint id;
	uint lv;
	int skills[4] = { -1, -1, -1, -1 };

	void learn_skills()
	{
		auto& unit_data = unit_datas[id];
		auto n = 0;
		for (auto it = unit_data.skillset.rbegin(); it != unit_data.skillset.rend(); it++)
		{
			if (it->first <= lv)
			{
				skills[n] = it->second;
				n++;
				if (n >= 4)
					break;
			}
		}
	}
};

bool add_neutral_camp(uint tile_id, const std::vector<NeutralUnit>& units, ChestType chest_type, uint chest_value)
{
	auto& tile = tiles[tile_id];
	if (tile.type != TileField)
		return false;

	NeutralCamp camp;
	camp.tile_id = tile_id;
	camp.units.resize(units.size());
	for (auto i = 0; i < units.size(); i++)
	{
		auto& unit = units[i];
		camp.units[i].init(unit.id, unit.lv, unit.skills);
		camp.defeat_gain_exp += calc_gain_exp(unit.lv);
	}
	camp.chest.type = chest_type;
	camp.chest.value = chest_value;

	tile.type = TileNeutralCamp;
	tile.idx1 = neutral_camps.size();

	neutral_camps.push_back(camp);
	return true;
}

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

struct BattlePlayer
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
	City* city = nullptr;
	NeutralCamp* camp = nullptr;
	std::vector<UnitDisplay> unit_displays;

	std::vector<UnitInstance>& get_units()
	{
		if (troop)
			return troop->units;
		return camp->units;
	}

	void refresh_display()
	{
		if (!troop && !camp)
			return;
		auto& units = get_units();
		unit_displays.resize(units.size());
		for (auto i = 0; i < units.size(); i++)
		{
			auto& unit = units[i];
			auto& display = unit_displays[i];
			display.unit_id = unit.id;
			display.init_pos = vec2(136.f + i * 75.f, 240.f + 100.f * (1 - side));
			display.HP = unit.stats[StatHP];
			display.pos = display.init_pos;
			display.scl = vec3(1.f);
			display.alpha = 1.f;
		}
	}
};

struct Lord
{
	uint id;

	uint resources[ResourceTypeCount];
	uint provide_population;
	uint consume_population;

	std::vector<City> cities;
	std::vector<uint> territories;
	std::vector<ResourceField> resource_fields;

	std::vector<TroopInstance> troop_instances;

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

		tile.type = TileResourceField;
		tile.idx1 = id;
		tile.idx2 = resource_fields.size();

		resource_fields.push_back(resource_field);

		return true;
	}

	bool build_city(uint tile_id)
	{
		auto& tile = tiles[tile_id];
		if (tile.type != TileField)
			return false;

		City city;
		city.id = cities.size();
		city.tile_id = tile_id;
		city.lord_id = id;
		city.loyalty = 30;
		city.production = 0;
		city.buildings.resize(building_slots.size());
		for (auto i = 0; i < building_slots.size(); i++)
		{
			auto type = building_slots[i].type;
			auto& building = city.buildings[i];
			building.slot = i;
			building.type = type;
		}
		upgrade_building(city, 0, BuildingTownCenter, true);

		city.add_exclusive_captures({ 0, 3, 6 }, "heros"_h, 5, 200);
		city.troops.emplace_back().target = tile_id;

		tile.type = TileCity;
		tile.idx1 = id;
		tile.idx2 = cities.size();

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

	bool upgrade_building(City& city, uint slot, BuildingType type, bool free = false)
	{
		auto& building = city.buildings[slot];
		if (type == BuildingTypeCount)
			type = building.type;

		if (!free)
		{
			if (auto base_data = get_building_base_data(type, building.lv); base_data)
			{
				if (/*resources[ResourceWood] < base_data->cost_wood ||
					resources[ResourceClay] < base_data->cost_clay ||
					resources[ResourceIron] < base_data->cost_iron ||
					resources[ResourceCrop] < base_data->cost_crop ||
					resources[ResourceGold] < base_data->cost_gold ||
					provide_population < consume_population + base_data->cost_population ||*/
					city.production < base_data->cost_production)
					return false;

				//resources[ResourceWood] -= base_data->cost_wood;
				//resources[ResourceClay] -= base_data->cost_clay;
				//resources[ResourceIron] -= base_data->cost_iron;
				//resources[ResourceCrop] -= base_data->cost_crop;
				//resources[ResourceGold] -= base_data->cost_gold;
				//consume_population += base_data->cost_population;
				city.production -= base_data->cost_production;
			}
		}

		building.type = type;

		switch (building.type)
		{
		case BuildingHouse:
		{
			auto& data = house_datas[building.lv];
			//if (building.lv > 0)
			//	provide_population -= house_datas[building.lv - 1].provide_population;
			//provide_population += data.provide_population;
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
		if (resources[ResourceGold] < capture.cost_gold/* ||
			provide_population < consume_population + unit_data.cost_population*/)
			return false;

		resources[ResourceGold] -= capture.cost_gold;
		//consume_population += unit_data.cost_population;

		city.add_unit(capture.unit_id, capture.lv);

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
uint main_player_id = 0;

int add_lord(uint tile_id)
{
	Lord lord;
	lord.id = lords.size();

	//lord.resources[ResourceWood] = 800;
	//lord.resources[ResourceClay] = 800;
	//lord.resources[ResourceIron] = 800;
	//lord.resources[ResourceCrop] = 800;
	lord.resources[ResourceGold] = 300;
	//lord.provide_population = 10;
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

int search_neutral_camp_location()
{
	std::vector<uint> candidates;
	for (auto i = 0; i < tiles.size(); i++)
	{
		auto& tile = tiles[i];
		if (tile.type == TileField)
			candidates.push_back(i);
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
bool game_over = false;
bool victory = false;
bool show_result = false;
BattlePlayer battle_players[2];
std::vector<int> battle_action_list;
std::vector<std::wstring> battle_log;
uint city_damge = 0;
float troop_anim_time = 0.f;
float anim_remain = 0;
float anim_time_scaling = 1.f;
uint exp_multiplier = 1;
uint damage_multiplier = 1;
uint city_damage_multiplier = 1;

void new_day()
{
	if (state == GameDay)
		return;
	state = GameDay;

	for (auto& lord : lords)
	{
		lord.troop_instances.clear();

		for (auto& city : lord.cities)
		{
			city.production += 1;

			std::vector<uint> training_exps;

			for (auto& building : city.buildings)
			{
				switch (building.type)
				{
				case BuildingHouse:
				{
					if (building.lv > 0)
					{
						auto& house_data = house_datas[building.lv - 1];
						lord.resources[ResourceGold] += house_data.gold_production;
					}
				}
					break;
				case BuildingPark:
				{
					if (building.lv > 0)
					{
						auto& park_data = park_datas[building.lv - 1];
						for (auto i = 0; i < park_data.capture_num; i++)
							city.add_capture(weighted_random(park_data.encounter_list), 5, 100);
					}
				}
					break;
				case BuildingTrainingMachine:
				{
					if (building.lv > 0)
					{
						auto& machine_data = training_machine_datas[building.lv - 1];
						training_exps.push_back(machine_data.exp);
					}
				}
					break;
				}
			}

			std::sort(training_exps.begin(), training_exps.end(), [](const auto& a, const auto& b) {
				return a > b;
			});

			auto& city_units = city.troops.front().units;
			auto n = min((int)training_exps.size(), (int)city_units.size());
			for (auto i = 0; i < n; i++)
			{
				auto& unit = city.units[city_units[i]];
				unit.gain_exp += training_exps[i];
			}
		}
	}

	// do ai for all computer players
	for (auto i = 1; i < lords.size(); i++)
	{
		auto& lord = lords[i];

		for (auto& city : lord.cities)
		{
			while (true)
			{
				std::vector<std::pair<uint, uint>> cands;
				auto get_weight = [&](BuildingType type) {
					auto ret = 1;
					switch (type)
					{
					case BuildingHouse:
						if (lord.resources[ResourceGold] < 100)
							ret = 100;
						break;
					case BuildingTrainingMachine:
						ret = 0;
						break;
					}
					return ret;
				};
				for (auto i = 0; i < building_slots.size(); i++)
				{
					auto& slot = building_slots[i];
					if (slot.type == BuildingTypeCount)
					{
						for (int j = BuildingInTownBegin; j <= BuildingInTownEnd; j++)
						{
							if (j == BuildingTownCenter)
								continue;
							if (auto base_data = get_building_base_data((BuildingType)j, 0); base_data)
							{
								auto cost_production = base_data->cost_production;
								if (cost_production <= city.production)
									cands.emplace_back(i * 100 + j, get_weight((BuildingType)j));
							}
						}
					}
					else
					{
						auto& building = city.buildings[i];
						if (auto base_data = get_building_base_data(building.type, building.lv); base_data)
						{
							auto cost_production = base_data->cost_production;
							if (cost_production <= city.production)
								cands.emplace_back(i * 100, get_weight(building.type));
						}
					}
				}
				if (cands.empty())
					break;
				auto sel = weighted_random(cands);
				auto i = sel / 100;
				auto j = sel % 100;
				auto& slot = building_slots[i];
				if (slot.type == BuildingTypeCount)
					lord.upgrade_building(city, i, (BuildingType)j);
				else
					lord.upgrade_building(city, i, slot.type);
			}

			while (true)
			{
				std::vector<uint> cands;
				for (auto i = 0; i < city.captures.size(); i++)
				{
					if (city.captures[i].cost_gold <= lord.resources[ResourceGold])
						cands.push_back(i);
				}
				if (cands.empty())
					break;
				auto i = cands[linearRand(0, (int)cands.size() - 1)];
				lord.buy_unit(city, i);
			}

			if (auto target_city = search_random_hostile_city(lord.id); target_city)
			{
				city.troops.front().units.clear();
				city.troops.resize(2);

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
			for (auto& unit : city.units)
				unit.gain_exp = 0;

			for (auto i = 1; i < city.troops.size(); i++)
			{
				auto& troop = city.troops[i];
				if (troop.target != city.tile_id && troop.path.empty() || troop.units.empty())
					continue;
				TroopInstance troop_ins;
				troop_ins.lord_id = lord.id;
				troop_ins.city_id = city.id;
				troop_ins.id = i;
				troop_ins.path = troop.path;
				for (auto idx : troop.units)
				{
					auto& unit = city.units[idx];
					UnitInstance unit_ins;
					unit_ins.init(unit);
					troop_ins.units.push_back(unit_ins);
					troop_ins.defeat_gain_exp += calc_gain_exp(unit.lv);
				}
				lord.troop_instances.push_back(troop_ins);
			}
		}
	}
}

void step_troop_moving()
{
	if (anim_remain > 0.f)
		return;
	anim_remain = 0.5f * anim_time_scaling;

	for (auto& lord : lords)
	{
		for (auto it = lord.cities.begin(); it != lord.cities.end(); )
		{
			if (it->loyalty == 0)
			{
				auto& tile = tiles[it->tile_id];
				tile.type = TileField;
				tile.idx1 = tile.idx2 = -1;
				it = lord.cities.erase(it);

				for (auto& _lord : lords)
				{
					for (auto& _city : _lord.cities)
					{
						for (auto it2 = _city.troops.begin(); it2 != _city.troops.end(); )
						{
							if (it2->target == tile.id)
								it2 = _city.troops.erase(it2);
							else
								it2++;
						}
					}
				}
			}
			else
				it++;
		}
	}

	auto no_troops = true;
	for (auto& lord : lords)
	{
		if (!lord.troop_instances.empty())
		{
			no_troops = false;
			break;
		}
	}
	if (no_troops)
	{
		if (lords[main_player_id].cities.empty())
		{
			game_over = true;
			victory = false;
		}
		else
		{
			auto no_opponents = true;
			for (auto i = 0; i < lords.size(); i++)
			{
				if (i != main_player_id)
				{
					if (!lords[i].cities.empty())
						no_opponents = false;
					break;
				}
			}
			if (no_opponents)
			{
				game_over = true;
				victory = true;
			}
		}
		if (!game_over)
			show_result = true;
		new_day();
		return;
	}

	for (auto& lord : lords)
	{
		for (auto& troop : lord.troop_instances)
		{
			if (troop.path_idx < troop.path.size() - 1)
				troop.path_idx++;
		}
	}

	for (auto i = 0; i < lords.size(); i++)
	{
		auto& lord = lords[i];
		for (auto j = 0; j < lord.troop_instances.size(); j++)
		{
			auto& troop = lord.troop_instances[j];
			for (auto ii = i + 1; ii < lords.size(); ii++)
			{
				auto& _lord = lords[ii];
				for (auto jj = 0; jj < _lord.troop_instances.size(); jj++)
				{
					auto& _troop = _lord.troop_instances[jj];
					if (troop.path[troop.path_idx] == _troop.path[_troop.path_idx])
					{
						state = GameBattle;
						{
							auto& player = battle_players[0];
							player.troop = &troop;
							player.refresh_display();
						}
						{
							auto& player = battle_players[1];
							player.troop = &_troop;
							player.refresh_display();
						}
						battle_action_list.clear();
						battle_log.clear();
						return;
					}
				}
			}
		}
	}

	for (auto i = 0; i < lords.size(); i++)
	{
		auto& lord = lords[i];
		for (auto j = 0; j < lord.troop_instances.size(); j++)
		{
			auto& troop = lord.troop_instances[j];
			for (auto ii = i + 1; ii < lords.size(); ii++)
			{
				auto& _lord = lords[ii];
				for (auto jj = 0; jj < _lord.troop_instances.size(); jj++)
				{
					auto& _troop = _lord.troop_instances[jj];
					if (troop.path[troop.path_idx] == _troop.path[_troop.path_idx < _troop.path.size() - 1 ? _troop.path_idx + 1 : _troop.path_idx] &&
						_troop.path[_troop.path_idx] == troop.path[troop.path_idx < troop.path.size() - 1 ? troop.path_idx + 1 : troop.path_idx])
					{
						state = GameBattle;
						{
							auto& player = battle_players[0];
							player.troop = &troop;
							player.refresh_display();
						}
						{
							auto& player = battle_players[1];
							player.troop = &_troop;
							player.refresh_display();
						}
						battle_action_list.clear();
						battle_log.clear();
						return;
					}
				}
			}
		}
	}

	for (auto& lord : lords)
	{
		for (auto& troop : lord.troop_instances)
		{
			if (troop.path_idx == troop.path.size() - 1)
			{
				auto tile_id = troop.path.back();
				auto& tile = tiles[tile_id];
				if (tile.type == TileCity || tile.type == TileNeutralCamp)
				{
					state = GameBattle;

					if (tile.type == TileCity)
					{
						auto& city = lords[tile.idx1].cities[tile.idx2];
						{
							auto& player = battle_players[0];
							player.city = &city;
							player.unit_displays.clear();
						}
						{
							auto& player = battle_players[1];
							player.troop = &troop;
							player.refresh_display();
						}
					}
					else if (tile.type == TileNeutralCamp)
					{
						auto& camp = neutral_camps[tile.idx1];
						{
							auto& player = battle_players[0];
							player.camp = &camp;
							player.refresh_display();
						}
						{
							auto& player = battle_players[1];
							player.troop = &troop;
							player.refresh_display();
						}
					}
					battle_action_list.clear();
					battle_log.clear();
				}

				return;
			}
		}
	}
}

void step_battle()
{
	if (anim_remain > 0.f)
		return;
	anim_remain = 0.5f * anim_time_scaling;

	if ((battle_players[0].troop || battle_players[0].camp) && battle_players[1].troop)
	{
		// remove dead units
		for (auto i = 0; i < 2; i++)
		{
			auto& units = battle_players[i].get_units();
			for (auto j = 0; j < units.size(); j++)
			{
				auto& unit = units[j];
				if (unit.stats[StatHP] <= 0)
				{
					for (auto it = battle_action_list.begin(); it != battle_action_list.end(); )
					{
						auto v = *it;
						if (v / 100 == i)
						{
							v = v % 100;
							if (v == j)
								it = battle_action_list.erase(it);
							else if (v > j)
							{
								*it = i * 100 + (v - 1);
								it++;
							}
							else
								it++;
						}
						else
							it++;
					}
					units.erase(units.begin() + j);
					j--;
				}
			}
		}

		auto& units0 = battle_players[0].get_units();
		auto& units1 = battle_players[1].get_units();
		if (units0.empty() || units1.empty())
		{
			auto calc_exp_for_winner = [](BattlePlayer& winner, BattlePlayer& loser) {
				auto exp = loser.troop ? loser.troop->defeat_gain_exp : loser.camp->defeat_gain_exp;
				auto& win_troop_city = lords[winner.troop->lord_id].cities[winner.troop->city_id];
				auto& original_win_troop = win_troop_city.troops[winner.troop->id];
				exp /= original_win_troop.units.size();
				exp *= exp_multiplier;
				for (auto idx : original_win_troop.units)
					win_troop_city.units[idx].gain_exp += exp;
			};
			if (units0.empty() && !units1.empty() && battle_players[1].troop)
				calc_exp_for_winner(battle_players[1], battle_players[0]);
			else if (!units0.empty() && units1.empty() && battle_players[0].troop)
				calc_exp_for_winner(battle_players[0], battle_players[1]);

			for (auto i = 0; i < 2; i++)
			{
				if (battle_players[i].troop)
				{
					if (battle_players[i].troop->units.empty())
					{
						auto& lord = lords[battle_players[i].troop->lord_id];
						for (auto it = lord.troop_instances.begin(); it != lord.troop_instances.end(); it++)
						{
							if (&(*it) == battle_players[i].troop)
							{
								lord.troop_instances.erase(it);
								break;
							}
						}
					}
				}
			}
			state = GameNight;
			battle_players[0].troop = battle_players[1].troop = nullptr;
			battle_players[0].camp = nullptr;
			return;
		}

		for (auto i = 0; i < 2; i++)
			battle_players[i].refresh_display();

		if (battle_action_list.empty())
		{
			std::vector<std::pair<uint, uint>> list;
			for (auto i = 0; i < 2; i++)
			{
				auto& units = battle_players[i].get_units();
				for (auto j = 0; j < units.size(); j++)
				{
					auto& unit = units[j];
					list.emplace_back(i * 100 + j, unit.stats[StatSP]);
				}
			}
			std::sort(list.begin(), list.end(), [](const auto& a, const auto& b) {
				return a.second > b.second;
			});
			battle_action_list.resize(list.size());
			for (auto i = 0; i < battle_action_list.size(); i++)
				battle_action_list[i] = list[i].first;
		}

		auto idx = battle_action_list.front();
		auto side = idx / 100;
		idx = idx % 100;
		auto& action_player = battle_players[side];
		auto& action_units = action_player.get_units();
		auto& opponent_player = battle_players[1 - side];
		auto& opponent_units = opponent_player.get_units();
		auto& caster = action_units[idx];
		auto& caster_unit_data = unit_datas[caster.id];
		auto target_idx = linearRand(0, (int)opponent_units.size() - 1);
		auto& target = opponent_units[target_idx];
		auto& target_unit_data = unit_datas[target.id];
		auto& cast_unit_display = action_player.unit_displays[idx];
		auto& target_unit_display = opponent_player.unit_displays[target_idx];

		if (auto skill_id = caster.choose_skill(target); skill_id != -1)
		{
			auto& skill_data = skill_datas[skill_id];

			uint damage = 0;
			StatChange caster_stat_change;
			StatChange target_stat_change;

			auto result = cast_skill(caster, target, skill_id, damage, caster_stat_change, target_stat_change);
			if (damage_multiplier > 1)
				damage *= damage_multiplier;

			{
				auto id = game.tween->begin_2d_targets();
				game.tween->add_2d_target(id, &cast_unit_display.pos, nullptr, &cast_unit_display.scl, nullptr);
				game.tween->add_2d_target(id, nullptr, nullptr, nullptr, &target_unit_display.alpha);
				game.tween->add_2d_target(id, &target_unit_display.label_pos, nullptr, nullptr, nullptr);
				game.tween->add_int_target(id, (int*)&target_unit_display.HP);
				game.tween->scale_to(id, vec2(1.5f), 0.3f * anim_time_scaling);
				auto target_pos = mix(cast_unit_display.pos, target_unit_display.pos, 0.8f);
				game.tween->move_to(id, target_pos, 0.3f * anim_time_scaling);
				game.tween->set_ease(id, EaseInCubic);
				auto time_cast = game.tween->get_time(id);

				game.tween->set_target(id, 2);
				game.tween->set_channel(id, 2, time_cast);
				std::wstring caster_label = L"";
				std::wstring target_label = L"";
				std::wstring log = L"";
				log = std::format(L"{} LV{} cast {} to {} LV{}", caster_unit_data.name, caster.lv, skill_data.name, target_unit_data.name, target.lv);
				if (result == SkillMiss)
				{
					target_label = L"Miss";
					log += L", missed";
				}
				if (result == SkillNoEffect)
				{
					target_label = L"No Effect";
					log += L", no effect";
				}
				if (damage > 0)
				{
					target_label = wstr(damage);
					log += std::format(L", inflicted {} damage", damage);
				}
				if (target_stat_change.changed)
				{
					for (auto i = (int)StatATK; i < StatCount; i++)
					{
						if (auto v = target_stat_change.stage[i]; v != 0)
						{
							if (!target_label.empty())
								target_label += L"\n";
							auto stage = target.stat_stage[i];
							auto new_val = clamp(stage + v, -6, +6);
							if (stage != new_val)
							{
								switch (new_val - stage)
								{
								case -1:
								{
									auto str = std::format(L"{} fell", get_stat_name((Stat)i));;
									target_label += str;
									log += L", target " + str;
								}
									break;
								case -2:
								{
									auto str = std::format(L"{} harshly fell", get_stat_name((Stat)i));
									target_label += str;
									log += L", target " + str;
								}
									break;
								case +1:
								{
									auto str = std::format(L"{} rose", get_stat_name((Stat)i));
									target_label += str;
									log += L", target " + str;
								}
									break;
								case +2:
								{
									auto str = std::format(L"{} rose sharply", get_stat_name((Stat)i));
									target_label += str;
									log += L", target " + str;
								}
									break;
								}
							}
							else
							{
								if (v < 0)
								{
									auto str = std::format(L"{} won't go any lower", get_stat_name((Stat)i));
									target_label += str;
									log += L", target " + str;
								}
								if (v > 0)
								{
									auto str = std::format(L"{} won't go any higher", get_stat_name((Stat)i));
									target_label += str;
									log += L", target " + str;
								}
							}
						}
					}
				}
				game.tween->set_callback(id, [&, target_label, log]() {
					target_unit_display.label = target_label;
					target_unit_display.label_pos = target_unit_display.init_pos + vec2(0.f, -45.f);
					battle_log.push_back(log);
					if (battle_log.size() > 5)
						battle_log.erase(battle_log.begin());
				});
				game.tween->move_to(id, target_unit_display.init_pos + vec2(0.f, -60.f), 0.4f * anim_time_scaling);
				game.tween->set_callback(id, [&]() {
					target_unit_display.label = L"";
				});

				game.tween->set_target(id, 3);
				game.tween->set_channel(id, 3, time_cast);
				game.tween->int_val_to(id, max(0, (int)target.stats[StatHP] - (int)damage), 0.4f * anim_time_scaling);

				if (target.stats[StatHP] <= 0)
				{
					game.tween->set_target(id, 1);
					game.tween->alpha_to(id, 0.f, 0.4f * anim_time_scaling);
					game.tween->set_ease(id, EaseInCubic);
				}

				game.tween->set_target(id, 0U);
				game.tween->set_channel(id, 0, time_cast);
				game.tween->move_to(id, cast_unit_display.pos, 0.3f * anim_time_scaling);
				game.tween->set_channel(id, 1, time_cast);
				game.tween->scale_to(id, vec2(1.f), 0.3f * anim_time_scaling);
				anim_remain = game.tween->end(id) + 0.1f;
			}

			if (damage > 0)
				target.stats[StatHP] = max(0, (int)target.stats[StatHP] - (int)damage);
			if (caster_stat_change.changed)
			{
				auto& unit_data = unit_datas[caster.id];
				for (auto i = (int)StatATK; i < StatCount; i++)
				{
					if (auto v = caster_stat_change.stage[i]; v != 0)
					{
						auto& stage = caster.stat_stage[i];
						auto new_val = clamp(stage + v, -6, +6);
						if (stage != new_val)
						{
							stage = new_val;
							caster.stats[i] = calc_stat(unit_data.stats[i], caster.lv) * get_stage_modifier(stage);
						}
					}
				}
			}
			if (target_stat_change.changed)
			{
				auto& unit_data = unit_datas[target.id];
				for (auto i = (int)StatATK; i < StatCount; i++)
				{
					if (auto v = target_stat_change.stage[i]; v != 0)
					{
						auto& stage = target.stat_stage[i];
						auto new_val = clamp(stage + v, -6, +6);
						if (stage != new_val)
						{
							stage = new_val;
							target.stats[i] = calc_stat(unit_data.stats[i], target.lv) * get_stage_modifier(stage);
						}
					}
				}
			}
		}

		battle_action_list.erase(battle_action_list.begin());
	}
	else if (battle_players[0].city)
	{
		auto& action_player = battle_players[1];

		if (battle_action_list.size() == 1 && battle_action_list[0] == -1)
		{
			{
				auto troop = battle_players[1].troop;
				auto& lord = lords[troop->lord_id];
				for (auto it = lord.troop_instances.begin(); it != lord.troop_instances.end(); it++)
				{
					if (&*it == troop)
					{
						lord.troop_instances.erase(it);
						break;
					}
				}
			}

			{
				// each unit that attacked the enemy city will level up
				auto& troop = *battle_players[1].troop;
				auto& city = lords[troop.lord_id].cities[troop.city_id];
				for (auto idx : city.troops[troop.id].units)
				{
					auto& unit = city.units[idx];
					unit.gain_exp += calc_exp(unit.lv + 1);;
				}
			}

			auto& city = *battle_players[0].city;
			if (city.loyalty > city_damge)
				city.loyalty -= city_damge;
			else
				city.loyalty = 0;
			city_damge = 0;

			state = GameNight;
			battle_players[1].troop = nullptr;
			battle_players[0].city = nullptr;
			return;
		}

		if (battle_action_list.empty())
		{
			auto& troop = battle_players[1];
			battle_action_list.resize(troop.troop->units.size());
			for (auto i = 0; i < battle_action_list.size(); i++)
				battle_action_list[i] = i;
		}

		auto idx = battle_action_list.front() % 100;
		auto& caster = action_player.troop->units[idx];
		auto& cast_unit_display = action_player.unit_displays[idx];

		{
			auto id = game.tween->begin_2d_targets();
			game.tween->add_2d_target(id, &cast_unit_display.pos, nullptr, &cast_unit_display.scl, nullptr);
			game.tween->add_2d_target(id, &cast_unit_display.label_pos, nullptr, nullptr, nullptr);
			game.tween->scale_to(id, vec2(1.1f), 0.2f * anim_time_scaling);
			auto damage = max(1U, caster.lv / 10);
			damage *= city_damage_multiplier;
			game.tween->set_callback(id, [&, damage]() {
				cast_unit_display.label = wstr(damage);
				cast_unit_display.label_pos = cast_unit_display.init_pos + vec2(0.f, 5.f);
			});
			game.tween->scale_to(id, vec2(1.f), 0.2f * anim_time_scaling);
			game.tween->set_target(id, 1);
			game.tween->move_to(id, vec2(450.f, 300.f), 0.5f * anim_time_scaling);
			game.tween->set_callback(id, [&, damage]() {
				cast_unit_display.label = L"";
				city_damge += damage;
			});
			game.tween->end(id);
		}

		battle_action_list.erase(battle_action_list.begin());
		if (battle_action_list.empty())
		{
			battle_action_list = {-1};
			anim_remain = 1.5f * anim_time_scaling;
			return;
		}
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
	img_camp = graphics::Image::get(L"assets/camp.png");
	img_ground = graphics::Image::get(L"assets/ground.png");
	imgs_building[BuildingTownCenter] = graphics::Image::get(L"assets/town_center.png");
	imgs_building[BuildingHouse] = graphics::Image::get(L"assets/house.png");
	//imgs_building[BuildingBarracks] = graphics::Image::get(L"assets/barracks.png");
	imgs_building[BuildingPark] = graphics::Image::get(L"assets/park.png");
	imgs_building[BuildingTower] = graphics::Image::get(L"assets/tower.png");
	imgs_building[BuildingWall] = graphics::Image::get(L"assets/wall.png");
	img_resources[ResourceWood] = graphics::Image::get(L"assets/icons/wood.png");
	img_resources[ResourceClay] = graphics::Image::get(L"assets/icons/clay.png");
	img_resources[ResourceIron] = graphics::Image::get(L"assets/icons/iron.png");
	img_resources[ResourceCrop] = graphics::Image::get(L"assets/icons/crop.png");
	img_resources[ResourceGold] = graphics::Image::get(L"assets/icons/gold.png");
	img_population = graphics::Image::get(L"assets/icons/population.png");
	img_production = graphics::Image::get(L"assets/icons/production.png");
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

	battle_players[0].side = 0;
	battle_players[1].side = 1;

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
			data.effect_text = sht->get_as_wstr(row, "effect_text"_h);
			auto effect = sht->get_as_wstr(row, "effect"_h);
			for (auto t : SUW::split(effect, ';'))
			{
				auto sp = SUW::split(t, ',');
				if (sp.size() > 0)
				{
					auto type = get_effect_type_from_name(sp[0]);
					if (type != EffectTypeCount)
					{
						switch (type)
						{
						case EffectUserStat:
							if (sp.size() == 4)
							{
								SkillEffect effect;
								effect.type = type;
								effect.data.stat.id = get_stat_from_name(sp[1]);
								effect.data.stat.state = s2t<int>(std::wstring(sp[2]));
								effect.data.stat.prob = s2t<float>(std::wstring(sp[3]));
								data.effects.push_back(effect);
							}
							break;
						case EffectOpponentStat:
							if (sp.size() == 4)
							{
								SkillEffect effect;
								effect.type = type;
								effect.data.stat.id = get_stat_from_name(sp[1]);
								effect.data.stat.state = s2t<int>(std::wstring(sp[2]));
								effect.data.stat.prob = s2t<float>(std::wstring(sp[3]));
								data.effects.push_back(effect);
							}
							break;
						case EffectStatus:

							break;
						}
					}
				}
			}
			if (data.category == SkillCateStatus)
			{
				auto no_target = true;
				for (auto& effect : data.effects)
				{
					if (effect.type == EffectOpponentStat)
					{
						no_target = false;
						break;
					}
				}
				if (no_target)
					data.target_type = TargetSelf;
			}
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
			data.evolution_lv = sht->get_as<uint>(row, "evolution_lv"_h);
			data.stats[StatHP] = sht->get_as<uint>(row, "HP"_h);
			data.stats[StatATK] = sht->get_as<uint>(row, "ATK"_h);
			data.stats[StatDEF] = sht->get_as<uint>(row, "DEF"_h);
			data.stats[StatSA] = sht->get_as<uint>(row, "SA"_h);
			data.stats[StatSD] = sht->get_as<uint>(row, "SD"_h);
			data.stats[StatSP] = sht->get_as<uint>(row, "SP"_h);
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
							data.skillset.emplace_back(lv, skill_id);
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
		for (auto i = 0; i < unit_datas.size(); i++)
		{
			auto& unit_data = unit_datas[i];
			if (unit_data.evolution_lv != 0)
			{
				auto& evo_unit_data = unit_datas[i + 1];
				evo_unit_data.skillset.insert(evo_unit_data.skillset.end(), unit_data.skillset.begin(), unit_data.skillset.end());
			}
		}
		for (auto& unit_data : unit_datas)
		{
			for (auto i = 0; i < unit_data.skillset.size(); i++)
			{
				auto v = unit_data.skillset[i];
				for (auto it = unit_data.skillset.begin() + i + 1; it != unit_data.skillset.end(); )
				{
					if (it->second == v.second)
					{
						if (it->first < v.first)
							v.first = it->first;
						it = unit_data.skillset.erase(it);
					}
					else
						it++;
				}
			}
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
			data.gold_production = sht->get_as<uint>(row, "gold_production"_h);
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
			{
				auto str = sht->get_as_wstr(row, "encounter_list"_h);
				for (auto t : SUW::split(str, ','))
				{
					auto sp = SUW::split(t, ':');
					if (sp.size() == 2)
					{
						auto name = std::wstring(sp[0]);
						auto weight = s2t<uint>(std::wstring(sp[1]));
						auto id = -1;
						for (auto i = 0; i < unit_datas.size(); i++)
						{
							if (unit_datas[i].name == name)
							{
								id = i;
								break;
							}
						}
						if (id != -1)
							data.encounter_list.emplace_back(id, weight);
					}
				}
			}
			data.capture_num = sht->get_as<uint>(row, "capture_num"_h);

			park_datas.push_back(data);
		}
	}
	if (auto sht = Sheet::get(L"assets/training_machine.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			TrainingMachineData data;
			auto& row = sht->rows[i];
			data.read(row, sht);
			data.exp = sht->get_as<uint>(row, "exp"_h);
			training_machine_datas.push_back(data);
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

	for (auto i = 0; i < 10; i++)
	{
		if (auto tile_id = search_neutral_camp_location(); tile_id != -1)
		{
			auto n = linearRand(2U, 5U);
			std::vector<NeutralUnit> units;
			units.resize(n);
			for (auto i = 0; i < n; i++)
			{
				auto& unit = units[i];
				std::vector<uint> cands;
				for (auto id = 9; id < 20; id++)
				{
					if (unit_datas[id].evolution_lv != 0 &&
						unit_datas[id - 1].evolution_lv == 0)
						cands.push_back(id);
				}
				unit.id = cands[linearRand(0U, (uint)cands.size() - 1)];
				unit.lv = linearRand(5, 10);
				unit.learn_skills();
			}
			auto chest_type = linearRand(0, 100) < 50 ? ChestGold : ChestProduction;
			auto chest_value = chest_type == ChestGold ? n * 50 + linearRand(0, 100) : 1;
			add_neutral_camp(tile_id, units, chest_type, chest_value);
		}
	}

	new_day();
}

void Game::on_render()
{
	if (lords.empty())
		return;

	auto& main_player = lords[main_player_id];
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
			draw_text(wstr(city.loyalty), 20, tile.pos + vec2(tile_sz, tile_sz_y) * 0.5f + vec2(0.f, -20.f), vec2(0.5f), hsv(lord.id * 60.f, 0.5f, 1.f, 1.f), vec2(1.f), cvec4(0, 0, 0, 255));
		}
		for (auto& field : lord.resource_fields)
		{
			auto pos = tiles[field.tile_id].pos + vec2(tile_sz, tile_sz_y) * 0.5f;
			draw_image(img_resources[field.type], pos, vec2(36.f, 24.f), vec2(0.5f));
		}
		//{
		//	std::vector<vec2> strips;
		//	for (auto id : lord.territories)
		//	{
		//		auto& tile = tiles[id];
		//		vec2 pos[6];
		//		for (auto i = 0; i < 6; i++)
		//			pos[i] = arc_point(tile.pos + vec2(tile_sz, tile_sz_y) * 0.5f, i * 60.f, tile_sz * 0.5f);
		//		if (tile.tile_rb == -1 || !lord.has_territory(tile.tile_rb))
		//			make_line_strips<2>(pos[0], pos[1], strips);
		//		if (tile.tile_b == -1 || !lord.has_territory(tile.tile_b))
		//			make_line_strips<2>(pos[1], pos[2], strips);
		//		if (tile.tile_lb == -1 || !lord.has_territory(tile.tile_lb))
		//			make_line_strips<2>(pos[2], pos[3], strips);
		//		if (tile.tile_lt == -1 || !lord.has_territory(tile.tile_lt))
		//			make_line_strips<2>(pos[3], pos[4], strips);
		//		if (tile.tile_t == -1 || !lord.has_territory(tile.tile_t))
		//			make_line_strips<2>(pos[4], pos[5], strips);
		//		if (tile.tile_rt == -1 || !lord.has_territory(tile.tile_rt))
		//			make_line_strips<2>(pos[5], pos[0], strips);
		//	}
		//	canvas->path = strips;
		//	canvas->stroke(2.f, hsv(lord.id * 60.f, 0.5f, 1.f, 0.5f), false);
		//}
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
				if (auto path_idx = move_idx + 1; path_idx < path.size())
				{
					auto t = state == GameBattle ? 0.5f : fract(1.f - anim_remain * 1.9f);
					end_point = mix(end_point, tiles[path[path_idx]].pos + vec2(tile_sz) * 0.5f, t);
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
		for (auto& troop : lord.troop_instances)
			draw_troop_path(troop.path, troop.path_idx);
	}
	for (auto& camp : neutral_camps)
	{
		auto& tile = tiles[camp.tile_id];
		draw_image(img_camp, tile.pos + vec2(tile_sz, tile_sz_y) * 0.5f, vec2(tile_sz, tile_sz_y) * 0.7f, vec2(0.5f));
	}

	if (!hud->is_modal())
	{
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

	auto show_pokemon_type = [&](PokemonType type) {
		hud->push_style_color(HudStyleColorTextBg, get_pokemon_type_color(type));
		hud->text(get_pokemon_type_name(type), 20);
		hud->pop_style_color(HudStyleColorTextBg);
	};

	auto show_unit_basic = [&](uint id, uint lv, uint HP, uint HP_MAX, uint ATK, uint DEF, uint SA, uint SD, uint SP, PokemonType type1, PokemonType type2) {
		auto& unit_data = unit_datas[id];
		hud->begin_layout(HudVertical);
		hud->text(std::format(L"{} LV {}", unit_data.name, lv));
		hud->begin_layout(HudHorizontal);
		if (unit_data.type1 != PokemonTypeCount)
			show_pokemon_type(unit_data.type1);
		if (unit_data.type2 != PokemonTypeCount)
			show_pokemon_type(unit_data.type2);
		hud->end_layout();
		if (HP_MAX == 0)
			hud->text(std::format(L"HP {}\nATK {}\nDEF {}\nSA {}\nSD {}\nSP {}", HP, ATK, DEF, SA, SD, SP), 20);
		else
			hud->text(std::format(L"HP {}/{}\nATK {}\nDEF {}\nSA {}\nSD {}\nSP {}", HP, HP_MAX, ATK, DEF, SA, SD, SP), 20);
		hud->end_layout();
	};

	auto show_skill = [&](uint id) {
		auto& skill_data = skill_datas[id];
		hud->begin_layout(HudVertical);
		hud->text(skill_data.name);
		show_pokemon_type(skill_data.type);
		if (skill_data.power > 0)
			hud->text(std::format(L"Power {}", skill_data.power));
		hud->text(skill_data.effect_text, 18);
		hud->end_layout();
	};

	auto popup_unit_detail = [&](const vec2& mpos, uint id, uint lv, uint HP, uint HP_MAX, uint ATK, uint DEF, uint SA, uint SD, uint SP, PokemonType type1, PokemonType type2, int* skills) {
		auto& unit_data = unit_datas[id];
		hud->begin("popup"_h, mpos + vec2(10.f, 4.f), vec2(0.f), cvec4(50, 50, 50, 255), vec2(0.f, 1.f));
		hud->begin_layout(HudHorizontal, vec2(0.f), vec2(4.f, 0.f));
		show_unit_basic(id, lv, HP, HP_MAX, ATK, DEF, SA, SD, SP, type1, type2);
		if (skills)
		{
			for (auto i = 0; i < 4; i++)
			{
				if (auto skill_id = skills[i]; skill_id != -1)
				{
					show_skill(skill_id);
					hud->stroke_item();
				}
			}
		}
		hud->end_layout();
		hud->end();
	};

	auto bottom_pannel_height = 275.f;
	hud->begin("bottom"_h, vec2(0.f, screen_size.y - bottom_pannel_height), vec2(screen_size.x, bottom_pannel_height), cvec4(0, 0, 0, 255));
	auto rect = hud->wnd_rect();
	if (selected_tile != -1)
	{
		auto& tile = tiles[selected_tile];
		switch (tile.type)
		{
		case TileField:
			//if (main_player.has_territory(selected_tile))
			//{
			//	hud->begin_layout(HudHorizontal);

			//	auto show_build_resource_field = [&](ResourceType type) {
			//		hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
			//		hud->text(get_resource_field_name(type));
			//		if (!resource_field_datas[type].empty())
			//		{
			//			auto& first_level = resource_field_datas[type].front();

			//			hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
			//			hud->text(L"Production: ", 22);
			//			hud->image(vec2(20.f, 13.f), img_resources[type]);
			//			hud->text(std::format(L"{}", first_level.production), 22);
			//			hud->end_layout();

			//			hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
			//			hud->image(vec2(18.f, 12.f), img_resources[ResourceWood]);
			//			hud->text(std::format(L"{}", first_level.cost_wood), 16);
			//			hud->image(vec2(18.f, 12.f), img_resources[ResourceClay]);
			//			hud->text(std::format(L"{}", first_level.cost_clay), 16);
			//			hud->image(vec2(18.f, 12.f), img_resources[ResourceIron]);
			//			hud->text(std::format(L"{}", first_level.cost_iron), 16);
			//			hud->image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
			//			hud->text(std::format(L"{}", first_level.cost_crop), 16);
			//			hud->end_layout();
			//			hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
			//			hud->image(vec2(18.f, 12.f), img_resources[ResourceGold]);
			//			hud->text(std::format(L"{}", first_level.cost_gold), 16);
			//			hud->image(vec2(18.f, 12.f), img_population);
			//			hud->text(std::format(L"{}", first_level.cost_population), 16);
			//			hud->end_layout();

			//			if (hud->button(L"Build", 18))
			//				main_player.build_resource_field(selected_tile, type);
			//		}
			//		hud->end_layout();
			//		hud->stroke_item();
			//	};

			//	show_build_resource_field(ResourceWood);
			//	show_build_resource_field(ResourceClay);
			//	show_build_resource_field(ResourceIron);
			//	show_build_resource_field(ResourceCrop);

			//	hud->end_layout();
			//}
			break;
		case TileCity:
		{
			enum Tab
			{
				TabBuildings,
				TabRecruit,
				TabUnits,
				TabTroops
			};
			static Tab tab = TabBuildings;

			auto& lord = lords[tile.idx1];
			auto& city = lord.cities[tile.idx2];

			hud->begin_layout(HudVertical, vec2(0.f), vec2(0.f, 8.f));
			hud->begin_layout(HudHorizontal);
			hud->text(lord.id == main_player_id ? L"Your City" : L"Enemy's City");
			if (lord.id == main_player_id)
			{
				hud->image(vec2(24.f, 24.f), img_production);
				hud->text(wstr(city.production));
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
					if (hud->button(std::format(L"Recruit({})", (int)city.captures.size())))
						tab = TabRecruit;
					hud->pop_style_color(HudStyleColorButton);
				}
				else
				{
					if (hud->button(std::format(L"Recruit({})", (int)city.captures.size())))
						tab = TabRecruit;
				}
				if (tab != TabUnits)
				{
					hud->push_style_color(HudStyleColorButton, cvec4(127, 127, 127, 255));
					if (hud->button(L"Units"))
						tab = TabUnits;
					hud->pop_style_color(HudStyleColorButton);
				}
				else
				{
					if (hud->button(L"Units"))
						tab = TabUnits;
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
			if (lord.id == main_player_id)
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
							if (slot.type != BuildingTypeCount && slot.type > BuildingInTownEnd)
								break;
							if (distance(mpos, c + slot.pos) < slot.radius)
							{
								hovering_slot = i;
								ok = true;
								break;
							}
						}
						//if (!ok)
						//{
						//	for (auto i = 0; i < 6; i++)
						//	{
						//		if (distance(mpos, corner_pos[i]) < 8.f)
						//		{
						//			hovering_slot = building_slots.size() - 2;
						//			ok = true;
						//			break;
						//		}
						//	}
						//}
						//if (!ok)
						//{
						//	for (auto i = 0; i < 6; i++)
						//	{
						//		if (convex_contains(mpos, { &wall_rect[4 * i], &wall_rect[4 * i + 4] }))
						//			hovering_slot = building_slots.size() - 1;
						//	}
						//}
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

					//{
					//	auto col = cvec4(255);
					//	if (hovering_slot == building_slots.size() - 1)
					//		col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
					//	if (city.get_building_lv(BuildingWall) == 0)
					//		col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
					//	for (auto i = 0; i < 6; i++)
					//	{
					//		canvas->draw_image_polygon(img_wall->get_view(), { wall_rect[4 * i + 0], wall_rect[4 * i + 3], wall_rect[4 * i + 2], wall_rect[4 * i + 1] },
					//			{ vec2(0.f, 1.f), vec2(2.f, 1.f), vec2(2.f, 0.f), vec2(0.f, 0.f) }, col, sp_repeat);
					//	}
					//}

					//{
					//	auto col = cvec4(255);
					//	if (hovering_slot == building_slots.size() - 2)
					//		col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
					//	if (city.get_building_lv(BuildingTower) == 0)
					//		col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
					//	for (auto i = 0; i < 6; i++)
					//		draw_image(img_tower, corner_pos[i], vec2(img_tower->extent) * 0.5f, vec2(0.5f, 0.8f), col);
					//}

					for (auto i = 0; i < building_slots.size(); i++)
					{
						auto& slot = building_slots[i];
						if (slot.type != BuildingTypeCount && slot.type > BuildingInTownEnd)
							break;
						auto col = cvec4(255);
						if (hovering_slot == i)
							col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
						if (city.get_building_lv(slot.type, i) == 0)
							col = cvec4(col.r / 2, col.g / 2, col.b / 2, 255);
						if (slot.type != BuildingTypeCount)
						{
							auto img = imgs_building[slot.type];
							draw_image(img, c + slot.pos, vec2(img->extent) * 0.5f, vec2(0.5f, 0.8f), col);
						}
						else
						{
							auto img = imgs_building[BuildingHouse];
							draw_image(img, c + slot.pos, vec2(img->extent) * 0.5f, vec2(0.5f, 0.8f), col);
						}
					}

					static int selected_building_slot = -1;
					if (input->mpressed(Mouse_Left))
					{
						if (hovering_slot != -1)
							selected_building_slot = hovering_slot;
					}

					if (selected_building_slot != -1)
					{
						auto& building = city.buildings[selected_building_slot];
						auto show_upgrade_building = [&](BuildingType type) {
							if (auto next_level = get_building_base_data(type, building.lv); next_level)
							{
								//hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
								//hud->image(vec2(18.f, 12.f), img_resources[ResourceWood]);
								//hud->text(std::format(L"{}", next_level->cost_wood), 16, main_player.resources[ResourceWood] >= next_level->cost_wood ? cvec4(255) : cvec4(255, 0, 0, 255));
								//hud->image(vec2(18.f, 12.f), img_resources[ResourceClay]);
								//hud->text(std::format(L"{}", next_level->cost_clay), 16, main_player.resources[ResourceClay] >= next_level->cost_clay ? cvec4(255) : cvec4(255, 0, 0, 255));
								//hud->image(vec2(18.f, 12.f), img_resources[ResourceIron]);
								//hud->text(std::format(L"{}", next_level->cost_iron), 16, main_player.resources[ResourceIron] >= next_level->cost_iron ? cvec4(255) : cvec4(255, 0, 0, 255));
								//hud->image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
								//hud->text(std::format(L"{}", next_level->cost_crop), 16, main_player.resources[ResourceCrop] >= next_level->cost_crop ? cvec4(255) : cvec4(255, 0, 0, 255));
								//hud->end_layout();
								hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
								//hud->image(vec2(18.f, 12.f), img_resources[ResourceGold]);
								//hud->text(std::format(L"{}", next_level->cost_gold), 16, main_player.resources[ResourceGold] >= next_level->cost_gold ? cvec4(255) : cvec4(255, 0, 0, 255));
								//hud->image(vec2(18.f, 12.f), img_population);
								//hud->text(std::format(L"{}", next_level->cost_population), 16, (int)main_player.provide_population - (int)main_player.consume_population >= next_level->cost_population ? cvec4(255) : cvec4(255, 0, 0, 255));
								hud->image(vec2(16.f, 16.f), img_production);
								hud->text(std::format(L"{}", next_level->cost_production), 16, city.production >= next_level->cost_production ? cvec4(255) : cvec4(255, 0, 0, 255));
								hud->end_layout();

								if (hud->button(building.lv == 0 ? L"Build" : L"Upgrade", 18))
									main_player.upgrade_building(city, selected_building_slot, type);
							}
						};
						auto show_encounter_list = [&](const std::vector<std::pair<uint, uint>>& encounter_list) {
							hud->begin("popup"_h, mpos + vec2(10.f, 4.f), vec2(0.f), cvec4(50, 50, 50, 255), vec2(0.f, 1.f));
							hud->begin_layout(HudHorizontal);
							for (auto i = 0; i < encounter_list.size(); i++)
							{
								auto& unit_data = unit_datas[encounter_list[i].first];
								if (unit_data.icon)
									hud->image(vec2(48.f), unit_data.icon);

							}
							hud->end_layout();
							hud->end();
						};
						switch (building.type)
						{
						case BuildingTownCenter:
							hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
							hud->text(std::format(L"Town Center LV: {}", building.lv));
							show_upgrade_building(building.type);
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
								hud->text(std::format(L"Current Gold Production: {}", data.gold_production), 22);
								//hud->text(std::format(L"Current Provide Population: {}", data.provide_population), 22);
							}
							if (building.lv < house_datas.size())
							{
								auto next_level = house_datas[building.lv];
								hud->text(std::format(L"Level {} Gold Production: {}", building.lv + 1, next_level.gold_production), 22);
								//hud->text(std::format(L"Level {} Provide Population: {}", building.lv + 1, next_level.provide_population), 22);
							}
							show_upgrade_building(building.type);
							hud->end_layout();
							hud->stroke_item();
						}
							break;
						//case BuildingBarracks:
						//{
						//	hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
						//	hud->text(std::format(L"Barracks LV: {}", building.lv));
						//	show_upgrade_building(building.type);
						//	hud->end_layout();
						//	hud->stroke_item();
						//}
						//	break;
						case BuildingPark:
						{
							auto hovering_list = -1;

							hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
							hud->text(std::format(L"Park LV: {}", building.lv));
							if (building.lv > 0)
							{
								auto& data = park_datas[building.lv - 1];
								hud->text(std::format(L"Current Encounter List: {}", (int)data.encounter_list.size()), 22);
								if (hud->item_hovered())
									hovering_list = 0;
							}
							if (building.lv < park_datas.size())
							{
								auto next_level = park_datas[building.lv];
								hud->text(std::format(L"Level {} Encounter List: {}", building.lv + 1, (int)next_level.encounter_list.size()), 22);
								if (hud->item_hovered())
									hovering_list = 1;
							}
							show_upgrade_building(building.type);
							hud->end_layout();
							hud->stroke_item();

							if (hovering_list != -1)
							{
								auto& data = park_datas[building.lv + (hovering_list == 0 ? -1 : 0)];
								show_encounter_list(data.encounter_list);
							}
						}
							break;
						case BuildingTrainingMachine:
							hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
							hud->text(std::format(L"Training Machine LV: {}", building.lv));
							if (building.lv > 0)
							{
								auto& data = training_machine_datas[building.lv - 1];
								hud->text(std::format(L"Current Exp: {}", data.exp), 22);
							}
							if (building.lv < training_machine_datas.size())
							{
								auto next_level = training_machine_datas[building.lv];
								hud->text(std::format(L"Level {} Exp: {}", building.lv + 1, next_level.exp), 22);
							}
							show_upgrade_building(building.type);
							hud->end_layout();
							hud->stroke_item();
							break;
						case BuildingTower:
							hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
							hud->text(std::format(L"Tower LV: {}", building.lv));
							show_upgrade_building(building.type);
							hud->end_layout();
							hud->stroke_item();
							break;
						case BuildingWall:
							hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
							hud->text(std::format(L"Wall LV: {}", building.lv));
							show_upgrade_building(building.type);
							hud->end_layout();
							hud->stroke_item();
							break;
						case BuildingTypeCount:
							for (int i = BuildingInTownBegin; i <= BuildingInTownEnd; i++)
							{
								if (i == BuildingTownCenter)
									continue;
								hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
								hud->text(std::format(L"Build {}", get_building_name((BuildingType)i)));
								hud->text(get_building_description((BuildingType)i), 20);
								show_upgrade_building((BuildingType)i);
								hud->end_layout();
								hud->stroke_item();
							}
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
						auto& capture = city.captures[i];
						auto& unit_data = unit_datas[capture.unit_id];
						if (unit_data.icon)
						{
							hud->begin_layout(HudVertical);
							hud->image_button(vec2(64.f), unit_data.icon);
							if (hud->item_hovered())
								hovered_unit = i;
							const auto scl = 0.7f;
							hud->begin_layout(HudHorizontal);
							hud->image(vec2(27.f, 18.f) * scl, img_resources[ResourceGold]);
							hud->text(std::format(L"{}", capture.cost_gold), 24 * scl);
							hud->end_layout();
							//hud->begin_layout(HudHorizontal);
							//hud->image(vec2(27.f, 18.f) * scl, img_population);
							//hud->text(std::format(L"{}", unit_data.cost_population), 24 * scl);
							//hud->end_layout();
							hud->end_layout();
						}
					}
					hud->end_layout();

					if (hovered_unit != -1)
					{
						auto& capture = city.captures[hovered_unit];
						auto& unit_data = unit_datas[capture.unit_id];
						popup_unit_detail(mpos, capture.unit_id, capture.lv, calc_hp_stat(unit_data.stats[StatHP], capture.lv), 0,
							calc_stat(unit_data.stats[StatATK], capture.lv), calc_stat(unit_data.stats[StatDEF], capture.lv), calc_stat(unit_data.stats[StatSA], capture.lv),
							calc_stat(unit_data.stats[StatSD], capture.lv), calc_stat(unit_data.stats[StatSP], capture.lv), unit_data.type1, unit_data.type2, nullptr);
						if (input->mpressed(Mouse_Left))
							lord.buy_unit(city, hovered_unit);
					}
				}
					break;
				case TabUnits:
				{
					static uint selected_unit = 0;
					static int dragging_skill = -1;
					auto hovered_unit = -1;
					auto hovered_skill = -1;
					if (selected_unit >= city.units.size())
						selected_unit = 0;
					const float size = 64.f;
					hud->begin_layout(HudHorizontal);
					if (city.units.empty())
						hud->rect(vec2(size), cvec4(0));
					for (auto i = 0; i < city.units.size(); i++)
					{
						auto& unit = city.units[i];
						auto& unit_data = unit_datas[unit.id];
						if (unit_data.icon)
						{
							if (hud->image_button(vec2(size), unit_data.icon))
								selected_unit = i;
							if (hud->item_hovered())
								hovered_unit = i;
						}
					}
					hud->end_layout();
					if (selected_unit < city.units.size())
					{
						auto& unit = city.units[selected_unit];
						auto& unit_data = unit_datas[unit.id];

						hud->begin_layout(HudHorizontal, vec2(0.f), vec2(16.f, 0.f));

						show_unit_basic(unit.id, unit.lv, calc_hp_stat(unit_data.stats[StatHP], unit.lv), 0,
							calc_stat(unit_data.stats[StatATK], unit.lv), calc_stat(unit_data.stats[StatDEF], unit.lv),
							calc_stat(unit_data.stats[StatSA], unit.lv), calc_stat(unit_data.stats[StatSD], unit.lv),
							calc_stat(unit_data.stats[StatSP], unit.lv), unit_data.type1, unit_data.type2);

						hud->begin_layout(HudVertical, vec2(0.f), vec2(0.f, 8.f));
						hud->text(L"Skills In Use:", 20);
						hud->begin_layout(HudHorizontal, vec2(0.f), vec2(8.f, 0.f));
						for (auto i = 0; i < 4; i++)
						{
							auto skill_id = unit.skills[i];
							hud->rect(vec2(64.f, 32.f), cvec4(100, 100, 100, 255));
							auto pos = hud->item_rect().a;
							if (hud->item_hovered())
							{
								hud->stroke_item();
								if (skill_id != -1)
									hovered_skill = skill_id;
								if (dragging_skill != -1 && input->mreleased(Mouse_Left))
								{
									if (dragging_skill > 1000)
										std::swap(unit.skills[dragging_skill - 1001], unit.skills[i]);
									else
									{
										auto skill_id = unit.learnt_skills[dragging_skill];
										for (auto i = 0; i < 4; i++)
										{
											if (unit.skills[i] == skill_id)
												unit.skills[i] = -1;
										}
										unit.skills[i] = skill_id;
									}
								}
							}
							if (hud->item_clicked())
							{
								if (skill_id != -1)
									dragging_skill = 1001 + i;
							}
							if (skill_id != -1)
							{
								auto& skill_data = skill_datas[skill_id];
								draw_text(skill_data.name, 18, pos);
							}
						}
						hud->end_layout();
						hud->text(L"Learnt Skills:", 20);
						hud->begin_layout(HudHorizontal, vec2(0.f), vec2(8.f, 0.f));
						for (auto i = 0; i < unit.learnt_skills.size(); i++)
						{
							auto skill_id = unit.learnt_skills[i];
							hud->rect(vec2(64.f, 32.f), cvec4(100, 100, 100, 255));
							auto pos = hud->item_rect().a;
							if (hud->item_hovered())
							{
								hud->stroke_item();
								hovered_skill = skill_id;
								if (dragging_skill != -1 && input->mreleased(Mouse_Left))
								{
									if (dragging_skill > 1000)
									{
										auto n = 0;
										for (auto i = 0; i < 4; i++)
										{
											if (unit.skills[i] != -1)
												n++;
										}
										if (n > 1)
											unit.skills[dragging_skill - 1001] = -1;
									}
								}
							}
							if (hud->item_clicked())
								dragging_skill = i;
							auto& skill_data = skill_datas[skill_id];
							draw_text(skill_data.name, 18, pos);
						}
						hud->end_layout();
						hud->end_layout();

						if (hovered_unit != -1)
						{
							auto& unit = city.units[hovered_unit];
							auto& unit_data = unit_datas[unit.id];
							popup_unit_detail(mpos, unit.id, unit.lv, calc_hp_stat(unit_data.stats[StatHP], unit.lv), 0,
								calc_stat(unit_data.stats[StatATK], unit.lv), calc_stat(unit_data.stats[StatDEF], unit.lv), calc_stat(unit_data.stats[StatSA], unit.lv),
								calc_stat(unit_data.stats[StatSD], unit.lv), calc_stat(unit_data.stats[StatSP], unit.lv), unit_data.type1, unit_data.type2, unit.skills);
						}

						if (hovered_skill != -1)
						{
							hud->begin("popup"_h, mpos + vec2(10.f, 4.f), vec2(0.f), cvec4(50, 50, 50, 255), vec2(0.f, 1.f));
							show_skill(hovered_skill);
							hud->end();
						}

						hud->end_layout();

						if (!input->mbtn[Mouse_Left])
							dragging_skill = -1;
						if (dragging_skill != -1)
						{
							auto skill_id = -1;
							if (dragging_skill > 1000)
								skill_id = unit.skills[dragging_skill - 1001];
							else
								skill_id = unit.learnt_skills[dragging_skill];
							if (skill_id != -1)
							{
								auto& skill_data = skill_datas[skill_id];
								draw_text(skill_data.name, 18, mpos);
							}
						}
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
								canvas->draw_rect_filled(pos, pos + vec2(size * MAX_TROOP_UNITS, size), cvec4(100, 100, 100, 255));
							}
						}
						canvas->draw_rect(pos, pos + vec2(size * MAX_TROOP_UNITS, size), 1.f, cvec4(255));
						hud->begin_layout(HudHorizontal);
						hud->begin_layout(HudHorizontal, vec2(size * MAX_TROOP_UNITS, size));
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
									hovered_unit = idx;
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
						if (auto target_city = search_random_hostile_city(lord.id); target_city)
						{
							auto& troop = city.troops.emplace_back();
							city.set_troop_target(troop, target_city->tile_id);
						}
					}

					if (hovered_unit != -1)
					{
						auto& unit = city.units[hovered_unit];
						auto& unit_data = unit_datas[unit.id];
						popup_unit_detail(mpos, unit.id, unit.lv, calc_hp_stat(unit_data.stats[StatHP], unit.lv), 0,
							calc_stat(unit_data.stats[StatATK], unit.lv), calc_stat(unit_data.stats[StatDEF], unit.lv), calc_stat(unit_data.stats[StatSA], unit.lv),
							calc_stat(unit_data.stats[StatSD], unit.lv), calc_stat(unit_data.stats[StatSP], unit.lv), unit_data.type1, unit_data.type2, unit.skills);
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
									if (tile.idx1 != main_player_id && (tile.type == TileCity || tile.type == TileNeutralCamp))
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
							draw_image(unit_data.icon, mpos, vec2(64.f), vec2(0.5f, 0.5f), cvec4(255, 255, 255, 127));
					}
					if (dragging_target != -1)
						draw_image(img_target, mpos, vec2(32.f), vec2(0.5f, 0.5f), cvec4(255, 255, 255, 127));
				}
					break;
				}
			}
			hud->end_layout();

		}
			break;
		case TileNeutralCamp:
		{
			auto& camp = neutral_camps[tile.idx1];
			hud->begin_layout(HudVertical, vec2(0.f), vec2(0.f, 8.f));
			hud->text(L"Neutral Camp");

			hud->begin_layout(HudHorizontal);
			for (auto i = 0; i < camp.units.size(); i++)
			{
				auto& unit = camp.units[i];
				auto& unit_data = unit_datas[unit.id];
				hud->begin_layout(HudVertical);
				if (unit_data.icon)
					hud->image(vec2(48.f), unit_data.icon);
				hud->text(wstr(unit.lv), 16);
				hud->end_layout();
			}
			hud->end_layout();

			hud->text(L"Reward:");
			switch (camp.chest.type)
			{
			case ChestProduction:
				hud->begin_layout(HudHorizontal);
				hud->image(vec2(24.f, 24.f), img_production);
				hud->text(wstr(camp.chest.value));
				hud->end_layout();
				break;
			case ChestGold:
				hud->begin_layout(HudHorizontal);
				hud->image(vec2(27.f, 18.f), img_resources[ResourceGold]);
				hud->text(wstr(camp.chest.value));
				hud->end_layout();
				break;
			}

			hud->end_layout();
		}
			break;
		case TileResourceField:
			//if (auto id = main_player.find_resource_field(selected_tile); id != -1)
			//{
			//	auto& resource_field = main_player.resource_fields[id];
			//	if (!resource_field_datas[resource_field.type].empty())
			//	{
			//		hud->begin_layout(HudVertical, vec2(220.f, bottom_pannel_height - 48.f));
			//		hud->text(std::format(L"{} LV: {}", get_resource_field_name(resource_field.type), resource_field.lv));
			//		hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
			//		hud->text(L"Current Production: ", 22);
			//		hud->image(vec2(20.f, 13.f), img_resources[resource_field.type]);
			//		hud->text(std::format(L"{}", resource_field.production), 22);
			//		hud->end_layout();
			//		if (resource_field.lv < resource_field_datas[resource_field.type].size())
			//		{
			//			auto& next_level = resource_field_datas[resource_field.type][resource_field.lv];

			//			hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
			//			hud->text(std::format(L"Level {} Production: ", resource_field.lv + 1), 22);
			//			hud->image(vec2(20.f, 13.f), img_resources[resource_field.type]);
			//			hud->text(std::format(L"{}", next_level.production), 22);
			//			hud->end_layout();

			//			hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
			//			hud->image(vec2(18.f, 12.f), img_resources[ResourceWood]);
			//			hud->text(std::format(L"{}", next_level.cost_wood), 16);
			//			hud->image(vec2(18.f, 12.f), img_resources[ResourceClay]);
			//			hud->text(std::format(L"{}", next_level.cost_clay), 16);
			//			hud->image(vec2(18.f, 12.f), img_resources[ResourceIron]);
			//			hud->text(std::format(L"{}", next_level.cost_iron), 16);
			//			hud->image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
			//			hud->text(std::format(L"{}", next_level.cost_crop), 16);
			//			hud->end_layout();
			//			hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
			//			hud->image(vec2(18.f, 12.f), img_resources[ResourceGold]);
			//			hud->text(std::format(L"{}", next_level.cost_gold), 16);
			//			hud->image(vec2(18.f, 12.f), img_population);
			//			hud->text(std::format(L"{}", next_level.cost_population), 16);
			//			hud->end_layout();

			//			if (hud->button(L"Upgrade", 18))
			//				main_player.upgrade_resource_field(resource_field);
			//		}
			//		hud->end_layout();
			//		hud->stroke_item();
			//	}
			//}
			break;
		}
	}
	hud->end();

	static bool show_cheat = false;
	hud->begin("top-right"_h, vec2(screen_size.x - 300.f, 30.f), vec2(0.f, 0.f), cvec4(0, 0, 0, 255));
	hud->begin_layout(HudHorizontal);
	if (hud->button(L"Cheat"))
		show_cheat = !show_cheat;
	if (hud->button(L"Save"))
	{
		pugi::xml_document doc;
		auto doc_root = doc.append_child("save");
		auto n_lords = doc_root.append_child("lords");
		for (auto& lord : lords)
		{
			auto n_lord = n_lords.append_child("lord");
			n_lord.append_attribute("gold").set_value(lord.resources[ResourceGold]);
			auto n_cities = n_lord.append_child("cities");
			for (auto& city : lord.cities)
			{
				auto n_city = n_cities.append_child("city");
				n_city.append_attribute("tile_id").set_value(city.tile_id);
				n_city.append_attribute("loyalty").set_value(city.loyalty);
				n_city.append_attribute("production").set_value(city.production);
				auto n_buildings = n_city.append_child("buildings");
				for (auto& building : city.buildings)
				{
					auto n_building = n_buildings.append_child("building");
					n_building.append_attribute("slot").set_value(building.slot);
					n_building.append_attribute("type").set_value(building.type);
					n_building.append_attribute("lv").set_value(building.lv);
				}
				auto n_captures = n_city.append_child("captures");
				for (auto& capture : city.captures)
				{
					auto n_capture = n_captures.append_child("capture");
					n_capture.append_attribute("unit_id").set_value(capture.unit_id);
					n_capture.append_attribute("exclusive_id").set_value(capture.exclusive_id);
					n_capture.append_attribute("lv").set_value(capture.lv);
					n_capture.append_attribute("cost_gold").set_value(capture.cost_gold);
				}
				auto n_units = n_city.append_child("units");
				for (auto& unit : city.units)
				{
					auto n_unit = n_units.append_child("unit");
					n_unit.append_attribute("id").set_value(unit.id);
					n_unit.append_attribute("lv").set_value(unit.lv);
					n_unit.append_attribute("exp").set_value(unit.exp);
					auto n_skills = n_unit.append_child("skills");
					for (auto i = 0; i < 4; i++)
						n_skills.append_child("skill").append_attribute("v").set_value(unit.skills[i]);
					auto n_learnt_skills = n_unit.append_child("learnt_skills");
					for (auto i = 0; i < unit.learnt_skills.size(); i++)
						n_learnt_skills.append_child("skill").append_attribute("v").set_value(unit.learnt_skills[i]);
				}
				auto n_troops = n_city.append_child("troops");
				for (auto& troop : city.troops)
				{
					auto n_troop = n_troops.append_child("troop");
					n_troop.append_attribute("target").set_value(troop.target);
					auto n_units = n_troop.append_child("units");
					for (auto i = 0; i < troop.units.size(); i++)
						n_units.append_child("unit").append_attribute("v").set_value(troop.units[i]);
				}
			}
		}
		auto n_camps = doc_root.append_child("neutral_camps");
		for (auto& camp : neutral_camps)
		{
			auto n_camp = n_camps.append_child("neutral_camp");
			n_camp.append_attribute("tile_id").set_value(camp.tile_id);
			auto n_units = n_camp.append_child("units");
			for (auto& unit : camp.units)
			{
				auto n_unit = n_units.append_child("unit");
				n_unit.append_attribute("id").set_value(unit.id);
				n_unit.append_attribute("lv").set_value(unit.lv);
				auto n_skills = n_unit.append_child("skills");
				for (auto i = 0; i < 4; i++)
					n_skills.append_child("skill").append_attribute("v").set_value(unit.skills[i]);
			}
			n_camp.append_attribute("chest_type").set_value(camp.chest.type);
			n_camp.append_attribute("chest_value").set_value(camp.chest.value);
		}

		auto filename = std::filesystem::path(L"1.save");
		doc.save_file(filename.c_str());
	}
	if (hud->button(L"Load"))
	{
		if (state == GameDay)
		{
			pugi::xml_document doc;
			pugi::xml_node doc_root;

			auto filename = std::filesystem::path(L"1.save");
			if (std::filesystem::exists(filename))
			{
				if (doc.load_file(filename.c_str()) && (doc_root = doc.first_child()).name() == std::string("save"))
				{
					lords.clear();
					neutral_camps.clear();
					for (auto& tile : tiles)
					{
						tile.type = TileField;
						tile.idx1 = tile.idx2 = -1;
					}

					auto lord_id = 0;
					for (auto n_lord : doc_root.child("lords"))
					{
						auto& lord = lords.emplace_back();
						lord.id = lord_id;
						lord.resources[ResourceGold] = n_lord.attribute("gold").as_uint();

						for (auto n_city : n_lord.child("cities"))
						{
							auto tile_id = n_city.attribute("tile_id").as_uint();
							lord.build_city(tile_id);
							auto& city = lord.cities.back();
							city.loyalty = n_city.attribute("loyalty").as_uint();
							city.production = n_city.attribute("production").as_uint();

							city.buildings.clear();
							for (auto n_building : n_city.child("buildings"))
							{
								auto& building = city.buildings.emplace_back();
								building.slot = n_building.attribute("slot").as_uint();
								building.type = (BuildingType)n_building.attribute("type").as_uint();
								building.lv = n_building.attribute("lv").as_uint();
							}
							city.captures.clear();
							for (auto n_capture : n_city.child("captures"))
							{
								auto& capture = city.captures.emplace_back();
								capture.unit_id = n_capture.attribute("unit_id").as_uint();
								capture.exclusive_id = n_capture.attribute("exclusive_id").as_uint();
								capture.lv = n_capture.attribute("lv").as_uint();
								capture.cost_gold = n_capture.attribute("cost_gold").as_uint();
							}
							city.units.clear();
							for (auto n_unit : n_city.child("units"))
							{
								auto& unit = city.units.emplace_back();
								unit.id = n_unit.attribute("id").as_uint();
								unit.lv = n_unit.attribute("lv").as_uint();
								unit.exp = n_unit.attribute("exp").as_uint();
								auto i = 0;
								for (auto n_skill : n_unit.child("skills"))
									unit.skills[i++] = n_skill.attribute("v").as_int();
								for (auto n_skill : n_unit.child("learnt_skills"))
									unit.learnt_skills.push_back(n_skill.attribute("v").as_uint());
							}
							city.troops.clear();
							for (auto n_troop : n_city.child("troops"))
							{
								auto& troop = city.troops.emplace_back();
								troop.target = n_troop.attribute("target").as_uint();
								for (auto n_unit : n_troop.child("units"))
									troop.units.push_back(n_unit.attribute("v").as_uint());
								city.set_troop_target(troop, troop.target);
							}
						}

						lord_id++;
					}
					for (auto n_camp : doc_root.child("neutral_camps"))
					{
						auto tile_id = n_camp.attribute("tile_id").as_uint();
						std::vector<NeutralUnit> units;
						for (auto n_unit : n_camp.child("units"))
						{
							auto& unit = units.emplace_back();
							unit.id = n_unit.attribute("id").as_uint();
							unit.lv = n_unit.attribute("lv").as_uint();
							auto i = 0;
							for (auto n_skill : n_unit.child("skills"))
								unit.skills[i++] = n_skill.attribute("v").as_int();
						}
						auto chest_type = (ChestType)n_camp.attribute("chest_type").as_int();
						auto chest_value = n_camp.attribute("chest_value").as_uint();
						add_neutral_camp(tile_id, units, chest_type, chest_value);
					}
				}
			}
		}
	}
	hud->end_layout();
	if (show_cheat)
	{
		static float time_scalings[] = {0.1f, 0.25f, 0.5f, 1.f, 2.f};
		hud->begin_layout(HudHorizontal);
		hud->text(std::format(L"Time Scaling: {}", anim_time_scaling));
		if (hud->button(L"+"))
		{
			auto it = std::find(time_scalings, time_scalings + countof(time_scalings), anim_time_scaling);
			if (it != time_scalings + countof(time_scalings))
				anim_time_scaling = *(it + 1);
		}
		if (hud->button(L"-"))
		{
			auto it = std::find(time_scalings, time_scalings + countof(time_scalings), anim_time_scaling);
			if (it != time_scalings)
				anim_time_scaling = *(it - 1);
		}
		hud->end_layout();

		static uint exp_multipliers[] = {1, 2, 5, 10, 100};
		hud->begin_layout(HudHorizontal);
		hud->text(std::format(L"Exp Multiplier: {}", exp_multiplier));
		if (hud->button(L"+"))
		{
			auto it = std::find(exp_multipliers, exp_multipliers + countof(exp_multipliers), exp_multiplier);
			if (it != exp_multipliers + countof(exp_multipliers))
				exp_multiplier = *(it + 1);
		}
		if (hud->button(L"-"))
		{
			auto it = std::find(exp_multipliers, exp_multipliers + countof(exp_multipliers), exp_multiplier);
			if (it != exp_multipliers)
				exp_multiplier = *(it - 1);
		}
		hud->end_layout();

		static uint damage_multipliers[] = { 1, 2, 5, 10, 100 };
		hud->begin_layout(HudHorizontal);
		hud->text(std::format(L"City Damage Multiplier: {}", damage_multiplier));
		if (hud->button(L"+"))
		{
			auto it = std::find(damage_multipliers, damage_multipliers + countof(damage_multipliers), damage_multiplier);
			if (it != damage_multipliers + countof(damage_multipliers))
				damage_multiplier = *(it + 1);
		}
		if (hud->button(L"-"))
		{
			auto it = std::find(damage_multipliers, damage_multipliers + countof(damage_multipliers), damage_multiplier);
			if (it != damage_multipliers)
				damage_multiplier = *(it - 1);
		}
		hud->end_layout();

		static uint city_damage_multipliers[] = { 1, 2, 5, 10, 100 };
		hud->begin_layout(HudHorizontal);
		hud->text(std::format(L"City Damage Multiplier: {}", city_damage_multiplier));
		if (hud->button(L"+"))
		{
			auto it = std::find(city_damage_multipliers, city_damage_multipliers + countof(city_damage_multipliers), city_damage_multiplier);
			if (it != city_damage_multipliers + countof(city_damage_multipliers))
				city_damage_multiplier = *(it + 1);
		}
		if (hud->button(L"-"))
		{
			auto it = std::find(city_damage_multipliers, city_damage_multipliers + countof(city_damage_multipliers), city_damage_multiplier);
			if (it != city_damage_multipliers)
				city_damage_multiplier = *(it - 1);
		}
		hud->end_layout();
	}
	hud->end();

	hud->begin("side"_h, vec2(screen_size.x - 100.f, screen_size.y - 300.f), vec2(160.f, 300.f), cvec4(0, 0, 0, 255));
	if (state == GameDay)
	{
		if (hud->button(L"Start Battle"))
			start_battle();
	}
	hud->end();

	if (state == GameBattle)
	{
		auto get_lord_id = [](BattlePlayer& player) {
			if (player.troop)
				return player.troop->lord_id;
			if (player.city)
				return player.city->lord_id;
			return 5U;
		};

		hud->begin("battle"_h, vec2(100.f, 150.f), vec2(0.f), cvec4(0, 0, 0, 255), vec2(0.f), {}, vec4(0.f), true);

		hud->begin_layout(HudVertical, vec2(0.f), vec2(0.f));
		hud->rect(vec2(750.f, 100.f), hsv(get_lord_id(battle_players[1]) * 60.f, 0.5f, 0.5f, 1.f));
		hud->rect(vec2(750.f, 100.f), hsv(get_lord_id(battle_players[0]) * 60.f, 0.5f, 0.5f, 1.f));
		if ((battle_players[0].troop || battle_players[0].camp) && battle_players[1].troop)
		{
			for (auto& log : battle_log)
				hud->text(log, 20);
		}
		hud->end_layout();

		auto hovered_unit = -1;

		for (auto i = 0; i < 2; i++)
		{
			auto& player = battle_players[i];
			for (auto j = 0; j < player.unit_displays.size(); j++)
			{
				auto& unit = player.get_units()[j];
				auto& display = player.unit_displays[j];
				auto& unit_data = unit_datas[display.unit_id];
				auto sz = vec2(64.f) * display.scl.x;
				if (unit_data.icon)
					draw_image(unit_data.icon, display.pos, sz, vec2(0.5f, 1.f), cvec4(255, 255, 255, 255 * display.alpha));
				Rect rect;
				rect.a = display.init_pos - sz * vec2(0.5f, 1.f);
				rect.b = rect.a + sz;
				if (rect.contains(mpos))
					hovered_unit = i * 100 + j;
				if ((battle_players[0].troop || battle_players[0].camp) && battle_players[1].troop)
				{
					draw_rect(display.init_pos + vec2(-20.f, 5.f), vec2(40.f, 5.f), vec2(0.f), cvec4(150, 150, 150, 255));
					draw_rect(display.init_pos + vec2(-20.f, 5.f), vec2(40.f * ((float)display.HP / (float)unit.HP_MAX), 5.f), vec2(0.f), cvec4(0, 255, 0, 255));
					draw_text(std::format(L"{}/{}", display.HP, unit.HP_MAX), 20, display.init_pos + vec2(0.f, 8.f), vec2(0.5f, 1.f));
				}
				if (!display.label.empty())
					draw_text(display.label, 20, display.label_pos, vec2(0.5f, 0.f), cvec4(255, 255, 255, 255), vec2(1.f), cvec4(0, 0, 0, 255));
			}
		}

		if (city_damge > 0)
			draw_text(wstr(city_damge), 20, vec2(450.f, 300.f), vec2(0.5f, 0.f), cvec4(255, 255, 255, 255), vec2(1.f), cvec4(0, 0, 0, 255));

		if (hovered_unit != -1)
		{
			auto i = hovered_unit / 100;
			auto j = hovered_unit % 100;

			auto& player = battle_players[i];
			auto& unit = player.get_units()[j];
			auto& display = player.unit_displays[j];
			auto& unit_data = unit_datas[display.unit_id];
			popup_unit_detail(mpos, display.unit_id, unit.lv, unit.stats[StatHP], unit.HP_MAX, unit.stats[StatATK], unit.stats[StatDEF], unit.stats[StatSA], unit.stats[StatSD], unit.stats[StatSP], 
				unit.type1, unit.type2, unit.skills);
		}

		hud->end();
	}

	hud->begin("top"_h, vec2(0.f, 0.f), vec2(screen_size.x, 20.f), cvec4(0, 0, 0, 255));
	//hud->begin_layout(HudHorizontal, vec2(0.f), vec2(16.f, 0.f));
	//hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	//hud->image(vec2(27.f, 18.f), img_resources[ResourceWood]);
	//hud->text(std::format(L"{} +{}", main_player.resources[ResourceWood], main_player.get_production(ResourceWood)), 24);
	//hud->end_layout();
	//if (hud->item_hovered())
	//{
	//	hud->begin("popup"_h, mpos + vec2(0.f, 10.f));
	//	hud->text(std::format(L"Wood: {}\nProduction: {}", main_player.resources[ResourceWood], main_player.get_production(ResourceWood)));
	//	hud->end();
	//}
	//hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	//hud->image(vec2(27.f, 18.f), img_resources[ResourceClay]);
	//hud->text(std::format(L"{} +{}", main_player.resources[ResourceClay], main_player.get_production(ResourceClay)), 24);
	//hud->end_layout();
	//if (hud->item_hovered())
	//{
	//	hud->begin("popup"_h, mpos + vec2(0.f, 10.f));
	//	hud->text(std::format(L"Clay: {}\nProduction: {}", main_player.resources[ResourceClay], main_player.get_production(ResourceClay)));
	//	hud->end();
	//}
	//hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	//hud->image(vec2(27.f, 18.f), img_resources[ResourceIron]);
	//hud->text(std::format(L"{} +{}", main_player.resources[ResourceIron], main_player.get_production(ResourceIron)), 24);
	//hud->end_layout();
	//if (hud->item_hovered())
	//{
	//	hud->begin("popup"_h, mpos + vec2(0.f, 10.f));
	//	hud->text(std::format(L"Iron: {}\nProduction: {}", main_player.resources[ResourceIron], main_player.get_production(ResourceIron)));
	//	hud->end();
	//}
	//hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	//hud->image(vec2(27.f, 18.f), img_resources[ResourceCrop]);
	//hud->text(std::format(L"{} +{}", main_player.resources[ResourceCrop], main_player.get_production(ResourceCrop)), 24);
	//hud->end_layout();
	//if (hud->item_hovered())
	//{
	//	hud->begin("popup"_h, mpos + vec2(0.f, 10.f));
	//	hud->text(std::format(L"Crop: {}\nProduction: {}", main_player.resources[ResourceCrop], main_player.get_production(ResourceCrop)));
	//	hud->end();
	//}
	hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	hud->image(vec2(27.f, 18.f), img_resources[ResourceGold]);
	hud->text(std::format(L"{}", main_player.resources[ResourceGold]), 24);
	hud->end_layout();
	if (hud->item_hovered())
	{
		hud->begin("popup"_h, mpos + vec2(0.f, 10.f));
		hud->text(std::format(L"Gold: {}", main_player.resources[ResourceGold]));
		hud->end();
	}
	//hud->begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
	//hud->image(vec2(27.f, 18.f), img_population);
	//hud->text(std::format(L"{}/{}", main_player.consume_population, main_player.provide_population), 24);
	//hud->end_layout();
	//if (hud->item_hovered())
	//{
	//	hud->begin("popup"_h, mpos + vec2(0.f, 10.f));
	//	hud->text(std::format(L"Population\nProvide: {}\nConsume: {}", main_player.provide_population, main_player.consume_population));
	//	hud->end();
	//}
	//hud->end_layout();
	hud->end();

	{
		enum Steps
		{
			StepShowExpGain,
			StepEnd
		};

		struct UnitDisplay
		{
			uint old_id;
			uint id;
			uint old_lv;
			uint lv;
			uint lv_exp;
			uint lv_exp_max;
			std::vector<uint> learnt_skills;
		};

		static Steps step = StepEnd;
		static std::vector<std::vector<UnitDisplay>> unit_displays;

		auto move_step = [&]() {
			switch (step)
			{
			case StepEnd:
				step = StepShowExpGain;
				break;
			case StepShowExpGain:
				unit_displays.erase(unit_displays.begin());
				if (unit_displays.empty())
				{
					step = StepEnd;

					for (auto& lord : lords)
					{
						for (auto& city : lord.cities)
						{
							for (auto& unit : city.units)
							{
								auto old_lv = unit.lv;
								unit.exp += unit.gain_exp;
								unit.gain_exp = 0;
								auto next_lv_exp = calc_exp(unit.lv + 1);

								while (true)
								{
									if (unit.exp < next_lv_exp)
										break;
									unit.lv++;
									next_lv_exp = calc_exp(unit.lv + 1);
								}

								if (unit.lv != old_lv)
								{
									while (true)
									{
										auto& unit_data = unit_datas[unit.id];
										if (unit_data.evolution_lv != 0 && unit.lv >= unit_data.evolution_lv)
											unit.id = unit.id + 1;
										else
											break;
									}

									unit.learn_skills();
									if (lord.id != main_player_id)
									{
										for (auto i = 0; i < 4; i++)
											unit.skills[i] = -1;
										auto n = 0;
										for (auto it = unit.learnt_skills.rbegin(); it != unit.learnt_skills.rend(); it++)
										{
											if (n >= 4)
												break;
											unit.skills[n] = *it;
											n++;
										}
									}
								}
							}
						}
					}
				}
				break;
			}
		};

		if (show_result)
		{
			unit_displays.resize(main_player.cities.size());
			for (auto i = 0; i < main_player.cities.size(); i++)
			{
				auto& city = main_player.cities[i];
				for (auto j = 0; j < city.units.size(); j++)
				{
					unit_displays[i].resize(city.units.size());
					auto& unit = city.units[j];
					auto curr_lv_exp = calc_exp(unit.lv);
					auto next_lv_exp = calc_exp(unit.lv + 1);
					auto& display = unit_displays[i][j];
					display.old_id = unit.id;
					display.id = unit.id;
					display.old_lv = unit.lv;
					display.lv = unit.lv;
					display.lv_exp = unit.exp - curr_lv_exp;
					display.lv_exp_max = next_lv_exp - curr_lv_exp;
					display.learnt_skills = unit.learnt_skills;

					if (auto gain_exp = unit.gain_exp; gain_exp > 0)
					{
						auto lv = unit.lv;
						auto start_exp = unit.exp;
						auto end_exp = unit.exp + gain_exp;

						auto id = game.tween->begin_2d_targets();
						game.tween->add_int_target(id, (int*)&display.lv_exp);

						while (true)
						{
							game.tween->set_callback(id, [&, curr_lv_exp, next_lv_exp]() {
								display.lv_exp_max = next_lv_exp - curr_lv_exp;
							});
							auto val = min(end_exp, next_lv_exp) - start_exp;
							game.tween->int_val_to(id, val, (float)val / (next_lv_exp - curr_lv_exp) * anim_time_scaling);
							if (end_exp < next_lv_exp)
								break;
							start_exp += val;
							lv++;
							curr_lv_exp = calc_exp(lv);
							next_lv_exp = calc_exp(lv + 1);
							game.tween->set_callback(id, [&, lv]() {
								display.lv_exp = 0;
								display.lv = lv;
								auto& unit_data = unit_datas[display.id];
								if (unit_data.evolution_lv != 0 && display.lv >= unit_data.evolution_lv)
									display.id = display.id + 1;
							});
						}

						game.tween->end(id);
					}
				}
			}

			move_step();
			show_result = false;
		}

		switch (step)
		{
		case StepShowExpGain:
		{
			hud->begin("show_exp_gain"_h, vec2(100.f, 250.f), vec2(0.f, 0.f), hsv(main_player_id * 60.f, 0.5f, 0.5f, 1.f), vec2(0.f), {}, vec4(0.f), true);
			hud->text(L"Exp Gain:");
			hud->begin_layout(HudHorizontal, vec2(0.f), vec2(8.f, 0.f));
			auto& displays = unit_displays.front();
			for (auto i = 0; i < displays.size(); i++)
			{
				auto& display = displays[i];
				auto& old_unit_data = unit_datas[display.old_id];
				auto& unit_data = unit_datas[display.id];
				hud->begin_layout(HudVertical);
				if (unit_data.icon)
					hud->image(vec2(64.f), unit_data.icon);
				hud->rect(vec2(40.f, 5.f), cvec4(150, 150, 150, 255));
				hud->set_cursor(hud->item_rect().a);
				hud->rect(vec2(40.f * ((float)display.lv_exp / (float)display.lv_exp_max), 5.f), cvec4(0, 0, 255, 255));
				hud->set_cursor(hud->item_rect().a);
				hud->text(std::format(L"{}/{}", display.lv_exp, display.lv_exp_max), 20);
				if (display.lv != display.old_lv)
				{
					auto old_hp = calc_hp_stat(old_unit_data.stats[StatHP], display.old_lv); auto new_hp = calc_hp_stat(unit_data.stats[StatHP], display.lv);
					auto old_atk = calc_stat(old_unit_data.stats[StatATK], display.old_lv); auto new_atk = calc_stat(unit_data.stats[StatATK], display.lv);
					auto old_def = calc_stat(old_unit_data.stats[StatDEF], display.old_lv); auto new_def = calc_stat(unit_data.stats[StatDEF], display.lv);
					auto old_sa = calc_stat(old_unit_data.stats[StatSA], display.old_lv); auto new_sa = calc_stat(unit_data.stats[StatSA], display.lv);
					auto old_sd = calc_stat(old_unit_data.stats[StatSD], display.old_lv); auto new_sd = calc_stat(unit_data.stats[StatSD], display.lv);
					auto old_sp = calc_stat(old_unit_data.stats[StatSP], display.old_lv); auto new_sp = calc_stat(unit_data.stats[StatSP], display.lv);
					hud->text(std::format(L"{} -> {}", display.old_lv, display.lv), 20);
					hud->text(std::format(L"HP +{} -> {}", new_hp - old_hp, new_hp), 20);
					hud->text(std::format(L"ATK +{} -> {}", new_atk - old_atk, new_atk), 20);
					hud->text(std::format(L"DEF +{} -> {}", new_def - old_def, new_def), 20);
					hud->text(std::format(L"SA +{} -> {}", new_sa - old_sa, new_sa), 20);
					hud->text(std::format(L"SD +{} -> {}", new_sd - old_sd, new_sd), 20);
					hud->text(std::format(L"SP +{} -> {}", new_sp - old_sp, new_sp), 20);
					for (auto& s : unit_data.skillset)
					{
						if (s.first > display.old_lv && s.first <= display.lv)
						{
							if (!has(display.learnt_skills, s.second))
							{
								auto& skill_data = skill_datas[s.second];
								hud->text(std::format(L"New Skill: {}", skill_data.name), 20);
							}
						}
					}
				}
				hud->end_layout();
			}
			hud->end_layout();
			if (hud->button(L"OK"))
				move_step();
			hud->end();
		}
			break;
		}
	}

	if (game_over)
	{
		hud->begin("game_over"_h, vec2(100.f, 250.f), vec2(0.f, 0.f), hsv(0.f, 0.5f, 0.5f, 1.f), vec2(0.f), {}, vec4(0.f), true);
		hud->text(L"Game Over");
		if (victory)
			hud->text(L"Victory, you defeated all opponents");
		else
			hud->text(L"Failed, you have lost all cities");
		hud->end();
	}

	UniverseApplication::on_render();
}

void Game::on_gui()
{

}

Game game;

int entry(int argc, char** args)
{
	{
		//auto copied = get_clipboard();
		//if (!copied.empty())
		//{
		//	std::wstring ret = L"";
		//	for (auto line : SUW::split(copied, L'\n'))
		//	{
		//		auto sp = SUW::split(line, L'\t');
		//		if (sp.size() >= 2)
		//		{
		//			auto lv = std::wstring(sp[0]);
		//			auto name = std::wstring(sp[1]);
		//			name = get_display_name(name);
		//			if (!ret.empty())
		//				ret += L",";
		//			ret += lv + L':' + name;
		//		}
		//	}
		//	ret = L"skillset=\"" + ret + L"\" ";
		//	set_clipboard(ret);
		//	return 0;
		//}
	}

	game.init();
	game.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
