#pragma once
#include "game_object.h"
#include "primitive_builder.h"

// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Renderer3D;
};

class GroundEnemy : public GameObject
{
	public:

		// ground enemy constructor

		GroundEnemy();

		// ground enemy initialiser function
		void Init(b2World* world_, PrimitiveBuilder* primitive_builder_);

		// setters and getters

		// ground enemy position
		void setPosition(b2Vec2 pos);
		b2Vec2 getPosition();

		// ground enemy body
		b2Body* getBody();
		void setBody(b2Body* body);

		// movement function
		// works similar to the moving platforms 
		// sets starting point and ending point which
		// the ground enemy will move between
		// taking in the speed value to go with it

		void Movement(float startPos, float endPos, float speed);

		// collision response if the ground enemy
		// collides with the player
		void PlayerCollisionResponse(Player* playerCheck, b2Body* playerBody);

		// render ground enemy function
		void RenderGroundEnemy(gef::Renderer3D* renderer, PrimitiveBuilder* primitive_builder_);
	private:

		// ground enemy variables

		// checks if endpoint of ground enemy
		// has been reached
		bool endPointReached;

		// ground enemy body and position
		b2Body* GEBody;
		b2Vec2 GEPosition;
};

