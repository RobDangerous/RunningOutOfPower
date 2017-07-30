#include "pch.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics2/Graphics.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Input/Gamepad.h>
#include <Kore/Audio2/Audio.h>
#include <Kore/Audio1/Audio.h>
#include <Kore/Audio1/Sound.h>
#include <Kore/Audio1/SoundStream.h>
#include <Kore/Math/Random.h>
#include <Kore/System.h>
#include <Kore/Log.h>
#include <Kore/Math/Core.h>
#include <Kore/Graphics1/Color.h>

#include "Tileset.h"

using namespace Kore;

namespace {
	Tileset* tileset;
	
	const int tileWidth = 128;
	const int tileHeight = 168;
	const int rows = 5;
	const int columns = 12;
	const int w = 768;
	const int h = 768;

	float playerWidth;
	float playerHeight;

	float camX = 0;
	float camY = 0;

    Graphics2::Graphics2* g2;
	
	Graphics4::Texture* playerImage;
	
	Graphics4::Texture* batteryImage;

	Graphics4::RenderTarget* screen;
	Graphics4::PipelineState* pipeline;
	Graphics4::ConstantLocation aspectLocation;
	Graphics4::ConstantLocation angleLocation;
	Graphics4::ConstantLocation playerLocation;
	Graphics4::ConstantLocation mouseLocation;
	Graphics4::ConstantLocation animLocation;
	Graphics4::ConstantLocation lightsLocation;
	Graphics4::ConstantLocation energyLocation;

	float energy = 1.0;

	float angle = 0.0f;

	float px = 0;
	float py = 0;
	float mx = 0.0f;
	float my = 0.0f;

	int lastDirection = 1;	// 0 - left, 1 - right
	bool left = false;
	bool right = false;
	bool down_ = false;
	bool up = false;
	
	int frameCount = 0;
	
	int runIndex = 0;
	
	Kravur* font14;
	Kravur* font24;
	Kravur* font34;
	Kravur* font44;
	
	char dText[42];
	float dTime = 0;
	
	char doorText[42];
	char closetText[42];
	
	vec4 doorButton;
	vec4 closetButton;
	vec2 debugText;
	
	bool inCloset = false;
	
	void createPipeline() {
		Graphics4::VertexStructure structure;
		structure.add("vertexPosition", Graphics4::Float3VertexData);
		structure.add("texPosition", Graphics4::Float2VertexData);
		structure.add("vertexColor", Graphics4::Float4VertexData);

		FileReader fs("flashlight.frag");
		FileReader vs("flashlight.vert");
		Graphics4::Shader* fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
		Graphics4::Shader* vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);

		pipeline = new Graphics4::PipelineState;
		pipeline->fragmentShader = fragmentShader;
		pipeline->vertexShader = vertexShader;

		pipeline->blendSource = Graphics4::BlendOne;
		pipeline->blendDestination = Graphics4::InverseSourceAlpha;
		pipeline->alphaBlendSource = Graphics4::SourceAlpha;
		pipeline->alphaBlendDestination = Graphics4::InverseSourceAlpha;

		pipeline->inputLayout[0] = &structure;
		pipeline->inputLayout[1] = nullptr;
		pipeline->compile();

		aspectLocation = pipeline->getConstantLocation("aspect");
		angleLocation = pipeline->getConstantLocation("angle");
		playerLocation = pipeline->getConstantLocation("player");
		mouseLocation = pipeline->getConstantLocation("mouse");
		animLocation = pipeline->getConstantLocation("anim");
		lightsLocation = pipeline->getConstantLocation("lights");
		energyLocation = pipeline->getConstantLocation("energy");
	}

	float flakyEnergy(float energy) {
		int value = Random::get(0, energy * 20);
		return value == 0 ? 0 : energy;
	}
	
	void drawGUI() {
		g2->setColor(Graphics1::Color::Black);
		
		g2->fillRect(0, h - 100, w, 100);
		
		// Draw buttons
		g2->drawString(doorText, doorButton.x(), doorButton.y());
		g2->drawString(closetText, closetButton.x(), closetButton.y());
		
		// Show debug text for 50 frames
		g2->drawString(dText, debugText.x(), debugText.y());
		if (dText[0] != '\0') dTime ++;
		if (dTime > 50) {
			sprintf(dText, "");
			dTime = 0;
		}
		
		g2->setColor(Graphics1::Color::White);
	}
	
	bool goThroughDoor() {
		int tile = tileset->getTileID(px + playerWidth / 2, py + playerHeight / 2);
		if (tile == Tileset::Door) {
			vec2 door = tileset->findDoor();
			px = door.x() + 32;
			py = door.y() + 36;
			return true;
		}
		else {
			return false;
		}
	}
	
	bool hideInCloset() {
		int tile = tileset->getTileID(px + playerWidth / 2, py + playerHeight / 2);
		
		if (tile == Tileset::Closet) {
			if (inCloset) {
				sprintf(closetText, "2: Get out of the closet");
			} else {
				sprintf(closetText, "2: Hide in the closet");
			}
			
			inCloset = !inCloset;
			
			return true;
		} else {
			sprintf(dText, "There is no closet");
			log(Info, "There is no closet");
			return false;
		}
	}

	void update() {
		Audio2::update();

		static int anim = 0;
		++anim;

		energy -= 0.001f;
		if (energy < 0) energy = 0;

		//if (up) {
		//	py -= 1;
		//}
		//if (down_) {
		//	py += 1;
		//}
		if (!inCloset) {
			if (left) {
				px -= 4;
			}
			if (right) {
				px += 4;
			}
		}

		float targetCamX = Kore::max(0.0f, px - w / 2 + playerWidth / 2);
		float targetCamY = Kore::max(0.0f, py - h / 2 + playerHeight / 2);

		vec2 cam(camX, camY);
		vec2 target(targetCamX, targetCamY);

		vec2 dir = target - cam;
		if (dir.getLength() < 6.0f) {
			camX = targetCamX;
			camY = targetCamY;
		}
		else {
			dir.setLength(5.0f);
			cam = cam + dir;
			camX = cam.x();
			camY = cam.y();
		}

		Graphics4::begin();
		Graphics4::setRenderTarget(screen);
        g2->begin(true);
		
		vec2 lights[lightCount];
		for (int i = 0; i < lightCount; ++i) {
			lights[i] = vec2(-1000, -1000);
		}
		tileset->drawTiles(g2, camX, camY, lights);
		for (int i = 0; i < lightCount; ++i) {
			lights[i] = vec2(lights[i].x() / w, lights[i].y() / h);
		}

		frameCount++;
		if (frameCount > 10) {
			frameCount = 0;
			
			runIndex = runIndex % 8;
			runIndex++;
		}

		if (!inCloset) {
			if (left)
				g2->drawScaledSubImage(playerImage, (runIndex+1)*playerWidth, 0, -playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
			else if (right)
				g2->drawScaledSubImage(playerImage, runIndex*playerWidth, 0, playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
			else if (lastDirection == 0)
				g2->drawScaledSubImage(playerImage, playerWidth, 0, -playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
			else if (lastDirection == 1)
				g2->drawScaledSubImage(playerImage, 0, 0, playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
		}

        g2->end();
		
		Graphics4::restoreRenderTarget();
		g2->begin();
		g2->setPipeline(pipeline);
		Graphics4::setPipeline(pipeline);
		Graphics4::setFloat(aspectLocation, w / h);
		angle += 0.01f;
		if (angle > pi) angle = -pi;
		Graphics4::setFloat(angleLocation, angle);
		Graphics4::setFloat2(playerLocation, vec2((px - camX + playerWidth / 2.0f) / w, (py - camY + playerHeight / 2.0f) / h));
		Graphics4::setFloat2(mouseLocation, vec2(mx / w, my / h));
		Graphics4::setInt(animLocation, anim);
		Graphics4::setFloats(lightsLocation, (float*)lights, lightCount * 2);
		Graphics4::setFloat(energyLocation, flakyEnergy(energy));
		if (!inCloset) g2->drawImage(screen, 0, 0);
	//	g2->end();
		g2->setPipeline(nullptr);
		
	//	g2->begin();
		drawGUI();
		
		// Draw battery status
		if (energy > 0.8f)		g2->drawScaledSubImage(batteryImage, 0,   0, 32, 64, w-40, h-80, 32, 64);
		else if (energy > 0.6f) g2->drawScaledSubImage(batteryImage, 32,  0, 32, 64, w-40, h-80, 32, 64);
		else if (energy > 0.4f) g2->drawScaledSubImage(batteryImage, 64,  0, 32, 64, w-40, h-80, 32, 64);
		else if (energy > 0.2f) g2->drawScaledSubImage(batteryImage, 96,  0, 32, 64, w-40, h-80, 32, 64);
		else					g2->drawScaledSubImage(batteryImage, 128, 0, 32, 64, w-40, h-80, 32, 64);
		
		g2->end();

        Graphics4::end();
		Graphics4::swapBuffers();
	}

	void keyDown(KeyCode code) {
		switch (code) {
		case KeyLeft:
		case KeyA:
			left = true;
			lastDirection = 0;
			break;
		case KeyRight:
		case KeyD:
			right = true;
			lastDirection = 1;
			break;
		case KeyDown:
		case KeyS:
			down_ = true;
			break;
		case KeyUp:
		case KeyW:
			up = true;
			break;
		case Key1:
			goThroughDoor();
			break;
		case Key2:
			hideInCloset();
			break;
		default:
			break;
		}
	}

	void keyUp(KeyCode code) {
		switch (code) {
		case KeyLeft:
		case KeyA:
			left = false;
			break;
		case KeyRight:
		case KeyD:
			right = false;
			break;
		case KeyDown:
		case KeyS:
			down_ = false;
			break;
		case KeyUp:
		case KeyW:
			up = false;
			break;
		default:
			break;
		}
	}

	void mouseMove(int window, int x, int y, int moveX, int moveY) {
		mx = x;
		my = y;
	}
	
	void mousePress(int windowId, int button, int x, int y) {
		if (x > doorButton.x() && y > doorButton.y() && x < doorButton.x() + doorButton.z() && y < doorButton.y() + doorButton.w()) {
			log(Info, "door button pressed");
			goThroughDoor();
		}
		
		if (x > closetButton.x() && y > closetButton.y() && x < closetButton.x() + closetButton.z() && y < closetButton.y() + closetButton.w()) {
			log(Info, "closet button pressed");
			hideInCloset();
		}
		
	}
}

int kore(int argc, char** argv) {
	System::init("Power", w, h);
	
	tileset = new Tileset("Tiles/school.csv", "Tiles/school.png", rows, columns, tileWidth, tileHeight);
    
    g2 = new Graphics2::Graphics2(w, h, false);
	screen = new Graphics4::RenderTarget(w, h, 0);
	createPipeline();

    //Sound::init();
	Audio1::init();
	Audio2::init();
	Random::init(static_cast<int>(System::time() * 1000));

	Kore::System::setCallback(update);

	playerImage = new Graphics4::Texture("player.png");
	playerWidth = playerImage->width / 10.0f;
	playerHeight = playerImage->height / 2.0f;
	px = 0;
	py = tileHeight - playerImage->height/2;
	
	batteryImage = new Graphics4::Texture("Tiles/battery.png");
	
	font14 = Kravur::load("Fonts/arial", FontStyle(), 14);
	font24 = Kravur::load("Fonts/arial", FontStyle(), 24);
	font34 = Kravur::load("Fonts/arial", FontStyle(), 34);
	font44 = Kravur::load("Fonts/arial", FontStyle(), 44);
	
	g2->setFont(font24);
	g2->setFontColor(Graphics1::Color::White);
	g2->setFontSize(24);
	
	sprintf(doorText, "1: Go through the door");
	sprintf(closetText, "2: Hide in the closet");
	
	doorButton = vec4(10, h - 80, g2->getFont()->stringWidth(doorText), 20); // xPos, yPos, width, height
	closetButton = vec4(10, h - 50, g2->getFont()->stringWidth(closetText), 20);
	debugText = vec2(w / 2, h - 80);

	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->Move = mouseMove;
	Mouse::the()->Press = mousePress;

	Kore::System::start();
    
    System::stop();

	return 0;
}
