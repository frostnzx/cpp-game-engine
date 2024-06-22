#ifndef ENTITY_M
#define ENTITY_M

#include "Entity.h"
#include <vector>
#include <map>

typedef std::vector<std::shared_ptr<Entity>> EntityVec; // typedef is just a safer and language supported define
typedef std::map<std::string, EntityVec> EntityMap; 

/*
Q : Why we define EntityVec using vector<shared_ptr<Entity>> instead of just vector<Entity> ?
	(why use heap when we can use stack?)
A : Because Entity is a big object , therefore it's require a large memory space
	that's why heap is a better fit for it

REF : https://stackoverflow.com/questions/102009/when-is-it-best-to-use-the-stack-instead-of-the-heap-and-vice-versa
*/

class EntityManager
{
	EntityVec m_entities;
	EntityVec m_entitiesToAdd; 
	EntityMap m_entityMap;
	size_t m_totalEntities = 0; 

	void removeDeadEntities(EntityVec& vec);

public:
	EntityManager();

	void update();

	std::shared_ptr<Entity> addEntity(const std::string& tag);

	const EntityVec& getEntities();
	const EntityVec& getEntities(const std::string& tag);
	const EntityMap& getEntityMap();
};


#endif