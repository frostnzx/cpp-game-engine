#include "EntityManager.h"

EntityManager::EntityManager() {

}

void EntityManager::update() {
	for (auto entity : m_entitiesToAdd) {
		m_entities.push_back(entity);
		m_entityMap[entity->tag()].push_back(entity);
	}
	m_entitiesToAdd.clear();

	removeDeadEntities(m_entities);
	for (auto& [tag, entityVec] : m_entityMap) {
		removeDeadEntities(entityVec);
	}
}

void EntityManager::removeDeadEntities(EntityVec& vec) {
	auto itr = vec.begin();
	while (itr != vec.end()) {
		if ((*itr)->isActive() == false) {
			itr = vec.erase(itr); 
			continue;
		}
		itr++;
	}
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag) {
	// ---- EXPLAINATION ----
	//std::shared_ptr<Entity> entity = std::make_shared<Entity>(m_totalEntities++, tag);
	// can't use method above bcuz the entity class has private constructor so you can't construct it with make_shared
	// even if the entityManager class is friend of Entity class it still won't let you use it
	// bcuz if we use make_shared the Entity class will be construct outside the EntityManager.

	auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++ , tag)); 
	// new Entity(args) is being construct in EntityManager so its fine

	m_entitiesToAdd.push_back(entity);                           

	return entity; 
}

const EntityVec& EntityManager::getEntities() {
	return m_entities; 
}
const EntityVec& EntityManager::getEntities(const std::string& tag) { // pass by const reference
	return m_entityMap[tag]; 
}
const EntityMap& EntityManager::getEntityMap() {
	return m_entityMap; 
}