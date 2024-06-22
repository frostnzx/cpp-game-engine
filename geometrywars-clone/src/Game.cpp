#include "Game.h"
#include "EntityManager.h"

#include <iostream>
#include <fstream>
#include <string>

#define DEBUG 0

struct RGB {
	int r, g, b;
};

Game::Game(const std::string& config) { // Game initializer
	loadConfigFile(config);
	ImGui::SFML::Init(m_window);

	spawnPlayer();
	m_running = true; 
	m_spawnInterval = m_enemyConfig.SI;
}

void Game::run() {
	while (m_running) {
		m_entities.update();

		ImGui::SFML::Update(m_window, m_deltaClock.restart()); // required update call to imgui

		if (m_paused == false) { // these shouldn't run while pausing
			if (m_spawning)
				sEnemySpawner();
			if(m_movement)
				sMovement();
			if(m_collision)
				sCollision();
			if(m_lifespan)
				sLifespan();
		}
		sUserInput();
		if(m_gui)
			sGUI();
		if(m_rendering)
			sRender();

		m_currentFrame++;
	}
}

void Game::spawnPlayer() {
	auto entity = m_entities.addEntity("Player");
	float speed = m_playerConfig.S; 

	// Entity customize from the config
	entity->cTransform = std::make_shared<CTransform>(Vec2(m_window.getSize().x / 2.0 , m_window.getSize().y / 2.0), Vec2(speed, speed), 0.0f);
	entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, 
		sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), 
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);
	entity->cInput = std::make_shared<CInput>();
	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

	// --- Explaination ---
	// Since we want this Entity to be our player , set our Game's player variable to be this Entity
	// This goes slightly against the EntityManager paradigm , but we use the player so much it's worth it
	// This goes against the paradigm because there shouldn't be any entity pointer live outside EntityManager's vector
	m_player = entity; 
}

void Game::spawnEnemy() {
	int x, y, vert, r, g, b; 
	int boundX = m_window.getSize().x - m_enemyConfig.CR, boundY = m_window.getSize().y - m_enemyConfig.CR;
	srand(time(NULL));
	x = (rand() % (boundX - m_enemyConfig.CR)) + m_enemyConfig.CR;
	y = (rand() % (boundY - m_enemyConfig.CR)) + m_enemyConfig.CR;
	vert = (rand() % m_enemyConfig.VMAX) + m_enemyConfig.VMIN; 
	r = rand() % 256, g = rand() % 256, b = rand() % 256; 

	Vec2 velo((rand() % 20) - 10, (rand() % 20) - 10); // random angle first
	velo.normalize(); // normalize it
	velo *= (rand() % (int)m_enemyConfig.SMAX) + m_enemyConfig.SMIN; // set speed for velo vector

	auto entity = m_entities.addEntity("Enemy");
	entity->cTransform = std::make_shared<CTransform>(Vec2(x , y), velo , 0.0f);
	entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
	entity->cScore = std::make_shared<CScore>(vert * 100);
	entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR , vert , sf::Color(r , g , b) , 
		sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB) , m_enemyConfig.OT );

	// record when the most recent enemy was spawned
	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e) {
	Vec2 spawnPos = e->cTransform->pos; 
	int vert = e->cShape->circle.getPointCount() , angle = 0 ;

	const float speed = 1.0 ; // const speed to 15

	while (angle <= 360) {
		auto entity = m_entities.addEntity("smallEnemy");
		Vec2 velo(speed * cosf(angle), speed * sinf(angle));
		entity->cTransform = std::make_shared<CTransform>(e->cTransform->pos, velo, 0.0f);
		entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR / 2.0);
		entity->cScore = std::make_shared<CScore>(e->cScore->score * 2);
		entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR / 2.0, vert, e->cShape->circle.getFillColor(),
		sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);
		entity->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
		angle += (360 / vert) ; // increment angle
	}
}

void Game::spawnBullet(const Vec2& target) {
	Vec2 veloVec = target - (m_player->cTransform->pos); 
	veloVec.normalize();
	veloVec *= m_bulletConfig.S; // multiply velocity size1 by scalar speed

	auto pos = m_player->cTransform->pos; 
	sf::Color bulletFillColor(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB);
	sf::Color bulletOutlineColor(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB);

	auto entity = m_entities.addEntity("bullet");
	entity->cTransform = std::make_shared<CTransform>(pos, veloVec, 0.0f);
	entity->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
	entity->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, bulletFillColor, bulletOutlineColor , m_bulletConfig.OT);
	entity->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity) {
	// TODO: implement your own special weapon
} 

void Game::sMovement() {
	// move player
	if (m_player->cInput->up && m_player->cTransform->pos.y - m_player->cCollision->radius - m_player->cTransform->velocity.y >= 0) 
		m_player->cTransform->pos.y -= m_player->cTransform->velocity.y; 
	if (m_player->cInput->down && m_player->cTransform->pos.y + m_player->cCollision->radius + m_player->cTransform->velocity.x <= m_window.getSize().y) 
		m_player->cTransform->pos.y += m_player->cTransform->velocity.y;
	if (m_player->cInput->left && m_player->cTransform->pos.x - m_player->cCollision->radius - m_player->cTransform->velocity.y >= 0) 
		m_player->cTransform->pos.x -= m_player->cTransform->velocity.x;
	if (m_player->cInput->right && m_player->cTransform->pos.x + m_player->cCollision->radius + m_player->cTransform->velocity.x <= m_window.getSize().x) 
		m_player->cTransform->pos.x += m_player->cTransform->velocity.x;

	// move enemies
	for (auto& enemy : m_entities.getEntities("Enemy")) {
		enemy->cTransform->pos += enemy->cTransform->velocity; 
	}
	// move small enemies
	for (auto& sEnemy : m_entities.getEntities("smallEnemy")) {
		sEnemy->cTransform->pos += sEnemy->cTransform->velocity;
	}
	// move bullets
	for (auto& bullet : m_entities.getEntities("bullet")) {
		bullet->cTransform->pos += bullet->cTransform->velocity; 
	}
}

void Game::sLifespan() {
	for (auto& entity : m_entities.getEntities()) {
		if (!entity->cLifespan) {
			continue;
		}
		if (entity->cLifespan->remaining > 0) {
			entity->cLifespan->remaining--;
			sf::Color fColor = entity->cShape->circle.getFillColor();
			sf::Color oColor = entity->cShape->circle.getOutlineColor();
			int alpha = 255.0 * (float(entity->cLifespan->remaining) / float(entity->cLifespan->total));
			entity->cShape->circle.setFillColor(sf::Color(fColor.r , fColor.g , fColor.b , alpha));
			entity->cShape->circle.setOutlineColor(sf::Color(oColor.r, oColor.g, oColor.b, alpha));
		}
		else {
			entity->destroy();
		}
	}
}

void Game::sCollision() {
	for (auto& enemy : m_entities.getEntities("Enemy")) {
		// enemies collision with border
		if (enemy->cTransform->pos.x + enemy->cCollision->radius >= m_window.getSize().x || enemy->cTransform->pos.x - enemy->cCollision->radius <= 0) {
			enemy->cTransform->velocity.x *= -1; 
		}
		if (enemy->cTransform->pos.y + enemy->cCollision->radius >= m_window.getSize().y || enemy->cTransform->pos.y - enemy->cCollision->radius <= 0) {
			enemy->cTransform->velocity.y *= -1;
		}

		// enemies collision with player
		if (enemy->cTransform->pos.dist(m_player->cTransform->pos) <= enemy->cCollision->radius + m_player->cCollision->radius) {
			enemy->destroy();
			spawnSmallEnemies(enemy);
			m_player->cTransform->pos = Vec2(m_window.getSize().x / 2.0, m_window.getSize().y / 2.0);
		}

		// enemies collision with bullet
		for (auto& bullet : m_entities.getEntities("bullet")) {
			if (enemy->cTransform->pos.dist(bullet->cTransform->pos) <= enemy->cCollision->radius + bullet->cCollision->radius) {
				bullet->destroy();
				enemy->destroy();
				m_score += enemy->cScore->score;
				spawnSmallEnemies(enemy);
			}
		}
	}

	// small enemies collision with bullet
	for (auto& enemy : m_entities.getEntities("smallEnemy")) {
		for (auto& bullet : m_entities.getEntities("bullet")) {
			if (enemy->cTransform->pos.dist(bullet->cTransform->pos) <= enemy->cCollision->radius + bullet->cCollision->radius) {
				bullet->destroy();
				enemy->destroy();
				m_score += enemy->cScore->score;
			}
		}
	}

}

void Game::sEnemySpawner() {
	if (m_currentFrame - m_lastEnemySpawnTime >= m_spawnInterval) {
		spawnEnemy();
	}
}

void Game::sGUI() {
	ImGui::Begin("Geometry Wars");

	if (ImGui::BeginTabBar("TabBar"))
	{
		if (ImGui::BeginTabItem("Systems"))
		{
			ImGui::Checkbox("Movement", &m_movement);
			ImGui::Checkbox("Lifespan", &m_lifespan);
			ImGui::Checkbox("Collision", &m_collision);
			ImGui::Checkbox("Spawning", &m_spawning);

			ImGui::SliderInt("Spawn Interval", &m_spawnInterval, 0, 100);
			if (ImGui::Button("Button")) {
				spawnEnemy();
			}
			ImGui::Checkbox("GUI", &m_gui);
			ImGui::Checkbox("Rendering", &m_rendering);

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Entity Manager"))
		{
			if (ImGui::CollapsingHeader("Entities by Tag")) {
				if (ImGui::CollapsingHeader("Enemy")) {
					for (auto& entity : m_entities.getEntities("Enemy")) {
						sf::Color eColor = entity->cShape->circle.getFillColor();
						std::string id = std::to_string(entity->id()) , 
							pos = "(" + std::to_string(entity->cTransform->pos.x) + "," + std::to_string(entity->cTransform->pos.y) + ")";

						if (ImGui::Button("Kill")) {
							entity->destroy();
						}
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(eColor.r / 255.0, eColor.g / 255.0, eColor.b / 255.0, 1.0f), id.c_str());
						ImGui::SameLine();
						ImGui::Text("Enemy  ");
						ImGui::SameLine();
						ImGui::Text(pos.c_str());
					}
				}
				if (ImGui::CollapsingHeader("Bullet")) {
					for (auto& entity : m_entities.getEntities("bullet")) {
						sf::Color eColor = entity->cShape->circle.getFillColor();
						std::string id = std::to_string(entity->id()),
							pos = "(" + std::to_string(entity->cTransform->pos.x) + "," + std::to_string(entity->cTransform->pos.y) + ")";

						if (ImGui::Button("Kill")) {
							entity->destroy();
						}
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(eColor.r / 255.0, eColor.g / 255.0, eColor.b / 255.0, 1.0f), id.c_str());
						ImGui::SameLine();
						ImGui::Text("Bullet  ");
						ImGui::SameLine();
						ImGui::Text(pos.c_str());
					}
				}
				if (ImGui::CollapsingHeader("Player")) {
					for (auto& entity : m_entities.getEntities("Player")) {
						sf::Color eColor = entity->cShape->circle.getFillColor();
						std::string id = std::to_string(entity->id()),
							pos = "(" + std::to_string(entity->cTransform->pos.x) + "," + std::to_string(entity->cTransform->pos.y) + ")";

						if (ImGui::Button("Kill")) {
							entity->destroy();
						}
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(eColor.r / 255.0, eColor.g / 255.0, eColor.b / 255.0, 1.0f), id.c_str());
						ImGui::SameLine();
						ImGui::Text("Player  ");
						ImGui::SameLine();
						ImGui::Text(pos.c_str());
					}
				}
				if (ImGui::CollapsingHeader("Small Enemy")) {
					for (auto& entity : m_entities.getEntities("smallEnemy")) {
						sf::Color eColor = entity->cShape->circle.getFillColor();
						std::string id = std::to_string(entity->id()),
							pos = "(" + std::to_string(entity->cTransform->pos.x) + "," + std::to_string(entity->cTransform->pos.y) + ")";

						if (ImGui::Button("Kill")) {
							entity->destroy();
						}
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(eColor.r / 255.0, eColor.g / 255.0, eColor.b / 255.0, 1.0f), id.c_str());
						ImGui::SameLine();
						ImGui::Text("Small Enemy  ");
						ImGui::SameLine();
						ImGui::Text(pos.c_str());
					}
				}
			}
			if (ImGui::CollapsingHeader("All Entities")) {
				for (auto& entity : m_entities.getEntities()) {
					sf::Color eColor = entity->cShape->circle.getFillColor();
					std::string id = std::to_string(entity->id()),
						pos = "(" + std::to_string(entity->cTransform->pos.x) + "," + std::to_string(entity->cTransform->pos.y) + ")";

					if (ImGui::Button("Kill")) {
						entity->destroy();
					}
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(eColor.r / 255.0, eColor.g / 255.0, eColor.b / 255.0, 1.0f), id.c_str());
					ImGui::SameLine();
					ImGui::Text((entity->tag()).c_str());
					ImGui::SameLine();
					ImGui::Text(pos.c_str());
				}
			}

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
}

void Game::sRender() {
	m_window.clear();

	for (auto& entity : m_entities.getEntities()) {
		if (entity->isActive()) {
			entity->cShape->circle.setPosition(entity->cTransform->pos.x, entity->cTransform->pos.y);
			entity->cTransform->angle += 1.0f;
			entity->cShape->circle.setRotation(entity->cTransform->angle);
			m_window.draw(entity->cShape->circle);
		}
	}

	// draw text
	m_text.setString("SCORE: " + std::to_string(m_score));
	m_window.draw(m_text);

	// draw Imgui last
	ImGui::SFML::Render(m_window);
	m_window.display();
}

void Game::sUserInput() {
	sf::Event event;
	while (m_window.pollEvent(event)) {
		// pass the event to imgui to be parsed
		ImGui::SFML::ProcessEvent(m_window, event);

		if (event.type == sf::Event::Closed) {
			m_running = false; 
		}
		if (event.type == sf::Event::KeyPressed) {
			switch (event.key.code) {
			case sf::Keyboard::W:
				m_player->cInput->up = true; 
				break;
			case sf::Keyboard::A:
				m_player->cInput->left = true;
				break;
			case sf::Keyboard::S:
				m_player->cInput->down = true;
				break;
			case sf::Keyboard::D:
				m_player->cInput->right = true;
				break;
			case sf::Keyboard::P:
				m_paused ^= 1; 
				break;
			case sf::Keyboard::Escape:
				m_window.close();
				break;
			default: break;
			}
		}
		if (event.type == sf::Event::KeyReleased) {
			switch (event.key.code) {
			case sf::Keyboard::W:
				m_player->cInput->up = false;
				break;
			case sf::Keyboard::A:
				m_player->cInput->left = false;
				break;
			case sf::Keyboard::S:
				m_player->cInput->down = false;
				break;
			case sf::Keyboard::D:
				m_player->cInput->right = false;
				break;
			default: break;
			}
		}
		if (event.type == sf::Event::MouseButtonPressed) {
			if (ImGui::GetIO().WantCaptureMouse) { continue; }  // this line ignores mouse events if ImGui is the thing being clicked

			if (event.mouseButton.button == sf::Mouse::Left) {
				std::cout << "Left Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n"; 
				spawnBullet(Vec2(event.mouseButton.x, event.mouseButton.y));
			}
			if (event.mouseButton.button == sf::Mouse::Right) {
				std::cout << "Right Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")\n";
				// call spawnSpecialWeapon here
			}
		}
	}
}


void Game::loadConfigFile(const std::string& path) {
	std::ifstream in(path);
	if (!in.is_open()) {
		std::cout << "Couldn't open config";
		return;
	}
	std::string token;

	int resWidth, resHeight, frameLimit, renderMode;
	std::string fontPath; int fontSize; RGB fontColor; 

	// Window 
	in >> token >> resWidth >> resHeight >> frameLimit >> renderMode;
	m_window.create(sf::VideoMode(resWidth, resHeight), "Assignment 2" , sf::Style::Titlebar);
	m_window.setFramerateLimit(frameLimit);

	// Font & Text
	in >> token >> fontPath >> fontSize >> fontColor.r >> fontColor.g >> fontColor.b; 
	m_font.loadFromFile(fontPath);
	m_text.setFillColor(sf::Color(fontColor.r , fontColor.g , fontColor.b));
	m_text.setCharacterSize(fontSize);
	m_text.setFont(m_font);
	m_text.setString("SCORE: 0");

	// Player
	in >> token >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >>
				   m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> 
				   m_playerConfig.OT  >> m_playerConfig.V;
	// Enemy
	in >> token >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX
				>> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT
				>> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
	// Bullet
	in >> token >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR
				>> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG
				>> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
}