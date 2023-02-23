#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <graphics/mesh_instance.h>
#include <box2d/Box2D.h>
#include "input/input_manager.h"
#include "input/keyboard.h"
#include "Timer.h"

// object states
// used to set different types
// to specific objects

enum OBJECT_TYPE
{
	PLAYER,
	GROUND_ENEMY,
	SPIKE,
	COLLECTABLE,
	ABILITY_DJ,
	ABILITY_DASH,
	ABILITY_RW,
	GROUND,
	WALL,
	STICKY_WALL,
	STICKY_ROOF,
	MOVING
};

class GameObject : public gef::MeshInstance
{
public:

	// Update the transform of this object from a physics rigid body
	void UpdateFromSimulation(const b2Body* body);

	inline void set_type(OBJECT_TYPE type) { type_ = type; }
	inline OBJECT_TYPE type() { return type_; }

	// size and scale functions

	void setSize(gef::Vector4 size);
	gef::Vector4 getSize();
	void setScale(gef::Vector4 scale) { object_scale_vec = scale; }
	
private:

	// object variabls
	// used for type and matrix transformations

	OBJECT_TYPE type_;
	gef::Vector4 object_translation;
	gef::Vector4 object_scale_vec;
	gef::Vector4 objectSize;
};

// player states
// used to determine what the 
// player is doing and how to control
// the player during said state

enum PLAYER_STATE
{
	STANDING,
	MOVING_RIGHT,
	MOVING_LEFT,
	DASHING_RIGHT,
	DASHING_LEFT,
	JUMPING,
	DOUBLE_JUMPING,
	ON_WALL,
	ON_MOVING_PLAT
};

class Player : public GameObject
{

public:

	// player constructor

	Player();
	
	// sets player states

	inline void setPlayerState(PLAYER_STATE playerStateType) 
	{ 
		secondPreviousPlayerState = previousPlayerState;
		previousPlayerState = currentPlayerState;
		currentPlayerState = playerStateType; 
	}

	// returns different player states

	inline PLAYER_STATE getPlayerState() { return currentPlayerState; }
	inline PLAYER_STATE getPlayerPreviousState() { return previousPlayerState; }
	inline PLAYER_STATE getPlayerSecondPreviousState() { return secondPreviousPlayerState; }

	// updates player inputs and variables

	void HandleInput(gef::InputManager* im, b2Body* body, float frame_time);

	// decreases health of player
	// sets player to invulnerable based on
	// a timer

	void DecrementHealth();

	// setters and getters

	// player health
	int getHealth();
	void setHealth(int newHealth);


	// invincible check
	bool getInvincibleCheck() { return invincibleCheck; }

	// player abilities
	void setDashActive(bool);
	bool getDashActive() { return dashActive; }
	void setDoubleJumpActive(bool);
	bool getDoubleJumpActive() { return doubleJumpActive; }
	void setResetWallActive(bool);
	bool getResetWallActive() { return resetWallActive; }
	
private:

	// player variables

	// health and invincible check
	int health;
	bool invincibleCheck;

	// player ability checks
	bool doubleJumpActive, dashActive, resetWallActive;

	// all player states

	PLAYER_STATE currentPlayerState;
	PLAYER_STATE previousPlayerState;
	PLAYER_STATE secondPreviousPlayerState;

	// timer for invulnerability
	Timer healthTimer;

};

class MovingPlatform : public GameObject
{
public:

	// moving platform constructor

	MovingPlatform();

	// sets starting point and ending point which
	// the platform will move between
	// taking in the speed value to go with it

	void setPlatPosAndSpeedYaxis(b2Body* body, float startPos, float endPos, float ySpeed);
	void setPlatPosAndSpeedXaxis(b2Body* body, float startPos, float endPos, float xSpeed);

private:

	// checks if endpoint of moving platform
	// has been reached

	bool maxPointHit;

};


#endif // _GAME_OBJECT_H