#include "Animation.hpp"
#include "Vec2.hpp"

#include <iostream>

Animation::Animation()
{
}

Animation::Animation(const std::string& name, const sf::Texture& texture)
	: Animation(name, texture, 1, 0)
{
}

Animation::Animation(const std::string& name, const sf::Texture& texture,
	size_t keyframesCount, size_t duration)
	: m_name(name)
	, m_sprite(texture)
	, m_keyframesCount(keyframesCount)
	, m_currentKeyframe(0)
	, m_duration(duration)
{
	m_size = Vec2((float)texture.getSize().x / keyframesCount, (float)texture.getSize().y);
	m_sprite.setOrigin(m_size.x / 2.0f, m_size.y / 2.0f); // set originpoint of the sprite to middle
	m_sprite.setTextureRect(sf::IntRect(std::floor(m_currentKeyframe) * m_size.x, 0, m_size.x, m_size.y));
	 
	// NOTE : originpoint is where you want in the texture to be drawn to that position in the screen
}
void Animation::update()
{
	m_currentKeyframe++; // how many frame this animation has been alive for ?
	int animationFrame = (m_currentKeyframe / m_duration) % m_keyframesCount;
	m_sprite.setTextureRect(sf::IntRect(animationFrame * m_size.x, 0, m_size.x, m_size.y));

	/*
	if (m_name == "MarioRun") {
		std::cerr << "m_currentKeyFrame = " << m_currentKeyframe << " , " << "animationFrame = " << animationFrame << std::endl;
	}
	*/
}

const Vec2& Animation::getSize() const
{
	return m_size;
}
const std::string& Animation::getName() const
{
	return m_name;
}

sf::Sprite& Animation::getSprite()
{
	return m_sprite;
}

bool Animation::hasEnded() const
{
	int animationFrame = (m_currentKeyframe / m_duration) % m_keyframesCount;
	return (animationFrame == m_keyframesCount - 1) ? true : false;
}