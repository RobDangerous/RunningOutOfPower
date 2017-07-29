#include "pch.h"

#include "Tileset.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Log.h>
#include <Kore/Graphics1/Image.h>
#include <Kore/TextureImpl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

Tileset::Tileset(const char* csvFile, const char* tileFile, int rows, int columns, int tileWidth, int tileHeight) : tileFile(tileFile), rows(rows), columns(columns), tileWidth(tileWidth), tileHeight(tileHeight) {
	
	loadCsv(csvFile, rows, columns);
}

void Tileset::loadCsv(const char* csvFile, int rows, int columns) {
	FileReader file(csvFile);
	
	void* data = file.readAll();
	int length = file.size();
	
	char* source = new char[length + 1];
	for (int i = 0; i < length; ++i) {
		source[i] = ((char*)data)[i];
	}
	source[length] = 0;
	
	this->source = new int[rows * columns];
	int i = 0;
	
	char delimiter[] = ",;";
	char* ptr = std::strtok(source, delimiter);
	while (ptr != nullptr) {
		assert(i < rows * columns);
		int num = atoi(ptr);
		//log(Info, "%i -> %i", i, num);
		this->source[i] = num;
		ptr = std::strtok(nullptr, delimiter);
		i++;
	}
	image = new Graphics4::Texture(tileFile);
}

void Tileset::drawTiles(Graphics2::Graphics2* g2) {	
	const int sourceColumns = image->texWidth / tileWidth;
	const int sourceRows = image->texHeight / tileHeight;
	//const int numOfTiles = rows * columns;
	//tiles = new Graphics4::Texture*[numOfTiles];
	
	for(int y = 0; y < rows; ++y) {
		for (int x = 0; x < columns; ++x) {
			int index = source[y * (columns-1) + x];
			
			int row    = (int)(index / sourceColumns);
			int column = index % sourceColumns;
			
			int xOffset = 0;//column;	// TODO: should be 0
			int yOffset = 0;//row;
			
			//Graphics4::Texture* tile = new Graphics4::Texture();
			g2->drawScaledSubImage(image, column*tileWidth+xOffset, row*tileHeight+yOffset , tileWidth, tileHeight, x*tileWidth, y*tileHeight, tileWidth, tileHeight);
		}
	}
	
}




