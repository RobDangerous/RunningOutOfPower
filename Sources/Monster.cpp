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

Monster::Monster() : x(400), y(0) {

}

void Monster::position() {
	y = tileHeight - height;
	status = WalkingRight;
}

void Monster::init() {
	texture = new Graphics4::Texture("fishy.png");
	width = texture->width / 9;
	height = texture->height / 2;
	doorLock = 0;
}

void Monster::update() {
	doorLock ++;
	int tile = getTileID(x + width / 2, y + height / 2);
	if (tile == Door && doorLock > 50) {
		
		vec2 door = findDoor();
		x = door.x() + 32;
		y = door.y() + tileHeight - height;
		
		doorLock = 0;
	}
	
	switch (status) {
	case WalkingLeft:
		x -= 4;
		++anim;
		if (x < 100) {
			status = WalkingRight;
		}
		break;
	case WalkingRight:
		x += 4;
		++anim;
		if (x > 1200) {
			status = WalkingLeft;
		}
		break;
	}
}

void Monster::changeFloor() {
	
	

}

void Monster::render(Kore::Graphics2::Graphics2* g2, float camX, float camY) {
	switch (status) {
	case WalkingRight: {
		int animIndex = (anim / 10) % 8 + 1;
		g2->drawScaledSubImage(texture, animIndex * width, height, width, height, x - camX, y - camY, width, height);
		break;
	}
	case WalkingLeft: {
		int animIndex = (anim / 10) % 8 + 1;
		g2->drawScaledSubImage(texture, animIndex * width, 0, width, height, x - camX, y - camY, width, height);
		break;
	}
	}
}
