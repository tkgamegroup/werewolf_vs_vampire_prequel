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
	uint lord_id;
	std::vector<Unit> units;
	std::vector<uint> path;
	uint idx = 0;

	float anim_time = 0.f;
};

struct BattleTroop
{
	Troop* troop;
	uint action_idx;
};

struct Game : UniverseApplication
{
	GameState state = GameDay;
	void* ev_step_battle = nullptr;
	BattleTroop battle_troops[2];
	uint battle_action_side = 0;
	float battle_anim_remain = 0;

	void init();
	void on_render() override;
	void on_gui() override;
	void new_day();
	void start_battle();
	void step_battle();
};

extern Game game;
