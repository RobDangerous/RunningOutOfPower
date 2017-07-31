#include "pch.h"

#include "Tileset.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Log.h>
#include <Kore/Graphics1/Image.h>
#include <Kore/TextureImpl.h>
#include <Kore/Math/Random.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

void initTiles(const char* csvFilePath, const char* tileFilePath) {
	tileFile = tileFilePath;
	loadCsv(csvFilePath);
	image = new Graphics4::Texture(tileFilePath);
}

void loadCsv(const char* csvFile) {
	FileReader file(csvFile);
	
	void* data = file.readAll();
	int length = file.size();
	
	char* cpyData = new char[length + 1];
	for (int i = 0; i < length; ++i) {
		cpyData[i] = ((char*)data)[i];
	}
	cpyData[length] = 0;
	
	source = new int[rows * columns];
	int i = 0;
	
	char delimiter[] = ",;\n";
	char* ptr = std::strtok(cpyData, delimiter);
	while (ptr != nullptr) {
		assert(i < rows * columns);
		int num = atoi(ptr);
		//log(Info, "%i -> %i", i, num);
		source[i] = num;
		ptr = std::strtok(nullptr, delimiter);
		i++;
	}

	doorCount = 0;
	spiderCountCurr = 0;
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < columns; ++x) {
			int index = source[y * columns + x];
			if (index == Door) {
				doors[doorCount] = vec2(x * tileWidth, y * tileHeight);
				++doorCount;
			}
			else if (index >= Spider1 && index <= Spider9) {
				assert(spiderCountCurr < spiderCountMax);
				spiderPos[spiderCountCurr] = vec2i(x, y);
				spiderState[spiderCountCurr] = Spider1;
				spiderDir[spiderCountCurr] = 1;
				++spiderCountCurr;
			}
		}
	}
}

void drawTiles(Graphics2::Graphics2* g2, float camX, float camY, vec2* lights) {
	int lightIndex = 0;

	const int sourceColumns = image->texWidth / tileWidth;
	const int sourceRows = image->texHeight / tileHeight;
	//const int numOfTiles = rows * columns;
	//tiles = new Graphics4::Texture*[numOfTiles];
	
	for(int y = 0; y < rows; ++y) {
		for (int x = 0; x < columns; ++x) {
			int index = source[y * columns + x];

			if (index == TableAndLamp) {
				lights[lightIndex] = vec2(x * tileWidth - camX + tileWidth - 40, y * tileHeight - camY + 60);
				++lightIndex;
			}
			
			int row    = (int)(index / sourceColumns);
			int column = index % sourceColumns;
			
			//Graphics4::Texture* tile = new Graphics4::Texture();
			g2->drawScaledSubImage(image, column * tileWidth, row * tileHeight , tileWidth, tileHeight, x * tileWidth - camX, y * tileHeight - camY, tileWidth, tileHeight);
		}
	}
}

bool isInLight(float x, float y, float px, float py, float mx, float my, float camX, float camY, float energy)
{
	//log(Info, "%i -> %i", getFloor(y), getFloor(py));
	// Light on
	return energy >= 0.1f &&
		// Same floor
		getFloor(y) == getFloor(py) &&
		// Distance small
		Kore::abs(px - x) <= 200 &&
		// Angle small
		Kore::abs(Kore::atan2(my - (py - camY), mx - (px - camX)) - Kore::atan2(y - (py - camY), x - (px - camX))) < 0.2 * Kore::pi;
}

void animateSpider(float px, float py, float mx, float my, float camX, float camY, float energy)
{
	static int frameCount = 0;
	++frameCount;
	if (frameCount >= 5)
	{
		frameCount = 0;
		for (int i = 0; i < spiderCountCurr; ++i)
		{
			//log(Info, "Spider %i:", i);
			spiderState[i] += spiderDir[i];
			bool inRange = vec2(spiderPos[i].x() * tileWidth - px, spiderPos[i].y() * tileHeight - py).squareLength() <= tileWidth * tileHeight * 4;
			bool active = inRange && !isInLight(spiderPos[i].x() * tileWidth, spiderPos[i].y() * tileHeight, px, py, mx, my, camX, camY, energy);
			if (spiderState[i] >= Spider9) spiderDir[i] = -1;

			else if (spiderState[i] <= Spider1) spiderDir[i] = active ? +1 : 0;
			else if (spiderState[i] > Spider1 && !active) spiderDir[i] = -1;
			source[spiderPos[i].y() * columns  + spiderPos[i].x()] = spiderState[i];
		}
	}
}


int getFloor(float py) {
	return ((int)py) / tileHeight;
}

int getTileID(float px, float py) {
	int x = px / tileWidth;
	int y = py / tileHeight;
	return source[y * columns  + x];
}

vec2 findDoor() {
	static vec2 last = doors[0];
	vec2 door = doors[Random::get(0, doorCount - 1)];
	while (door == last) {
		door = doors[Random::get(0, doorCount - 1)];
	}
	last = door;
	return door;
}
