#include<iostream>
#include <SFML/Graphics.hpp>
#include "Vec2.h"
#include "Game.h"

int main(int argc, char* argv[]) {

	Game g("bin/config.txt");
	g.run();
}