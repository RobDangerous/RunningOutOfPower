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
	
	const int w = 40 * 32 + 1;
	const int h = 40 * 32 + 1;

    Graphics2::Graphics2* g2;
	
	Graphics4::Texture* playerImage;

	Graphics4::RenderTarget* screen;
	Graphics4::PipelineState* pipeline;
	Graphics4::ConstantLocation aspectLocation;
	Graphics4::ConstantLocation angleLocation;
	Graphics4::ConstantLocation playerLocation;
	Graphics4::ConstantLocation mouseLocation;

	float angle = 0.0f;

	float px = 100.0f;
	float py = 200.0f;
	float mx = 0.0f;
	float my = 0.0f;

	bool left = false;
	bool right = false;
	bool down_ = false;
	bool up = false;

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
	}

	void update() {
		Audio2::update();

		if (up) {
			py -= 1;
		}
		if (down_) {
			py += 1;
		}
		if (left) {
			px -= 1;
		}
		if (right) {
			px += 1;
		}

		float playerWidth = playerImage->width / 10.0f;
		float playerHeight = playerImage->height / 2.0f;

		Graphics4::begin();
		Graphics4::setRenderTarget(screen);
        g2->begin(true);
		
		tileset->drawTiles(g2);

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
		g2->drawImage(screen, 0, 0);
		g2->end();
		g2->setPipeline(nullptr);

        Graphics4::end();
		Graphics4::swapBuffers();
	}

	void keyDown(KeyCode code) {
		switch (code) {
		case KeyLeft:
			left = true;
			break;
		case KeyRight:
			right = true;
			break;
		case KeyDown:
			down_ = true;
			break;
		case KeyUp:
			up = true;
			break;
		default:
			break;
		}
	}

	void keyUp(KeyCode code) {
		switch (code) {
		case KeyLeft:
			left = false;
			break;
		case KeyRight:
			right = false;
			break;
		case KeyDown:
			down_ = false;
			break;
		case KeyUp:
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
	
	tileset = new Tileset("Tiles/desert.csv", "Tiles/tmw_desert_spacing.png", 40, 40, 32, 32);
    
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
