#include "Entity.h"
#include <SFML/Graphics.hpp>

Entity::Entity(const size_t i , const std::string & t)
	: m_id(i) 
	, m_tag(t)
{}

// getter usually made const so they can be called on both const and 
// non-const object

bool Entity::isActive() const {
	return m_active;
}

const std::string& Entity::tag() const {
	return m_tag;
}
const size_t Entity::id() const {
	return m_id;
}
void Entity::destroy() {
	m_active = false;
}