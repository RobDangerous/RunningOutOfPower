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

#include "Tileset.h"

using namespace Kore;

namespace {
	Tileset* tileset;
	
	const int tileWidthHeight = 128;
	const int rows = 1;
	const int columns = 50;
	const int w = 768;// columns * tileWidthHeight;
	const int h = 768;// rows * tileWidthHeight;

    Graphics2::Graphics2* g2;
	
	Graphics4::Texture* playerImage;

	Graphics4::RenderTarget* screen;
	Graphics4::PipelineState* pipeline;
	Graphics4::ConstantLocation aspectLocation;
	Graphics4::ConstantLocation angleLocation;
	Graphics4::ConstantLocation playerLocation;
	Graphics4::ConstantLocation mouseLocation;
	Graphics4::ConstantLocation animLocation;

	float angle = 0.0f;

	float px = 0;
	float py = 0;
	float mx = 0.0f;
	float my = 0.0f;

	int lastDirection = 0;	// 0 - left, 1 - right
	bool left = false;
	bool right = false;
	bool down_ = false;
	bool up = false;
	
	int frameCount = 0;
	
	int runIndex = 0;

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
	}

	void update() {
		Audio2::update();

		static int anim = 0;
		++anim;

		if (up) {
			py -= 1;
		}
		if (down_) {
			py += 1;
		}
		if (left) {
			px -= 4;
		}
		if (right) {
			px += 4;
		}

		float playerWidth = playerImage->width / 10.0f;
		float playerHeight = playerImage->height / 2.0f;

		Graphics4::begin();
		Graphics4::setRenderTarget(screen);
        g2->begin(true);
		
		tileset->drawTiles(g2);

		frameCount++;
		if (frameCount > 10) {
			frameCount = 0;
			
			runIndex = runIndex % 8;
			runIndex++;
		}
		if (left)
			g2->drawScaledSubImage(playerImage, runIndex*playerWidth, playerHeight, playerWidth, playerHeight, px, py, playerWidth, playerHeight);
		else if (right)
			g2->drawScaledSubImage(playerImage, runIndex*playerWidth, 0, playerWidth, playerHeight, px, py, playerWidth, playerHeight);
		else if (lastDirection == 0)
			g2->drawScaledSubImage(playerImage, 0, playerHeight, playerWidth, playerHeight, px, py, playerWidth, playerHeight);
		else if (lastDirection == 1)
			g2->drawScaledSubImage(playerImage, 0, 0, playerWidth, playerHeight, px, py, playerWidth, playerHeight);

        g2->end();
		
		Graphics4::restoreRenderTarget();
		g2->begin();
		g2->setPipeline(pipeline);
		Graphics4::setPipeline(pipeline);
		Graphics4::setFloat(aspectLocation, w / h);
		angle += 0.01f;
		if (angle > pi) angle = -pi;
		Graphics4::setFloat(angleLocation, angle);
		Graphics4::setFloat2(playerLocation, vec2((px + playerWidth / 2.0f) / w, (py + playerHeight / 2.0f) / h));
		Graphics4::setFloat2(mouseLocation, vec2(mx / w, my / h));
		Graphics4::setInt(animLocation, anim);
		g2->drawImage(screen, 0, 0);
		g2->end();
		g2->setPipeline(nullptr);

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
}

int kore(int argc, char** argv) {
	System::init("Power", w, h);
	
	tileset = new Tileset("Tiles/school.csv", "Tiles/school.png", rows, columns, tileWidthHeight, tileWidthHeight);
    
    g2 = new Graphics2::Graphics2(w, h, false);
	screen = new Graphics4::RenderTarget(w, h, 0);
	createPipeline();

    //Sound::init();
	Audio1::init();
	Audio2::init();
	Random::init(static_cast<int>(System::time() * 1000));

	Kore::System::setCallback(update);

	playerImage = new Graphics4::Texture("player.png");

	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->Move = mouseMove;

	Kore::System::start();
    
    System::stop();

	return 0;
}
