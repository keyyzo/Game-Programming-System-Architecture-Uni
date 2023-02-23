#include "Spike.h"

// spike constructor
// initialises spike variables

Spike::Spike()
{
	set_type(SPIKE);
}

// Update the transform of this object from a physics rigid body 

void Spike::UpdateFromSimulationSpike(const b2Body* body)
{
	if (body)
	{
		// setup object rotation
		// takes in rotation value for specific spikes
		gef::Matrix44 object_rotation;
		object_rotation.RotationZ(gef::DegToRad(rotateValue));

		// setup the object translation
		// takes in offset values for specific  spikes
		object_translation = gef::Vector4(body->GetPosition().x + offsetXValue, body->GetPosition().y+ offsetYValue, 0.0f);

		// scale matrix
		// specific values for scale object

		gef::Matrix44 Scale;
		Scale.Scale(gef::Vector4(0.175f,0.12f,0.15f));

		

		// build object transformation matrix
		gef::Matrix44 object_transform = object_rotation * Scale;
		object_transform.SetTranslation(object_translation);
		set_transform(object_transform);

	}
}