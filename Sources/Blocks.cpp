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

using namespace Kore;

namespace {
    Graphics2::Graphics2* g2;
    
	Graphics4::Texture* titleImage;
	Graphics4::Texture* boardImage;
	Graphics4::Texture* scoreImage;

	SoundStream* music;

	enum BlockColor {
		Blue, Green, Orange, Purple, Red, Violet, Yellow
	};

	Graphics4::Texture** blockImages;

	enum GameState {
		TitleState, InGameState, GameOverState
	};
	GameState state = TitleState;

	class Block;
	Block** blocked;

	class Block {
	public:
		const static int xsize = 12;
		const static int ysize = 23;

		Graphics4::Texture* image;
		vec2i pos;
		vec2i lastpos;

		Block(int xx, int yy, Graphics4::Texture* image) : image(image) {
			pos = vec2i(xx, yy);
			lastpos = vec2i(xx, yy);
		}

		void draw(Graphics2::Graphics2* g2) {
			//272x480
            if (image != nullptr) g2->drawImage(image, 16 + pos.x() * 16.0f, 400.0f - pos.y() * 16.0f);
		}

		int getX() {
			return pos.x();
		}

		int getY() {
			return pos.y();
		}

		vec2i getPos() {
			return pos;
		}

		bool right(int i = 1) {
			if (pos.x() + i < xsize && pos.y() < ysize && blocked[pos.y() * xsize + pos.x() + i] != nullptr) {
				return false;
			}
			lastpos = pos;
			pos.x() += i;
			return true;
		}

		bool left(int i = 1) {
			if (pos.x() - i < xsize && pos.y() < ysize && blocked[pos.y() * xsize + pos.x() - i] != nullptr) {
				return false;
			}
			lastpos = pos;
			pos.x() -= i;
			return true;
		}

		bool down(int i = 1) {
			if (pos.x() < xsize && pos.y() - i < ysize && blocked[(pos.y() - i) * xsize + pos.x()] != nullptr) {
				return false;
			}
			lastpos = pos;
			pos.y() -= i;
			return true;
		}

		bool up(int i = 1) {
			if (pos.x() < xsize && pos.y() + i < ysize && blocked[(pos.y() + i) * xsize + pos.x()] != nullptr) {
				return false;
			}
			lastpos = pos;
			pos.y() += i;
			return true;
		}

		bool rotate(vec2i center) {
			vec2i newpos(center.x() - (pos.y() - center.y()), center.y() + (pos.x() - center.x()));
			if (blocked[newpos.y() * xsize + newpos.x()] != nullptr) return false;
			lastpos = pos;
			pos = newpos;
			return true;
		}

		void back() {
			pos = lastpos;
		}
	};

	Sound* rotateSound;
	Sound* lineSound;
	Sound* klackSound;

	class BigBlock {
	public:
		static BigBlock* next;
		static BigBlock* current;

		vec2i center;
		Block** blocks;

		BigBlock(int xx, int yy) {
			center = vec2i(xx, yy);
			blocks = new Block*[4];
			for (int i = 0; i < 4; ++i) blocks[i] = nullptr;
		}

		void draw(Graphics2::Graphics2* g2) {
			for (int i = 0; i < 4; ++i) blocks[i]->draw(g2);
		}

		void right() {
			int i = 0;
			while (i < 4) {
				if (!blocks[i]->right()) goto retreat;
				++i;
			}
			++center.x();
			return;
		retreat:
			--i;
			while (i >= 0) {
				blocks[i]->back();
				--i;
			}
		}

		void left() {
			int i = 0;
			while (i < 4) {
				if (!blocks[i]->left()) goto retreat;
				++i;
			}
			--center.x();
			return;
		retreat:
			--i;
			while (i >= 0) {
				blocks[i]->back();
				--i;
			}
		}

		bool down() {
			int i = 0;
			while (i < 4) {
				if (!blocks[i]->down()) goto retreat;
				++i;
			}
			--center.y();
			return true;
		retreat:
			--i;
			while (i >= 0) {
				blocks[i]->back();
				--i;
			}
			return false;
		}

		Block* getBlock(int b) {
			return blocks[b];
		}

		virtual void rotate() {
			Audio1::play(rotateSound);
			int i = 0;
			while (i < 4) {
				if (!blocks[i]->rotate(center)) goto retreat;
				++i;
			}
			return;
		retreat:
			--i;
			while (i >= 0) {
				blocks[i]->back();
				--i;
			}
		}

		bool hop() {
			for (int i = 0; i < 4; ++i) {
				blocks[i]->up(8);
				blocks[i]->left(8);
				if (!blocks[i]->down(4)) return false;
			}
			center.x() -= 8;
			center.y() += 4;
			return true;
		}
	};

	class IBlock : public BigBlock {
	public:
		IBlock() : BigBlock(13, 17) {
			Graphics4::Texture* image = blockImages[Red];
			blocks[0] = new Block(13, 18, image); blocks[1] = new Block(13, 17, image);
			blocks[2] = new Block(13, 16, image); blocks[3] = new Block(13, 15, image);
		}
	};

	class OBlock : public BigBlock {
	public:
		OBlock() : BigBlock(13, 17) {
			Graphics4::Texture* image = blockImages[Orange];
			blocks[0] = new Block(12, 18, image); blocks[1] = new Block(12, 17, image);
			blocks[2] = new Block(13, 18, image); blocks[3] = new Block(13, 17, image);
		}

		void rotate() override {

		}
	};

	class LBlock : public BigBlock {
	public:
		LBlock() : BigBlock(12, 17) {
			Graphics4::Texture* image = blockImages[Blue];
			blocks[0] = new Block(12, 18, image); blocks[1] = new Block(12, 17, image);
			blocks[2] = new Block(12, 16, image); blocks[3] = new Block(13, 16, image);
		}
	};

	class JBlock : public BigBlock {
	public:
		JBlock() : BigBlock(12, 17) {
			Graphics4::Texture* image = blockImages[Yellow];
			blocks[0] = new Block(13, 18, image); blocks[1] = new Block(13, 17, image);
			blocks[2] = new Block(13, 16, image); blocks[3] = new Block(12, 16, image);
		}
	};

	class TBlock : public BigBlock {
	public:
		TBlock() : BigBlock(13, 17) {
			Graphics4::Texture* image = blockImages[Green];
			blocks[0] = new Block(12, 18, image); blocks[1] = new Block(13, 18, image);
			blocks[2] = new Block(14, 18, image); blocks[3] = new Block(13, 17, image);
		}
	};

	class ZBlock : public BigBlock {
	public:
		ZBlock() : BigBlock(13, 18) {
			Graphics4::Texture* image = blockImages[Purple];
			blocks[0] = new Block(12, 18, image); blocks[1] = new Block(13, 18, image);
			blocks[2] = new Block(13, 17, image); blocks[3] = new Block(14, 17, image);
		}
	};

	class SBlock : public BigBlock {
	public:
		SBlock() : BigBlock(13, 17) {
			Graphics4::Texture* image = blockImages[Violet];
			blocks[0] = new Block(12, 17, image); blocks[1] = new Block(13, 17, image);
			blocks[2] = new Block(13, 18, image); blocks[3] = new Block(14, 18, image);
		}
	};

	bool left = false;
	bool right = false;
	bool lastleft = false;
	bool lastright = false;
	bool down_ = false;
	bool button = false;
	BigBlock* current = nullptr;
	BigBlock* next = nullptr;
	int xcount = 0;
	double lastDownTime = 0;

	bool isBlocked(int x, int y) {
		return blocked[y * Block::xsize + x] != nullptr;
	}

	bool lineBlocked(int y) {
		return isBlocked(1, y) && isBlocked(2, y) && isBlocked(3, y) && isBlocked(4, y) && isBlocked(5, y) &&
			isBlocked(6, y) && isBlocked(7, y) && isBlocked(8, y) && isBlocked(9, y) && isBlocked(10, y);
	}

	void check() {
		bool lineDeleted = false;
		for (int i = 0; i < 4; ++i) {
			int y = 1;
			while (y < Block::ysize) {
				if (lineBlocked(y)) {
					lineDeleted = true;
					for (int x = 1; x < Block::xsize - 1; ++x) {
						blocked[y * Block::xsize + x] = nullptr;
					}
					y += 1;
					while (y < Block::ysize) {
						for (int x = 1; x < Block::xsize - 1; ++x) if (blocked[y * Block::xsize + x] != nullptr) {
							blocked[y * Block::xsize + x]->down();
							blocked[(y - 1) * Block::xsize + x] = blocked[y * Block::xsize + x];
							blocked[y * Block::xsize + x] = nullptr;
						}
						++y;
					}
				}
				++y;
			}
		}
		if (lineDeleted) Audio1::play(lineSound);
		else Audio1::play(klackSound);
	}

	BigBlock* createRandomBlock();

	void down() {
		if (!current->down()) {
			down_ = false;
			for (int i = 0; i < 4; ++i) {
				Block* block = current->getBlock(i);
				blocked[block->getY() * Block::xsize + block->getX()] = block;
			}
			current = next;
			next = createRandomBlock();
			check();
			if (!current->hop()) {
				state = GameOverState;
			}
		}
	}

	void update() {
		Audio2::update();
		if (state == InGameState) {
			lastleft = left;
			lastright = right;

			++xcount;
			if (right && !lastright) {
				current->right();
				xcount = 0;
			}
			if (left && !lastleft) {
				current->left();
				xcount = 0;
			}
			if (xcount % 4 == 0) {
				if (right && lastright) current->right();
				else if (left && lastleft) current->left();
			}
			if (button) {
				current->rotate();
				button = false;
			}
			if (down_) down();
			else {
				double time = System::time();
				//printf("now %f last %f\n", time, lastDownTime);
				if (time - lastDownTime > 1.0) {
					lastDownTime += 1.0;
					down();
				}
			}
		}

		Graphics4::begin();
        g2->begin();

		if (state == InGameState) {
            g2->drawImage(boardImage, 0, 0);
            
			for (int y = 0; y < Block::ysize; ++y) for (int x = 0; x < Block::xsize; ++x) {
				Block* block = blocked[y * Block::xsize + x];
				if (block != nullptr) {
					block->draw(g2);
				}
			}
			if (current != nullptr) current->draw(g2);
			if (next != nullptr) next->draw(g2);
        } else if (state == GameOverState) {
            g2->drawImage(scoreImage, 0, 0);
        }

        g2->end();
        Graphics4::end();
		Graphics4::swapBuffers();
	}

	BigBlock* createRandomBlock() {
		switch (Random::get(0, 6)) {
		case 0: return new IBlock();
		case 1: return new LBlock();
		case 2: return new JBlock();
		case 3: return new TBlock();
		case 4: return new ZBlock();
		case 5: return new SBlock();
		case 6: return new OBlock();
		}
		return nullptr;
	}

	void startGame() {
		blocked = new Block*[Block::ysize * Block::xsize];
		for (int y = 0; y < Block::ysize; ++y) for (int x = 0; x < Block::xsize; ++x) blocked[y * Block::xsize + x] = nullptr;
		for (int y = 0; y < Block::ysize; ++y) blocked[y * Block::xsize + 0] = new Block(0, y, nullptr);
		for (int y = 0; y < Block::ysize; ++y) blocked[y * Block::xsize + Block::xsize - 1] = new Block(Block::xsize - 1, y, nullptr);
		for (int x = 0; x < Block::xsize; ++x) blocked[x] = new Block(x, 0, nullptr);

		current = createRandomBlock();
		current->hop();
		next = createRandomBlock();

		lastDownTime = System::time();
		state = InGameState;
	}

	void keyDown(KeyCode code) {
		switch (state) {
		case TitleState:
			startGame();
			break;
		case InGameState:
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
				button = true;
				break;
            default:
				break;
			}
			break;
        default:
            break;
		}
	}

	void keyUp(KeyCode code) {
		switch (state) {
		case InGameState:
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
				button = false;
				break;
            default:
                break;
			}
        default:
            break;
		}
	}

	void mouseUp(int windowId, int button, int x, int y) {
		switch (state) {
		case TitleState:
			startGame();
			break;
        default:
            break;
		}
	}
    
    void gamepadAxis(int axis, float value) {
        if (axis == 0 || axis == 2) {
            if (value < -0.1) {
                left = true;
                right = false;
            }
            else if (value > 0.1) {
                right = true;
                left = false;
            }
            else {
                left = false;
                right = false;
            }
        }
        if (axis == 1 || axis == 3) {
            if (value < -0.1) {
                down_ = true;
            }
            else {
                down_ = false;
            }
        }
    }
    
    void gamepadButton(int buttonNr, float value) {
        if (value > 0.1) {
            button = true;
        }
        else {
            button = false;
        }
    }
}

int kore(int argc, char** argv) {
    int w = 272;
    int h = 480;

	System::init("Power", w, h);
    
    g2 = new Graphics2::Graphics2(w, h);

    //Sound::init();
	Audio1::init();
	Audio2::init();
	Random::init(static_cast<int>(System::time() * 1000));

	Kore::System::setCallback(update);

	music = new SoundStream("Sound/blocks.ogg", true);
    rotateSound = new Sound("Sound/rotate.wav");
	lineSound = new Sound("Sound/line.wav");
	klackSound = new Sound("Sound/klack.wav");

	titleImage = new Graphics4::Texture("Graphics/title.png");
	boardImage = new Graphics4::Texture("Graphics/board.png");
	scoreImage = new Graphics4::Texture("Graphics/score.png");

	blockImages = new Graphics4::Texture*[7];
	//Blue, Green, Orange, Purple, Red, Violet, Yellow
	blockImages[Blue] = new Graphics4::Texture("Graphics/block_blue.png");
	blockImages[Green] = new Graphics4::Texture("Graphics/block_green.png");
	blockImages[Orange] = new Graphics4::Texture("Graphics/block_orange.png");
	blockImages[Purple] = new Graphics4::Texture("Graphics/block_purple.png");
	blockImages[Red] = new Graphics4::Texture("Graphics/block_red.png");
	blockImages[Violet] = new Graphics4::Texture("Graphics/block_violet.png");
	blockImages[Yellow] = new Graphics4::Texture("Graphics/block_yellow.png");


	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->Release = mouseUp;
    Gamepad::get(0)->Axis = gamepadAxis;
    Gamepad::get(0)->Button = gamepadButton;

	Audio1::play(music);

	startGame();

	lastDownTime = System::time();
	Kore::System::start();
    
    System::stop();

	return 0;
}
