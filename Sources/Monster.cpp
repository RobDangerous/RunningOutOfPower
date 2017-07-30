#include "pch.h"

#include "Monster.h"
#include "Tileset.h"

#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

namespace {
	Graphics4::Texture* texture;
	int width, height;
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
}

void Monster::update() {
	switch (status) {
	case WalkingLeft:
		x -= 1;
		++anim;
		if (x < 100) {
			status = WalkingRight;
		}
		break;
	case WalkingRight:
		x += 1;
		++anim;
		if (x > 1200) {
			status = WalkingLeft;
		}
		break;
	}
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
