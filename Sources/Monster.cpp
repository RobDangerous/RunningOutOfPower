#include "pch.h"

#include "Monster.h"
#include "Tileset.h"

#include <Kore/Math/Random.h>
#include <Kore/Log.h>

using namespace Kore;

Monster::Monster() : initX(400), initY(0) {
	
}

void Monster::reset(bool firstFloor) {	
	if (firstFloor) {
		x = 100;
		y = tileHeight - height;
	} else {
		x = Random::get(0, columns * tileWidth);
		y = Random::get(1, rows) * tileHeight + tileHeight - height;
	}
	
	anim = 0;
	status = WalkingRight;
	
	frameCount = 0;
	directionLock = 0;
	
	log(Info, "Monster at floor %i", getFloor(y));
}

void Monster::init(const char* textureName, int animTiles) {
	texture = new Graphics4::Texture(textureName);
	width = texture->width / animTiles;
	height = texture->height / 1;
	//doorLock = 0;
	this->animTiles = animTiles;
}

bool Monster::update(float px, float py, float fx, float fy, float mx_world, float my_world, float energy) {
	/*doorLock ++;
	int tile = getTileID(x + width / 2, y + height / 2);
	if (tile == Door && doorLock > 1000 && Random::get(0, 1)) {
		vec2 door = findDoor(x + width / 2, y + height / 2);
		x = door.x() + 32;
		y = door.y() + tileHeight - height;
		
		doorLock = 0;
		
		log(Info, "Monster at floor %i", getFloor(y));
	}*/
	
	/*if (directionLock == 0) {
		int r = Random::get(0, 4);
		switch (status) {
			case 0:
				status = WalkingLeft;
				break;
			case 1:
				status = WalkingRight;
				break;
			case 2:
				if
				status = WalkingRight;
				break;
			case 3:
				status = WalkingRight;
				break;
			default:
				break;
		}
	}*/

	bool inLight = isInLight(x + width / 2, y, y + height / 2, fx, fy, mx_world, my_world, energy);
	switch (status) {
	case WalkingLeft:
		x -= 2;
		++anim;
		if (x < 0 || (inLight && fx < x)) {
			if (x > columns * tileWidth - 100)
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
		if (x > columns * tileWidth - 50 || (inLight && fx > x)) {
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
	
	++ frameCount;
	directionLock = frameCount % 300;
	
	return (Kore::abs(x + width / 2 - px) < tileWidth * 0.25f && getFloor(y + height / 2) == getFloor(py));
}

void Monster::changeFloor() {
	
	

}

void Monster::render(Kore::Graphics2::Graphics2* g2, float camX, float camY) {
	int animIndex = (anim / 10) % animTiles;
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
