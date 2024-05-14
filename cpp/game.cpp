#include "game.h"

#include <flame/foundation/sheet.h>
#include <flame/foundation/system.h>
#include <flame/graphics/canvas.h>

cCameraPtr camera;

graphics::ImagePtr img_tile_frame;
graphics::ImagePtr img_tile_grass;

auto tile_cx = 10U;
auto tile_cy = 10U;
auto tile_sz = 50.f;

enum TileType
{
	TileField,
	TileCity,
	TileResourceField
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

struct City
{
	uint tile_id;
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

struct ResourceField
{
	uint lv;
	ResourceType type;
	uint value;
};

SheetPtr sht_resource_field[ResourceTypeCount] = {};

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

	bool build_resource_field(uint tile_id, ResourceType type)
	{
		auto& tile = tiles[tile_id];
		if (tile.type != TileField)
			return;

		auto sht = sht_resource_field[type];
		if (!sht || sht->rows.empty())
			return false;
		auto& first_level = sht->rows.front();
		auto cost_wood = sht->get_as<uint>(first_level, "cost_wood"_h);
		auto cost_clay = sht->get_as<uint>(first_level, "cost_clay"_h);
		auto cost_iron = sht->get_as<uint>(first_level, "cost_iron"_h);
		auto cost_crop = sht->get_as<uint>(first_level, "cost_crop"_h);
		auto cost_gold = sht->get_as<uint>(first_level, "cost_gold"_h);
		auto cost_population = sht->get_as<uint>(first_level, "cost_population"_h);
		if (resources[ResourceWood] < cost_wood ||
			resources[ResourceClay] < cost_clay ||
			resources[ResourceIron] < cost_iron ||
			resources[ResourceCrop] < cost_crop ||
			resources[ResourceGold] < cost_gold ||
			provide_population < consume_population + cost_population)
			return false;

		resources[ResourceWood] -= cost_wood;
		resources[ResourceClay] -= cost_clay;
		resources[ResourceIron] -= cost_iron;
		resources[ResourceCrop] -= cost_crop;
		resources[ResourceGold] -= cost_gold;
		consume_population += cost_population;

		ResourceField resource_field;
		resource_field.lv = 1;
		resource_field.type = type;
		resource_field.value = sht->get_as<uint>(first_level, "production"_h);
		resource_fields.push_back(resource_field);

		tile.type = TileResourceField;

		return true;
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

	City city;
	city.tile_id = tile_id;
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

	sht_resource_field[ResourceWood] = Sheet::get(L"assets/woodcutter.sht");
	sht_resource_field[ResourceClay] = Sheet::get(L"assets/clay_pit.sht");
	sht_resource_field[ResourceIron] = Sheet::get(L"assets/iron_mine.sht");
	sht_resource_field[ResourceCrop] = Sheet::get(L"assets/crop_field.sht");

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
		renderer->hud_begin_layout(HudHorizontal, vec2(0.f), vec2(8.f, 0.f));
		renderer->hud_text(std::format(L"Wood: {}", main_player->resources[ResourceWood]), 20);
		renderer->hud_text(std::format(L"Clay: {}", main_player->resources[ResourceClay]), 20);
		renderer->hud_text(std::format(L"Iron: {}", main_player->resources[ResourceIron]), 20);
		renderer->hud_text(std::format(L"Crop: {}", main_player->resources[ResourceCrop]), 20);
		renderer->hud_text(std::format(L"Gold: {}", main_player->resources[ResourceGold]), 20);
		renderer->hud_end_layout();
		renderer->hud_end();
	}

	renderer->hud_begin(vec2(0.f, screen_size.y - 160.f), vec2(screen_size.x, 160.f), cvec4(0, 0, 0, 255));
	if (selected_tile != -1)
	{
		auto& tile = tiles[selected_tile];
		switch (tile.type)
		{
		case TileField:
			if (main_player)
			{
				if (main_player->has_territory(selected_tile))
				{
					renderer->hud_begin_layout(HudHorizontal);

					renderer->hud_begin_layout(HudVertical, vec2(150.f));
					renderer->hud_text(L"Woodcutter");
					if (renderer->hud_button(L"Build", 18))
						;
					renderer->hud_end_layout();
					renderer->hud_stroke_item();

					renderer->hud_begin_layout(HudVertical, vec2(150.f));
					renderer->hud_text(L"Clay Pit");
					if (renderer->hud_button(L"Build", 18))
						;
					renderer->hud_end_layout();
					renderer->hud_stroke_item();

					renderer->hud_begin_layout(HudVertical, vec2(150.f));
					renderer->hud_text(L"Iron Mine");
					if (renderer->hud_button(L"Build", 18))
						;
					renderer->hud_end_layout();
					renderer->hud_stroke_item();

					renderer->hud_begin_layout(HudVertical, vec2(150.f));
					renderer->hud_text(L"Crop Field");
					if (renderer->hud_button(L"Build", 18))
						;
					renderer->hud_end_layout();
					renderer->hud_stroke_item();

					renderer->hud_end_layout();
				}
			}
			break;
		case TileResourceField:
			break;
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
