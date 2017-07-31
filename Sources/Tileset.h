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
	
	vec2i spiderPos[spiderCountMax];
	int spiderState[spiderCountMax];
	int spiderCountCurr;
	
	int rows = 5;
	int columns = 12;
	
	int* source;

	vec2 doors[32];
	int doorCount;
}

void loadCsv(const char* csvFile);

void initTiles(const char* csvFile, const char* tileFile);
void drawTiles(Graphics2::Graphics2* g2, float camX, float camY, vec2* lights);
	
int getTileID(float px, float py);
vec2 findDoor();

int getFloor(float py);
int getTileID(float px, float py);
vec2 findDoor();
bool isInLight(float x, float y, float px, float py, float mx, float my, float camX, float camY, float energy);

bool animateSpider(float px, float py, float mx, float my, float camX, float camY, float energy);
	
enum TileID {Door = 0, Window = 1, Books = 2, Closet = 3, TableGlobus = 4, TableAndLamp = 5, SpiderWeb = 6, Spider1 = 7, Spider2 = 8, Spider3 = 9, Spider4 = 10, Spider5 = 11, Spider6 = 12, Spider7 = 13, Spider8 = 14, Spider9 = 15, Wall = 16, Heater = 17, Table = 18, PC = 19, DrinkAutomat = 20, KokaKola = 21, Plant1 = 22, Plant2 = 23, Clock = 24, Poster = 25, Bank = 26, Cactus = 27, Pillar = 28, DirtyWall = 29, LightSwitch = 30, Picture = 31};
