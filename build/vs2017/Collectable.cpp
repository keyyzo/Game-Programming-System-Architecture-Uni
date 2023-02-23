#include "Collectable.h"

// collectable constructor
// intialises constructor variables

Collectable::Collectable()
{
	set_type(COLLECTABLE);
}


// Update the transform of this object from a physics rigid body 
void Collectable::UpdateFromSimulationCollectable(const b2Body* body)
{
	if (body)
	{
		// setup object rotation
		// sets to 90 to turn object round
		// facing the camera a certain way
		gef::Matrix44 object_rotation;
		object_rotation.RotationY(gef::DegToRad(90));

		// setup the object translation

		object_translation = gef::Vector4(body->GetPosition().x, body->GetPosition().y, 0.0f);

		// scale matrix
		// specific values to correctly 
		// resize and display collectable
		gef::Matrix44 Scale;
		Scale.Scale(gef::Vector4(4.0f, 4.0f, 4.0f));



		// build object transformation matrix
		gef::Matrix44 object_transform = object_rotation * Scale;
		object_transform.SetTranslation(object_translation);
		set_transform(object_transform);

	}
}