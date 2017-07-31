#pragma once

#include <Kore/Graphics2/Graphics.h>

class Monster {
public:
	
	Monster();
	void reset();
	static void init();
	bool update(float px, float py, float fx, float fy, float mx, float my, float camX, float camY, float energy);
	void render(Kore::Graphics2::Graphics2* g2, float camX, float camY);
	void changeFloor();

private:
	float x, y;
	float initX, initY;
	int anim;
	
	enum Status {
		WalkingLeft, WalkingRight, StandingLeft, StandingRight
	} status;
};
