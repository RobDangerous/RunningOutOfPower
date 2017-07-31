#include "pch.h"

#include "Monster.h"
#include "Tileset.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Math/Random.h>
#include <Kore/Log.h>

using namespace Kore;

namespace {
	Graphics4::Texture* texture;
	int width, height;
	
	int doorLock;
}

Monster::Monster() : initX(400), initY(0) {
	reset();
}

void Monster::reset() {
	x = Random::get(0, columns * tileWidth);
	y = Random::get(0, rows) * tileHeight + tileHeight - height;
	
	anim = 0;
	status = WalkingRight;
	
	log(Info, "Monster at floor %i", getFloor(y));
}

void Monster::init() {
	texture = new Graphics4::Texture("janitor.png");
	width = texture->width / 4;
	height = texture->height / 1;
	doorLock = 0;
}

bool Monster::update(float px, float py, float fx, float fy, float mx_world, float my_world, float energy) {
	doorLock ++;
	int tile = getTileID(x + width / 2, y + height / 2);
	if (tile == Door && doorLock > 1000 && Random::get(0, 1)) {
		vec2 door = findDoor(x + width / 2, y + height / 2);
		x = door.x() + 32;
		y = door.y() + tileHeight - height;
		
		doorLock = 0;
		
		log(Info, "Monster at floor %i", getFloor(y));
	}

	bool inLight = isInLight(x + width / 2, y, y + height / 2, fx, fy, mx_world, my_world, energy);
	switch (status) {
	case WalkingLeft:
		x -= 2;
		++anim;
		if (x < 0 || (inLight && fx < x)) {
			if (x > 1300)
			{
				status = StandingRight;
			}
			else
			{
				status = WalkingRight;
			}
		}
		break;
	case WalkingRight:
		x += 2;
		++anim;
		if (x > 1350 || (inLight && fx > x)) {
			if (x < 50)
			{
				status = StandingLeft;
			}
			else
			{
				status = WalkingLeft;
			}
		}
		break;
	case StandingLeft:
		if (!inLight) {
			status = WalkingLeft;
		}
		break;
	case StandingRight:
		if (!inLight) {
			status = WalkingRight;
		}
		break;
	}
	return (Kore::abs(x + width / 2 - px) < tileWidth * 0.25f && getFloor(y + height / 2) == getFloor(py));
}

void Monster::changeFloor() {
	
	

}

void Monster::render(Kore::Graphics2::Graphics2* g2, float camX, float camY) {
	int animIndex = (anim / 10) % 4;
	switch (status) {
	case WalkingRight:
	case StandingRight: {
		g2->drawScaledSubImage(texture, (animIndex + 1) * width, 0, -width, height, x - camX, y - camY, width, height);
		break;
	}
	case WalkingLeft:
    case StandingLeft: {
		g2->drawScaledSubImage(texture, animIndex * width, 0, width, height, x - camX, y - camY, width, height);
		break;
	}
	}
}
