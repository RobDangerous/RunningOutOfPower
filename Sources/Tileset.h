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

	Graphics4::Texture* image;
	
	void loadCsv(const char* csvFile, int rows, int columns);
	
public:
	
	Tileset(const char* csvFile, const char* tileFile, int rows, int columns, int tileWidth, int tileHeight);
	void drawTiles(Graphics2::Graphics2* g2, float camX, float camY, vec2* lights);
	
	int getTileID(float px, float py);
	
	enum TileID {Door = 0, Window = 1, Books = 2, Closet = 3, Table = 4, TableAndLamp = 5};
};
