#ifndef GAME_H
#define GAME_H

#include "imgui.h"
#include "imgui-SFML.h"
#include "EntityManager.h"
#include <SFML/Graphics.hpp>

struct PlayerConfig { int SR, CR, FR, FG , FB , OR, OG, OB, OT, V; float S; };
struct EnemyConfig  { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };

class Game {
	sf::RenderWindow	m_window;		// the window we wil draw to
	EntityManager		m_entities;		// vector of entities to maintain
	sf::Font			m_font;			// the font we will use to draw
	sf::Text			m_text;			// the score text to be drawn to the scene
	PlayerConfig		m_playerConfig;
	EnemyConfig			m_enemyConfig;
	BulletConfig		m_bulletConfig;
	sf::Clock			m_deltaClock;

	std::shared_ptr<Entity>	     m_player; 
	int							 m_score = 0;
	uint32_t					 m_currentFrame = 0;
	int							 m_lastEnemySpawnTime;
	int							 m_spawnInterval; 
	bool						 m_running; 
	bool						 m_paused = false;

	bool						 m_movement = true; 
	bool						 m_lifespan = true; 
	bool						 m_collision = true; 
	bool						 m_spawning = true;
	bool						 m_gui = true;
	bool						 m_rendering = true;

public:
	Game();
	Game(const std::string& config);
	void init(const std::string& path);
	void run();

	// these functions will be call by systems
	void loadConfigFile(const std::string& path);
	void setPaused(bool paused);
	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemies(std::shared_ptr<Entity> e);
	void spawnBullet(const Vec2& target);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity);

	// systems
	void sMovement();
	void sLifespan();
	void sCollision();
	void sEnemySpawner();
	void sGUI();
	void sRender();
	void sUserInput();

};


#endif GAME_H