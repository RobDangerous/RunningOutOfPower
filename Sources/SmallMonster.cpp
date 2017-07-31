#include "pch.h"

#include "SmallMonster.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Math/Core.h>
#include <Kore/Math/Random.h>
#include <Kore/Log.h>

using namespace Kore;

namespace {
	Graphics4::Texture* texture;
	int width, height;
}

SmallMonster::SmallMonster() : initX(400), initY(0) {

}

void SmallMonster::reset() {
	x = initX;
	y = initY;
	anim = 0;
}

void SmallMonster::init() {
	texture = new Graphics4::Texture("eye.png");
	width = texture->width / 2;
	height = texture->height / 1;
}

bool SmallMonster::update(float px, float py, float fx, float fy, float mx_world, float my_world, float energy) {
	x += 2;
	y = Kore::sin(x / 50.0f) * 20 + 50;
	return false;
}

void SmallMonster::render(Kore::Graphics2::Graphics2* g2, float camX, float camY) {
	static int anim = 0;
	++anim;
	int index = anim / 20 % 2;
	index = 0;
	//g2->drawScaledSubImage(texture, width * index, 0, width, height, x - camX, y - camY, width, height);
	g2->drawScaledSubImage(texture, width * (index + 1), 0, -width, height, x - camX, y - camY, width, height);
}
