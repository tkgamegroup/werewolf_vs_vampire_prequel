#pragma once

#include <flame/universe/application.h>

struct Game : UniverseApplication
{
	void init();
	void on_render() override;
	void on_gui() override;
};

extern Game game;
