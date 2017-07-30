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
}

void Monster::init() {
	texture = new Graphics4::Texture("fishy.png");
	width = texture->width / 9;
	height = texture->height / 2;
}

void Monster::update() {
	x += 1;
	++anim;
}

void Monster::render(Kore::Graphics2::Graphics2* g2, float camX, float camY) {
	int animIndex = (anim / 10) % 8 + 1;
	g2->drawScaledSubImage(texture, animIndex * width, height, width, height, x - camX, y - camY, width, height);
}
