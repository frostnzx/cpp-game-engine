#include "Physics.hpp"
#include "Components.hpp"

#include <math.h>
#include <iostream>

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	auto& aPos = a->getComponent<CTransform>().pos, bPos = b->getComponent<CTransform>().pos; 
	auto& aBox = a->getComponent<CBoundingBox>().halfSize, bBox = b->getComponent<CBoundingBox>().halfSize; 

	Vec2 delta(abs(aPos.x - bPos.x), abs(aPos.y - bPos.y));
	float ox = aBox.x + bBox.x - delta.x;
	float oy = aBox.y + bBox.y - delta.y;

	return Vec2(ox, oy);
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	auto& aPos = a->getComponent<CTransform>().prevPos, bPos = b->getComponent<CTransform>().prevPos;
	auto& aBox = a->getComponent<CBoundingBox>().halfSize, bBox = b->getComponent<CBoundingBox>().halfSize;

	Vec2 delta(abs(aPos.x - bPos.x), abs(aPos.y - bPos.y));
	float ox = aBox.x + bBox.x - delta.x;
	float oy = aBox.y + bBox.y - delta.y;

	return Vec2(ox, oy);
}


/*
Top collision -> previous.overlap.x > 0 and its y is higher
Bottom collision -> previous.overlap.x > 0 and its y is lower

Left collision -> previous.overlap.y > 0 and its x is lower
Right collision -> previous.overlap.y > 0 and its x is higher
*/