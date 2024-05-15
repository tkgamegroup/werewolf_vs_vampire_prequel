#include "game.h"

#include <flame/foundation/sheet.h>
#include <flame/foundation/system.h>
#include <flame/graphics/canvas.h>

cCameraPtr camera;

graphics::ImagePtr img_tile_frame;
graphics::ImagePtr img_tile_grass;
graphics::ImagePtr img_tile_house;

auto tile_cx = 10U;
auto tile_cy = 10U;
auto tile_sz = 50.f;

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
	vec2 pos;
	TileType type;

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
	std::deque<std::pair<uint, uint>> candidates;
	candidates.push_back({ start_id, start_id });
	while (!candidates.empty())
	{
		auto id = candidates.front().first;
		auto parent = candidates.front().second;
		candidates.pop_front();

		if (marks[id])
			continue;
		marks[id] = true;

		if (id == end_id)
		{
			ret.push_back(end_id);
			id = parent;
			while (id != start_id)
			{
				ret.push_back(id);
				id = candidates[id].second;
			}
			ret.push_back(start_id);
			break;
		}

		auto& tile = tiles[id];
		if (tile.tile_lt != -1)
			candidates.push_back({ tile.tile_lt, id });
		if (tile.tile_t != -1)
			candidates.push_back({ tile.tile_t, id });
		if (tile.tile_rt != -1)
			candidates.push_back({ tile.tile_rt, id });
		if (tile.tile_lb != -1)
			candidates.push_back({ tile.tile_lb, id });
		if (tile.tile_b != -1)
			candidates.push_back({ tile.tile_b, id });
		if (tile.tile_rb != -1)
			candidates.push_back({ tile.tile_rb, id });
	}
	return ret;
}

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

struct Building
{
	BuildingType type;
	uint lv = 0;
};

struct BuildingBaseData
{
	uint cost_wood;
	uint cost_clay;
	uint cost_iron;
	uint cost_crop;
	uint cost_gold;
	uint cost_population;
};

struct TownCenterData : BuildingBaseData
{
};
std::vector<TownCenterData> town_center_datas;

BuildingBaseData* get_building_base_data(BuildingType type, uint lv)
{
	BuildingBaseData* ret = nullptr;
	switch (type)
	{
	case BuildingTownCenter: 
		if (town_center_datas.size() > lv)
			ret = &town_center_datas[lv];
		break;
	}
	return ret;
}

struct City
{
	uint tile_id;
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
	uint resources[ResourceTypeCount];
	uint provide_population;
	uint consume_population;

	std::vector<City> cities;
	std::vector<uint> territories;
	std::vector<ResourceField> resource_fields;

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

	bool build_building(City& city, uint slot, bool free = false)
	{
		auto type = building_slots[slot].type;

		if (!free)
		{
			if (auto base_data = get_building_base_data(type, 0); base_data)
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

		auto& building = city.buildings[slot];
		building.lv = 1;

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

		building.lv++;

		return true;
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

Lord* main_player = nullptr;

void add_lord(uint tile_id)
{
	Lord lord;
	lord.resources[ResourceWood] = 800;
	lord.resources[ResourceClay] = 800;
	lord.resources[ResourceIron] = 800;
	lord.resources[ResourceCrop] = 800;
	lord.resources[ResourceGold] = 5000;
	lord.provide_population = 10;
	lord.consume_population = 0;

	City city;
	city.tile_id = tile_id;
	city.buildings.resize(building_slots.size());
	for (auto i = 0; i < building_slots.size(); i++)
	{
		auto type = building_slots[i].type;
		city.buildings[i].type = type;
		if (type == BuildingTownCenter)
			lord.build_building(city, i, true);
	}
	tiles[tile_id].type = TileCity;

	lord.cities.push_back(city);
	lord.update_territories();
	lords.push_back(lord);

	main_player = &lords.front();
}

int search_lord_location()
{
	std::vector<uint> candidates;
	for (auto i = 0; i < tiles.size(); i++)
	{
		auto& tile = tiles[i];
		if (tile.type == TileField)
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
	if (candidates.empty())
		return -1;
	auto idx = linearRand(0, (int)candidates.size() - 1);
	return candidates[idx];
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
			tile.pos = vec2(x * tile_sz * 0.75f, y * tile_sz) + vec2(0.f, 24.f);
			tile.type = TileField;
			if (x % 2 == 1)
				tile.pos.y += tile_sz * 0.5f;
			
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

	if (auto id = search_lord_location(); id != -1)
		add_lord(id);
	if (auto id = search_lord_location(); id != -1)
		add_lord(id);
}

void Game::on_render()
{
	auto canvas = renderer->render_tasks.front()->canvas;

	for (auto& tile : tiles)
	{
		canvas->add_image(img_tile_grass->get_view(), tile.pos, tile.pos + tile_sz, vec4(0.f, 0.f, 1.f, 1.f), cvec4(255));
		//canvas->add_text(nullptr, 16, tile.pos + vec2(10.f), wstr(tile.id), cvec4(255));
	}
	for (auto& lord : lords)
	{
		for (auto& city : lord.cities)
		{
			auto& tile = tiles[city.tile_id];
			canvas->add_image(img_tile_frame->get_view(), tile.pos, tile.pos + tile_sz, vec4(0.f, 0.f, 1.f, 1.f), cvec4(255));
		}
	}

	auto mpos = input->mpos;
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

	auto screen_size = renderer->hud_screen_size();

	if (main_player)
	{
		renderer->hud_begin(vec2(0.f, 0.f), vec2(screen_size.x, 20.f), cvec4(0, 0, 0, 255));
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(3.f, 0.f));
		renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceWood]);
		renderer->hud_text(std::format(L"{} +{}", main_player->resources[ResourceWood], main_player->get_production(ResourceWood)), 16);
		renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceClay]);
		renderer->hud_text(std::format(L"{} +{}", main_player->resources[ResourceClay], main_player->get_production(ResourceClay)), 16);
		renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceIron]);
		renderer->hud_text(std::format(L"{} +{}", main_player->resources[ResourceIron], main_player->get_production(ResourceIron)), 16);
		renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceCrop]);
		renderer->hud_text(std::format(L"{} +{}", main_player->resources[ResourceCrop], main_player->get_production(ResourceCrop)), 16);
		renderer->hud_image(vec2(18.f, 12.f), img_resources[ResourceGold]);
		renderer->hud_text(std::format(L"{}", main_player->resources[ResourceGold]), 16);
		renderer->hud_image(vec2(18.f, 12.f), img_pop);
		renderer->hud_text(std::format(L"{}/{}", main_player->consume_population, main_player->provide_population), 16);
		renderer->hud_end_layout();
		renderer->hud_end();
	}

	renderer->hud_begin(vec2(0.f, screen_size.y - 160.f), vec2(screen_size.x, 160.f), cvec4(0, 0, 0, 255));
	auto rect = renderer->hud_hud_rect();
	if (selected_tile != -1)
	{
		auto& tile = tiles[selected_tile];
		switch (tile.type)
		{
		case TileField:
			if (main_player && main_player->has_territory(selected_tile))
			{
				renderer->hud_begin_layout(HudHorizontal);

				auto show_build_resource_field = [&](ResourceType type) {
					renderer->hud_begin_layout(HudVertical, vec2(200.f, 150.f));
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
							main_player->build_resource_field(selected_tile, type);
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
			if (main_player)
			{
				if (auto id = main_player->find_city(selected_tile); id != -1)
				{
					auto& city = main_player->cities[id];
					for (auto i = 0; i < building_slots.size(); i++)
					{
						auto& slot = building_slots[i];
						auto& building = city.buildings[i];
						if (building.lv > 0)
							canvas->add_image(img_tile_house->get_view(), c + slot.pos - vec2(12.f), c + slot.pos + vec2(12.f), vec4(0.f, 0.f, 1.f, 1.f), cvec4(255));
					}

					if (input->mpressed(Mouse_Left))
					{
						for (auto i = 0; i < building_slots.size(); i++)
						{
							auto& slot = building_slots[i];
							if (slot.type > BuildingInTownEnd)
								break;
							if (distance(mpos, c + slot.pos) < slot.radius)
							{
								selected_building_slot = i;
								break;
							}
						}
						if (selected_building_slot == -1)
						{
							for (auto i = 0; i < 6; i++)
							{
								if (distance(mpos, tower_pos[i]) < 8.f)
								{
									selected_building_slot = building_slots.size() - 2;
									break;
								}
							}
						}
						if (selected_building_slot == -1)
						{
							if (auto d = distance(mpos, c); d > 70.f && d < 80.f)
								selected_building_slot = building_slots.size() - 1;
						}
					}

					if (selected_building_slot != -1)
					{
						auto& building = city.buildings[selected_building_slot];
						auto type = building_slots[selected_building_slot].type;
						switch (type)
						{
						case BuildingTownCenter:
							renderer->hud_set_cursor(rect.a + vec2(180.f, 0.f));
							renderer->hud_begin_layout(HudVertical, vec2(200.f, 150.f));
							renderer->hud_text(L"Town Center");
							renderer->hud_end_layout();
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
									main_player->upgrade_building(city, selected_building_slot);
							}
							renderer->hud_stroke_item();
							break;
						case BuildingTower:
							renderer->hud_set_cursor(rect.a + vec2(180.f, 0.f));
							renderer->hud_text(L"Tower");
							break;
						case BuildingWall:
							renderer->hud_set_cursor(rect.a + vec2(180.f, 0.f));
							renderer->hud_text(L"Wall");
							break;
						}
					}
				}
			}
		}
			break;
		case TileResourceField:
			if (auto id = main_player ? main_player->find_resource_field(selected_tile) : -1; id != -1)
			{
				auto& resource_field = main_player->resource_fields[id];
				if (!resource_field_datas[resource_field.type].empty())
				{
					renderer->hud_begin_layout(HudVertical, vec2(200.f, 150.f));
					renderer->hud_text(get_resource_field_name(resource_field.type));
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
							main_player->upgrade_resource_field(resource_field);
					}
					renderer->hud_end_layout();
					renderer->hud_stroke_item();
				}
			}
			break;
		}
	}	

	for (auto& lord : lords)
	{
		for (auto& city : lord.cities)
		{

		}
		for (auto& field : lord.resource_fields)
		{
			auto pos = tiles[field.tile_id].pos + vec2(tile_sz) * 0.5f - vec2(18.f, 12.f);
			canvas->add_image(img_resources[field.type]->get_view(), pos, pos + vec2(36.f, 24.f), vec4(0.f, 0.f, 1.f, 1.f), cvec4(255));
		}
	}
	renderer->hud_end();

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
