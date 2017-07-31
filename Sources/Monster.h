#pragma once

#include <Kore/Graphics2/Graphics.h>

class Monster {
public:
	float x, y;
	int anim;
	
	Monster();
	void position();
	static void init();
	bool update(float px, float py, float fx, float fy, float mx, float my, float camX, float camY, float energy);
	void render(Kore::Graphics2::Graphics2* g2, float camX, float camY);
	void changeFloor();

private:
	
	enum Status {
		WalkingLeft, WalkingRight, StandingLeft, StandingRight
	} status;
};
