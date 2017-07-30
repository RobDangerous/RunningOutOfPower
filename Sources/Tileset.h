#pragma once

#include <Kore/Graphics2/Graphics.h>

using namespace Kore;

const int lightCount = 8;
const int spiderCountMax = 16;
const int tileWidth = 128;
const int tileHeight = 168;

class Tileset {
private:
	int* source;
	Graphics4::Texture** tiles;
	
	const char* tileFile;
	int tileWidth;
	int tileHeight;
	
	int rows;
	int columns;

	vec2 doors[32];
	int doorCount;
	vec2i spiderPos[spiderCountMax];
	int spiderState[spiderCountMax];
	int spiderDir[spiderCountMax];
	int spiderCountCurr;

	Graphics4::Texture* image;
	
	void loadCsv(const char* csvFile, int rows, int columns);
	
public:
	
	Tileset(const char* csvFile, const char* tileFile, int rows, int columns, int tileWidth, int tileHeight);
	void drawTiles(Graphics2::Graphics2* g2, float camX, float camY, vec2* lights);

	int getFloor(float py);
	int getTileID(float px, float py);
	vec2 findDoor();
	bool isInLight(float x, float y, float px, float py, float mx, float my, float camX, float camY, float energy);
	
	void animateSpider(float px, float py, float mx, float my, float camX, float camY, float energy);
	
	enum TileID {Door = 0, Window = 1, Books = 2, Closet = 3, Table = 4, TableAndLamp = 5, SpiderWeb = 6, Spider1 = 7, Spider2 = 8, Spider3 = 9, Spider4 = 10, Spider5 = 11, Spider6 = 12, Spider7 = 13, Spider8 = 14, Spider9 = 15};
};
