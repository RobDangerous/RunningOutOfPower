#pragma once

#include <Kore/Graphics2/Graphics.h>

class Monster {
public:
	float x, y;
	int anim;
	
	Monster();
	void position();
	static void init();
	void update();
	void render(Kore::Graphics2::Graphics2* g2, float camX, float camY);
	void changeFloor();

private:
	
	enum Status {
		WalkingLeft, WalkingRight
	} status;
};
