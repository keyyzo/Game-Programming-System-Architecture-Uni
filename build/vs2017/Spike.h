#pragma once
#include "game_object.h"
#include <maths/math_utils.h>
class Spike : public GameObject
{
public:

	// spike constructor
	Spike();

	// Update the transform of this object from a physics rigid body 
	void UpdateFromSimulationSpike(const b2Body* body);

	// setters for individual spike variables

	// rotation
	void setRotate(int r) { rotateValue = r; }

	// offset values for spike position
	void setOffsetXValue(float s) { offsetXValue = s; }
	void setOffsetYValue(float s) { offsetYValue = s; }
private:

	// spike variables

	gef::Vector4 object_translation;
	int rotateValue;
	float offsetXValue, offsetYValue;

};

