#pragma once

#include <Kore/Graphics2/Graphics.h>

using namespace Kore;

namespace {
	const char* tileFile;
	Graphics4::Texture* image;
	
	const int lightCount = 8;
	const int spiderCountMax = 16;
	const int tileWidth = 128;
	const int tileHeight = 168;
	const int spiderCooldownMax = 30;

	int spiderFrameCount = 0;
	vec2i spiderPos[spiderCountMax];
	int spiderState[spiderCountMax];
	int spiderCooldownCurr[spiderCountMax];
	int spiderCountCurr;
	
	int rows = 5;
	int columns = 24;
	
	int* source;

	vec2* doors;
	int doorCount;
}

void loadCsv(const char* csvFile);

void initTiles(const char* csvFile, const char* tileFile);
void drawTiles(Graphics2::Graphics2* g2, float camX, float camY, vec2* lights);

int getFloor(float py);
int getTileID(float px, float py);
int getTileIndex(float px, float py);
vec2 findDoor(float lastX, float lastY);
bool isInLight(float x, float yMin, float yMax, float fx, float fy, float mx_world, float my_world, float energy);

void resetSpiders();
bool animateSpider(float px, float py, float fx, float fy, float mx_world, float my_world, float energy);
	
enum TileID {Door = 0, Window = 1, Books = 2, Closet = 3, TableGlobus = 4, TableAndLamp = 5, SpiderWeb = 6, Spider1 = 7, Spider2 = 8, Spider3 = 9, Spider4 = 10, Spider5 = 11, Spider6 = 12, Spider7 = 13, Spider8 = 14, Spider9 = 15, Wall = 16, Heater = 17, Table = 18, PC = 19, DrinkAutomat = 20, KokaKola = 21, Plant1 = 22, Plant2 = 23, Clock = 24, Poster = 25, Bank = 26, Cactus = 27, Pillar = 28, DirtyWall = 29, LightSwitch = 30, Picture = 31};
