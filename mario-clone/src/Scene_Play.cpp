#include "Scene_Play.hpp"
#include "Physics.hpp"
#include "Assets.hpp"
#include "GameEngine.hpp"
#include "Components.hpp"
#include "Action.hpp"

#include <iostream>
#include <fstream>

ScenePlay::ScenePlay(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, m_levelPath(levelPath)
{
	init(m_levelPath);
}

void ScenePlay::init(const std::string& levelPath)
{
	// Debugging actions
	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE"); // Toggle drawing (T)extures
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION"); // Toggle drawing (C)ollision
	registerAction(sf::Keyboard::G, "TOGGLE_GRID"); // Toggle drawing (G)rid

	// Movement actions
	registerAction(sf::Keyboard::W, "MOVE_JUMP");
	registerAction(sf::Keyboard::A, "MOVE_LEFT");
	registerAction(sf::Keyboard::D, "MOVE_RIGHT");
	registerAction(sf::Keyboard::S, "MOVE_DOWN");

	// TODO: Register all other gameplay Actions

	m_gridText.setCharacterSize(12);
	m_gridText.setFont(m_game->assets().getFont("Joystix"));
	loadLevel(levelPath);
}

Vec2 ScenePlay::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
	Vec2 startPos = {gridX * 64.0f , (11 - gridY) * 64.0f};
	Vec2 animationSize = entity->getComponent<CAnimation>().animation.getSize(); 
	Vec2 midPoint = { startPos.x + animationSize.x/2.0f , startPos.y + animationSize.y/2.0f };

	return midPoint; 
}

void ScenePlay::loadLevel(const std::string& filename)
{
	// reset the entity manager every time we load a level
	m_entityManager = EntityManager();

	// load from files
	std::ifstream fin(filename);
	std::string param, inAnimation , B;
	float GX, GY;
	while (!fin.eof()) {
		fin >> param;
		if (param == "Player") {
			fin >> m_playerConfig.GX >> m_playerConfig.GY >> m_playerConfig.CW >> m_playerConfig.CH
				>> m_playerConfig.SX >> m_playerConfig.SY >> m_playerConfig.SM >> m_playerConfig.G >> m_playerConfig.B;
		}
		else {
			fin >> inAnimation >> GX >> GY;

			auto thisEntity = m_entityManager.addEntity(param);
			thisEntity->addComponent<CAnimation>(m_game->assets().getAnimation(inAnimation), true);
			thisEntity->addComponent<CTransform>(gridToMidPixel(GX , GY , thisEntity));
			thisEntity->addComponent<CState>(inAnimation); // Animation and state use the same name to ref each other.

			if (param == "Tile") {
				thisEntity->addComponent<CBoundingBox>(m_game->assets().getAnimation(inAnimation).getSize());
			}
		}
	}
	spawnPlayer();
}

void ScenePlay::spawnPlayer()
{
	m_player = m_entityManager.addEntity("Player");
	m_player->addComponent<CAnimation>(m_game->assets().getAnimation("MarioIdle"), true); 
	m_player->addComponent<CTransform>(gridToMidPixel(m_playerConfig.GX , m_playerConfig.GY , m_player));
	m_player->addComponent<CBoundingBox>(m_game->assets().getAnimation("MarioIdle").getSize());
	m_player->addComponent<CState>("MarioJump");
	m_player->addComponent<CGravity>(m_playerConfig.G);
}

void ScenePlay::spawnBullet(std::shared_ptr<Entity> entity)
{
	// TODO: this should spawn a bullet at the given entity, going in the direction the entity is facing
}

void ScenePlay::update()
{
	if (!m_paused) {
		m_entityManager.update();
		sMovement();
		sLifespan();
		sCollision();
		sAnimation();
	}
	sRender();

	m_player->getComponent<CTransform>().prevPos = m_player->getComponent<CTransform>().pos;
}

void ScenePlay::sMovement()
{	
	// ----- All player movement -----
	
	auto& playerInput = m_player->getComponent<CInput>();
	auto& playerTransform = m_player->getComponent<CTransform>();
	auto& playerState = m_player->getComponent<CState>();
	
	if (playerInput.up && (playerState.state == "MarioIdle" || playerState.state == "MarioRun") ){
		playerTransform.velocity.y = m_playerConfig.SY; 
		playerState.state = "MarioJump";
	}
	if (playerInput.left) {
		playerTransform.velocity.x += m_playerConfig.SX * -1;
		if (playerState.state == "MarioIdle") {
			playerState.state = "MarioRun";
		}
		playerTransform.scale.x = -1;
	}
	if (playerInput.right) {
		playerTransform.velocity.x += m_playerConfig.SX;
		if (playerState.state == "MarioIdle") {
			playerState.state = "MarioRun";
		}
		playerTransform.scale.x = 1;
	}
	if (playerTransform.velocity.x > m_playerConfig.SM) playerTransform.velocity.x = m_playerConfig.SM; 
	if (playerTransform.velocity.x < m_playerConfig.SM * -1) playerTransform.velocity.x = m_playerConfig.SM * -1; 


	// ----- Enemy movement -----

	// --------------------------





	// ----- Gravity & velocity resolve -----
	for (auto& e : m_entityManager.getEntities()) {
		if (!e->hasComponent<CTransform>()) {
			continue;
		}
		auto& eTransform = e->getComponent<CTransform>(); 
		if (e->hasComponent<CGravity>()) {
			eTransform.velocity.y += e->getComponent<CGravity>().gravity; 
		}
		eTransform.pos += eTransform.velocity; 
	}

}

void ScenePlay::sLifespan()
{
	// TODO: Check lifespawn of entities that have them, and destroy them if they go over
}

void ScenePlay::sCollision()
{
	//Player collision with tiles
	auto& playerTransform = m_player->getComponent<CTransform>(); 
	for (auto& entity : m_entityManager.getEntities("Tile")) {
		auto overlap = Physics::GetOverlap(m_player, entity);
		if (overlap.x > 0 && overlap.y > 0) { 
			//std::cerr << "Collision detected" << std::endl;

			auto prevOverlap = Physics::GetPreviousOverlap(m_player, entity);
			auto& entityPos = entity->getComponent<CTransform>().pos; 

			if (prevOverlap.x > 0 && playerTransform.pos.y < entityPos.y) {
				playerTransform.pos.y -= overlap.y;
				playerTransform.velocity.y = 0;
				if (m_player->getComponent<CState>().state == "MarioJump") {
					m_player->getComponent<CState>().state = "MarioIdle";
				}
			}
			if (prevOverlap.x > 0 && playerTransform.pos.y > entityPos.y) {
				playerTransform.pos.y += overlap.y; 
				playerTransform.velocity.y = 0;
			}
			if (prevOverlap.y > 0 && playerTransform.pos.x < entityPos.x) {
				playerTransform.pos.x -= overlap.x;
				playerTransform.velocity.x = 0;
			}
			if (prevOverlap.y > 0 && playerTransform.pos.x > entityPos.x) {
				playerTransform.pos.x += overlap.x;
				playerTransform.velocity.x = 0;
			}
		}
	}
	
}

void ScenePlay::sDoAction(const Action& action)
{
	auto& playerInput = m_player->getComponent<CInput>();
	auto& playerTransform = m_player->getComponent<CTransform>();
	auto& playerState = m_player->getComponent<CState>();

	if (action.type() == "START") {
		if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION") { m_drawCollision = !m_drawCollision; }
		else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }
		else if (action.name() == "PAUSE") { setPaused(!m_paused); }
		else if (action.name() == "QUIT") { onEnd(); }
		else if (action.name() == "MOVE_JUMP") { 
			playerInput.up = true; 
		}
		else if (action.name() == "MOVE_LEFT") {
			playerInput.left = true; 
		}
		else if (action.name() == "MOVE_RIGHT") {
			playerInput.right = true; 
		}
	}
	else if (action.type() == "END") {
		if (action.name() == "MOVE_JUMP") {
			playerInput.up = false;
		}
		else if (action.name() == "MOVE_LEFT") {
			playerInput.left = false;
			playerTransform.velocity.x = 0;
			if (playerState.state == "MarioRun") { // Fix the sky jumping glitch
				playerState.state = "MarioIdle";
			}
		}
		else if (action.name() == "MOVE_RIGHT") {
			playerInput.right = false;
			playerTransform.velocity.x = 0;
			if (playerState.state == "MarioRun") {
				playerState.state = "MarioIdle";
			}
		}
	}
}

void ScenePlay::sAnimation()
{
	for (auto& e : m_entityManager.getEntities()) {
		auto& eAnimation = e->getComponent<CAnimation>(); // just component not animation object
		auto& eState = e->getComponent<CState>().state; 
		if (eAnimation.animation.getName() != eState) {
			eAnimation.animation = m_game->assets().getAnimation(eState);
		}
		eAnimation.animation.update();

		if (!eAnimation.repeat && eAnimation.animation.hasEnded()) { // destroy if animation has ended
			e->destroy();
		}
	}

}

void ScenePlay::onEnd()
{
	// TODO: When the scene ends , change back to the MENU scene
	//		 use m_game->changeScene(correct params);
}

void ScenePlay::sRender()
{
	auto& m_window = m_game->window(); 

	if (!m_paused) {
		m_window.clear(sf::Color(100, 100, 255));
	}
	else { 
		m_window.clear(sf::Color(50, 50, 150));
	}

	if (m_drawTextures)
	{
		for (auto e : m_entityManager.getEntities())
		{
			auto& pos = e->getComponent<CTransform>().pos;
			auto& scale = e->getComponent<CTransform>().scale;
			auto& sprite = e->getComponent<CAnimation>().animation.getSprite();
			sprite.setPosition(pos.x, pos.y);
			sprite.setScale(scale.x, scale.y);
			m_window.draw(sprite);
		}
	}
	if (m_drawCollision)
	{
		for (auto e : m_entityManager.getEntities()) {
			if (e->hasComponent<CBoundingBox>()) {
				auto& eBoundingBox = e->getComponent<CBoundingBox>();
				auto& eTransform = e->getComponent<CTransform>();

				sf::RectangleShape rect;
				rect.setSize(sf::Vector2f(eBoundingBox.size.x, eBoundingBox.size.y));
				rect.setOrigin(sf::Vector2f(eBoundingBox.halfSize.x, eBoundingBox.halfSize.y));
				rect.setPosition(eTransform.pos.x, eTransform.pos.y);
				rect.setFillColor(sf::Color(0, 0, 0, 0));
				rect.setOutlineColor(sf::Color(255, 255, 255, 255));
				rect.setOutlineThickness(1);

				sf::CircleShape circle; 
				circle.setRadius(10);
				circle.setOrigin(sf::Vector2f(10, 10));
				circle.setPosition(eTransform.pos.x, eTransform.pos.y);
				circle.setFillColor(sf::Color(0, 0, 0, 0));
				circle.setOutlineColor(sf::Color(255, 255, 255, 255));
				circle.setOutlineThickness(1);

				m_window.draw(rect);
				m_window.draw(circle);
			}
		}
	}
	m_game->window().display();
}
