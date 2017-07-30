#include "pch.h"

#include "Monster.h"

#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

namespace {
	Graphics4::Texture* texture;
}

Monster::Monster() {

}

void Monster::init() {
	texture = new Graphics4::Texture("fishy.png");
}
