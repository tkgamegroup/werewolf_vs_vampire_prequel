#include "game.h"

#include <flame/foundation/sheet.h>
#include <flame/foundation/system.h>
#include <flame/graphics/canvas.h>

cCameraPtr camera;

graphics::ImagePtr img_tile_frame;
graphics::ImagePtr img_tile_grass;
graphics::ImagePtr img_tile_house;
graphics::ImagePtr img_tile_castle;

auto tile_cx = 18U;
auto tile_cy = 9U;
auto tile_sz = 50.f;
auto tile_sz_y = tile_sz * 0.5f * 1.732050807569;

enum TileType
{
	TileField,
	TileCity,
	TileResourceField,

	TileTypeCount
};

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
	uint cost_wood;
	uint cost_clay;
	uint cost_iron;
	uint cost_crop;
	uint cost_gold;
	uint cost_population;
	uint required_barracks_lv;
	uint hp;
	uint atk;
};
std::vector<UnitData> unit_datas;

enum BuildingType
{
	BuildingTownCenter,
	BuildingHouse,
	BuildingBarracks,
	BuildingTower,
	BuildingWall,

	BuildingTypeCount,
	BuildingInTownBegin = BuildingTownCenter,
	BuildingInTownEnd = BuildingBarracks,
};

const wchar_t* get_building_name(BuildingType type)
{
	switch (type)
	{
	case BuildingTownCenter: return L"Town Center";
	case BuildingHouse: return L"House";
	case BuildingBarracks: return L"Barracks";
	case BuildingTower: return L"Tower";
	case BuildingWall: return L"Wall";
	}
	return L"";
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
	std::vector<uint> buys;
};
std::vector<BarracksData> barracks_datas;

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

struct Buy
{
	uint id;
	uint num;
};

struct Building
{
	uint slot;
	BuildingType type;
	uint lv = 0;
	std::vector<Buy> buys;
};

struct City
{
	uint tile_id;
	uint lord_id;
	std::vector<Building> buildings;
};

struct BuildingSlot
{
	vec2 pos;
	float radius;
	BuildingType type;
};
std::vector<BuildingSlot> building_slots;

int selected_building_slot = -1;

enum ResourceType
{
	ResourceWood,
	ResourceClay,
	ResourceIron,
	ResourceCrop,
	ResourceGold,

	ResourceTypeCount
};

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

graphics::ImagePtr img_resources[ResourceTypeCount] = {};
graphics::ImagePtr img_pop;

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
		case BuildingBarracks:
		{
			auto& data = barracks_datas[building.lv];
			auto old_buys_count = building.buys.size();
			building.buys.resize(data.buys.size());
			for (auto i = old_buys_count; i < data.buys.size(); i++)
			{
				auto& buy = building.buys[i];
				buy.id = data.buys[i];
				buy.num = 0;
			}
		}
			break;
		}
		building.lv++;

		return true;
	}

	bool buy_unit(City& city, Building& building, uint id)
	{
		auto& buy = building.buys[id];
		auto& unit_data = unit_datas[buy.id];

		if (resources[ResourceWood] < unit_data.cost_wood ||
			resources[ResourceClay] < unit_data.cost_clay ||
			resources[ResourceIron] < unit_data.cost_iron ||
			resources[ResourceCrop] < unit_data.cost_crop ||
			resources[ResourceGold] < unit_data.cost_gold ||
			provide_population < consume_population + unit_data.cost_population)
			return false;

		resources[ResourceWood] -= unit_data.cost_wood;
		resources[ResourceClay] -= unit_data.cost_clay;
		resources[ResourceIron] -= unit_data.cost_iron;
		resources[ResourceCrop] -= unit_data.cost_crop;
		resources[ResourceGold] -= unit_data.cost_gold;
		consume_population += unit_data.cost_population;

		buy.num++;
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

	City city;
	city.tile_id = tile_id;
	city.lord_id = lord.id;
	city.buildings.resize(building_slots.size());
	for (auto i = 0; i < building_slots.size(); i++)
	{
		auto type = building_slots[i].type;
		auto& building = city.buildings[i];
		building.slot = i;
		building.type = type;
		if (type == BuildingTownCenter)
			lord.upgrade_building(city, i, true);
	}

	auto& tile = tiles[tile_id];
	tile.type = TileCity;
	tile.lord_id = lord.id;

	lord.cities.push_back(city);
	lord.update_territories();
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

void Game::init()
{
	create("Werewolf VS Vampire", uvec2(1280, 720), WindowStyleFrame, false, true, 
		{ {"mesh_shader"_h, 0} });

	auto root = world->root.get();
	root->add_component<cNode>();
	camera = root->add_component<cCamera>();

	renderer->add_render_task(RenderModeShaded, camera, main_window, {}, graphics::ImageLayoutPresent);

	Path::set_root(L"assets", L"assets");

	img_tile_frame = graphics::Image::get(L"assets/HexTilesetv3.png_slices/00.png");
	img_tile_grass = graphics::Image::get(L"assets/HexTilesetv3.png_slices/01.png");
	img_tile_house = graphics::Image::get(L"assets/HexTilesetv3.png_slices/212.png");
	img_tile_castle = graphics::Image::get(L"assets/HexTilesetv3.png_slices/216.png");
	img_resources[ResourceWood] = graphics::Image::get(L"assets/icons/wood.png");
	img_resources[ResourceClay] = graphics::Image::get(L"assets/icons/clay.png");
	img_resources[ResourceIron] = graphics::Image::get(L"assets/icons/iron.png");
	img_resources[ResourceCrop] = graphics::Image::get(L"assets/icons/crop.png");
	img_resources[ResourceGold] = graphics::Image::get(L"assets/icons/gold.png");
	img_pop = graphics::Image::get(L"assets/icons/pop.png");

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

	if (auto sht = Sheet::get(L"assets/units.sht"); sht)
	{
		for (auto i = 0; i < sht->rows.size(); i++)
		{
			UnitData data;
			auto& row = sht->rows[i];
			data.name = sht->get_as_wstr(row, "name"_h);
			data.cost_wood = sht->get_as<uint>(row, "cost_wood"_h);
			data.cost_clay = sht->get_as<uint>(row, "cost_clay"_h);
			data.cost_iron = sht->get_as<uint>(row, "cost_iron"_h);
			data.cost_crop = sht->get_as<uint>(row, "cost_crop"_h);
			data.cost_gold = sht->get_as<uint>(row, "cost_gold"_h);
			data.cost_population = sht->get_as<uint>(row, "cost_population"_h);
			data.required_barracks_lv = sht->get_as<uint>(row, "required_barracks_lv"_h);
			data.hp = sht->get_as<uint>(row, "hp"_h);
			data.atk = sht->get_as<uint>(row, "atk"_h);
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
			for (auto j = 0; j < unit_datas.size(); j++)
			{
				auto& unit_data = unit_datas[j];
				if (unit_data.required_barracks_lv <= i + 1)
					data.buys.push_back(j);
			}
			barracks_datas.push_back(data);
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
		add_lord(tile_id);
	if (auto tile_id = search_lord_location(); tile_id != -1)
	{
		// give a barracks and some units to ai
		if (auto id = add_lord(tile_id); id != -1)
		{
			auto& lord = lords[id];
			auto& city = lord.cities.front();
			for (auto& building : city.buildings)
			{
				if (building.type == BuildingBarracks)
				{
					lord.upgrade_building(city, building.slot, true);
					if (!building.buys.empty())
						building.buys[0].num = 3;
					break;
				}
			}
		}
	}
}

void Game::on_render()
{
	if (lords.empty())
		return;

	auto& main_player = lords.front();
	auto canvas = renderer->render_tasks.front()->canvas;
	auto screen_size = canvas->size;
	auto mpos = input->mpos;

	if (state == GameBattle)
	{
		if (battle_anim_remain > 0.f)
		{
			battle_anim_remain -= delta_time;
			return;
		}

		for (auto i = 0; i < 2; i++)
		{
			auto& battle_troop = battle_troops[i];
			auto& lord = lords[battle_troop.troop->lord_id];
			for (auto j = 0; j < battle_troop.troop->units.size(); j++)
			{
				auto& unit = battle_troop.troop->units[j];
				auto& unit_data = unit_datas[unit.id];
				auto pos = vec2(i * 580.f + 20.f, j * 60.f + 50.f);
				if (battle_action_side == i && j == battle_troop.action_idx)
					pos.x += 25.f * (i == 0 ? +1 : -1);
				canvas->add_text(nullptr, 24, pos, unit_data.name, hsv(lord.id * 60.f, 0.5f, 1.f, 1.f));
				canvas->add_text(nullptr, 24, pos + vec2(20.f, 28), std::format(L"ATK {} HP {}", unit.atk, unit.hp), cvec4(255));
			}
		}
	}
	else
	{
		for (auto& tile : tiles)
		{
			canvas->add_image(img_tile_grass->get_view(), tile.pos, tile.pos + vec2(tile_sz, tile_sz_y), vec4(0.f, 0.f, 1.f, 1.f), cvec4(255));
			//canvas->add_text(nullptr, 16, tile.pos + vec2(10.f), wstr(tile.id), cvec4(255));
		}
		for (auto& lord : lords)
		{
			for (auto& city : lord.cities)
			{
				auto& tile = tiles[city.tile_id];
				canvas->add_image(img_tile_castle->get_view(), tile.pos, tile.pos + vec2(tile_sz, tile_sz_y), vec4(0.f, 0.f, 1.f, 1.f), cvec4(255));
			}
			for (auto& field : lord.resource_fields)
			{
				auto pos = tiles[field.tile_id].pos + vec2(tile_sz, tile_sz_y) * 0.5f - vec2(18.f, 12.f);
				canvas->add_image(img_resources[field.type]->get_view(), pos, pos + vec2(36.f, 24.f), vec4(0.f, 0.f, 1.f, 1.f), cvec4(255));
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
				for (auto id : troop.path)
					canvas->path.push_back(tiles[id].pos + vec2(tile_sz) * 0.5f);
				canvas->stroke(2.f, hsv(lord.id * 60.f, 0.5f, 0.8f, 0.8f), false);
				for (auto i = 0; i < troop.path.size() - 1; i++)
				{
					for (auto j = 0; j < 4; j++)
					{
						if (abs(int(i * 4 + j - troop.anim_time * 12.f) % 20) < 4)
						{
							auto a = tiles[troop.path[i]].pos;
							auto b = tiles[troop.path[i + 1]].pos;
							canvas->add_circle_filled(mix(a, b, j / 4.f) + vec2(tile_sz) * 0.5f, 3.f, hsv(lord.id * 60.f, 0.5f, 0.8f, 0.8f));
						}
					}
				}
				troop.anim_time += delta_time;

				canvas->add_circle_filled(tiles[troop.path[troop.idx]].pos + vec2(tile_sz) * 0.5f, 6.f, hsv(lord.id * 60.f, 0.5f, 1.f, 1.f));
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

		renderer->hud_begin(vec2(0.f, 0.f), vec2(screen_size.x, 20.f), cvec4(0, 0, 0, 255));
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(16.f, 0.f));
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_resources[ResourceWood]);
		renderer->hud_text(std::format(L"{} +{}", main_player.resources[ResourceWood], main_player.get_production(ResourceWood)), 24);
		renderer->hud_end_layout();
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_resources[ResourceClay]);
		renderer->hud_text(std::format(L"{} +{}", main_player.resources[ResourceClay], main_player.get_production(ResourceClay)), 24);
		renderer->hud_end_layout();
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_resources[ResourceIron]);
		renderer->hud_text(std::format(L"{} +{}", main_player.resources[ResourceIron], main_player.get_production(ResourceIron)), 24);
		renderer->hud_end_layout();
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_resources[ResourceCrop]);
		renderer->hud_text(std::format(L"{} +{}", main_player.resources[ResourceCrop], main_player.get_production(ResourceCrop)), 24);
		renderer->hud_end_layout();
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_resources[ResourceGold]);
		renderer->hud_text(std::format(L"{}", main_player.resources[ResourceGold]), 24);
		renderer->hud_end_layout();
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(27.f, 18.f), img_pop);
		renderer->hud_text(std::format(L"{}/{}", main_player.consume_population, main_player.provide_population), 24);
		renderer->hud_end_layout();
		renderer->hud_end_layout();
		renderer->hud_end();

		renderer->hud_begin(vec2(0.f, screen_size.y - 160.f), vec2(screen_size.x, 160.f), cvec4(0, 0, 0, 255));
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
						renderer->hud_begin_layout(HudVertical, vec2(220.f, 150.f));
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
							renderer->hud_image(vec2(18.f, 12.f), img_pop);
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
				auto c = rect.a + vec2(100.f, 80.f);
				canvas->add_circle_filled(c, 80.f, cvec4(255));

				renderer->hud_begin_layout(HudHorizontal);
				renderer->hud_rect(vec2(200.f, 160.f), cvec4(0));

				vec2 tower_pos[6];
				for (auto i = 0; i < 6; i++)
					tower_pos[i] = arc_point(c, i * 60.f, 80.f);

				auto& path = canvas->path;
				for (auto i = 0; i < 6; i++)
					path.push_back(tower_pos[i]);
				canvas->stroke(4.f, cvec4(255, 127, 127, 255), true);

				for (auto i = 0; i < 6; i++)
					canvas->add_circle_filled(tower_pos[i], 8.f, cvec4(255, 80, 80, 255));

				for (auto i = 0; i < building_slots.size(); i++)
				{
					auto& slot = building_slots[i];
					if (slot.type > BuildingInTownEnd)
						break;
					canvas->add_circle_filled(c + slot.pos, slot.radius, cvec4(255, 127, 127, 255));
				}
				if (auto id = main_player.find_city(selected_tile); id != -1)
				{
					auto& city = main_player.cities[id];
					for (auto i = 0; i < building_slots.size(); i++)
					{
						auto& slot = building_slots[i];
						auto& building = city.buildings[i];
						if (building.lv > 0)
							canvas->add_image(img_tile_house->get_view(), c + slot.pos - vec2(12.f), c + slot.pos + vec2(12.f), vec4(0.f, 0.f, 1.f, 1.f), cvec4(255));
					}

					if (input->mpressed(Mouse_Left))
					{
						auto ok = false;
						for (auto i = 0; i < building_slots.size(); i++)
						{
							auto& slot = building_slots[i];
							if (slot.type > BuildingInTownEnd)
								break;
							if (distance(mpos, c + slot.pos) < slot.radius)
							{
								selected_building_slot = i;
								ok = true;
								break;
							}
						}
						if (!ok)
						{
							for (auto i = 0; i < 6; i++)
							{
								if (distance(mpos, tower_pos[i]) < 8.f)
								{
									selected_building_slot = building_slots.size() - 2;
									ok = true;
									break;
								}
							}
						}
						if (!ok)
						{
							if (auto d = distance(mpos, c); d > 70.f && d < 80.f)
								selected_building_slot = building_slots.size() - 1;
						}
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
								renderer->hud_text(std::format(L"{}", next_level->cost_wood), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceClay]);
								renderer->hud_text(std::format(L"{}", next_level->cost_clay), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceIron]);
								renderer->hud_text(std::format(L"{}", next_level->cost_iron), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
								renderer->hud_text(std::format(L"{}", next_level->cost_crop), 16);
								renderer->hud_end_layout();
								renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
								renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceGold]);
								renderer->hud_text(std::format(L"{}", next_level->cost_gold), 16);
								renderer->hud_image(vec2(18.f, 12.f), img_pop);
								renderer->hud_text(std::format(L"{}", next_level->cost_population), 16);
								renderer->hud_end_layout();

								if (renderer->hud_button(L"Upgrade", 18))
									main_player.upgrade_building(city, selected_building_slot);
							}
						};
						switch (type)
						{
						case BuildingTownCenter:
							renderer->hud_begin_layout(HudVertical, vec2(220.f, 150.f));
							renderer->hud_text(std::format(L"Town Center LV: {}", building.lv));
							show_upgrade_building();
							renderer->hud_end_layout();
							renderer->hud_stroke_item();
							break;
						case BuildingHouse:
						{
							renderer->hud_begin_layout(HudVertical, vec2(220.f, 150.f));
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
							renderer->hud_begin_layout(HudVertical, vec2(220.f, 150.f));
							renderer->hud_text(std::format(L"Barracks LV: {}", building.lv));
							show_upgrade_building();
							renderer->hud_end_layout();
							renderer->hud_stroke_item();

							if (building.lv > 0)
							{
								for (auto i = 0; i < building.buys.size(); i++)
								{
									auto& buy = building.buys[i];
									auto& unit_data = unit_datas[buy.id];
									renderer->hud_begin_layout(HudVertical, vec2(220.f, 150.f));
									renderer->hud_text(std::format(L"{} X {}", unit_data.name, buy.num));

									renderer->hud_text(std::format(L"ATK: {}", unit_data.atk), 22);
									renderer->hud_text(std::format(L"HP: {}", unit_data.hp), 22);

									renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
									renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceWood]);
									renderer->hud_text(std::format(L"{}", unit_data.cost_wood), 16);
									renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceClay]);
									renderer->hud_text(std::format(L"{}", unit_data.cost_clay), 16);
									renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceIron]);
									renderer->hud_text(std::format(L"{}", unit_data.cost_iron), 16);
									renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
									renderer->hud_text(std::format(L"{}", unit_data.cost_crop), 16);
									renderer->hud_end_layout();
									renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
									renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceGold]);
									renderer->hud_text(std::format(L"{}", unit_data.cost_gold), 16);
									renderer->hud_image(vec2(18.f, 12.f), img_pop);
									renderer->hud_text(std::format(L"{}", unit_data.cost_population), 16);
									renderer->hud_end_layout();

									if (renderer->hud_button(L"Buy", 18))
										main_player.buy_unit(city, building, i);

									renderer->hud_end_layout();
									renderer->hud_stroke_item();
								}
							}
						}
							break;
						case BuildingTower:
							renderer->hud_begin_layout(HudVertical, vec2(220.f, 150.f));
							renderer->hud_text(std::format(L"Tower LV: {}", building.lv));
							show_upgrade_building();
							renderer->hud_end_layout();
							renderer->hud_stroke_item();
							break;
						case BuildingWall:
							renderer->hud_begin_layout(HudVertical, vec2(220.f, 150.f));
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
						renderer->hud_begin_layout(HudVertical, vec2(220.f, 150.f));
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
							renderer->hud_image(vec2(18.f, 12.f), img_pop);
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

		renderer->hud_begin(vec2(screen_size.x - 160.f, screen_size.y - 300.f), vec2(160.f, 300.f), cvec4(0, 0, 0, 255));
		if (state == GameDay)
		{
			if (renderer->hud_button(L"Start Battle"))
				start_battle();
		}
		renderer->hud_end();
	}

	UniverseApplication::on_render();
}

void Game::on_gui()
{

}

void Game::new_day()
{
	for (auto i = 1; i < lords.size(); i++)
	{

	}
}

void Game::start_battle()
{
	if (state == GameNight)
		return;
	state = GameNight;

	if (lords.size() < 2)
		return;

	for (auto& lord : lords)
	{
		lord.troops.clear();

		for (auto& city : lord.cities)
		{
			if (auto target_city = search_random_hostile_city(lord.id); target_city)
			{
				Troop troop;
				troop.lord_id = lord.id;
				troop.path = find_path(city.tile_id, target_city->tile_id);
				if (!troop.path.empty())
				{
					for (auto& building : city.buildings)
					{
						if (building.type == BuildingBarracks)
						{
							for (auto& buy : building.buys)
							{
								for (auto i = 0; i < buy.num; i++)
								{
									auto& unit = troop.units.emplace_back();
									auto& unit_data = unit_datas[unit.id];
									unit.id = buy.id;
									unit.atk = unit_data.atk;
									unit.hp = unit_data.hp;
								}
							}
						}
					}
					lord.troops.push_back(troop);
				}
			}
		}
	}

	ev_step_battle = add_event([this]() {
		step_battle();
		return true;
	}, 0.5f);
}

void Game::step_battle()
{
	if (state == GameBattle)
	{
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

		auto unit_take_damage = [&](Unit& caster, Unit& target, uint damage) {
			if (target.hp > damage)
				target.hp -= damage;
			else
				target.hp = 0;
		};

		auto& action_troop = battle_troops[battle_action_side];
		auto& other_troop = battle_troops[1 - battle_action_side];
		auto& cast_unit = action_troop.troop->units[action_troop.action_idx];
		auto& target_unit = other_troop.troop->units[linearRand(0, (int)other_troop.troop->units.size() - 1)];
		unit_take_damage(cast_unit, target_unit, cast_unit.atk);

		action_troop.action_idx++;
		if (action_troop.action_idx >= action_troop.troop->units.size())
			action_troop.action_idx = 0;
		battle_action_side = 1 - battle_action_side;

		for (auto i = 0; i < 2; i++)
		{
			auto& battle_troop = battle_troops[i];
			for (auto j = 0; j < battle_troop.troop->units.size(); j++)
			{
				if (battle_troop.troop->units[j].hp == 0)
				{
					if (j < battle_troop.action_idx)
						battle_troop.action_idx--;
					battle_troop.troop->units.erase(battle_troop.troop->units.begin() + j);
					j--;
				}
			}
		}

		return;
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
						battle_troops[0] = { &troop, 0 };
						battle_troops[1] = { &_troop, 0 };
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
			if (troop.idx < troop.path.size() - 1)
				troop.idx++;
		}
	}

	for (auto& lord : lords)
	{
		for (auto it = lord.troops.begin(); it != lord.troops.end(); )
		{
			if (it->idx == it->path.size() - 1)
				it = lord.troops.erase(it);
			else
				it++;
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
						battle_troops[0] = { &troop, 0 };
						battle_troops[1] = { &_troop, 0 };
						battle_action_side = 0;
						return;
					}
				}
			}
		}
	}
}

Game game;

int entry(int argc, char** args)
{
	game.init();
	game.run();

	return 0;
}

FLAME_EXE_MAIN(entry)
