#include "game_object.h"
#include <system/debug_log.h>

#define JUMP_VALUE 11.0f
#define DOUBLE_JUMP_VALUE 11.0f

//
// UpdateFromSimulation
// 
// Update the transform of this object from a physics rigid body
//

void GameObject::UpdateFromSimulation(const b2Body* body)
{
	if (body)
	{
		// setup object rotation
		gef::Matrix44 object_rotation;
		object_rotation.RotationZ(body->GetAngle());

		// setup the object translation

		object_translation = gef::Vector4(body->GetPosition().x, body->GetPosition().y, 0.0f);

		// build object transformation matrix
		gef::Matrix44 object_transform = object_rotation;
		object_transform.SetTranslation(object_translation);
		set_transform(object_transform);

	}
}


// gets and sets size of object

void GameObject::setSize(gef::Vector4 size)
{
	objectSize = size;
}

gef::Vector4 GameObject::getSize()
{
	return objectSize;
}

// player constructor
// initialising player values

Player::Player()
{
	set_type(PLAYER);
	setPlayerState(STANDING);
	previousPlayerState = STANDING;
	secondPreviousPlayerState = STANDING;
	invincibleCheck = false;
	health = 5;
}

// handles player inputs, updating the players state
// and position based on the inputs
// also updates player variables

void Player::HandleInput(gef::InputManager* im, b2Body* body, float frame_time)
{

	// creates keyboard controls inside the player class

	const gef::Keyboard* kb = im->keyboard();


	if (kb)
	{
		// checks for current state of player
		// based on current state allows for certain controls
		// and actions to occur

		switch (currentPlayerState)
		{
		case STANDING:

			// jumps if player is standing still

			if (kb->IsKeyPressed(gef::Keyboard::KC_UP) && (body->GetLinearVelocity().y <= 0.05f && body->GetLinearVelocity().y >= -0.05f))
			{
				setPlayerState(JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, JUMP_VALUE), true);
				
			}

			// allows for player to jump after coming off a Double Jump Reset wall

			else if (kb->IsKeyPressed(gef::Keyboard::KC_UP) && previousPlayerState == ON_WALL)
			{
				setPlayerState(JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, JUMP_VALUE), true);

			}

			// allows for player to double jump while falling before reaching a certain velocity

			else if (kb->IsKeyPressed(gef::Keyboard::KC_UP) && body->GetLinearVelocity().y > -25.0f && secondPreviousPlayerState == JUMPING && (previousPlayerState == MOVING_LEFT || previousPlayerState == MOVING_RIGHT) && doubleJumpActive == true)
			{
				setPlayerState(DOUBLE_JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, DOUBLE_JUMP_VALUE), true);
				
			}

			// moves the player right

			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT))
			{
				setPlayerState(MOVING_RIGHT);
			}

			// moves the player left

			else if (kb->IsKeyDown(gef::Keyboard::KC_LEFT))
			{
				setPlayerState(MOVING_LEFT);
			}
			break;
		case MOVING_RIGHT:

			// limits speed player can reach when moving right
			if (body->GetLinearVelocity().x <= 6.0f)
			{
				body->ApplyLinearImpulseToCenter(b2Vec2(1.0f, 0.0f), true);
			}
			
			// dashes player right if ability is active

			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT) && kb->IsKeyPressed(gef::Keyboard::KC_LCONTROL) && dashActive)
			{
				
				setPlayerState(DASHING_RIGHT);

				// limits dash speed
				if (body->GetLinearVelocity().x <= 16.0f)
					body->ApplyLinearImpulseToCenter(b2Vec2(8.0f, 0.0f), true);
			}

			// moves player left

			else if (kb->IsKeyDown(gef::Keyboard::KC_LEFT))
			{
				setPlayerState(MOVING_LEFT);
			}

			// allows player to jump while moving right
			// stops player from jumping infinitely
			if (kb->IsKeyPressed(gef::Keyboard::KC_UP) && (body->GetLinearVelocity().y <= 0.05f && body->GetLinearVelocity().y >= -0.05f))
			{
				setPlayerState(JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, JUMP_VALUE), true);
				
				
			}

			// alows player to jump on a moving platform while moving right
			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT) && kb->IsKeyPressed(gef::Keyboard::KC_UP) && previousPlayerState == ON_MOVING_PLAT && secondPreviousPlayerState != JUMPING)
			{
				setPlayerState(JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, JUMP_VALUE), true);


			}

			// alows player to double jump after being on a moving platform while moving right
			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT) && kb->IsKeyPressed(gef::Keyboard::KC_UP) && previousPlayerState == ON_MOVING_PLAT && secondPreviousPlayerState == JUMPING && doubleJumpActive == true)
			{
				setPlayerState(DOUBLE_JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, DOUBLE_JUMP_VALUE), true);


			}

			// allows player to double jump while moving right if ability is active
			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT) && kb->IsKeyPressed(gef::Keyboard::KC_UP) && previousPlayerState == JUMPING && doubleJumpActive == true)
			{
				setPlayerState(DOUBLE_JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, DOUBLE_JUMP_VALUE), true);


			}


			// allows player to jump after being on a Double Jump Reset wall
			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT) && kb->IsKeyPressed(gef::Keyboard::KC_UP) && previousPlayerState == ON_WALL)
			{
				setPlayerState(JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, JUMP_VALUE), true);


			}

			// brings player to a standing state
			else if(kb->IsKeyReleased(gef::Keyboard::KC_RIGHT)) 
			{
				setPlayerState(STANDING);
			}
			break;
		case MOVING_LEFT:
			
			// limits speed player can reach when moving left
			if (body->GetLinearVelocity().x >= -6.0f)
			{
				body->ApplyLinearImpulseToCenter(b2Vec2(-1.0f, 0.0f), true);
			}
			
			// dashes player left if ability is active

			else if (kb->IsKeyDown(gef::Keyboard::KC_LEFT) && kb->IsKeyPressed(gef::Keyboard::KC_LCONTROL) && dashActive)
			{
				
				setPlayerState(DASHING_LEFT);

				// limits dash speed
				if (body->GetLinearVelocity().x >= -16.0f)
					body->ApplyLinearImpulseToCenter(b2Vec2(-8.0f, 0.0f), true);
			}

			// moves player right

			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT))
			{
				setPlayerState(MOVING_RIGHT);
			}


			// allows player to jump while moving left
			// stops player from jumping infinitely
			if (kb->IsKeyPressed(gef::Keyboard::KC_UP) && (body->GetLinearVelocity().y <= 0.05f && body->GetLinearVelocity().y >= -0.05f))
			{
				setPlayerState(JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, JUMP_VALUE), true);
				
				
			}

			// alows player to jump on a moving platform while moving left
			else if (kb->IsKeyDown(gef::Keyboard::KC_LEFT) && kb->IsKeyPressed(gef::Keyboard::KC_UP) && previousPlayerState == ON_MOVING_PLAT && secondPreviousPlayerState != JUMPING)
			{
				setPlayerState(JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, JUMP_VALUE), true);


			}

			// alows player to double jump after being on a moving platform while moving left
			else if (kb->IsKeyDown(gef::Keyboard::KC_LEFT) && kb->IsKeyPressed(gef::Keyboard::KC_UP) && previousPlayerState == ON_MOVING_PLAT && secondPreviousPlayerState == JUMPING && doubleJumpActive == true)
			{
				setPlayerState(DOUBLE_JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, DOUBLE_JUMP_VALUE), true);


			}

			// allows player to double jump while moving left if ability is active
			else if (kb->IsKeyDown(gef::Keyboard::KC_LEFT) && kb->IsKeyPressed(gef::Keyboard::KC_UP) && previousPlayerState == JUMPING && doubleJumpActive == true)
			{
				setPlayerState(DOUBLE_JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, DOUBLE_JUMP_VALUE), true);


			}

			// allows player to jump after being on a Double Jump Reset wall
			else if (kb->IsKeyDown(gef::Keyboard::KC_LEFT) && kb->IsKeyPressed(gef::Keyboard::KC_UP) && previousPlayerState == ON_WALL)
			{
				setPlayerState(JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, JUMP_VALUE), true);


			}

			// brings player to a standing state
			else if (kb->IsKeyReleased(gef::Keyboard::KC_LEFT)) 
			{
				setPlayerState(STANDING);
			}
			break;
		case DASHING_RIGHT:

			// moves player left
			if (kb->IsKeyDown(gef::Keyboard::KC_LEFT))
			{
				setPlayerState(MOVING_LEFT);
			}

			// moves player right
			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT))
			{
				setPlayerState(MOVING_RIGHT);
			}

			break;
		case DASHING_LEFT:
			
			// moves player left
			if (kb->IsKeyDown(gef::Keyboard::KC_LEFT))
			{
				setPlayerState(MOVING_LEFT);
			}


			// moves player right
			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT))
			{
				setPlayerState(MOVING_RIGHT);
			}

			
			break;

			
		case JUMPING:
			
			// allows player to double jump if ability is active
			if (kb->IsKeyPressed(gef::Keyboard::KC_UP) && doubleJumpActive == true)
			{
				setPlayerState(DOUBLE_JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, DOUBLE_JUMP_VALUE), true);
			}

			// brings player to a standing state
			// if no input is present and the player comes
			// to a complete stop

			else if (body->GetLinearVelocity().y <= 0.05f && body->GetLinearVelocity().y >= -0.05f)
			{
				setPlayerState(STANDING);
			}

			// moves player right
			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT))
			{
				setPlayerState(MOVING_RIGHT);
			}

			// moves player left
			else if (kb->IsKeyDown(gef::Keyboard::KC_LEFT))
			{
				setPlayerState(MOVING_LEFT);
				
			}

			break;
		case DOUBLE_JUMPING:

			// brings player to a standing state
			// if no input is present and the player comes
			// to a complete stop
			if (body->GetLinearVelocity().y <= 0.05f && body->GetLinearVelocity().y >= -0.05f)
			{
				setPlayerState(STANDING);
			}

			// moves player right
			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT))
			{
				setPlayerState(MOVING_RIGHT);
			}

			// moves player left
			else if (kb->IsKeyDown(gef::Keyboard::KC_LEFT))
			{
				setPlayerState(MOVING_LEFT);
			}

			break;

		case ON_WALL:


			// moves player right
			// off from wall

			if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT))
			{
				
				setPlayerState(MOVING_RIGHT);
			}

			// moves player left
			// off from wall

			else if (kb->IsKeyDown(gef::Keyboard::KC_LEFT))
			{
				
				setPlayerState(MOVING_LEFT);
			}


			break;

		case ON_MOVING_PLAT:


			// allows player to jump from moving platform

			if (kb->IsKeyPressed(gef::Keyboard::KC_UP) & previousPlayerState != ON_MOVING_PLAT)
			{
				setPlayerState(JUMPING);
				body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, JUMP_VALUE), true);
			}

			// allows player to move right on moving platform

			else if (kb->IsKeyDown(gef::Keyboard::KC_RIGHT))
			{
				setPlayerState(MOVING_RIGHT);
			}

			// allows player to move left on moving platform
			else if (kb->IsKeyDown(gef::Keyboard::KC_LEFT))
			{
				setPlayerState(MOVING_LEFT);
			}

			break;

		default: STANDING;
			break;
		}
		
	}
	
	// checks if player has been hit
	// and is invulnerable or not

	if (invincibleCheck)
	{
		// once timer goes over a certain limit
		// stops and resets the timer
		// takes player out of invulnerability

		if (healthTimer.elapsedSeconds() > 2.0)
		{
			healthTimer.Stop();
			invincibleCheck = false;

		}
	}

}

void Player::DecrementHealth()
{
	
	// checks if player has been hit
	// and is invulnerable or not

	if (!invincibleCheck)
	{
		// if not,
		// decreases player health
		// makes player invulnerable 
		// starts the timer

		health--;
		invincibleCheck = true;
		healthTimer.Start();

	}

}

// getters and setters

// player health

int Player::getHealth()
{
	return health;
}

void Player::setHealth(int newHealth)
{
	health = newHealth;
}

// player abilities

void Player::setDashActive(bool s)
{
	dashActive = s;
}

void Player::setDoubleJumpActive(bool s)
{
	 doubleJumpActive = s;
}

void Player::setResetWallActive(bool s)
{
	resetWallActive = s;
}

// moving platform constructor
// initialising moving platform values

MovingPlatform::MovingPlatform()
{
	set_type(MOVING);
	maxPointHit = false;
}


// sets starting point and ending point which
// the platform will move between
// taking in the speed value to go with it
// For platforms moving on the Y axis

void MovingPlatform::setPlatPosAndSpeedYaxis(b2Body* body, float startPos, float endPos, float ySpeed)
{
	if (endPos > startPos)
	{
		if (body->GetPosition().y >= endPos)
		{
			maxPointHit = true;
		}

		if (body->GetPosition().y <= startPos)
		{
			maxPointHit = false;
		}

		if (!maxPointHit)
		{
			body->SetLinearVelocity(b2Vec2(0.0f, ySpeed));
		}

		else
		{
			body->SetLinearVelocity(b2Vec2(0.0f, -ySpeed));
		}

	}

	else if(startPos > endPos)
	{

		if (body->GetPosition().y <= endPos)
		{
			maxPointHit = true;
		}

		if (body->GetPosition().y >= startPos)
		{
			maxPointHit = false;
		}

		if (!maxPointHit)
		{
			body->SetLinearVelocity(b2Vec2(0.0f, -ySpeed));
		}

		else
		{
			body->SetLinearVelocity(b2Vec2(0.0f, ySpeed));
		}
	}

	
}

// sets starting point and ending point which
// the platform will move between
// taking in the speed value to go with it
// For platforms moving on the X axis

void MovingPlatform::setPlatPosAndSpeedXaxis(b2Body* body, float startPos, float endPos, float xSpeed)
{
	

	if (endPos > startPos)
	{
		if (body->GetPosition().x >= endPos)
		{
			maxPointHit = true;
		}

		if (body->GetPosition().x <= startPos)
		{
			maxPointHit = false;
		}

		if (!maxPointHit)
		{
			body->SetLinearVelocity(b2Vec2(xSpeed, 0.0f));
		}

		else
		{
			body->SetLinearVelocity(b2Vec2(-xSpeed, 0.0f));
		}
	}

	else if (startPos > endPos)
	{
		if (body->GetPosition().x <= endPos)
		{
			maxPointHit = true;
		}

		if (body->GetPosition().x >= startPos)
		{
			maxPointHit = false;
		}

		if (!maxPointHit)
		{
			body->SetLinearVelocity(b2Vec2(-xSpeed, 0.0f));
		}

		else
		{
			body->SetLinearVelocity(b2Vec2(xSpeed, 0.0f));
		}
	}


}





