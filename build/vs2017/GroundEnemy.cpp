#include "GroundEnemy.h"
#include <graphics/renderer_3d.h>

// ground enemy constructor
// initialising ground enemy values

GroundEnemy::GroundEnemy()
{
	set_type(GROUND_ENEMY);
	endPointReached = false;
}

// ground enemy initialisier function
// passes in the b2World and primitive builder
// used within the game

void GroundEnemy::Init(b2World* world_, PrimitiveBuilder* primitive_builder_)
{
	// sets size of ground enemy

	this->setSize(gef::Vector4(1.0f, 2.0f, 1.0f));

	// sets mesh of ground enemy
	this->set_mesh(primitive_builder_->CreateBoxMesh(this->getSize()));

	// create a physics body for the ground enemy
	b2BodyDef body_def;
	body_def.type = b2_dynamicBody;
	body_def.position = b2Vec2(this->getPosition());
	body_def.fixedRotation = true;

	GEBody = world_->CreateBody(&body_def);

	// create the shape for the ground enemy
	b2PolygonShape shape;
	shape.SetAsBox(this->getSize().x(), this->getSize().y());

	// create the fixture
	b2FixtureDef fixture_def;
	fixture_def.shape = &shape;
	fixture_def.density = 2.0f;

	// create the fixture on the rigid body
	GEBody->CreateFixture(&fixture_def);



	// update visuals from simulation data
	this->UpdateFromSimulation(GEBody);

	// create a connection between the rigid body and GameObject
	GEBody->SetUserData(this);
}

// getters and setters

// ground enemy position

void GroundEnemy::setPosition(b2Vec2 pos)
{
	GEPosition = pos;
}

b2Vec2 GroundEnemy::getPosition()
{
	return GEPosition;


}


// ground enemy body

b2Body* GroundEnemy::getBody()
{
	return GEBody;
}

void GroundEnemy::setBody(b2Body* body)
{
	GEBody = body;
}

// collision response if the ground enemy
// collides with the player

// checks which way the enemy is moving
// if the player collides with the enemy
// when the enemy is moving towards the player
// the player will be pushed back based on the speed of the enemy

void GroundEnemy::PlayerCollisionResponse(Player* playerCheck, b2Body* playerBody)
{
	if (playerBody->GetPosition().x > GEBody->GetPosition().x && GEBody->GetLinearVelocity().x > 0.0f)
	{
		playerBody->ApplyLinearImpulseToCenter(b2Vec2(GEBody->GetLinearVelocity().x * 5, GEBody->GetLinearVelocity().x / 1.5f), true);
	}

	else if (playerBody->GetPosition().x < GEBody->GetPosition().x && GEBody->GetLinearVelocity().x < 0.0f)
	{
		playerBody->ApplyLinearImpulseToCenter(b2Vec2(GEBody->GetLinearVelocity().x * 5, GEBody->GetLinearVelocity().x / 1.5f), true);
	}

}


// sets starting point and ending point which
// the ground enemy will move between
// taking in the speed value to go with it

// ground enemy only moves on the x-axis

void GroundEnemy::Movement(float startPos, float endPos, float speed)
{
	if (endPos > startPos)
	{
		if (GEBody->GetPosition().x >= endPos)
		{
			endPointReached = true;
		}

		if (GEBody->GetPosition().x <= startPos)
		{
			endPointReached = false;
		}

		if (!endPointReached)
		{
			if (GEBody->GetLinearVelocity().x <= speed)
			{
				GEBody->ApplyLinearImpulseToCenter(b2Vec2(speed, 0.0f), true);
			}
		}

		else
		{
			if (GEBody->GetLinearVelocity().x >= -speed)
			{
				GEBody->ApplyLinearImpulseToCenter(b2Vec2(-speed, 0.0f), true);
			}

		}

	}

	else if (startPos > endPos)
	{

		if (GEBody->GetPosition().x <= endPos)
		{
			endPointReached = true;
		}

		if (GEBody->GetPosition().x >= startPos)
		{
			endPointReached = false;
		}

		if (!endPointReached)
		{
			if (GEBody->GetLinearVelocity().x >= -speed)
			{
				GEBody->ApplyLinearImpulseToCenter(b2Vec2(-speed, 0.0f), true);
			}
		}

		else
		{
			if (GEBody->GetLinearVelocity().x <= speed)
			{
				GEBody->ApplyLinearImpulseToCenter(b2Vec2(speed, 0.0f), true);
			}

		}
	}
}

// renders the ground enemy when called in scene app

void GroundEnemy::RenderGroundEnemy(gef::Renderer3D* renderer, PrimitiveBuilder* primitive_builder_)
{
	renderer->DrawMesh(*this);
}