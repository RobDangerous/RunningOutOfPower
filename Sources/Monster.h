#pragma once

#include <Kore/Graphics2/Graphics.h>
#include <Kore/Graphics4/Graphics.h>

class Monster {
public:
	enum MonsterTyp {
		Teacher, Janitor, Book
	} monsterTyp;
	
	Monster();
	void reset(bool firstFloor);
	void init(const char* textureName, int animTiles, MonsterTyp typ);
	bool update(float px, float py, float fx, float fy, float mx_world, float my_world, float energy);
	void render(Kore::Graphics2::Graphics2* g2, float camX, float camY);
	void changeFloor();
	
	float x, y;
	MonsterTyp typ;

private:
	float initX, initY;
	int anim;
	int animTiles;
	
	Kore::Graphics4::Texture* texture;
	int width, height;
	
	int frameCount;
	//int doorLock;
	int directionLock;
	
	enum Status {
		WalkingLeft, WalkingRight, StandingLeft, StandingRight
	} status;
	
};
