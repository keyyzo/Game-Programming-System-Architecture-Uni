#ifndef _PRIMITIVE_BUILDER_H
#define _PRIMITIVE_BUILDER_H

#include <maths/vector4.h>
#include <graphics/material.h>
#include <cstddef>

namespace gef
{
	class Mesh;
	class Platform;
}

class PrimitiveBuilder
{
public:
	/// @brief Constructor.
	/// @param[in] platform		The platform the primitive builder is being created on.
	PrimitiveBuilder(gef::Platform& platform);

	/// @brief Default destructor.
	~PrimitiveBuilder();

	/// @brief Initialises the primitive builder.
	void Init();

	/// @brief Clean up resources created by the primitive builder.
	void CleanUp();

	/// @brief Creates a box shaped mesh
	/// @return The mesh created
	/// @param[in] half_size	The half size of the box.
	/// @param[in] centre		The centre of the box.
	/// @param[in] materials	an array of Material pointers. One for each face. 6 in total.
	gef::Mesh* CreateBoxMesh(const gef::Vector4& half_size, gef::Vector4 centre = gef::Vector4(0.0f, 0.0f, 0.0f), gef::Material** materials = NULL);


	/// @brief Creates a sphere shaped mesh
	/// @return The mesh created
	/// @param[in] radius		The radius of the sphere.
	/// @param[in] centre		The centre of the centre.
	/// @param[in] materials	Pointer to material used to render all faces. NULL is valid.
	gef::Mesh* CreateSphereMesh(const float radius, const int phi, const int theta, gef::Vector4 centre = gef::Vector4(0.0f, 0.0f, 0.0f), gef::Material* material = NULL);


	/// @brief Get the default cube mesh.
	/// @return The mesh for the default cube.
	/// @note The default cube has dimensions 1 x 1 x 1 with the centre at 0, 0, 0.
	inline const gef::Mesh* GetDefaultCubeMesh() const {
		return default_cube_mesh_;
	};

	/// @brief Get the default sphere mesh.
	/// @return The mesh for the default cube.
	/// @note The default sphere has radius 0.5 with the centre at 0, 0, 0.
	inline const gef::Mesh* GetDefaultSphereMesh() const {
		return default_sphere_mesh_;
	};


	/// @brief Get the red material.
	/// @return The reference to the red material.
	inline const gef::Material& red_material() const {
		return red_material_;
	}

	/// @brief Get the red material.
	/// @return The reference to the green material.
	inline const gef::Material& green_material() const {
		return green_material_;
	}

	/// @brief Get the blue material.
	/// @return The reference to the blue material.
	inline const gef::Material& blue_material() const {
		return blue_material_;
	}

	// colour functions added for game

	/// @brief Get the orange material.
	/// @return The reference to the orange material.
	inline const gef::Material& orange_material() const {
		return orange_material_;
	}

	/// @brief Get the yellow material.
	/// @return The reference to the yellow material.
	inline const gef::Material& yellow_material() const {
		return yellow_material_;
	}

	/// @brief Get the purple material.
	/// @return The reference to the purple material.
	inline const gef::Material& purple_material() const {
		return purple_material_;
	}

	/// @brief Get the gray material.
	/// @return The reference to the gray material.
	inline const gef::Material& gray_material() const {
		return gray_material_;
	}

	/// @brief Get the grass green material.
	/// @return The reference to the grass green material.
	inline const gef::Material& grass_green_material() const {
		return grass_green_material_;
	}

	/// @brief Get the brown material.
	/// @return The reference to the brown material.
	inline const gef::Material& brown_material() const {
		return brown_material_;
	}

	/// @brief Get the light blue material.
	/// @return The reference to the light blue material.
	inline const gef::Material& light_blue_material() const {
		return light_blue_material_;
	}

	/// @brief Get the light pink material.
	/// @return The reference to the light pink material.
	inline const gef::Material& light_pink_material() const {
		return light_pink_material_;
	}

protected:
	gef::Platform& platform_;

	gef::Mesh* default_cube_mesh_;
	gef::Mesh* default_sphere_mesh_;

	gef::Material red_material_;
	gef::Material blue_material_;
	gef::Material green_material_;

	// colour variabls added for game
	gef::Material orange_material_;
	gef::Material yellow_material_;
	gef::Material purple_material_;
	gef::Material gray_material_;
	gef::Material grass_green_material_;
	gef::Material brown_material_;
	gef::Material light_blue_material_;
	gef::Material light_pink_material_;


	
};

#endif // _PRIMITIVE_BUILDER_H