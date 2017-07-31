#include "pch.h"

#include "SmallMonster.h"
#include "Tileset.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Math/Core.h>
#include <Kore/Math/Random.h>
#include <Kore/Log.h>

using namespace Kore;

namespace {
	Graphics4::Texture* textures[3];
	int widths[3], heights[3];
}

SmallMonster::SmallMonster() : monsterIndex(2) {

}

void SmallMonster::reset(int row) {
	anim = 0;
	status = WalkingRight;
	monsterIndex = Random::get(0, 2);
	x = Random::get(200, tileWidth * columns - 100);
	y = startY = row * tileHeight + tileHeight - heights[monsterIndex];
}

void SmallMonster::init() {
	textures[0] = new Graphics4::Texture("eye.png");
	textures[1] = new Graphics4::Texture("beard.png");
	textures[2] = new Graphics4::Texture("pac.png");
	widths[0] = textures[0]->width / 2;
	heights[0] = textures[0]->height / 1;
	widths[1] = textures[1]->width / 2;
	heights[1] = textures[1]->height / 1;
	widths[2] = textures[2]->width / 2;
	heights[2] = textures[2]->height / 1;
}

bool SmallMonster::update(float px, float py, float fx, float fy, float mx_world, float my_world, float energy) {
	bool inLight = isInLight(x + widths[monsterIndex] / 2, y, y + heights[monsterIndex] / 2, fx, fy, mx_world, my_world, energy);
	
	if (inLight && fx < x) {
		status = WalkingRight;
	}
	else if (inLight && fx > x + widths[monsterIndex]) {
		status = WalkingLeft;
	}

	if (status == WalkingRight && x > columns * tileWidth - 100) {
		status = WalkingLeft;
	}
	else if (status == WalkingLeft && x < 100) {
		status = WalkingRight;
	}

	if (status == WalkingRight) {
		if (x < columns * tileWidth - 102) {
			x += 2;
		}
	}
	else {
		if (x > 102) {
			x -= 2;
		}
	}
	
	y = -Kore::abs(Kore::sin(x / 50.0f)) * 20 + startY;
	return (Kore::abs(x + widths[monsterIndex] / 2 - px) < tileWidth * 0.25f && getFloor(y + heights[monsterIndex] / 2) == getFloor(py));
}

void SmallMonster::render(Kore::Graphics2::Graphics2* g2, float camX, float camY) {
	++anim;
	int index = (anim / 10) % 2;
	if (status == WalkingLeft) {
		g2->drawScaledSubImage(textures[monsterIndex], widths[monsterIndex] * index, 0, widths[monsterIndex], heights[monsterIndex], x - camX, y - camY, widths[monsterIndex], heights[monsterIndex]);
	}
	else {
		g2->drawScaledSubImage(textures[monsterIndex], widths[monsterIndex] * (index + 1), 0, -widths[monsterIndex], heights[monsterIndex], x - camX, y - camY, widths[monsterIndex], heights[monsterIndex]);
	}
}
