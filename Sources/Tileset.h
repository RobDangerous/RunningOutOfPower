#pragma once

#include <Kore/Graphics2/Graphics.h>

using namespace Kore;

const int lightCount = 8;

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

	Graphics4::Texture* image;
	
	int spiderID = 0;
	int as = 1;
	
	void loadCsv(const char* csvFile, int rows, int columns);
	
public:
	
	Tileset(const char* csvFile, const char* tileFile, int rows, int columns, int tileWidth, int tileHeight);
	void drawTiles(Graphics2::Graphics2* g2, float camX, float camY, vec2* lights);
	
	int getTileID(float px, float py);
	vec2 findDoor();
	
	void animateSpider(float px, float py);
	
	enum TileID {Door = 0, Window = 1, Books = 2, Closet = 3, Table = 4, TableAndLamp = 5, SpiderWeb = 6, Spider1 = 7, Spider2 = 8, Spider3 = 9};
};
