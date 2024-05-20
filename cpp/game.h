#pragma once

#include <flame/universe/application.h>

enum GameState
{
	GameDay,
	GameNight,
	GameBattle
};

struct Unit
{
	uint id;
	uint hp;
	uint atk;
};

struct Troop
{
	std::vector<Unit> units;
	std::vector<uint> path;
	uint idx = 0;
};

struct Game : UniverseApplication
{
	GameState state = GameDay;
	void* ev_step_battle = nullptr;
	Troop* battle_troop_left;
	Troop* battle_troop_right;
	int battle_action_index_left;
	int battle_action_index_right;

	void init();
	void on_render() override;
	void on_gui() override;
	void new_data();
	void start_battle();
	void step_battle();
};

extern Game game;
