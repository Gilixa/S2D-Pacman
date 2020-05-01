#pragma once

#include "S2D\S2D.h"

using namespace S2D;

/// <summary>
/// Controls the collision detection and response behavior of a tile.
/// </summary>
enum class TileCollision
{
	/// <summary>
	/// A passable tile is one which does not hinder player motion at all.
	/// </summary>
	Passable = 0,

	/// <summary>
	/// An impassable tile is one which does not allow the player to move through
	/// it at all. It is completely solid.
	/// </summary>
	Impassable = 1,

};

struct Tile
{
public:
	Texture2D* Texture;
	TileCollision Collision;

	static const int Width;
	static const int Height;

	static const Vector2* Size;

	Tile(Texture2D* texture, TileCollision collision);
	~Tile(void);
};