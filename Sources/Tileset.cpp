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
				spiderCooldownCurr[spiderCountCurr] = 0;
				++spiderCountCurr;
			}
		}
	}
}

void resetSpiders()
{
	for (int i = 0; i < spiderCountCurr; ++i)
	{
		spiderState[i] = Spider1;
		spiderCooldownCurr[i] = 0;
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

bool isInLight(float x, float y, float fx, float fy, float mx, float my, float camX, float camY, float energy)
{
	// Light on
	return energy >= 0.1f &&
		// Same floor
		getFloor(y) == getFloor(fy) &&
		// Distance small
		Kore::abs(fx - x) <= 3 * energy * tileWidth &&
		// Angle small
		Kore::abs(Kore::atan2(my - (fy - camY), mx - (fx - camX)) - Kore::atan2(y - fy, x - fx)) < 0.2 * Kore::pi;
}

bool animateSpider(float px, float py, float fx, float fy, float mx, float my, float camX, float camY, float energy)
{
	bool caughtPlayer = false;
	static int frameCount = 0;
	++frameCount;

	bool doMove = false;
	if (frameCount >= 5)
	{
		doMove = true;
		frameCount = 0;
	}

	for (int i = 0; i < spiderCountCurr; ++i)
	{
		int collx = (spiderPos[i].x() + .5f) * tileWidth;
		int colly = spiderPos[i].y() * tileHeight + 9 + (spiderState[i] - Spider1) * 11 + 14;
		spiderCooldownCurr[i] -= 1;
		if (doMove)
		{
			int collynext = colly + 11;
			bool inRange = collx - px <= tileWidth && getFloor(colly) == getFloor(py);
			bool inLight = isInLight(collx, colly, fx, fy, mx, my, camX, camY, energy);
			if (inLight)
				spiderCooldownCurr[i] = spiderCooldownMax;
			bool active = inRange && !inLight;
			if (active && spiderState[i] < Spider9 && !isInLight(collx, collynext, fx, fy, mx, my, camX, camY, energy) && spiderCooldownCurr[i] <= 0) ++spiderState[i];
			else if (!active && spiderState[i] > Spider1) --spiderState[i];
			source[spiderPos[i].y() * columns  + spiderPos[i].x()] = spiderState[i];
		}
		caughtPlayer |= (spiderState[i] >= Spider3 && Kore::abs(collx - px) < tileWidth * 0.25f && getFloor(colly) == getFloor(py));
	}
	return caughtPlayer;
}


int getFloor(float py) {
	return ((int)py) / tileHeight;
}

int getTileID(float px, float py) {
	int x = px / tileWidth;
	int y = py / tileHeight;
	return source[y * columns  + x];
}

vec2 findDoor(float lastX, float lastY) {
	vec2 door = doors[Random::get(0, doorCount - 1)];
	while (door.x() - lastX <= 0.0001 && door.y() - lastY <= 0.0001) {
		door = doors[Random::get(0, doorCount - 1)];
	}
	return door;
}
