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

#include "Monster.h"
#include "Tileset.h"

using namespace Kore;

namespace {
	const int w = 1280 / 2;
	const int h = 720 / 2;
	
	const double fps = 1.l / 60.l;

	double startTime;
	double lastTime;

	float playerWidth;
	float playerHeight;

	float camX = 0;
	float camY = 0;

    Graphics2::Graphics2* g2;
	
	Graphics4::Texture* intro;
	Graphics4::Texture* playerImage;
	Graphics4::Texture* playerChargeImage;
	Graphics4::Texture* playerDoorImage;

	Graphics4::Texture* batteryImage;
	
	Graphics4::Texture* fightImage;
	
	Graphics4::Texture* spiderAnimImage;

	Graphics4::RenderTarget* screen;
	Graphics4::PipelineState* pipeline;
	Graphics4::ConstantLocation aspectLocation;
	Graphics4::ConstantLocation angleLocation;
	Graphics4::ConstantLocation playerLocation;
	Graphics4::ConstantLocation mouseLocation;
	Graphics4::ConstantLocation animLocation;
	Graphics4::ConstantLocation redLocation;
	Graphics4::ConstantLocation lightsLocation;
	Graphics4::ConstantLocation energyLocation;
	Graphics4::ConstantLocation topLocation;
	Graphics4::ConstantLocation bottomLocation;
	Graphics4::ConstantLocation lightOnLocation;

	bool dead = false;
	float energy = 1.0f;
	
	float px = 0;
	float py = 0;
	float mx = 0.0f;
	float my = 0.0f;

	int lastDirection = 1;	// 0 - left, 1 - right
	bool left = false;
	bool right = false;
	bool up = false;
	
	bool charging = false;
	bool inCloset = false;
	bool takeDoor = false;
	bool doorAnim = false;

	const char* helpText;
	const char* const chargeText = "Hold space to charge";
	const char* const doorText = "Key Up: Go through the door";
	const char* const closetInText = "Key Up: Hide in the closet";
	const char* const closetOutText = "Key Up: Get out of the closet";
	const char* const switchLighOnText = "Key Up: Switch the light on";
	const char* const switchLighOffText = "Key Down: Switch the light off";
	const char* const skipText = "Skip";
	const char* const resetText = "Press R to Restart";
	
	int frameCount = 0;
	int anim = 0;
	float red = 0.f;
	
	int runIndex = 0;
	int chargeIndex = 0;
	int doorIndex = 0;
	int fightIndex = 0;
	
	Kravur* font14;
	Kravur* font24;
	Kravur* font34;
	Kravur* font44;
	
	char dText[42];
	float dTime = 0;
		
	vec4 doorButton;
	vec4 closetButton;
	vec2 debugText;
	vec4 skipButton;
	
	const int monsterCount = 1;
	Monster monsters[monsterCount];
	
	bool lightOn = false;

	enum State {
		Start,
		Game,
		End
	} state;

	void reset()
	{
		camX = 0;
		camY = 0;

		dead = false;
		energy = 1.0;

		px = 0;
		py = tileHeight - playerImage->height / 2;
		//mx = 0.0f;
		//my = 0.0f;

		lastDirection = 1;
		left = false;
		right = false;
		up = false;

		charging = false;
		inCloset = false;
		takeDoor = false;
		doorAnim = false;

		frameCount = 0;
		anim = 0;
		red = 0.f;

		runIndex = 0;
		chargeIndex = 0;
		doorIndex = 0;
		fightIndex = 0;

		sprintf(dText, "");
		dTime = 0;

		lightOn = false;

		state = Start;

		for (int i = 0; i < monsterCount; ++i) {
			monsters[i].reset();
		}
		resetSpiders();
	}
	
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
		redLocation = pipeline->getConstantLocation("red");
		lightsLocation = pipeline->getConstantLocation("lights");
		energyLocation = pipeline->getConstantLocation("energy");
		topLocation = pipeline->getConstantLocation("top");
		bottomLocation = pipeline->getConstantLocation("bottom");
		lightOnLocation = pipeline->getConstantLocation("lightOn");
	}

	float flakyEnergy(float energy) {
		int value = Random::get(0, energy * 20);
		return value == 0 ? 0 : energy;
	}
	
	void drawGUI() {
		g2->setColor(0x77000000);
		
		g2->fillRect(0, h * 2 - 100, w * 2, 100);
		
		if (helpText != nullptr) {
			g2->drawString(helpText, closetButton.x(), closetButton.y());
		}

		// Show debug text for 50 frames
		g2->drawString(dText, debugText.x(), debugText.y());
		if (dText[0] != '\0') dTime ++;
		if (dTime > 50) {
			sprintf(dText, "");
			dTime = 0;
		}
		
		g2->setColor(Graphics1::Color::White);
	}
	
	bool goThroughTheDoor() {
		int tile = getTileID(px + playerWidth / 2, py + playerHeight / 2);
		if (tile == Door && !doorAnim && !takeDoor) {
			doorIndex = 0;
			doorAnim = true;
			return false;
		}
		else if(tile == Door && doorAnim && takeDoor) {
			doorAnim = false;
			takeDoor = false;
			vec2 door = findDoor(px + playerWidth / 2, py + playerHeight / 2);
			px = door.x() + 32;
			py = door.y() + 36;
			return true;
		}
		else {
			return false;
		}
	}
	
	bool hideInTheCloset() {
		int tile = getTileID(px + playerWidth / 2, py + playerHeight / 2);
		
		if (tile == Closet) {
			inCloset = !inCloset;
			return true;
		} else {
			return false;
		}
	}
	
	bool switchTheLightOn() {
		int tile = getTileID(px + playerWidth / 2, py + playerHeight / 2);
		
		if (tile == LightSwitch) {
			lightOn = !lightOn;
			
			if (lightOn) {
				log(Info, "WON!");
				state = End;
			}
			return true;
		} else {
			return false;
		}
	}

	void update() {
		double t = System::time() - startTime;
		double deltaT = t - lastTime;

		if (deltaT <= fps)
			return;

		lastTime = t;

		Audio2::update();

		float flxoff = 0;
		float flyoff = 0;
		++frameCount;
		++anim;

		if (!dead && state == Game)
		{
			helpText = nullptr;

			if (charging) {
				energy += 0.002f;
				if (energy > 1) energy = 1;
			}
			else {
				energy -= 0.0005f;
				if (energy < 0.2f) {
					helpText = chargeText;
				}
				if (energy < 0) energy = 0;
			}
		
			// Draw buttons
			int tile = getTileID(px + playerWidth / 2, py + playerHeight / 2);
			if (tile == Door) {
				helpText = doorText;
			}
			else if (tile == Closet) {
				if (inCloset) {
					helpText = closetOutText;
				}
				else {
					helpText = closetInText;
				}
			} else if (tile == LightSwitch) {
				if (lightOn)
					helpText = switchLighOffText;
				else
					helpText = switchLighOnText;
			}

			if (!inCloset) {
				if (left && px >= -10) {
					px -= 4;
				}
				if (right && px <= columns * tileWidth - 70) {
					px += 4;
				}
			}

			float targetCamX = Kore::min(Kore::max(0.0f, px - w / 2 + playerWidth / 2), 1.f * columns * tileWidth - w);
			float targetCamY = Kore::min(Kore::max(0.0f, py - h / 2 + playerHeight / 2), 1.f * rows * tileHeight - h);

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

			// Flashlight position
			if (!inCloset) {
				if (charging) {
					my = py - camY + playerHeight / 2;
					if (left || lastDirection == 0) {
						flxoff = -10;
						flyoff = 0;
						mx = px - camX - 100;
					}
					else {
						flxoff = 10;
						flyoff = 0;
						mx = px - camX + 100;
					}
				}
				else if (doorAnim) {
					if (left || lastDirection == 0) {
						mx = px + playerWidth / 2 - camX - 100;
					}
					else {
						mx = px + playerWidth / 2 - camX + 100;
					}
				}
				else {
					float angle = Kore::atan2(my - (py + playerHeight / 2 - camY), mx - (px + playerWidth / 2 - camX));
					if (left || lastDirection == 0) {
						vec3 c(20, 0, 0);
						vec3 d(-30, 0, 0);
						vec3 r = c + mat3::RotationZ(angle + pi) * d;
						flxoff = r.x();
						flyoff = r.y();
					}
					else {
						vec3 c(-20, 0, 0);
						vec3 d(30, 0, 0);
						vec3 r = c + mat3::RotationZ(angle) * d;
						flxoff = r.x();
						flyoff = r.y();
					}
				}
			}

			dead = animateSpider(px + playerWidth / 2, py + playerHeight / 2, px + playerWidth / 2 + flxoff, py + playerHeight / 2 + flyoff, mx, my, camX, camY, energy);

			for (int i = 0; i < monsterCount; ++i) {
				//if (Kore::abs(px - monsters[i].x) < 100 && mx > px) {

				//}
				dead |= (monsters[i].update(px + playerWidth / 2, py + playerHeight / 2, px + playerWidth / 2 + flxoff, py + playerHeight / 2 + flyoff, mx, my, camX, camY, energy) && !inCloset);
			}

			if (frameCount > 10) {
				frameCount = 0;

				runIndex = runIndex % 8;
				runIndex++;

				++chargeIndex;
				chargeIndex %= 4;

				++doorIndex;
				if (doorIndex >= 6) {
					if (doorAnim) {
						takeDoor = true;
						goThroughTheDoor();
					}
				}
				doorIndex %= 6;
			}
		}
		else {
			helpText = resetText;
		}

		Graphics4::begin();
		Graphics4::setRenderTarget(screen);
		g2->begin(true, w, h);

		vec2 lights[lightCount];
		for (int i = 0; i < lightCount; ++i) {
			lights[i] = vec2(-1000, -1000);
		}
		drawTiles(g2, camX, camY, lights);
		for (int i = 0; i < lightCount; ++i) {
			lights[i] = vec2(lights[i].x() / w, lights[i].y() / h);
		}

		if (!inCloset && !dead && state == Game) {
			if (charging) {
				if (left || lastDirection == 0) {
					g2->drawScaledSubImage(playerChargeImage, (chargeIndex + 1) * playerWidth, 0, -playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
					g2->drawScaledSubImage(playerChargeImage, playerWidth * 2, playerHeight, -playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
				}
				else {
					g2->drawScaledSubImage(playerChargeImage, chargeIndex * playerWidth, 0, playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
					g2->drawScaledSubImage(playerChargeImage, playerWidth, playerHeight, playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
				}
			}
			else if(doorAnim) {
				if (left || lastDirection == 0) {
					g2->drawScaledSubImage(playerDoorImage, (doorIndex + 1) * playerWidth, 0, -playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
					g2->drawScaledSubImage(playerDoorImage, (doorIndex + 1) * playerWidth * 2, playerHeight, -playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
				}
				else {
					g2->drawScaledSubImage(playerDoorImage, doorIndex * playerWidth, 0, playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
					g2->drawScaledSubImage(playerDoorImage, doorIndex * playerWidth, playerHeight, playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
				}
			}
			else {
				if (left)
					g2->drawScaledSubImage(playerImage, (runIndex + 1)*playerWidth, 0, -playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
				else if (right)
					g2->drawScaledSubImage(playerImage, runIndex*playerWidth, 0, playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
				else if (lastDirection == 0)
					g2->drawScaledSubImage(playerImage, playerWidth, 0, -playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
				else if (lastDirection == 1)
					g2->drawScaledSubImage(playerImage, 0, 0, playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);

				float angle = Kore::atan2(my - (py + playerHeight / 2 - camY), mx - (px + playerWidth / 2 - camX));
				mat3 m = mat3::Identity();
				if (left || lastDirection == 0) {
					if (Kore::abs(angle) < Kore::pi * 0.5f)
						m[1][1] = -1;
					float xoff = 60;
					float yoff = 60;

					g2->transformation = mat3::Translation(px - camX + xoff, py - camY + yoff) * mat3::RotationZ(angle + pi) * m * mat3::Translation(-xoff, -yoff);
					g2->drawScaledSubImage(playerImage, 10 * playerWidth, 0, -playerWidth, playerHeight, 0, 0, playerWidth, playerHeight);
				}
				else {
					if (Kore::abs(angle) > Kore::pi * 0.5f)
						m[1][1] = -1;
					float xoff = 20;
					float yoff = 60;

					g2->transformation = mat3::Translation(px - camX + xoff, py - camY + yoff) * mat3::RotationZ(angle) * m * mat3::Translation(-xoff, -yoff);
					g2->drawScaledSubImage(playerImage, 9 * playerWidth, 0, playerWidth, playerHeight, 0, 0, playerWidth, playerHeight);
				}
				g2->transformation = mat3::Identity();
			}
		}
		
		for (int i = 0; i < monsterCount; ++i) {
			monsters[i].render(g2, camX, camY);
		}
		
		if (state == Start) {
			g2->drawImage(intro, 0, 0);
		}

		if (dead) {
			energy = 0;
			
			int tile = getTileID(px + playerWidth / 2, py + playerHeight / 2);
			if (tile >= Spider1 && tile < Spider9) {
				if (fightIndex < 6) fightIndex = frameCount / 10;
				//g2->drawScaledSubImage(spiderAnimImage, fightIndex * tileWidth, 0, tileWidth, tileHeight, px - camX - 20, getFloor(py) * tileHeight - camY, tileWidth, tileHeight);
				g2->drawScaledSubImage(spiderAnimImage, fightIndex * playerWidth, 0, -playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
				g2->drawScaledSubImage(spiderAnimImage, fightIndex * playerWidth * 2, playerHeight, -playerWidth, playerHeight, px - camX, py - camY, playerWidth, playerHeight);
			} else {
				fightIndex = (frameCount / 10) % 4;
				g2->drawScaledSubImage(fightImage, fightIndex * tileWidth, 0, tileWidth, tileHeight, px - camX - 20, getFloor(py) * tileHeight - camY, tileWidth, tileHeight);
			}
		}
		
		g2->end();
		
		Graphics4::restoreRenderTarget();
		g2->begin(false, w * 2, h * 2);
		g2->setPipeline(pipeline);
		Graphics4::setPipeline(pipeline);
		Graphics4::setFloat(aspectLocation, (float)w / (float)h);
		vec2 mouse(mx / w, my / h);
		vec2 player((px - camX + playerWidth / 2.0f) / w, (py - camY + playerHeight / 2.0f) / h);
		if (state == Start) {
			Graphics4::setFloat(angleLocation, 0.0f);
			Graphics4::setFloat4(playerLocation, vec4(10.76f, 10.22f, 0.76f, 0.145f));
		}
		else {
#ifdef KORE_DIRECT3D
			Graphics4::setFloat(angleLocation, Kore::atan2(mouse.x() - player.x(), mouse.y() - player.y()));
#else
			Graphics4::setFloat(angleLocation, Kore::atan2(mouse.y() - player.y(), mouse.x() - player.x()) - pi / 2.0f);
#endif
			Graphics4::setFloat4(playerLocation, vec4(player.x(), player.y(),
				(px - camX + playerWidth / 2.0f + flxoff) / w, (py - camY + playerHeight / 2.0f + flyoff) / h));
		}
		Graphics4::setFloat2(mouseLocation, mouse);
		Graphics4::setInt(animLocation, anim);
		Graphics4::setFloat(redLocation, red);
#ifdef KORE_DIRECT3D
		vec4 vec4lights[lightCount];
		for (int i = 0; i < lightCount; ++i) {
			vec4lights[i].x() = lights[i].x();
			vec4lights[i].y() = lights[i].y();
		}
		Graphics4::setFloats(lightsLocation, (float*)vec4lights, lightCount * 4);
#else
		Graphics4::setFloats(lightsLocation, (float*)lights, lightCount * 2);
#endif
		if (state == Game) {
			Graphics4::setFloat(energyLocation, flakyEnergy(energy));
			Graphics4::setFloat(topLocation, (py - camY - 32) / h);
			Graphics4::setFloat(bottomLocation, (py - camY + 128) / h);
			Graphics4::setBool(lightOnLocation, lightOn);
		}
		else {
			if (anim - 60 * 4 > 600.0f) {
				state = Game;
				energy = 1;
			}
			else {
				Graphics4::setFloat(energyLocation, flakyEnergy(0.6f - (anim - 60 * 4) / 1000.0f));
				Graphics4::setFloat(topLocation, 0);
				Graphics4::setFloat(bottomLocation, 1);
				Graphics4::setBool(lightOnLocation, anim < 60 * 4);
			}
		}
		if (!inCloset) g2->drawScaledSubImage(screen, 0, 0, w, h, 0, 0, w * 2, h * 2);
	//	g2->end();
		g2->setPipeline(nullptr);
		
	//	g2->begin();
		if (state == Game) {
			drawGUI();

			// Draw battery status
			if (energy > 0.8f)		g2->drawScaledSubImage(batteryImage, 0, 0, 32, 64, w * 2 - 40, h * 2 - 80, 32, 64);
			else if (energy > 0.6f) g2->drawScaledSubImage(batteryImage, 32, 0, 32, 64, w * 2 - 40, h * 2 - 80, 32, 64);
			else if (energy > 0.4f) g2->drawScaledSubImage(batteryImage, 64, 0, 32, 64, w * 2 - 40, h * 2 - 80, 32, 64);
			else if (energy > 0.2f) g2->drawScaledSubImage(batteryImage, 96, 0, 32, 64, w * 2 - 40, h * 2 - 80, 32, 64);
			else					g2->drawScaledSubImage(batteryImage, 128, 0, 32, 64, w * 2 - 40, h * 2 - 80, 32, 64);
		} else if (state == Start) {
			g2->setFontColor(Graphics1::Color::White);
			g2->drawString(skipText, skipButton.x(), skipButton.y());
		}
		
		g2->end();

        Graphics4::end();
		Graphics4::swapBuffers();
	}

	void keyDown(KeyCode code) {		
		if (dead) return;

		charging = false;
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
		case KeyUp:
		case KeyW:
			up = true;
			takeDoor = false;
			goThroughTheDoor();
			hideInTheCloset();
			switchTheLightOn();
			break;
		case KeySpace:
			left = false;
			right = false;
			charging = true;
			break;
		default:
			break;
		}
	}

	void keyUp(KeyCode code) {
		if (code == KeyR && state != Start) reset();
		if (code == KeyEscape && state == Start) state = Game;
		if (dead) return;

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
		case KeyUp:
		case KeyW:
			up = false;
			break;
		case KeySpace:
			charging = false;
			break;
		default:
			break;
		}
	}

	void mouseMove(int window, int x, int y, int moveX, int moveY) {
		if (dead) return;

		mx = x / 2.0f;
		my = y / 2.0f;
	}
	
	void mousePress(int windowId, int button, int x, int y) {
		if (dead) return;

		if (x > doorButton.x() && y > doorButton.y() && x < doorButton.x() + doorButton.z() && y < doorButton.y() + doorButton.w()) {
			log(Info, "door button pressed");
			goThroughTheDoor();
		}
		
		if (x > closetButton.x() && y > closetButton.y() && x < closetButton.x() + closetButton.z() && y < closetButton.y() + closetButton.w()) {
			log(Info, "closet button pressed");
			hideInTheCloset();
		}
		
		if (x > skipButton.x() && y > skipButton.y() && x < skipButton.x() + skipButton.z() && y < skipButton.y() + skipButton.w()) {
			log(Info, "skip button pressed");
			state = Game;
		}
		
	}
}

int kore(int argc, char** argv) {
	System::init("Power", w * 2, h * 2);
	startTime = System::time();
	
	initTiles("Tiles/school.csv", "Tiles/school.png");
    
    g2 = new Graphics2::Graphics2(w, h, false);
	screen = new Graphics4::RenderTarget(w, h, 0);
	createPipeline();

    //Sound::init();
	Audio1::init();
	Audio2::init();
	Random::init(static_cast<int>(System::time() * 1000));

	Kore::System::setCallback(update);

	intro = new Graphics4::Texture("schoolClass.png");
	playerImage = new Graphics4::Texture("player.png");
	playerChargeImage = new Graphics4::Texture("playerRechargeAnim.png");
	playerDoorImage = new Graphics4::Texture("playerDoorAnim.png");
	playerWidth = playerImage->width / 10.0f;
	playerHeight = playerImage->height / 2.0f;
	
	Monster::init();
	reset();
	
	batteryImage = new Graphics4::Texture("Tiles/battery.png");
	
	fightImage = new Graphics4::Texture("Tiles/fight.png");
	spiderAnimImage = new Graphics4::Texture("playerSpiderAnim.png");

	SoundStream* music = new SoundStream("loop.ogg", true);
	Audio1::play(music);
	
	font14 = Kravur::load("Fonts/arial", FontStyle(), 14);
	font24 = Kravur::load("Fonts/arial", FontStyle(), 24);
	font34 = Kravur::load("Fonts/arial", FontStyle(), 34);
	font44 = Kravur::load("Fonts/arial", FontStyle(), 44);
	
	g2->setFont(font24);
	g2->setFontColor(Graphics1::Color::White);
	g2->setFontSize(24);
	
	doorButton = vec4(10, h * 2 - 80, g2->getFont()->stringWidth(doorText), 20); // xPos, yPos, width, height
	closetButton = vec4(10, h * 2 - 50, g2->getFont()->stringWidth(closetInText), 20);
	debugText = vec2(w * 2 / 2, h * 2 - 80);
	
	skipButton = vec4(w * 2 - g2->getFont()->stringWidth(skipText) - 10, 10, g2->getFont()->stringWidth(skipText), 20);

	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->Move = mouseMove;
	Mouse::the()->Press = mousePress;

	Kore::System::start();
    
    System::stop();

	return 0;
}
