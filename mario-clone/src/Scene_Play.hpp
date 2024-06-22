#pragma once

#include "Action.hpp"
#include "Entity.hpp"
#include "EntityManager.hpp"
#include "Scene.hpp"

class ScenePlay : public Scene
{
private:
	struct PlayerConfig
	{
		float GX, GY, CW, CH, SX, SY, SM, G; 
		std::string B;
	};

	std::shared_ptr<Entity> m_player;
	PlayerConfig            m_playerConfig;
	EntityManager           m_entityManager;
	std::string             m_levelPath;
	bool                    m_drawTextures  = true;
	bool                    m_drawCollision = false;
	bool                    m_drawGrid      = false;
	const Vec2              m_gridSize      = { 64, 64 };
	sf::Text                m_gridText;

public:
	void update()                        override;
	void onEnd()                         override;
	void sRender()                       override;
	void sDoAction(const Action& action) override;

	void init(const std::string& levelPath);
	void loadLevel(const std::string& levelPath);
	void sMovement();
	void sAnimation();
	void sCollision();
	void setAnimation(std::shared_ptr<Entity> entity,
					  const std::string& animationName, bool repeat); // helper function
	void drawLine(const Vec2& p1, const Vec2& p2);
	void spawnPlayer();

	Vec2 gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);
	void spawnBullet(std::shared_ptr<Entity> entity);
	void sLifespan();

	ScenePlay(GameEngine* gameEngine, const std::string& levelPath);
};