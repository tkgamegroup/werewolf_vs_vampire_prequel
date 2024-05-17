#pragma once

#include <flame/universe/application.h>

enum GameState
{
	GameDay,
	GameNight
};

struct Game : UniverseApplication
{
	GameState state = GameDay;

	void init();
	void on_render() override;
	void on_gui() override;
	void start_battle();
};

extern Game game;
