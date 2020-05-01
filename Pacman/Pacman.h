#pragma once

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
	#ifndef _DEBUG
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
#endif


#define GHOSTCOUNT 0



// Just need to include main header file
#include "S2D/S2D.h"

#include "Tile.h"



// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;




//Strucutre Definition
struct Player
{
	// Data to represent Pacman
	bool dead;
	Vector2 *position;
	Vector2* originalPos;
	Rect *sourceRect;
	Texture2D *texture;
	int lives;


	// Data to change the direction that pacman is looking for animations
	int direction;
	int currentDirection;
	bool changeDirection;
	int frame;
	int currentFrameTime;
};

struct MovingEnemy
{
	Vector2 *position;
	Vector2* originalPos;
	Texture2D* texture;
	Rect* sourceRect;
	int direction;
	float speed;
};

struct Pickup
{

	// Data to represent Munchie
	Texture2D *texture;
	Rect *sourceRect;
	Vector2 *position;

	//Animating the munchie or cherry (using munchie Code since they can just pulse with the munchie)
	int currentFrameTime;
	int frame;
	int frameTime;
};

struct Menu
{
	//Data for the Menu
	Texture2D* background;
	Rect* rectangle;
	Vector2* stringPosition;
	bool startMenu;
	bool lostLife;
	bool paused;
	bool pKeyDown;

	Vector2* scorePosition;
	Vector2* livesPosition;
	Vector2* timePosition;
};

struct Tile;

//Class Definition
// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
class Pacman : public Game
{
private:

	//Data for functions
	void Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);
	void CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey,Input::Keys spaceKey);
	void CheckViewportCollision();
	void UpdatePacman(int elapsedTime);
	void UpdateCherry(Pickup & _cherry, int elapsedTime);
	void UpdatePacmanMovement();

	//Structure data
	Player *_pacman;
	Pickup *_cherry;
	Menu* _menu;

	//Constants for passive animations updating
	const int _cPacmanFrameTime;
	const int _cPickupFrameTime;
	const float _cPacmanSpeed;

	// Position for String
	Vector2* _stringPosition;

	//Data for sound
	SoundEffect* _pop;
	SoundEffect* _pacDeath;
	SoundEffect* _pacIntro;

	int score;
	int levelSelection;
	bool win;

	bool CheckDead();
	bool CheckWin();
	void PacLostLife();
	void ResetPositions();

	vector<Pickup*>_munchies;
	vector<Pickup*>_junction;
	vector<MovingEnemy*>_ghosts;

	vector<vector<Tile*>>* _tiles; //A vector of vectors
	Tile* LoadTile(const char tileType, int x, int y);
	
	Tile* LoadTile(const char* name, TileCollision collision);
	Tile* LoadMunchie(int x, int y);
	Tile* LoadPacman(float x, float y);
	Tile* LoadGhost(float x, float y);
	Tile* LoadCherry(int x, int y);
	void  LoadJunction(int x, int y);
	TileCollision GetCollision(int x, int y);
	
	void LoadTiles();
	void DrawTiles();
	void DrawMunchies();
	void DrawGhosts();
	void UpdateMunchies(int munchieNumber, int elapsedTime);
	void UpdateGhosts(int ghostNumber);

	void CheckJunctionCollisionGhost(int ghostNumber);
	void CheckJunctionCollisionPacman(int elapsedTime);
	void CheckWallCollisions();
	void CheckMunchiesCollision();
	void CheckWallGhostCollision(int ghostNumber);
	void CheckGhostsCollision();
	void CheckCherryCollision();
	int GetWidth();
	int GetHeight();


public:
	/// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[], int level);

	/// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	/// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	/// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	/// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);
};
