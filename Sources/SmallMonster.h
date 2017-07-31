#pragma once

#include <Kore/Graphics2/Graphics.h>

class SmallMonster {
public:

	SmallMonster();
	void reset();
	static void init();
	bool update(float px, float py, float fx, float fy, float mx_world, float my_world, float energy);
	void render(Kore::Graphics2::Graphics2* g2, float camX, float camY);
	
private:
	float x, y;
	float initX, initY;
	int anim;
	enum Status {
		WalkingLeft, WalkingRight
	} status;
};
