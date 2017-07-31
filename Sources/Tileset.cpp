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
	int expectedDoorCount = (rows - 1) * 2; // Two for each floor other than the first and second to last
	doors = new vec2[(rows - 1) * 2];
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
	assert(doorCount == expectedDoorCount);
	shuffleDoors();
}

void shuffleDoors()
{
	// Shuffle floors
	for (int i = 1; i < rows * 2; ++i)
	{
		int r = Random::get(1, rows - 2);
		int r2 = Random::get(1, rows - 2);
		vec2 t1 = doors[1 + (r - 1)];
		vec2 t2 = doors[2 + (r - 1)];
		doors[1 + (r - 1)] = doors[1 + (r2 - 1)];
		doors[2 + (r - 1)] = doors[2 + (r2 - 1)];
		doors[1 + (r2 - 1)] = t1;
		doors[2 + (r2 - 1)] = t2;
	}

	// Flip floors
	for (int r = 1; r < rows - 1; ++r)
	{
		if (Random::get(0, 1))
		{
			vec2 t1 = doors[1 + (r - 1)];
			doors[1 + (r - 1)] = doors[2 + (r - 1)];
			doors[2 + (r - 1)] = t1;
		}
	}
}

void resetSpiders()
{
	spiderFrameCount = 0;
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

bool isInLight(float x, float yMin, float yMax, float fx, float fy, float mx_world, float my_world, float energy)
{
	// Light on
	if (energy < 0.1f)
		return false;
	// Same floor
	if (getFloor(yMin) != getFloor(fy))
		return false;
	// Distance small
	if (Kore::abs(fx - x) > 3 * energy * tileWidth)
		return false;
	// Angles have different signs -> flashlight must be between positions
	float angleMin = Kore::atan2(yMin - fy, x - fx);
	float angleMax = Kore::atan2(yMax - fy, x - fx);
	if (angleMin * angleMax <= 0.1f)
		return true;
	// Angles small otherwise
	float maxAngle = 0.15 * energy * Kore::pi;
	float angleFlashlight = Kore::atan2(my_world - fy, mx_world - fx);

	return Kore::abs(angleFlashlight - angleMin) < maxAngle || Kore::abs(angleFlashlight - angleMax) < maxAngle;
}

bool animateSpider(float px, float py, float fx, float fy, float mx_world, float my_world, float energy)
{
	bool caughtPlayer = false;
	++spiderFrameCount;

	bool doMove = false;
	if (spiderFrameCount >= 5)
	{
		doMove = true;
		spiderFrameCount = 0;
	}

	for (int i = 0; i < spiderCountCurr; ++i)
	{
		int collx = (spiderPos[i].x() + .5f) * tileWidth;
		int collyMin = spiderPos[i].y() * tileHeight + 9 + (spiderState[i] - Spider1) * 11;
		int collyMax = collyMin + 28;
		spiderCooldownCurr[i] -= 1;
		if (doMove)
		{
			int collynextMin = collyMin + 11;
			int collynextMax = collyMax + 11;
			bool inRange = collx - px <= tileWidth && getFloor(collyMin) == getFloor(py);
			bool inLight = isInLight(collx, collyMin, collyMax, fx, fy, mx_world, my_world, energy);
			if (inLight)
				spiderCooldownCurr[i] = spiderCooldownMax;
			bool active = inRange && !inLight;
			if (active && spiderState[i] < Spider9 && !isInLight(collx, collynextMin, collynextMax, fx, fy, mx_world, my_world, energy) && spiderCooldownCurr[i] <= 0) ++spiderState[i];
			else if (!active && spiderState[i] > Spider1) --spiderState[i];
			source[spiderPos[i].y() * columns  + spiderPos[i].x()] = spiderState[i];
		}
		caughtPlayer |= (spiderState[i] >= Spider3 && Kore::abs(collx - px) < tileWidth * 0.25f && getFloor(collyMin) == getFloor(py));
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

int getTileIndex(float px, float py) {
	int x = px / tileWidth;
	int y = py / tileHeight;
	return y * columns + x;
}

vec2 findDoor(float lastX, float lastY)
{
	int compIndex = getTileIndex(lastX, lastY);
	for (int i = 0; i < doorCount; ++i)
	{
		int index = getTileIndex(doors[i].x(), doors[i].y());
		if (index == compIndex)
		{
			if (i % 2 == 0)
				return doors[i + 1];
			else
				return doors[i - 1];
		}
	}
	return doors[doorCount - 1];
}
