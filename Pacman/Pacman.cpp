#include "Pacman.h"

#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <random>
#include <chrono>

Pacman::Pacman(int argc, char* argv[], int level) : Game(argc, argv), _cPacmanFrameTime(250), _cPickupFrameTime(1000), _cPacmanSpeed(4.0f)
{
	//Pacman struct
	_pacman = new Player();
	_pacman->currentFrameTime = 0;
	_pacman->direction = 2;
	_pacman->frame = 0;
	_pacman->dead = false;
	_pacman->lives = 3;
	_pacman->changeDirection = true;
	score = 0;

	levelSelection = level;

	//Initialise cherries
	_cherry = new Pickup();
	_cherry->texture = new Texture2D();
	_cherry->position = new Vector2(-100, 100);
	_cherry->sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	//Menu Struct
	_menu = new Menu();
	_menu->paused = false;
	_menu->startMenu = TRUE;
	_menu->pKeyDown = false;

	//Initialise sound
	_pop = new SoundEffect();
	_pacDeath = new SoundEffect();
	_pacIntro = new SoundEffect();

	//Initialise important Game aspects
	Audio::Initialise();
	Graphics::Initialise(argc, argv, this, 1024, 1024, false, 24, 24, "Pacman", 60);
	Input::Initialise();

	//Plays startup sound
	Audio::Play(_pacIntro);

	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();

}

Pacman::~Pacman()
{
	//Clean up pointers within the pacman structrue
	delete _pacman->texture;
	delete _pacman->sourceRect;
	delete _pacman->position;

	//Cleanup pointers to pickup structure
	delete _cherry->texture;
	delete _cherry->sourceRect;
	delete _cherry->position;

	//Sound cleanup
	delete _pop;
	delete _pacDeath;
	delete _pacIntro;

	//Clean up the structure pointers
	delete _pacman;

	delete _menu;
	delete _cherry;


	for (int i = 0; i < _ghosts.size(); i++)
	{
		delete _ghosts.at(i);
	}

	for (int i = 0; i < _munchies.size(); i++)
	{
		delete _munchies.at(i);
	}
	delete _tiles;


}

void Pacman::LoadContent()
{
	//Set Menu Paramters
	_menu->background = new Texture2D();
	_menu->background->Load("Textures/Transparency.png", false);
	_menu->rectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_menu->stringPosition = new Vector2(Graphics::GetViewportWidth() / 2.5f, Graphics::GetViewportHeight() / 2.0f);

	_menu->scorePosition = new Vector2(Graphics::GetViewportWidth() - 100.0f, 25.0f);
	_menu->livesPosition = new Vector2(10.0f, Graphics::GetViewportHeight() - 10.0f);
	_menu->timePosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, 25.0f);

	//LoadLevel data from file
	LoadTiles();

	//Load sounds
	_pop->Load("Sounds/pop.wav");
	_pacDeath->Load("Sounds/pacDeath.wav");
	_pacIntro->Load("Sounds/pacIntro.wav");

	// Set string position
	_stringPosition = new Vector2(10.0f, 25.0f);
}

void Pacman::Update(int elapsedTime)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();

	//Gets the current state of the mouse
	Input::MouseState* mouseState = Input::Mouse::GetState();

	//Check Paused
	CheckPaused(keyboardState, Input::Keys::P, Input::Keys::SPACE);

	_pacman->dead = CheckDead();
	win = CheckWin();
	if (!_menu->paused && !_menu->startMenu && !_menu->lostLife && !win)
	{
		if (!_pacman->dead)
		{
			Input(elapsedTime, keyboardState, mouseState);

			//This is a temp just in case i decide i want to have multiple cherries
			UpdateCherry(*_cherry, elapsedTime);

			for (int i = 0; i < _munchies.size(); i++)
			{
				UpdateMunchies(i, elapsedTime);
			}

			for (int i = 0; i < _ghosts.size(); i++)
			{
				UpdateGhosts(i);
				CheckWallGhostCollision(i);
				CheckJunctionCollisionGhost(i);
			}

			CheckViewportCollision();
			CheckMunchiesCollision();
			CheckGhostsCollision();
			UpdatePacman(elapsedTime);
			UpdatePacmanMovement();
			CheckWallCollisions();
			CheckJunctionCollisionPacman(elapsedTime);
			CheckCherryCollision();
		}
	}
}

void Pacman::Draw(int elapsedTime)
{
	// Allows us to easily create a string
	std::stringstream stream;
	std:stringstream menuStream;
	std::stringstream scoreStream;
	std::stringstream liveStream;

	stream << "Pacman X: " << _pacman->position->X << " Y: " << _pacman->position->Y;

	scoreStream << "Score: " << score;
	liveStream << "Lives: " << _pacman->lives;

	SpriteBatch::BeginDraw(); // Starts Drawing

	//Draws the map
	DrawTiles();

	//Draws the munchies
	DrawMunchies();

	//Draws the ghosts
	DrawGhosts();

	//Draws pacman, but only if the player isnt dead
	if (!_pacman->dead)
	{
		SpriteBatch::Draw(_pacman->texture, _pacman->position, _pacman->sourceRect);
	}

	//Draws cherry
	SpriteBatch::Draw(_cherry->texture, _cherry->position, _cherry->sourceRect);

	//Draws Start
	if (_menu->startMenu)
	{
		menuStream << "Press Space bar to start";

		SpriteBatch::Draw(_menu->background, _menu->rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->stringPosition, Color::Red);
	}
	//Draws lostlife menu
	if (_menu->lostLife && !_pacman->dead)
	{
		menuStream << "You lost a life" << endl << "You have " << _pacman->lives << " lives left" << endl << "Press Space bar to continue";
		SpriteBatch::Draw(_menu->background, _menu->rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->stringPosition, Color::Red);
	}

	//Draws pause menu
	if (_menu->paused)
	{
		menuStream << "PAUSED!";

		SpriteBatch::Draw(_menu->background, _menu->rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->stringPosition, Color::Red);
	}
	//Draws "you died" if player is dead
	if (_pacman->dead)
	{
		menuStream << "You died" << endl << "Score: " << score << endl << "Press Space bar to exit";
		SpriteBatch::Draw(_menu->background, _menu->rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->stringPosition, Color::Red);
	}
	//Draws "win" if you got all the munchies. Displays lives left and a score. Score will increase based off how many lives left (10 points per life)
	if (win == true)
	{
		int finalScore = score + (_pacman->lives * 10);
		//Lives are worth 10 points each
		menuStream << "You won" << endl << "Score: " << finalScore << endl << "Lives left: " << _pacman->lives << endl << "Press Space bar to exit";
		SpriteBatch::Draw(_menu->background, _menu->rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->stringPosition, Color::Red);
	}
	else
	{
		// Draws String
		SpriteBatch::DrawString(stream.str().c_str(), _stringPosition, Color::Green);
		SpriteBatch::DrawString(scoreStream.str().c_str(), _menu->scorePosition, Color::Green);
		SpriteBatch::DrawString(liveStream.str().c_str(), _menu->livesPosition, Color::Green);
	}



	SpriteBatch::EndDraw(); // Ends Drawing
}

void Pacman::Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState)
{
	// Checks if D key is pressed

	if (state->IsKeyDown(Input::Keys::D))
	{
		if (_pacman->currentDirection == 2)
		{
			_pacman->changeDirection = true;
		}
		_pacman->direction = 4;
	}
	// Checks if A key is pressed
	else if (state->IsKeyDown(Input::Keys::A))
	{
		if (_pacman->currentDirection == 4)
		{
			_pacman->changeDirection = true;
		}

		_pacman->direction = 2;
	}
	// Checks if W key is pressed
	else if (state->IsKeyDown(Input::Keys::W))
	{
		if (_pacman->currentDirection == 1)
		{
			_pacman->changeDirection = true;
		}

		_pacman->direction = 3;
	}
	// Checks if S key is pressed
	else if (state->IsKeyDown(Input::Keys::S))
	{
		if (_pacman->currentDirection == 3)
		{
			_pacman->changeDirection = true;
		}

		_pacman->direction = 1;
	}

}

void Pacman::CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey, Input::Keys spaceKey)
{
	//Turns off Start menu
	if (state->IsKeyDown(Input::Keys::SPACE) && (_menu->startMenu == true || _menu->lostLife == true || win == true))
	{
		_menu->startMenu = false;
		_menu->lostLife = false;
		ResetPositions();

		if (_pacman->dead == true)
		{
			Graphics::Destroy();
		}

		if (win == true)
		{
			Graphics::Destroy();
		}
	}

	if (!_menu->startMenu)
	{
		//Pauses game is P is pressed down, ! will invert the value of paused
		if (state->IsKeyDown(Input::Keys::P) && !_menu->pKeyDown)
		{
			_menu->pKeyDown = true;
			_menu->paused = !_menu->paused;
		}
		if (state->IsKeyUp(Input::Keys::P))
			_menu->pKeyDown = false;
	}

}

void Pacman::CheckViewportCollision()
{
	//checking to see if pacman is falling right side wall
	if (_pacman->position->X + _pacman->sourceRect->Width > Graphics::GetViewportWidth())
	{
		//Wrapping pacman from right to left
		_pacman->position->X = 32 - _pacman->sourceRect->Width;
		//If you use the wrapping, ghosts slow down until next check (junction)
		for (int i = 0; i < _ghosts.size(); i++)
		{
			_ghosts.at(i)->speed = 2.0f;
		}
	}
	//checking to see if pacman is falling off the left side wall
	if (_pacman->position->X + _pacman->sourceRect->Width < 32)
	{
		//Wrapping from left to right
		_pacman->position->X = Graphics::GetViewportWidth() - _pacman->sourceRect->Width;

		for (int i = 0; i < _ghosts.size(); i++)
		{
			_ghosts.at(i)->speed = 2.0f;
		}
	}
	//checking to see if pacman is falling off the top wall
	if (_pacman->position->Y + _pacman->sourceRect->Height > Graphics::GetViewportHeight())
	{
		// Pacman hit right wall - resets position
		_pacman->position->Y = Graphics::GetViewportHeight() - _pacman->sourceRect->Height;
	}
	//checking to see if pacman is falling off the bottom wall
	if (_pacman->position->Y + _pacman->sourceRect->Height < 32) // 32 is 0 + the width of the pacman sprite so the pacman doesnt fall off
	{
		// Pacman hit right wall - resets position
		_pacman->position->Y = 32 - _pacman->sourceRect->Height;
	}

	//GhostsViewPortCollision
	for (int i = 0; i < _ghosts.size(); i++)
	{
		if (_ghosts.at(i)->position->X + _ghosts.at(i)->sourceRect->Width > Graphics::GetViewportWidth())
		{
			//Wrapping pacman from right to left
			_ghosts.at(i)->position->X = 32 - _ghosts.at(i)->sourceRect->Width;
		}
		//checking to see if pacman is falling off the left side wall
		if (_ghosts.at(i)->position->X + _pacman->sourceRect->Width < 32)
		{
			//Wrapping from left to right
			_ghosts.at(i)->position->X = Graphics::GetViewportWidth() - _ghosts.at(i)->sourceRect->Width;
		}

	}

}

void Pacman::UpdatePacman(int elapsedTime)
{
	//Gets current time since pacman ran
	_pacman->currentFrameTime += elapsedTime;

	//Updates pacman animation
	if (_pacman->currentFrameTime > _cPacmanFrameTime)
	{
		_pacman->frame++;
		if (_pacman->frame >= 2)
			_pacman->frame = 0;

		_pacman->currentFrameTime = 0;
	}

	//Uses sprite sheet, uses X direction to play an animation, code above updates that animation
	_pacman->sourceRect->X = _pacman->sourceRect->Width * _pacman->frame;

	//Uses sprite sheet to choose, reference sheet pacman.tgg to see, uses the Y direction for arrow key direction.
	_pacman->sourceRect->Y = _pacman->sourceRect->Height * _pacman->currentDirection;


}

void Pacman::UpdateCherry(Pickup& _cherry, int elapsedTime)
{
	_cherry.currentFrameTime += elapsedTime;

	if (_cherry.currentFrameTime > _cPickupFrameTime)
	{
		_cherry.frame++;

		if (_cherry.frame >= 2)
			_cherry.frame = 0;

		_cherry.currentFrameTime = 0;

	}
	//Swaps the cherry texture
	_cherry.sourceRect->X = _cherry.sourceRect->Width * _cherry.frame;



}

int Pacman::GetHeight()
{
	return _tiles->at(0).size();
}

int Pacman::GetWidth()
{
	return _tiles->size();
}

void Pacman::LoadTiles()
{
	// Load the level and ensure all of the lines are the same length.
	int width;
	vector<string>* lines = new vector<string>();
	fstream stream;
	stringstream ss;

	ss << "Levels/" << levelSelection << ".txt";
	stream.open(ss.str(), fstream::in);

	char* line = new char[256];
	stream.getline(line, 256);
	string* sline = new string(line);
	width = sline->size();
	while (!stream.eof())
	{
		lines->push_back(*sline);
		if (sline->size() != width)
			cout << "Bad level load \n";
		stream.getline(line, 256);
		delete sline;
		sline = new string(line);
	}

	delete[] line;
	delete sline;

	//alocate grid
	_tiles = new vector<vector<Tile*>>(width, vector<Tile*>(lines->size()));

	for (int y = 0; y < GetHeight(); ++y)
	{
		for (int x = 0; x < GetWidth(); ++x)
		{
			//to load each tile
			char tileType = lines->at(y)[x];
			(*_tiles)[x][y] = LoadTile(tileType, x, y);
		}
	}

	delete lines;
}

Tile* Pacman::LoadTile(const char tileType, int x, int y)
{
	switch (tileType)
	{
		//Blank
	case '.':

		return new Tile(nullptr, TileCollision::Passable);
	case 'w':
		return LoadTile("Wall", TileCollision::Impassable);
	case 'm':
		return LoadMunchie(x, y);
	case 'p':
		return LoadPacman(x, y);
	case 'j':
		LoadJunction(x, y);
		return LoadMunchie(x, y);
	case 'g':
		LoadMunchie(x, y);
		LoadJunction(x, y);
		return LoadGhost(x, y);
	case 'c':
		LoadCherry(x, y);

	}
}

Tile* Pacman::LoadTile(const char* name, TileCollision collision)
{
	stringstream ss;
	ss << "Textures/" << name << ".png";

	Texture2D* tex = new Texture2D();
	tex->Load(ss.str().c_str(), true);

	return new Tile(tex, collision);
}

Tile* Pacman::LoadMunchie(int x, int y)
{
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures/Munchies.tga", true);

	_munchies.push_back(new Pickup());
	_munchies.back()->texture = munchieTex;
	_munchies.back()->position = new Vector2((x * 32) + 12, (y * 32) + 12); //Dictates a position of the munchie
	_munchies.back()->sourceRect = new Rect(0.0f, 0.0f, 12, 12);

	_munchies.back()->currentFrameTime = 0;
	_munchies.back()->frame = rand() % 1;
	_munchies.back()->frameTime = rand() % 500 + 50;
	return new Tile(nullptr, TileCollision::Passable);
}

Tile* Pacman::LoadPacman(float x, float y)
{
	_pacman->texture = new Texture2D();
	_pacman->texture->Load("Textures/Pacman.tga", false);
	_pacman->position = new Vector2(x * 32, y * 32);
	_pacman->originalPos = new Vector2(x * 32, y * 32);
	_pacman->sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	return new Tile(nullptr, TileCollision::Passable);
}

Tile* Pacman::LoadGhost(float x, float y)
{
	Texture2D* ghostTex = new Texture2D();
	ghostTex->Load("Textures/GhostBlue.tga", false);
	_ghosts.push_back(new MovingEnemy());
	_ghosts.back()->texture = ghostTex;
	_ghosts.back()->position = new Vector2(x * 32, y * 32);
	_ghosts.back()->originalPos = new Vector2(x * 32, y * 32);
	_ghosts.back()->sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	_ghosts.back()->speed = 4.0f;
	return new Tile(nullptr, TileCollision::Passable);

}

Tile* Pacman::LoadCherry(int x, int y)
{
	_cherry->texture = new Texture2D();
	_cherry->texture->Load("Textures/Cherrys.png", false);
	_cherry->position = new Vector2(x * 32, y * 32);
	_cherry->sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	return new Tile(nullptr, TileCollision::Passable);
}

void Pacman::LoadJunction(int x, int y)
{
	_junction.push_back(new Pickup());
	_junction.back()->position = new Vector2(x * 32, y * 32); //Dictates a position of the junction
	_junction.back()->sourceRect = new Rect(0.0f, 0.0f, 32, 32);
}

void Pacman::DrawTiles()
{
	for (int y = 0; y < GetHeight(); y++)
	{
		for (int x = 0; x < GetWidth(); x++)
		{
			//if there is a visiable 
			Texture2D* texture = _tiles->at(x).at(y)->Texture;
			if (texture != nullptr)
			{
				//Draw it on the screen
				Vector2 position((float)x, (float)y);
				position *= *Tile::Size;
				SpriteBatch::Draw(texture, &position);
			}
		}
	}
}

void Pacman::DrawMunchies()
{
	for (int i = 0; i < _munchies.size(); i++)
	{
		Vector2 position = *_munchies.at(i)->position;
		SpriteBatch::Draw(_munchies.at(i)->texture, &position, _munchies.at(i)->sourceRect); // Draws munchie
	}
}

void Pacman::DrawGhosts()
{
	for (int i = 0; i < _ghosts.size(); i++)
	{
		Vector2 position = *_ghosts.at(i)->position;
		SpriteBatch::Draw(_ghosts.at(i)->texture, &position, _ghosts.at(i)->sourceRect); // Draws munchie
	}
}

void Pacman::UpdateMunchies(int munchieNumber, int elapsedTime)
{

	//Updates pickups animation
	_munchies.at(munchieNumber)->currentFrameTime += elapsedTime;

	if (_munchies.at(munchieNumber)->currentFrameTime > _cPickupFrameTime)
	{
		_munchies.at(munchieNumber)->frame++;

		if (_munchies.at(munchieNumber)->frame >= 2)
			_munchies.at(munchieNumber)->frame = 0;

		_munchies.at(munchieNumber)->currentFrameTime = munchieNumber;

	}

	_munchies.at(munchieNumber)->sourceRect->X = _munchies.at(munchieNumber)->sourceRect->Width * _munchies.at(munchieNumber)->frame;
}

void Pacman::CheckWallCollisions()
{
	//Local Variables
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;



	for (int y = 0; y < GetHeight(); y++)
	{
		for (int x = 0; x < GetWidth(); x++)
		{
			//if there is a visiable wall
			Texture2D* texture = _tiles->at(x).at(y)->Texture;
			if (texture != nullptr)
			{
				//Gets wall positions mapped
				Vector2 position((float)x, (float)y);
				position *= *Tile::Size;

				bottom2 = position.Y + 32;
				left2 = position.X;
				right2 = position.X + 32;
				top2 = position.Y;


				//Checks collision with pacman in the direction he is moving (can interact with pacman if hes not moving that direction)
				// 1 = Down, 2 = Left, 3 = Up, 4 = Right
				if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2) && (_pacman->currentDirection == 1))
				{
					_pacman->position->Y = top2 - _pacman->sourceRect->Height;
					//Audio::Play(_pop);
				}

				else if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2) && (_pacman->currentDirection == 2))
				{
					_pacman->position->X = right2;
					//Audio::Play(_pop);
				}

				else if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2) && (_pacman->currentDirection == 3))
				{
					_pacman->position->Y = bottom2;
					//Audio::Play(_pop);
				}
				else if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2) && (_pacman->currentDirection == 4))
				{
					_pacman->position->X = left2 - _pacman->sourceRect->Width;
					//Audio::Play(_pop);
				}
			}
		}
	}

}

void Pacman::CheckMunchiesCollision()
{
	//Local Variables
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;

	for (i = 0; i < _munchies.size(); i++)
	{
		//Populate variables with munchie data
		bottom2 = _munchies.at(i)->position->Y + _munchies.at(i)->sourceRect->Height;
		left2 = _munchies.at(i)->position->X;
		right2 = _munchies.at(i)->position->X + _munchies.at(i)->sourceRect->Width;
		top2 = _munchies.at(i)->position->Y;

		//Checks for an overlap
		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
		{
			//Delete munchie from game
			_munchies.erase(_munchies.begin() + i);
			Audio::Play(_pop);
			score++;
		}
	}
}

void Pacman::CheckJunctionCollisionGhost(int ghostNumber)
{

	int bottom1 = _ghosts.at(ghostNumber)->position->Y + _ghosts.at(ghostNumber)->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _ghosts.at(ghostNumber)->position->X;
	int left2 = 0;
	int right1 = _ghosts.at(ghostNumber)->position->X + _ghosts.at(ghostNumber)->sourceRect->Width;
	int right2 = 0;
	int top1 = _ghosts.at(ghostNumber)->position->Y;
	int top2 = 0;
	bool reroll = true;
	int direction;

	//Randomly slowing the ghost down on a junction. Game too hard without
	int changeSpeed;

	//Tracking pacman
	int followPac;
	int pacPosX;
	int pacPosY;
	int ghostPosX = left1;
	int ghostPosY = top1;
	int xDiff;
	int yDiff;
	int xyDiff;

	//checks collision with junctions 
	for (int i = 0; i < _junction.size(); i++)
	{
		//populating with junction data
		bottom2 = _junction.at(i)->position->Y + _junction.at(i)->sourceRect->Height;
		left2 = _junction.at(i)->position->X;
		right2 = _junction.at(i)->position->X + _junction.at(i)->sourceRect->Width;
		top2 = _junction.at(i)->position->Y;

		// 1 = Down, 2 = Left, 3 = Up, 4 = Right
		//checks if junction co-ordinates are equal to the ghosts (collision)
		if ((bottom1 == bottom2) && (top1 == top2) && (right1 == right2) && (left1 == left2))
		{
			random_device rd;
			mt19937 gen(rd());
			uniform_int_distribution<> folP(0, 1);
			uniform_int_distribution<> slowG(1, 6);
			followPac = folP(gen);
			changeSpeed = slowG(gen);

			//Ghosts speed will half 1/6 chance on a junction
			if (changeSpeed == 6)
			{
				_ghosts.at(ghostNumber)->speed = 2.0f;
			}
			else
			{
				_ghosts.at(ghostNumber)->speed = 4.0f;
			}

			//Chance that the ghost will try to track towards the pacman
			//0 = ignore, 1 = follow
			if (followPac == 1)
			{
				pacPosX = _pacman->position->X;
				pacPosY = _pacman->position->Y;

				xDiff = ghostPosX - pacPosX;
				yDiff = ghostPosY - pacPosY;
				xyDiff = abs(xDiff) - abs(yDiff);

				//If pacman is further away in the X direction, go X.
				if (xyDiff > 0)
				{
					if (xDiff > 0)
					{
						direction = 2;
						reroll = false;
					}
					else
					{
						direction = 4;
						reroll = false;
					}
				}
				else
				{
					if (yDiff > 0)
					{
						direction = 3;
						reroll = false;
					}
					else
					{
						direction = 1;
						reroll = false;
					}
				}
			}
			//Stops ghost from turning around on a junction.
			while (reroll == true)
			{
				uniform_int_distribution<> dir(1, 4);
				direction = dir(gen);
				reroll = false;
				if ((_ghosts.at(ghostNumber)->direction == 1) && (direction == 3))
				{
					reroll = true;
				}
				else if ((_ghosts.at(ghostNumber)->direction == 3) && (direction == 1))
				{
					reroll = true;
				}
				else if ((_ghosts.at(ghostNumber)->direction == 4) && (direction == 2))
				{
					reroll = true;
				}
				else if ((_ghosts.at(ghostNumber)->direction == 1) && (direction == 4))
				{
					reroll = true;
				}
			}
			// 1 = Down, 2 = Left, 3 = Up, 4 = Right
			//Gets a random direction for the ghost to 
			_ghosts.at(ghostNumber)->direction = direction;
		}
	}

}

void Pacman::CheckJunctionCollisionPacman(int elapsedTime)
{

	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;

	//checks collision with junctions 
	for (int i = 0; i < _junction.size(); i++)
	{
		//populating with junction data
		bottom2 = _junction.at(i)->position->Y + _junction.at(i)->sourceRect->Height;
		left2 = _junction.at(i)->position->X;
		right2 = _junction.at(i)->position->X + _junction.at(i)->sourceRect->Width;
		top2 = _junction.at(i)->position->Y;

		//checks if junction co-ordinates are equal to the ghosts
		if ((bottom1 == bottom2) && (top1 == top2) && (right1 == right2) && (left1 == left2))
		{
			_pacman->changeDirection = true;
		}
	}
}

void Pacman::CheckWallGhostCollision(int ghostNumber)
{
	//Local Variables
	int bottom1 = _ghosts.at(ghostNumber)->position->Y + _ghosts.at(ghostNumber)->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _ghosts.at(ghostNumber)->position->X;
	int left2 = 0;
	int right1 = _ghosts.at(ghostNumber)->position->X + _ghosts.at(ghostNumber)->sourceRect->Width;
	int right2 = 0;
	int top1 = _ghosts.at(ghostNumber)->position->Y;
	int top2 = 0;


	//Stops the ghost if it hits a wall

	for (int y = 0; y < GetHeight(); y++)
	{
		for (int x = 0; x < GetWidth(); x++)
		{
			//if there is a visiable wall
			Texture2D* texture = _tiles->at(x).at(y)->Texture;
			if (texture != nullptr)
			{
				//Gets wall positions mapped
				Vector2 position((float)x, (float)y);
				position *= *Tile::Size;

				bottom2 = position.Y + 32;
				left2 = position.X;
				right2 = position.X + 32;
				top2 = position.Y;


				//Checks collision with pacman in the direction he is moving (can interact with pacman if hes not moving that direction)
				// 1 = Down, 2 = Left, 3 = Up, 4 = Right
				if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2) && (_ghosts.at(ghostNumber)->direction == 1))
				{
					_ghosts.at(ghostNumber)->position->Y = top2 - _ghosts.at(ghostNumber)->sourceRect->Height;
				}

				else if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2) && (_ghosts.at(ghostNumber)->direction == 2))
				{
					_ghosts.at(ghostNumber)->position->X = right2;

				}

				else if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2) && (_ghosts.at(ghostNumber)->direction == 3))
				{
					_ghosts.at(ghostNumber)->position->Y = bottom2;
				}
				else if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2) && (_ghosts.at(ghostNumber)->direction == 4))
				{
					_ghosts.at(ghostNumber)->position->X = left2 - _ghosts.at(ghostNumber)->sourceRect->Width;
				}
			}
		}
	}


}

void Pacman::UpdateGhosts(int ghostNumber)
{
	int direction = _ghosts.at(ghostNumber)->direction;
	int ghostSpeed = _ghosts.at(ghostNumber)->speed;

	if (direction == 4)
	{
		_ghosts.at(ghostNumber)->position->X += ghostSpeed; //Moves the ghost across X axis
	}
	// Checks if A key is pressed
	else if (direction == 2)
	{
		_ghosts.at(ghostNumber)->position->X += -ghostSpeed; //Moves ghost across X axis

	}
	// Checks if W key is pressed
	else if (direction == 3)
	{
		_ghosts.at(ghostNumber)->position->Y += -ghostSpeed; //Moves ghost across Y axis

	}
	// Checks if S key is pressed
	else if (direction == 1)
	{
		_ghosts.at(ghostNumber)->position->Y += ghostSpeed; //Moves ghost across Y axis
	}


}

void Pacman::CheckGhostsCollision()
{
	//Local Variables
	int i = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;

	for (i = 0; i < _ghosts.size(); i++)
	{
		//Populate variables with ghost data
		bottom2 = _ghosts.at(i)->position->Y + _ghosts.at(i)->sourceRect->Height;
		left2 = _ghosts.at(i)->position->X;
		right2 = _ghosts.at(i)->position->X + _ghosts.at(i)->sourceRect->Width;
		top2 = _ghosts.at(i)->position->Y;


		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
		{
			PacLostLife();
		}
	}
}

void Pacman::CheckCherryCollision()
{
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;
	int top1 = _pacman->position->Y;
	int top2 = 0;

	//Populate variables with cherry data
	bottom2 = _cherry->position->Y + _cherry->sourceRect->Height;
	left2 = _cherry->position->X;
	right2 = _cherry->position->X + _cherry->sourceRect->Width;
	top2 = _cherry->position->Y;

	//Checks for an overlap
	if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
	{
		//Move cherry offscreen
		_cherry->position->X = -100;
		_cherry->position->Y = -100;
		Audio::Play(_pop);
		score += 10;

	}
}

bool Pacman::CheckDead()
{
	if (_pacman->lives == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Pacman::CheckWin()
{
	if (_munchies.size() == 0)
		return true;
	else
		return false;
}

void Pacman::PacLostLife()
{
	_pacman->lives--;
	Audio::Play(_pacDeath);
	_menu->lostLife = true;
}

void Pacman::ResetPositions()
{


	for (int i = 0; i < _ghosts.size(); i++)
	{

		_ghosts.at(i)->position->X = _ghosts.at(i)->originalPos->X;
		_ghosts.at(i)->position->Y = _ghosts.at(i)->originalPos->Y;
	}
	_pacman->position->X = _pacman->originalPos->X;
	_pacman->position->Y = _pacman->originalPos->Y;
	_pacman->direction = 2;
	_pacman->changeDirection = true;
}

void Pacman::UpdatePacmanMovement()
{

	//For pacman movement and direction changing
	int pacSpeed = _cPacmanSpeed;
	if (_pacman->changeDirection == true)
	{
		_pacman->currentDirection = _pacman->direction;
		_pacman->changeDirection = false;
	}

	int currentDirection = _pacman->currentDirection;

	if (currentDirection == 4)
	{
		_pacman->position->X += pacSpeed; //Moves the ghost across X axis
	}

	else if (currentDirection == 2)
	{
		_pacman->position->X += -pacSpeed; //Moves ghost across X axis

	}

	else if (currentDirection == 3)
	{
		_pacman->position->Y += -pacSpeed; //Moves ghost across Y axis

	}

	else if (currentDirection == 1)
	{
		_pacman->position->Y += pacSpeed; //Moves ghost across Y axis
	}
}

TileCollision Pacman::GetCollision(int x, int y)
{
	// Prevent escaping past the level ends.
	if (x < 0 || x >= GetWidth())
		return TileCollision::Impassable;
	// Allow jumping past the level top and falling through the bottom.
	if (y < 0 || y >= GetHeight())
		return TileCollision::Passable;

	return _tiles->at(x).at(y)->Collision;
}