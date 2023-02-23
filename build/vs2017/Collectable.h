#pragma once
#include "game_object.h"
#include <maths/math_utils.h>

class Collectable : public GameObject
{
public:

	// collectable constructor

	Collectable();

	// Update the transform of this object from a physics rigid body 
	void UpdateFromSimulationCollectable(const b2Body* body);

	// setter for collectable rotation value
	void setRotate(int r) { rotateValue = r; }
private:

	// collectable variables

	gef::Vector4 object_translation;
	int rotateValue;

};

