#include "scene_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>
#include <system/debug_log.h>
#include <graphics/renderer_3d.h>
#include <graphics/mesh.h>
#include <maths/math_utils.h>
#include "input\keyboard.h"
#include "load_texture.h"
#include <set>

#define SMALL_PLATFORM_NUM_SA 95
#define MEDIUM_PLATFORM_NUM_MA 12
#define MOVING_PLATFORM_NUM 13
#define VERY_SMALL_PLATFORM_NUM 31
#define BIG_PLATFORM_NUM 4
#define BLOCKING_WALL_NUM 8
#define BIGGER_BLOCKING_WALL_NUM 2
#define AREA_WALL_NUM 5
#define RESET_WALL_NUM 10
#define GROUND_ENEMY_NUM 11
#define SPIKE_NUM 40

SceneApp::SceneApp(gef::Platform& platform) :
	Application(platform),
	sprite_renderer_(NULL),
	renderer_3d_(NULL),
	primitive_builder_(NULL),
	input_manager_(NULL),
	font_(NULL),
	world_(NULL),
	player_body_(NULL),
	spike_body_(NULL),
	collectable_body_(NULL),
	dashPickup_body_(NULL),
	doubleJumpPickup_body_(NULL),
	resetWallPickup_body_(NULL),
	bottom_border_body(NULL),
	top_border_body(NULL),
	small_platform_body(NULL),
	medium_platform_body(NULL),
	big_platform_body(NULL),
	movPlatform_body(NULL),
	right_border_body_(NULL),
	left_border_body_(NULL),
	very_small_plat_body_(NULL),
	blocking_wall_body_(NULL),
	bigger_blocking_wall_body_(NULL),
	area_wall_body_(NULL),
	reset_wall_body_(NULL),
	splash_screen_(NULL),
	menu_screen_(NULL),
	game_screen_(NULL),
	game_screen_diff_camera(NULL),
	game_over_screen_(NULL),
	options_background_(NULL),
	htp_background_(NULL),
	win_game_background(NULL),
	audio_manager(NULL),
	scene_assets_(NULL)
{
}

void SceneApp::Init()
{
	

	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	InitFont();

	// initialise input manager
	input_manager_ = gef::InputManager::Create(platform_);
	audio_manager = gef::AudioManager::Create();

	// loads all audio samples

	menu_music = audio_manager->LoadSample("win-music.wav", platform_);
	level_music = audio_manager->LoadSample("level_music.wav", platform_);
	game_over = audio_manager->LoadSample("SamusDeath_1.wav", platform_);
	player_wins = audio_manager->LoadSample("menu-music.wav", platform_);
	ability_pickup = audio_manager->LoadSample("ability-pickup.wav", platform_);
	jump_sound = audio_manager->LoadSample("jump.wav", platform_);
	dash_sound = audio_manager->LoadSample("dash-sound.wav", platform_);
	hit_sound = audio_manager->LoadSample("game-hit.wav", platform_);
	menu_button_sound = audio_manager->LoadSample("menu-click.wav", platform_);
	

	// initialising bool values

	level_music_playing = false;
	game_over_playing = false;
	player_wins_playing = false;
	ability_pickup_playing = false;
	
	// play menu music / sound

	menu_music_playing = true;
	audio_manager->PlaySample(0,true);
	
	// initialise controls for keyboard
	kb = input_manager_->keyboard();

	// set initial app state
	set_type_gamestate(INIT);

	// set initial difficulty and volume states
	difficulty = DIFF_NORMAL;
	volume = SEVENTY_FIVE;

	// set app is running
	isApplicationRunning = true;

	// call frontend initialiser
	FrontendInit();
	
}

void SceneApp::CleanUp()
{
	delete input_manager_;
	input_manager_ = NULL;

	CleanUpFont();

	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	audio_manager->UnloadAllSamples();

}

bool SceneApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;

	if(input_manager_)
		input_manager_->Update();

	if (kb)
	{
		// returns current function false
		// closes app
		if (kb->IsKeyPressed(gef::Keyboard::KC_ESCAPE))
		{
			return false;
		}
	}

	// sets volume based on 
	// current active volume state

	switch (volume)
	{
	case ONE_HUNDRED:
		audio_manager->SetMasterVolume(100.0f);
		break;
	case SEVENTY_FIVE:
		audio_manager->SetMasterVolume(75.0f);
		break;
	case FIFTY:
		audio_manager->SetMasterVolume(50.0f);
		break;
	case TWENTY_FIVE:
		audio_manager->SetMasterVolume(25.0f);
		break;
	case ZERO:
		audio_manager->SetMasterVolume(0.0f);
		break;
	default:
		break;
	}
	
	UpdateGameStateMachine(frame_time);

	return isApplicationRunning;
}

void SceneApp::Render()
{
	
	RenderGameStateMachine();

}

gef::Scene* SceneApp::LoadSceneAssets(gef::Platform& platform, const char* filename)
{
	gef::Scene* scene = new gef::Scene();

	if (scene->ReadSceneFromFile(platform, filename))
	{
		// if scene file loads successful
		// create material and mesh resources from the scene data
		scene->CreateMaterials(platform);
		scene->CreateMeshes(platform);
	}
	else
	{
		delete scene;
		scene = NULL;
	}

	return scene;
}

gef::Mesh* SceneApp::GetMeshFromSceneAssets(gef::Scene* scene)
{
	gef::Mesh* mesh = NULL;

	// if the scene data contains at least one mesh
	// return the first mesh
	if (scene && scene->meshes.size() > 0)
		mesh = scene->meshes.front();

	return mesh;
}

void SceneApp::InitPlayer()
{
	// Initialiase player variables

	player_.setSize(gef::Vector4(0.5f,0.5f,0.0f));
	player_.setDashActive(false);
	player_.setDoubleJumpActive(false);
	player_.setResetWallActive(false);

	// changes starting health value
	// based on difficulty selected in options

	switch (difficulty)
	{
	case DIFF_EASY:
		player_.setHealth(10);
		break;
	case DIFF_NORMAL:
		player_.setHealth(5);
		break;
	case DIFF_HARD:
		player_.setHealth(3);
		break;
	case DIFF_ONESHOT:
		player_.setHealth(1);
		break;
	}

	// setup the mesh for the player
	player_.set_mesh(primitive_builder_->GetDefaultCubeMesh());

	// create a physics body for the player
	b2BodyDef player_body_def;
	player_body_def.type = b2_dynamicBody;
	player_body_def.position = b2Vec2(0.0f, 8.0f);
	player_body_def.fixedRotation = true;

	player_body_ = world_->CreateBody(&player_body_def);

	// create the shape for the player
	b2PolygonShape player_shape;
	player_shape.SetAsBox(player_.getSize().x(), player_.getSize().y());

	// create the fixture
	b2FixtureDef player_fixture_def;
	player_fixture_def.shape = &player_shape;
	player_fixture_def.density = 1.0f;

	// create the fixture on the rigid body
	player_body_->CreateFixture(&player_fixture_def);



	// update visuals from simulation data
	player_.UpdateFromSimulation(player_body_);

	// create a connection between the rigid body and GameObject
	player_body_->SetUserData(&player_);
}

void SceneApp::InitAllSpikes()
{
	// initialise spike type variable

	spikeObj.setSize(gef::Vector4(0.25f, 1.25f, 0.5f));

	// load in 3d model for all spikes
	// and set the 3d model to the spike mesh

	const char* scene_assest_filename = "spike.scn";
	
	scene_assets_ = LoadSceneAssets(platform_, scene_assest_filename);

	if (scene_assets_)
	{
		spikeObj.set_mesh(GetMeshFromSceneAssets(scene_assets_));
	}

	else
	{
		gef::DebugOut("Scene file %s failed to load\n", scene_assest_filename);
	}


	// push back values for spike obj and body vectors

	for (int i = 0; i < SPIKE_NUM; i++)
	{
		spikes_vec.push_back(spikeObj);
		spikes_bodies_vec.push_back(spike_body_);
	}

	// create physics bodies, shapes and fixtures for 
	// all spikes

	b2BodyDef spike_def[SPIKE_NUM];
	b2PolygonShape spikeShape[SPIKE_NUM];
	b2FixtureDef spike_fixture_def[SPIKE_NUM];

	// position each spike into the game level

	spike_def[0].position = b2Vec2(85.0f, 137.75f);
	spike_def[1].position = b2Vec2(88.0f, 186.65f);
	spike_def[2].position = b2Vec2(68.0f, 186.65f);
	spike_def[3].position = b2Vec2(68.0f, 284.25f);
	spike_def[4].position = b2Vec2(-17.0f, 284.25f);
	spike_def[5].position = b2Vec2(23.0f, 290.0f);
	spike_def[6].position = b2Vec2(175.0f, 146.5f);
	spike_def[7].position = b2Vec2(149.0f, 135.1f);
	spike_def[8].position = b2Vec2(129.5f, 134.15f);
	spike_def[9].position = b2Vec2(-125.0f, 9.05f);
	spike_def[10].position = b2Vec2(-181.0f, 24.0f);
	spike_def[11].position = b2Vec2(-95.5f, 24.0f);
	spike_def[12].position = b2Vec2(-34.5f, 70.0f);


	spike_def[13].position = b2Vec2(-136.25f, 113.0f);
	spike_def[13].angle = gef::DegToRad(90);

	spike_def[14].position = b2Vec2(-176.0f, 144.0f);

	spike_def[15].position = b2Vec2(-187.0f, 200.0f);
	spike_def[16].position = b2Vec2(-174.0f, 216.0f);
	spike_def[17].position = b2Vec2(-134.0f, 216.0f);

	spike_def[18].position = b2Vec2(-105.0f, 214.0f);
	spike_def[19].position = b2Vec2(-80.0f, 214.0f);
	spike_def[20].position = b2Vec2(-70.0f, 214.0f);
	spike_def[21].position = b2Vec2(-60.0f, 214.0f);
	spike_def[22].position = b2Vec2(-35.0f, 214.0f);

	spike_def[23].position = b2Vec2(-80.0f, 230.0f);
	spike_def[24].position = b2Vec2(-60.0f, 230.0f);

	spike_def[25].position = b2Vec2(-105.0f, 258.0f);
	spike_def[26].position = b2Vec2(-130.0f, 258.0f);
	spike_def[27].position = b2Vec2(-160.0f, 258.0f);
	spike_def[28].position = b2Vec2(-175.5f, 288.0f);
	spike_def[29].position = b2Vec2(-125.0f, 289.25f);

	spike_def[30].position = b2Vec2(111.0f, 2.15f);
	spike_def[31].position = b2Vec2(116.0f, 2.15f);
	spike_def[32].position = b2Vec2(121.0f, 2.15f);
	spike_def[33].position = b2Vec2(126.0f, 2.15f);
	spike_def[34].position = b2Vec2(131.0f, 2.15f);
	spike_def[35].position = b2Vec2(136.0f, 2.15f);
	spike_def[36].position = b2Vec2(141.0f, 2.15f);
	spike_def[37].position = b2Vec2(146.0f, 2.15f);
	spike_def[38].position = b2Vec2(151.0f, 2.15f);
	spike_def[39].position = b2Vec2(156.0f, 2.15f);

	for (int i = 0; i < SPIKE_NUM; i++)
	{
		spike_def[i].type = b2_staticBody;

		spikes_bodies_vec[i] = world_->CreateBody(&spike_def[i]);

		// set size of physics body

		spikeShape[i].SetAsBox(spikeObj.getSize().x(), spikeObj.getSize().y());

		// fixture settings

		spike_fixture_def[i].shape = &spikeShape[i];
		spike_fixture_def[i].density = 3.0f;
		spike_fixture_def[i].restitution = 1.0f;

		spikes_bodies_vec[i]->CreateFixture(&spike_fixture_def[i]);


		// more initialising spike variable values

		spikes_vec[i].setRotate(90);
		spikes_vec[i].setOffsetXValue(0.0f);
		spikes_vec[i].setOffsetYValue(2.0f);

		// individual spike variable values 
		// for spikes needing different values
		
		spikes_vec[13].setRotate(180);
		spikes_vec[13].setOffsetXValue(-1.5f);
		spikes_vec[13].setOffsetYValue(-0.8f);
		spikes_vec[8].setRotate(-90);
		spikes_vec[8].setOffsetYValue(-2.0f);
		spikes_vec[28].setRotate(-90);
		spikes_vec[28].setOffsetYValue(-1.75f);

		// update visuals from simulation data
		// using unique spike version
		spikes_vec[i].UpdateFromSimulationSpike(spikes_bodies_vec[i]);

		// create a connection between the rigid body and GameObject
		spikes_bodies_vec[i]->SetUserData(&spikes_vec[i]);

	}

}

void SceneApp::InitCollectables()
{
	// initialising collectable type variable

	collectable_.set_type(COLLECTABLE);

	// load in 3d model for the collectable
	// and set the 3d model to the collectable mesh

	const char* scene_assest_filename = "morph-ball.scn";

	scene_assets_ = LoadSceneAssets(platform_, scene_assest_filename);

	if (scene_assets_)
	{
		collectable_.set_mesh(GetMeshFromSceneAssets(scene_assets_));
	}

	else
	{
		gef::DebugOut("Scene file %s failed to load\n", scene_assest_filename);
	}

	gef::Vector4 collectable_half_dimensions(1.75f, 1.75, 2.0f);

	// create a physics body for the collectable
	b2BodyDef collectable_body_def;
	collectable_body_def.type = b2_staticBody;
	collectable_body_def.position = b2Vec2(-185.0f, 294.0f);

	collectable_body_ = world_->CreateBody(&collectable_body_def);

	// create the shape for the collectable

	b2PolygonShape collectable_shape;
	collectable_shape.SetAsBox(collectable_half_dimensions.x(), collectable_half_dimensions.y());

	// create the fixture
	b2FixtureDef collectable_fixture_def;
	collectable_fixture_def.shape = &collectable_shape;
	collectable_fixture_def.isSensor = true;

	// create the fixture on the rigid body
	collectable_body_->CreateFixture(&collectable_fixture_def);

	// update visuals from simulation data
	// using unique collectable version

	collectable_.UpdateFromSimulationCollectable(collectable_body_);

	// create a connection between the rigid body and GameObject
	collectable_body_->SetUserData(&collectable_);
}

void SceneApp::InitAllAbilities()
{
	// initialising dash type variable

	dashPickup_.set_type(ABILITY_DASH);

	gef::Vector4 dash_half_dimensions(1.0f, 1.0, 1.0f);

	// setting mesh and mesh values for dash object

	dashPickupMesh = primitive_builder_->CreateSphereMesh(1.0f, 15.0f, 15.0f, dash_half_dimensions); 

	dashPickup_.set_mesh(dashPickupMesh);

	// create a physics body for the dash object
	b2BodyDef dash_body_def;
	dash_body_def.type = b2_staticBody;
	dash_body_def.position = b2Vec2(174.0f, 20.0f);

	dashPickup_body_ = world_->CreateBody(&dash_body_def);

	// create the shape for the dash object

	b2PolygonShape dash_shape;
	dash_shape.SetAsBox(dash_half_dimensions.x(), dash_half_dimensions.y());

	// create the fixture
	b2FixtureDef dash_fixture_def;
	dash_fixture_def.shape = &dash_shape;
	dash_fixture_def.isSensor = true;

	// create the fixture on the rigid body
	dashPickup_body_->CreateFixture(&dash_fixture_def);

	// update visuals from simulation data
	dashPickup_.UpdateFromSimulation(dashPickup_body_);

	// create a connection between the rigid body and GameObject
	dashPickup_body_->SetUserData(&dashPickup_);

	
	// initialise double jump type variable

	doubleJumpPickup_.set_type(ABILITY_DJ);

	gef::Vector4 doubleJump_half_dimensions(1.0f, 1.0, 1.0f);

	// setting mesh and mesh values for double jump object

	doubleJumpPickupMesh = primitive_builder_->CreateSphereMesh(1.0f, 15.0f, 15.0f, doubleJump_half_dimensions); 

	doubleJumpPickup_.set_mesh(doubleJumpPickupMesh);

	// create a physics body for the double jump object
	b2BodyDef doubleJump_body_def;
	doubleJump_body_def.type = b2_staticBody;
	doubleJump_body_def.position = b2Vec2(-51.0f, 33.0f);

	doubleJumpPickup_body_ = world_->CreateBody(&doubleJump_body_def);

	// create the shape for the double jump object

	b2PolygonShape doubleJump_shape;
	doubleJump_shape.SetAsBox(doubleJump_half_dimensions.x(), doubleJump_half_dimensions.y());

	// create the fixture
	b2FixtureDef doubleJump_fixture_def;
	doubleJump_fixture_def.shape = &doubleJump_shape;
	doubleJump_fixture_def.isSensor = true;

	// create the fixture on the rigid body
	doubleJumpPickup_body_->CreateFixture(&doubleJump_fixture_def);

	// update visuals from simulation data
	doubleJumpPickup_.UpdateFromSimulation(doubleJumpPickup_body_);

	// create a connection between the rigid body and GameObject
	doubleJumpPickup_body_->SetUserData(&doubleJumpPickup_);


	// initialising reset wall type variable

	resetWallPickup_.set_type(ABILITY_RW);

	gef::Vector4 resetWall_half_dimensions(1.0f, 1.0, 1.0f);

	// setting mesh and mesh values for reset wall object

	resetWallPickupMesh = primitive_builder_->CreateSphereMesh(1.0f, 15.0f, 15.0f, resetWall_half_dimensions); 

	resetWallPickup_.set_mesh(resetWallPickupMesh);

	// create a physics body for the reset wall object
	b2BodyDef resetWall_body_def;
	resetWall_body_def.type = b2_staticBody;
	resetWall_body_def.position = b2Vec2(-71.0f, 201.0f);

	resetWallPickup_body_ = world_->CreateBody(&resetWall_body_def);

	// create the shape for the reset wall object

	b2PolygonShape resetWall_shape;
	resetWall_shape.SetAsBox(resetWall_half_dimensions.x(), resetWall_half_dimensions.y());

	// create the fixture
	b2FixtureDef resetWall_fixture_def;
	resetWall_fixture_def.shape = &resetWall_shape;
	resetWall_fixture_def.isSensor = true;

	// create the fixture on the rigid body
	resetWallPickup_body_->CreateFixture(&resetWall_fixture_def);



	// update visuals from simulation data
	resetWallPickup_.UpdateFromSimulation(resetWallPickup_body_);

	// create a connection between the rigid body and GameObject
	resetWallPickup_body_->SetUserData(&resetWallPickup_);
}

void SceneApp::InitBottomBorder()
{
	// ground dimensions
	bottom_border_.set_type(GROUND);


	gef::Vector4 bottom_border_half_dimensions(200.0f, 0.5f, 0.5f);

	// setup the mesh for the ground
	bottom_border_mesh_ = primitive_builder_->CreateBoxMesh(bottom_border_half_dimensions);
	bottom_border_.set_mesh(bottom_border_mesh_);

	// create a physics body
	b2BodyDef bottom_border_def;
	bottom_border_def.type = b2_staticBody;
	bottom_border_def.position = b2Vec2(0.0f, 0.0f);


	bottom_border_body = world_->CreateBody(&bottom_border_def);

	// create the shape
	b2PolygonShape bottomBorderShape;
	bottomBorderShape.SetAsBox(bottom_border_half_dimensions.x(), bottom_border_half_dimensions.y());

	// create the fixture
	b2FixtureDef bottom_border_fixture_def;
	bottom_border_fixture_def.shape = &bottomBorderShape;
	bottom_border_fixture_def.friction = 1.25f;

	// create the fixture on the rigid body
	bottom_border_body->CreateFixture(&bottom_border_fixture_def);

	// update visuals from simulation data
	bottom_border_.UpdateFromSimulation(bottom_border_body);

	bottom_border_body->SetUserData(&bottom_border_);
}

void SceneApp::InitTopBorder()
{
	// roof dimensions
	top_border_.set_type(GROUND);


	gef::Vector4 top_border_half_dimensions(200.0f, 0.5f, 0.5f);

	// setup the mesh for the roof
	top_border_mesh_ = primitive_builder_->CreateBoxMesh(top_border_half_dimensions);
	top_border_.set_mesh(top_border_mesh_);

	// create a physics body
	b2BodyDef top_border_def;
	top_border_def.type = b2_staticBody;
	top_border_def.position = b2Vec2(0.0f, 300.0f);


	top_border_body = world_->CreateBody(&top_border_def);

	// create the shape
	b2PolygonShape topBorderShape;
	topBorderShape.SetAsBox(top_border_half_dimensions.x(), top_border_half_dimensions.y());

	// create the fixture
	b2FixtureDef top_border_fixture_def;
	top_border_fixture_def.shape = &topBorderShape;

	// create the fixture on the rigid body
	top_border_body->CreateFixture(&top_border_fixture_def);

	// update visuals from simulation data
	top_border_.UpdateFromSimulation(top_border_body);

	top_border_body->SetUserData(&top_border_);
}

void SceneApp::InitRightBorder()
{
	// wall dimensions
	right_border_.set_type(WALL);


	gef::Vector4 right_border_half_dimensions(0.5f, 150.0f, 0.5f);

	// setup the mesh for the right wall
	right_border_mesh_ = primitive_builder_->CreateBoxMesh(right_border_half_dimensions);
	right_border_.set_mesh(right_border_mesh_);

	// create a physics body
	b2BodyDef right_border_def;
	right_border_def.type = b2_staticBody;
	right_border_def.position = b2Vec2(200.0f, 150.0f);


	right_border_body_ = world_->CreateBody(&right_border_def);

	// create the shape
	b2PolygonShape rightBorderShape;
	rightBorderShape.SetAsBox(right_border_half_dimensions.x(), right_border_half_dimensions.y());

	// create the fixture
	b2FixtureDef right_border_fixture_def;
	right_border_fixture_def.shape = &rightBorderShape;
	right_border_fixture_def.friction = 0.0f;

	// create the fixture on the rigid body
	right_border_body_->CreateFixture(&right_border_fixture_def);

	// update visuals from simulation data
	right_border_.UpdateFromSimulation(right_border_body_);

	right_border_body_->SetUserData(&right_border_);
}

void SceneApp::InitLeftBorder()
{
	// wall dimensions
	left_border_.set_type(WALL);


	gef::Vector4 left_border_half_dimensions(0.5f, 150.0f, 0.5f);

	// setup the mesh for the left wall
	left_border_mesh_ = primitive_builder_->CreateBoxMesh(left_border_half_dimensions);
	left_border_.set_mesh(left_border_mesh_);

	// create a physics body
	b2BodyDef left_border_def;
	left_border_def.type = b2_staticBody;
	left_border_def.position = b2Vec2(-200.0f, 150.0f);


	left_border_body_ = world_->CreateBody(&left_border_def);

	// create the shape
	b2PolygonShape leftBorderShape;
	leftBorderShape.SetAsBox(left_border_half_dimensions.x(), left_border_half_dimensions.y());

	// create the fixture
	b2FixtureDef left_border_fixture_def;
	left_border_fixture_def.shape = &leftBorderShape;
	left_border_fixture_def.friction = 0.0f;

	// create the fixture on the rigid body
	left_border_body_->CreateFixture(&left_border_fixture_def);

	// update visuals from simulation data
	left_border_.UpdateFromSimulation(left_border_body_);

	left_border_body_->SetUserData(&left_border_);
}

void SceneApp::InitSmallPlatformsSA()
{
	// initialising small platforms type variable

	small_platform.set_type(GROUND);

	gef::Vector4 small_platform_half_dimensions(5.0f, 0.5, 0.5f);

	// setting mesh and mesh values for small platforms

	small_platform_mesh_SA = primitive_builder_->CreateBoxMesh(small_platform_half_dimensions);
	small_platform.set_mesh(small_platform_mesh_SA);

	// push back values for small platform obj and body vectors

	for (int i = 0; i < SMALL_PLATFORM_NUM_SA; i++)
	{
		small_platforms_SA.push_back(small_platform);
		small_platform_bodies_SA.push_back(small_platform_body);
	}

	// create physics bodies, shapes and fixtures for 
	// all small platforms

	b2BodyDef small_platform_def[SMALL_PLATFORM_NUM_SA];
	b2PolygonShape smallPlatformShape[SMALL_PLATFORM_NUM_SA];
	b2FixtureDef small_platform_fixture_def[SMALL_PLATFORM_NUM_SA];

	// positioning all small platforms in game level

	// plats to middle area - on way to dash ability
	small_platform_def[0].position = b2Vec2(15.0f,4.0f);
	small_platform_def[1].position = b2Vec2(30.0f, 9.0f);
	small_platform_def[2].position = b2Vec2(48.0f, 13.0f);
	small_platform_def[3].position = b2Vec2(67.0f, 14.0f);
	small_platform_def[25].position = b2Vec2(75.0f, 19.0f);
	small_platform_def[26].position = b2Vec2(80.0f, 24.0f);
	small_platform_def[27].position = b2Vec2(87.0f, 29.0f);
	small_platform_def[28].position = b2Vec2(100.0f, 34.0f);
	small_platform_def[29].position = b2Vec2(85.0f, 39.0f);
	small_platform_def[30].position = b2Vec2(100.0f, 44.0f);
	small_platform_def[31].position = b2Vec2(85.0f, 49.0f);
	small_platform_def[32].position = b2Vec2(100.0f, 54.0f);
	small_platform_def[37].position = b2Vec2(100.0f, 100.0f);
	small_platform_def[38].position = b2Vec2(167.0f, 85.0f);

	// dash area
	small_platform_def[4].position = b2Vec2(105.0f, 4.0f);
	small_platform_def[22].position = b2Vec2(180.0f, 40.0f);
	small_platform_def[23].position = b2Vec2(150.0f, 18.0f);
	small_platform_def[24].position = b2Vec2(175.0f, 15.0f);

	// double jump area - alternate entrance to middle area
	small_platform_def[5].position = b2Vec2(-80.0f, 4.0f);
	small_platform_def[6].position = b2Vec2(-100.0f, 4.0f);
	small_platform_def[7].position = b2Vec2(-125.0f, 7.0f);
	small_platform_def[8].position = b2Vec2(-148.0f, 12.0f);
	small_platform_def[9].position = b2Vec2(-160.0f, 17.0f);
	small_platform_def[10].position = b2Vec2(-183.0f, 22.0f);
	small_platform_def[11].position = b2Vec2(-190.0f, 40.0f);
	small_platform_def[12].position = b2Vec2(-185.0f, 55.0f);
	small_platform_def[13].position = b2Vec2(-165.0f, 55.0f);
	small_platform_def[14].position = b2Vec2(-175.0f, 65.0f);

	// double jump area
	small_platform_def[15].position = b2Vec2(-137.0f, 22.0f);
	small_platform_def[16].position = b2Vec2(-115.0f, 27.0f);
	small_platform_def[17].position = b2Vec2(-93.0f, 22.0f);
	small_platform_def[18].position = b2Vec2(-80.0f, 25.0f);
	small_platform_def[19].position = b2Vec2(-50.0f, 29.0f);
	small_platform_def[20].position = b2Vec2(-35.0f, 47.5f);
	small_platform_def[21].position = b2Vec2(-10.0f, 25.0f);
	
	// alternate entrance of middle area - leading to final area
	small_platform_def[33].position = b2Vec2(-140.0f, 95.0f);
	small_platform_def[34].position = b2Vec2(-175.0f, 105.0f);
	small_platform_def[35].position = b2Vec2(-155.0f, 125.0f);
	small_platform_def[36].position = b2Vec2(-190.0f, 140.0f);

	// short path leading to top of middle area
	small_platform_def[39].position = b2Vec2(40.0f,85.0f);
	small_platform_def[40].position = b2Vec2(30.0f,89.0f);
	small_platform_def[41].position = b2Vec2(20.0f,85.0f);
	small_platform_def[42].position = b2Vec2(45.0f,94.0f);
	small_platform_def[43].position = b2Vec2(60.0f,99.0f);
	small_platform_def[44].position = b2Vec2(50.0f,104.0f);
	small_platform_def[45].position = b2Vec2(120.0f, 120.0f);
	small_platform_def[46].position = b2Vec2(138.0f, 128.0f);
	small_platform_def[47].position = b2Vec2(148.0f,133.0f);
	
	// area around / inside Reset wall ability
	small_platform_def[48].position = b2Vec2(-130.0f, 190.0f);
	small_platform_def[49].position = b2Vec2(-104.5f, 185.0f);
	small_platform_def[50].position = b2Vec2(-85.0f, 170.0f);
	small_platform_def[51].position = b2Vec2(-50.0f, 163.0f);
	small_platform_def[52].position = b2Vec2(-40.0f, 168.0f);
	small_platform_def[53].position = b2Vec2(-35.0f, 173.0f);
	small_platform_def[54].position = b2Vec2(-35.0f, 188.0f);
	small_platform_def[55].position = b2Vec2(-70.0f, 235.0f);
	small_platform_def[56].position = b2Vec2(-50.0f, 220.0f);
	small_platform_def[57].position = b2Vec2(-90.0f, 220.0f);
	small_platform_def[58].position = b2Vec2(-90.0f, 253.5f);
	small_platform_def[59].position = b2Vec2(-187.5f, 273.0f);
	small_platform_def[60].position = b2Vec2(-163.0f, 285.0f);
	small_platform_def[61].position = b2Vec2(-114.0f, 250.0f);
	small_platform_def[62].position = b2Vec2(-144.0f, 250.0f);

	// right side entrance to final area
	small_platform_def[63].position = b2Vec2(160.0f, 140.0f);
	small_platform_def[64].position = b2Vec2(175.0f, 145.0f);
	small_platform_def[65].position = b2Vec2(190.0f, 150.0f);
	small_platform_def[66].position = b2Vec2(175.0f, 160.0f);

	// dash route to RS final area
	small_platform_def[67].position = b2Vec2(150.0f, 160.0f);
	small_platform_def[68].position = b2Vec2(125.0f, 163.0f);
	small_platform_def[69].position = b2Vec2(105.0f, 168.0f);
	small_platform_def[70].position = b2Vec2(85.0f, 173.0f);
	small_platform_def[71].position = b2Vec2(55.0f, 170.0f);
	small_platform_def[72].position = b2Vec2(35.0f, 170.0f);
	small_platform_def[73].position = b2Vec2(15.0f, 175.0f);
	small_platform_def[74].position = b2Vec2(25.0f, 180.0f);

	// vSmall plat path to RS final area
	small_platform_def[75].position = b2Vec2(175.0f, 175.0f);

	// moving plats jump puzzle area - right
	small_platform_def[76].position = b2Vec2(88.5f, 215.0f);
	small_platform_def[77].position = b2Vec2(100.0f, 220.0f);

	// moving plats jump puzzle area - left
	small_platform_def[78].position = b2Vec2(55.0f, 220.0f);
	small_platform_def[79].position = b2Vec2(75.0f, 225.0f);
	small_platform_def[80].position = b2Vec2(15.0f, 235.0f);
	small_platform_def[81].position = b2Vec2(35.0f, 240.0f);

	// small path to last area of map with no / limited abilities
	small_platform_def[82].position = b2Vec2(88.0f, 260.0f);
	small_platform_def[83].position = b2Vec2(78.0f, 264.0f);
	small_platform_def[84].position = b2Vec2(88.0f, 268.0f);
	small_platform_def[85].position = b2Vec2(78.0f, 272.0f);
	small_platform_def[86].position = b2Vec2(88.0f, 276.0f);
	small_platform_def[87].position = b2Vec2(78.0f, 280.0f);

	// last jumping area of map before end of level
	small_platform_def[88].position = b2Vec2(-50.0f, 288.0f);
	small_platform_def[89].position = b2Vec2(-70.0f, 292.0f);
	small_platform_def[91].position = b2Vec2(-100.0f, 287.0f);

	small_platform_def[90].position = b2Vec2(-80.0f, 295.0f);

	small_platform_def[92].position = b2Vec2(-125.0f, 287.0f);

	small_platform_def[93].position = b2Vec2(-40.0f, 278.0f);
	small_platform_def[94].position = b2Vec2(-155.0f, 290.0f);

	for (int i = 0; i < SMALL_PLATFORM_NUM_SA; i++)
	{
		small_platform_def[i].type = b2_staticBody;
		small_platform_def[i].allowSleep = true;
		small_platform_def[i].awake = false;
		

		small_platform_bodies_SA[i] = world_->CreateBody(&small_platform_def[i]);


		// set size of physics body
		smallPlatformShape[i].SetAsBox(small_platform_half_dimensions.x(),small_platform_half_dimensions.y());

		// fixture settings
		small_platform_fixture_def[i].shape = &smallPlatformShape[i];
		small_platform_fixture_def[i].friction = 1.5f;
		
		

		small_platform_bodies_SA[i]->CreateFixture(&small_platform_fixture_def[i]);

		// update visuals from simulation data
		small_platforms_SA[i].UpdateFromSimulation(small_platform_bodies_SA[i]);

		// create a connection between the rigid body and GameObject
		small_platform_bodies_SA[i]->SetUserData(&small_platforms_SA[i]);

	}


}

void SceneApp::InitMeduimPlatformsMA()
{
	// initialisng medium platform type variable

	medium_platform.set_type(GROUND);

	gef::Vector4 medium_platform_half_dimensions(12.5f, 0.5, 0.5f);

	// setting mesh and mesh values for medium platforms

	medium_platform_mesh_MA = primitive_builder_->CreateBoxMesh(medium_platform_half_dimensions);
	medium_platform.set_mesh(medium_platform_mesh_MA);

	// push back values for medium platform obj and body vectors

	for (int i = 0; i < MEDIUM_PLATFORM_NUM_MA; i++)
	{
		medium_platforms_MA.push_back(medium_platform);
		medium_platform_bodies_MA.push_back(medium_platform_body);
	}

	// create physics bodies, shapes and fixtures for 
	// all medium platforms

	b2BodyDef medium_platform_def[MEDIUM_PLATFORM_NUM_MA];
	b2PolygonShape mediumPlatformShape[MEDIUM_PLATFORM_NUM_MA];
	b2FixtureDef medium_platform_fixture_def[MEDIUM_PLATFORM_NUM_MA];

	// bottom level of middle area

	medium_platform_def[0].position = b2Vec2(-147.5f, 80.0f);
	medium_platform_def[1].position = b2Vec2(-121.5f, 80.0f);
	medium_platform_def[2].position = b2Vec2(-70.0f, 80.0f);
	medium_platform_def[3].position = b2Vec2(60.0f, 80.0f);
	medium_platform_def[4].position = b2Vec2(125.0f, 80.0f);
	medium_platform_def[5].position = b2Vec2(155.0f, 80.0f);

	// path to top-right area
	medium_platform_def[6].position = b2Vec2(70.0f, 110.0f);
	medium_platform_def[7].position = b2Vec2(100.0f, 115.0f);
	medium_platform_def[8].position = b2Vec2(140.0f, 125.0f);


	// final path area
	medium_platform_def[9].position = b2Vec2(83.0f, 255.5f);
	medium_platform_def[10].position = b2Vec2(-70.0f, 195.5f);
	medium_platform_def[11].position = b2Vec2(-187.5f, 290.0f);

	for (int i = 0; i < MEDIUM_PLATFORM_NUM_MA; i++)
	{
		medium_platform_def[i].type = b2_staticBody;
		medium_platform_def[i].allowSleep = true;
		medium_platform_def[i].awake = false;

		medium_platform_bodies_MA[i] = world_->CreateBody(&medium_platform_def[i]);

		// set size of physics body

		mediumPlatformShape[i].SetAsBox(medium_platform_half_dimensions.x(), medium_platform_half_dimensions.y());

		// fixture settings

		medium_platform_fixture_def[i].shape = &mediumPlatformShape[i];
		medium_platform_fixture_def[i].friction = 1.3f;

		medium_platform_bodies_MA[i]->CreateFixture(&medium_platform_fixture_def[i]);

		// update visuals from simulation data
		medium_platforms_MA[i].UpdateFromSimulation(medium_platform_bodies_MA[i]);

		// create a connection between the rigid body and GameObject
		medium_platform_bodies_MA[i]->SetUserData(&medium_platforms_MA[i]);

	}
}

void SceneApp::InitBigPlatforms()
{
	// initialising big platform type variable

	big_platform.set_type(GROUND);

	gef::Vector4 big_platform_half_dimensions(50.0f, 0.5, 0.5f);


	// setting mesh and mesh values for big platforms

	big_platform_mesh_ = primitive_builder_->CreateBoxMesh(big_platform_half_dimensions);
	big_platform.set_mesh(big_platform_mesh_);


	// push back values for big platform obj and body vectors

	for (int i = 0; i < BIG_PLATFORM_NUM; i++)
	{
		big_platforms_vec.push_back(big_platform);
		big_platform_bodies_vec.push_back(big_platform_body);
	}


	// create physics bodies, shapes and fixtures for 
	// all big platforms

	b2BodyDef big_platform_def[BIG_PLATFORM_NUM];
	b2PolygonShape bigPlatformShape[BIG_PLATFORM_NUM];
	b2FixtureDef big_platform_fixture_def[BIG_PLATFORM_NUM];

	
	// positions of all big platforms

	big_platform_def[0].position = b2Vec2(83.0f,136.0f);
	big_platform_def[1].position = b2Vec2(-70.0f, 205.0f);
	big_platform_def[2].position = b2Vec2(83.0f, 185.0f);
	big_platform_def[3].position = b2Vec2(23.0f, 282.5f);

	for (int i = 0; i < BIG_PLATFORM_NUM; i++)
	{
		big_platform_def[i].type = b2_staticBody;
		big_platform_def[i].allowSleep = true;
		big_platform_def[i].awake = false;

		big_platform_bodies_vec[i] = world_->CreateBody(&big_platform_def[i]);

		// set size of physics body

		bigPlatformShape[i].SetAsBox(big_platform_half_dimensions.x(), big_platform_half_dimensions.y());

		// fixture settings

		big_platform_fixture_def[i].shape = &bigPlatformShape[i];
		big_platform_fixture_def[i].friction = 1.15f;

		big_platform_bodies_vec[i]->CreateFixture(&big_platform_fixture_def[i]);


		// update visuals from simulation data
		big_platforms_vec[i].UpdateFromSimulation(big_platform_bodies_vec[i]);

		// create a connection between the rigid body and GameObject
		big_platform_bodies_vec[i]->SetUserData(&big_platforms_vec[i]);

	}
}

void SceneApp::InitMovingPlatforms()
{
	gef::Vector4 movPlatform_half_dimensions(5.0f, 0.5, 0.5f);

	// setting mesh and mesh values for moving platforms

	movPlatform_mesh = primitive_builder_->CreateBoxMesh(movPlatform_half_dimensions);
	movPlatform.set_mesh(movPlatform_mesh);

	// push back values for moving platform obj and body vectors

	for (int i = 0; i < MOVING_PLATFORM_NUM; i++)
	{
		movPlatformsVec.push_back(movPlatform);
		movPlatform_bodies_vec.push_back(movPlatform_body);
	}

	// create physics bodies, shapes and fixtures for 
	// all moving platforms

	b2BodyDef movPlatform_def[MOVING_PLATFORM_NUM];
	b2PolygonShape movPlatformShape[MOVING_PLATFORM_NUM];
	b2FixtureDef movPlatform_fixture_def[MOVING_PLATFORM_NUM];

	// start and middle area 

	movPlatform_def[0].position = b2Vec2(83.75f, 52.0f);
	movPlatform_def[1].position = b2Vec2(25.0f,136.0f);
	movPlatform_def[2].position = b2Vec2(-65.0f,150.0f);
	movPlatform_def[3].position = b2Vec2(-95.0f, 90.0f);

	// right side of top right area

	movPlatform_def[4].position = b2Vec2(100.0f, 187.5f);
	movPlatform_def[5].position = b2Vec2(112.5f, 245.0f);
	movPlatform_def[6].position = b2Vec2(125.0f, 230.0f);
	movPlatform_def[7].position = b2Vec2(137.5f, 255.0f);
	movPlatform_def[8].position = b2Vec2(150.0f, 240.0f);
	movPlatform_def[9].position = b2Vec2(162.5f, 265.0f);

	// left side of top right area

	movPlatform_def[10].position = b2Vec2(75.0f, 187.5f);
	movPlatform_def[11].position = b2Vec2(55.0f, 230.0f);
	movPlatform_def[12].position = b2Vec2(45.0f, 245.0f);

	for (int i = 0; i < MOVING_PLATFORM_NUM; i++)
	{
		movPlatform_def[i].type = b2_kinematicBody;
		movPlatform_def[i].allowSleep = true;
		movPlatform_def[i].awake = false;

		movPlatform_bodies_vec[i] = world_->CreateBody(&movPlatform_def[i]);

		// set size of physics body

		movPlatformShape[i].SetAsBox(movPlatform_half_dimensions.x(), movPlatform_half_dimensions.y());

		// fixture settings

		movPlatform_fixture_def[i].shape = &movPlatformShape[i];
		movPlatform_fixture_def[i].friction = 1.5f;

		movPlatform_bodies_vec[i]->CreateFixture(&movPlatform_fixture_def[i]);

		// update visuals from simulation data
		movPlatformsVec[i].UpdateFromSimulation(movPlatform_bodies_vec[i]);

		// create a connection between the rigid body and GameObject
		movPlatform_bodies_vec[i]->SetUserData(&movPlatformsVec[i]);

	}

}

void SceneApp::InitVerySmallPlats()
{
	// initialise v-small plat type variable

	very_small_plat_.set_type(WALL);

	gef::Vector4 very_small_platform_half_dimensions(1.75f, 1.0, 0.5f);


	// setting mesh and mesh values for v-small platforms

	very_small_plat_mesh_ = primitive_builder_->CreateBoxMesh(very_small_platform_half_dimensions);
	very_small_plat_.set_mesh(very_small_plat_mesh_);


	// push back values for v-small platform obj and body vectors

	for (int i = 0; i < VERY_SMALL_PLATFORM_NUM; i++)
	{
		very_small_plat_vec.push_back(very_small_plat_);
		very_small_plat_bodies_vec.push_back(very_small_plat_body_);
	}

	// create physics bodies, shapes and fixtures for 
	// all v-small platforms

	b2BodyDef very_small_platform_def[VERY_SMALL_PLATFORM_NUM];
	b2PolygonShape verySmallPlatformShape[VERY_SMALL_PLATFORM_NUM];
	b2FixtureDef very_small_platform_fixture_def[VERY_SMALL_PLATFORM_NUM];

	// middle area - double jump early jump puzzle

	very_small_platform_def[0].position = b2Vec2(10.0f,90.0f);
	very_small_platform_def[1].position = b2Vec2(7.0f, 95.0f);
	very_small_platform_def[2].position = b2Vec2(10.0f, 100.0f);
	very_small_platform_def[3].position = b2Vec2(5.0f, 105.0f);
	very_small_platform_def[4].position = b2Vec2(0.0f, 100.0f);
	very_small_platform_def[5].position = b2Vec2(-10.0f, 85.0f);
	very_small_platform_def[6].position = b2Vec2(-15.0f, 90.0f);
	very_small_platform_def[7].position = b2Vec2(-15.0f, 95.0f);
	very_small_platform_def[8].position = b2Vec2(-25.0f, 90.0f);
	very_small_platform_def[9].position = b2Vec2(-25.0f, 100.0f);

	// small path to get reset wall ability
	// without double jump / dash

	very_small_platform_def[10].position = b2Vec2(-45.0f, 178.0f);
	very_small_platform_def[11].position = b2Vec2(-55.0f, 183.0f);
	very_small_platform_def[12].position = b2Vec2(-45.0f, 188.0f);
	very_small_platform_def[13].position = b2Vec2(-35.0f, 193.0f);
	very_small_platform_def[14].position = b2Vec2(-40.0f, 198.0f);
	very_small_platform_def[15].position = b2Vec2(-50.0f, 198.0f);
	

	// right side path way to top right area

	very_small_platform_def[16].position = b2Vec2(195.0f, 154.0f);
	very_small_platform_def[17].position = b2Vec2(190.0f, 159.0f);
	very_small_platform_def[18].position = b2Vec2(170.0f, 165.0f);
	very_small_platform_def[19].position = b2Vec2(165.0f, 170.0f);
	very_small_platform_def[20].position = b2Vec2(165.0f, 180.0f);
	very_small_platform_def[21].position = b2Vec2(155.0f, 185.0f);
	very_small_platform_def[22].position = b2Vec2(145.0f, 190.0f);
	very_small_platform_def[30].position = b2Vec2(155.0f, 137.5f);

	// final area pathway - for players without abilities

	very_small_platform_def[23].position = b2Vec2(-148.0f, 284.5f);
	very_small_platform_def[24].position = b2Vec2(-137.0f, 284.5f);
	very_small_platform_def[25].position = b2Vec2(-113.0f, 284.5f);
	very_small_platform_def[26].position = b2Vec2(-88.0f, 284.5f);
	very_small_platform_def[27].position = b2Vec2(-55.0f, 280.0f);
	very_small_platform_def[28].position = b2Vec2(-65.0f, 280.0f);
	very_small_platform_def[29].position = b2Vec2(-75.0f, 282.5f);



	for (int i = 0; i < VERY_SMALL_PLATFORM_NUM; i++)
	{
		very_small_platform_def[i].type = b2_staticBody;
		very_small_platform_def[i].allowSleep = true;
		very_small_platform_def[i].awake = false;

		very_small_plat_bodies_vec[i] = world_->CreateBody(&very_small_platform_def[i]);

		// set size of physics body

		verySmallPlatformShape[i].SetAsBox(very_small_platform_half_dimensions.x(), very_small_platform_half_dimensions.y());

		// fixture settings

		very_small_platform_fixture_def[i].shape = &verySmallPlatformShape[i];
		very_small_platform_fixture_def[i].friction = 2.0f;

		very_small_plat_bodies_vec[i]->CreateFixture(&very_small_platform_fixture_def[i]);


		// update visuals from simulation data
		very_small_plat_vec[i].UpdateFromSimulation(very_small_plat_bodies_vec[i]);

		// create a connection between the rigid body and GameObject
		very_small_plat_bodies_vec[i]->SetUserData(&very_small_plat_vec[i]);

	}
}

void SceneApp::InitBlockingWalls()
{
	// initialising blocking wall type variable

	blocking_wall_.set_type(WALL);

	gef::Vector4 blocking_wall_half_dimensions(0.45, 2.48, 0.45f);

	// setting mesh and mesh values for blocking walls

	blocking_wall_mesh_ = primitive_builder_->CreateBoxMesh(blocking_wall_half_dimensions);
	blocking_wall_.set_mesh(blocking_wall_mesh_);

	// push back values for blocking wall obj and body vectors

	for (int i = 0; i < BLOCKING_WALL_NUM; i++)
	{
		blocking_wall_vec.push_back(blocking_wall_);
		blocking_wall_bodies_vec.push_back(blocking_wall_body_);
	}

	// create physics bodies, shapes and fixtures for 
	// all blocking walls

	b2BodyDef blocking_wall_def[BLOCKING_WALL_NUM];
	b2PolygonShape blockingWallShape[BLOCKING_WALL_NUM];
	b2FixtureDef blocking_wall_fixture_def[BLOCKING_WALL_NUM];

	// blocking wall positions

	blocking_wall_def[0].position = b2Vec2(70.5f, 17.0f);
	blocking_wall_def[1].position = b2Vec2(75.5f, 22.0f);
	blocking_wall_def[2].position = b2Vec2(82.5f, 27.0f);
	blocking_wall_def[3].position = b2Vec2(100.5f, 2.0f);

	blocking_wall_def[4].position = b2Vec2(-75.5f, 2.0f);
	blocking_wall_def[5].position = b2Vec2(-115.0f, 25.0f);

	blocking_wall_def[6].position = b2Vec2(133.5f, 131.0f);
	blocking_wall_def[7].position = b2Vec2(152.5f, 135.0f);

	
	for (int i = 0; i < BLOCKING_WALL_NUM; i++)
	{
		blocking_wall_def[i].type = b2_staticBody;
		blocking_wall_def[i].allowSleep = true;

		blocking_wall_bodies_vec[i] = world_->CreateBody(&blocking_wall_def[i]);

		// set size of physics body

		blockingWallShape[i].SetAsBox(blocking_wall_half_dimensions.x(), blocking_wall_half_dimensions.y());

		// fixture settings

		blocking_wall_fixture_def[i].shape = &blockingWallShape[i];
		blocking_wall_fixture_def[i].friction = 0.0f;

		blocking_wall_bodies_vec[i]->CreateFixture(&blocking_wall_fixture_def[i]);


		// update visuals from simulation data
		blocking_wall_vec[i].UpdateFromSimulation(blocking_wall_bodies_vec[i]);

		// create a connection between the rigid body and GameObject
		blocking_wall_bodies_vec[i]->SetUserData(&blocking_wall_vec[i]);

	}
}

void SceneApp::InitBiggerBlockingWalls()
{
	// initialising bigger blocking wall type variable

	bigger_blocking_wall_.set_type(WALL);

	gef::Vector4 bigger_blocking_wall_half_dimensions(0.5, 7.5, 0.5f);

	// setting mesh and mesh values for bigger blocking walls

	bigger_blocking_wall_mesh_ = primitive_builder_->CreateBoxMesh(bigger_blocking_wall_half_dimensions);
	bigger_blocking_wall_.set_mesh(bigger_blocking_wall_mesh_);

	// push back values for bigger blocking wall obj and body vectors

	for (int i = 0; i < BIGGER_BLOCKING_WALL_NUM; i++)
	{
		bigger_blocking_wall_vec.push_back(bigger_blocking_wall_);
		bigger_blocking_wall_bodies_vec.push_back(bigger_blocking_wall_body_);
	}

	// create physics bodies, shapes and fixtures for 
	// all bigger blocking walls

	b2BodyDef bigger_blocking_wall_def[BIGGER_BLOCKING_WALL_NUM];
	b2PolygonShape bigger_blockingWallShape[BIGGER_BLOCKING_WALL_NUM];
	b2FixtureDef bigger_blocking_wall_fixture_def[BIGGER_BLOCKING_WALL_NUM];

	// bigger blocking wall positions

	bigger_blocking_wall_def[0].position = b2Vec2(-13.0f, 101.0f);
	bigger_blocking_wall_def[1].position = b2Vec2(-29.0f, 92.25f);

	bigger_blocking_wall_def[0].angle = gef::DegToRad(200);
	bigger_blocking_wall_def[1].angle = gef::DegToRad(190);


	for (int i = 0; i < BIGGER_BLOCKING_WALL_NUM; i++)
	{
		bigger_blocking_wall_def[i].type = b2_staticBody;
		bigger_blocking_wall_def[i].allowSleep = true;

		bigger_blocking_wall_bodies_vec[i] = world_->CreateBody(&bigger_blocking_wall_def[i]);

		// set size of physics body

		bigger_blockingWallShape[i].SetAsBox(bigger_blocking_wall_half_dimensions.x(), bigger_blocking_wall_half_dimensions.y());

		// fixture settings

		bigger_blocking_wall_fixture_def[i].shape = &bigger_blockingWallShape[i];
		bigger_blocking_wall_fixture_def[i].friction = 0.0f;

		bigger_blocking_wall_bodies_vec[i]->CreateFixture(&bigger_blocking_wall_fixture_def[i]);


		// update visuals from simulation data
		bigger_blocking_wall_vec[i].UpdateFromSimulation(bigger_blocking_wall_bodies_vec[i]);

		// create a connection between the rigid body and GameObject
		bigger_blocking_wall_bodies_vec[i]->SetUserData(&bigger_blocking_wall_vec[i]);

	}
}

void SceneApp::InitAreaWalls()
{
	// initialising area wall type variable

	area_wall_.set_type(WALL);

	gef::Vector4 area_wall_half_dimensions(0.5, 17.5, 0.5f);

	// setting mesh and mesh values for area walls

	area_wall_mesh_ = primitive_builder_->CreateBoxMesh(area_wall_half_dimensions);
	area_wall_.set_mesh(area_wall_mesh_);

	// push back values for area wall obj and body vectors

	for (int i = 0; i < AREA_WALL_NUM; i++)
	{
		area_walls_vec.push_back(area_wall_);
		area_walls_bodies_vec.push_back(area_wall_body_);
	}

	// create physics bodies, shapes and fixtures for 
	// all area walls

	b2BodyDef area_wall_def[AREA_WALL_NUM];
	b2PolygonShape areaWallShape[AREA_WALL_NUM];
	b2FixtureDef area_wall_fixture_def[AREA_WALL_NUM];

	// area wall positions

	area_wall_def[0].position = b2Vec2(-134.5f, 97.0f);
	area_wall_def[1].position = b2Vec2(-110.0f, 187.5f);
	area_wall_def[2].position = b2Vec2(-30.0f, 187.5f);
	area_wall_def[3].position = b2Vec2(83.0f, 202.5f);
	area_wall_def[4].position = b2Vec2(83.0f, 237.5f);

	


	for (int i = 0; i < AREA_WALL_NUM; i++)
	{
		area_wall_def[i].type = b2_staticBody;
		area_wall_def[i].allowSleep = true;

		area_walls_bodies_vec[i] = world_->CreateBody(&area_wall_def[i]);

		// set size of physics body

		areaWallShape[i].SetAsBox(area_wall_half_dimensions.x(), area_wall_half_dimensions.y());

		// fixture settings

		area_wall_fixture_def[i].shape = &areaWallShape[i];
		area_wall_fixture_def[i].friction = 0.0f;

		area_walls_bodies_vec[i]->CreateFixture(&area_wall_fixture_def[i]);


		// update visuals from simulation data
		area_walls_vec[i].UpdateFromSimulation(area_walls_bodies_vec[i]);

		// create a connection between the rigid body and GameObject
		area_walls_bodies_vec[i]->SetUserData(&area_walls_vec[i]);

	}
}

void SceneApp::InitResetWalls()
{
	// initialising reset wall type variables

	reset_wall_.set_type(STICKY_WALL);

	gef::Vector4 reset_wall_half_dimensions(0.5, 6.0, 0.5f);

	// setting mesh and mesh values for reset walls

	reset_wall_mesh_ = primitive_builder_->CreateBoxMesh(reset_wall_half_dimensions);
	reset_wall_.set_mesh(reset_wall_mesh_);

	// push back values for reset wall obj and body vectors

	for (int i = 0; i < RESET_WALL_NUM; i++)
	{
		reset_walls_vec.push_back(reset_wall_);
		reset_walls_bodies_vec.push_back(reset_wall_body_);
	}

	// create physics bodies, shapes and fixtures for 
	// all reset walls

	b2BodyDef reset_wall_def[RESET_WALL_NUM];
	b2PolygonShape resetWallShape[RESET_WALL_NUM];
	b2FixtureDef reset_wall_fixture_def[RESET_WALL_NUM];

	// reset wall positions

	reset_wall_def[0].position = b2Vec2(-195.0f, 165.0f);
	reset_wall_def[1].position = b2Vec2(-185.0f, 165.0f);
	reset_wall_def[2].position = b2Vec2(-195.0f, 190.0f);
	reset_wall_def[3].position = b2Vec2(-180.0f, 205.0f);
	reset_wall_def[4].position = b2Vec2(-165.0f, 220.0f);
	reset_wall_def[5].position = b2Vec2(-150.0f, 190.0f);
	reset_wall_def[6].position = b2Vec2(-130.0f, 148.0f);
	reset_wall_def[7].position = b2Vec2(-175.0f, 257.5f);
	reset_wall_def[8].position = b2Vec2(-145.0f, 257.5f);
	reset_wall_def[9].position = b2Vec2(-115.0f, 257.5f);

	for (int i = 0; i < RESET_WALL_NUM; i++)
	{
		reset_wall_def[i].type = b2_staticBody;
		

		reset_walls_bodies_vec[i] = world_->CreateBody(&reset_wall_def[i]);

		// set size of physics body

		resetWallShape[i].SetAsBox(reset_wall_half_dimensions.x(), reset_wall_half_dimensions.y());

		reset_wall_fixture_def[i].shape = &resetWallShape[i];
		
		reset_walls_bodies_vec[i]->CreateFixture(&reset_wall_fixture_def[i]);


		// update visuals from simulation data
		reset_walls_vec[i].UpdateFromSimulation(reset_walls_bodies_vec[i]);

		// create a connection between the rigid body and GameObject
		reset_walls_bodies_vec[i]->SetUserData(&reset_walls_vec[i]);

	}
}


void SceneApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void SceneApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void SceneApp::DrawHUD()
{
	if(font_)
	{
		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(830.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_LEFT, "FPS: %.1f", fps_);

		if (gamestatetype == LEVEL1)
		{
			// displays message to show when the player can and can't be damaged

			if (player_.getInvincibleCheck())
			{
				font_->RenderText(sprite_renderer_, gef::Vector4(770.0f, 430.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "INVULNERABLE");
			}

			else
			{
				font_->RenderText(sprite_renderer_, gef::Vector4(770.0f, 430.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "");
			}
			gef::Sprite healthValue[5];

			for (int i = 0; i < 5; i++)
			{
				healthValue[i].set_height(15.0f);
				healthValue[i].set_width(15.0f);
			}
			
			// displaying the amount of health through a sprite
			// changing colours depending on how much health the 
			// player has left

			// showing different value of health based on difficulty 
			// chosen also

			switch (difficulty)
			{
			case DIFF_EASY:
				if (player_.getHealth() > 5)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(770.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: x%i", player_.getHealth());
					healthValue[0].set_position(gef::Vector4(930.0f, 495.0f, -0.9f));
					healthValue[0].set_colour(0xffa60f30);
					sprite_renderer_->DrawSprite(healthValue[0]);
				}

				else if(player_.getHealth() == 5)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(730.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(830.0f, 495.0f, -0.9f));
					healthValue[1].set_position(gef::Vector4(860.0f, 495.0f, -0.9f));
					healthValue[2].set_position(gef::Vector4(890.0f, 495.0f, -0.9f));
					healthValue[3].set_position(gef::Vector4(920.0f, 495.0f, -0.9f));
					healthValue[4].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					for (int i = 0; i < 5; i++)
					{
						healthValue[i].set_colour(0xffa60f30);
						sprite_renderer_->DrawSprite(healthValue[i]);
					}
					
				}

				else if (player_.getHealth() == 4)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(760.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(860.0f, 495.0f, -0.9f));
					healthValue[1].set_position(gef::Vector4(890.0f, 495.0f, -0.9f));
					healthValue[2].set_position(gef::Vector4(920.0f, 495.0f, -0.9f));
					healthValue[3].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					for (int i = 0; i < 4; i++)
					{
						healthValue[i].set_colour(0xffa60f30);
						sprite_renderer_->DrawSprite(healthValue[i]);
					}

				}

				else if (player_.getHealth() == 3)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(790.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(890.0f, 495.0f, -0.9f));
					healthValue[1].set_position(gef::Vector4(920.0f, 495.0f, -0.9f));
					healthValue[2].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					for (int i = 0; i < 3; i++)
					{
						healthValue[i].set_colour(0xffa60f30);
						sprite_renderer_->DrawSprite(healthValue[i]);
					}

				}

				else if (player_.getHealth() == 2)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(820.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(920.0f, 495.0f, -0.9f));
					healthValue[1].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					for (int i = 0; i < 2; i++)
					{
						healthValue[i].set_colour(0xff0f41a6);
						sprite_renderer_->DrawSprite(healthValue[i]);
					}

				}

				else if (player_.getHealth() == 1)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					
					healthValue[0].set_colour(0xff0000ff);
					sprite_renderer_->DrawSprite(healthValue[0]);
					

				}
				

				break;
			case DIFF_NORMAL:
				if (player_.getHealth() == 5)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(730.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(830.0f, 495.0f, -0.9f));
					healthValue[1].set_position(gef::Vector4(860.0f, 495.0f, -0.9f));
					healthValue[2].set_position(gef::Vector4(890.0f, 495.0f, -0.9f));
					healthValue[3].set_position(gef::Vector4(920.0f, 495.0f, -0.9f));
					healthValue[4].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					for (int i = 0; i < 5; i++)
					{
						healthValue[i].set_colour(0xffa60f30);
						sprite_renderer_->DrawSprite(healthValue[i]);
					}

				}

				else if (player_.getHealth() == 4)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(760.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(860.0f, 495.0f, -0.9f));
					healthValue[1].set_position(gef::Vector4(890.0f, 495.0f, -0.9f));
					healthValue[2].set_position(gef::Vector4(920.0f, 495.0f, -0.9f));
					healthValue[3].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					for (int i = 0; i < 4; i++)
					{
						healthValue[i].set_colour(0xffa60f30);
						sprite_renderer_->DrawSprite(healthValue[i]);
					}

				}

				else if (player_.getHealth() == 3)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(790.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(890.0f, 495.0f, -0.9f));
					healthValue[1].set_position(gef::Vector4(920.0f, 495.0f, -0.9f));
					healthValue[2].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					for (int i = 0; i < 3; i++)
					{
						healthValue[i].set_colour(0xffa60f30);
						sprite_renderer_->DrawSprite(healthValue[i]);
					}

				}

				else if (player_.getHealth() == 2)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(820.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(920.0f, 495.0f, -0.9f));
					healthValue[1].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					for (int i = 0; i < 2; i++)
					{
						healthValue[i].set_colour(0xff0f41a6);
						sprite_renderer_->DrawSprite(healthValue[i]);
					}

				}

				else if (player_.getHealth() == 1)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					healthValue[0].set_colour(0xff0000ff);
					sprite_renderer_->DrawSprite(healthValue[0]);


				}

				break;
			case DIFF_HARD:
				if (player_.getHealth() == 3)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(790.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(890.0f, 495.0f, -0.9f));
					healthValue[1].set_position(gef::Vector4(920.0f, 495.0f, -0.9f));
					healthValue[2].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					for (int i = 0; i < 3; i++)
					{
						healthValue[i].set_colour(0xffa60f30);
						sprite_renderer_->DrawSprite(healthValue[i]);
					}

				}

				else if (player_.getHealth() == 2)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(820.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(920.0f, 495.0f, -0.9f));
					healthValue[1].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					for (int i = 0; i < 2; i++)
					{
						healthValue[i].set_colour(0xff0f41a6);
						sprite_renderer_->DrawSprite(healthValue[i]);
					}

				}

				else if (player_.getHealth() == 1)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					healthValue[0].set_colour(0xff0000ff);
					sprite_renderer_->DrawSprite(healthValue[0]);


				}

				break;
			case DIFF_ONESHOT:

				if (player_.getHealth() == 1)
				{
					font_->RenderText(sprite_renderer_, gef::Vector4(850.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Health: ");
					healthValue[0].set_position(gef::Vector4(950.0f, 495.0f, -0.9f));

					healthValue[0].set_colour(0xff0000ff);
					sprite_renderer_->DrawSprite(healthValue[0]);


				}
				break;
			}

			font_->RenderText(sprite_renderer_, gef::Vector4(30.0f, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_LEFT, "Time: %.1f", gameTimer.elapsedSeconds());


			font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, 480.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_CENTRE, "Abilities Active");

			if (player_.getDashActive() == true)
			{
				font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 50, 515.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_CENTRE, "Dash");
			}

			else
			{
				font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 50, 515.0f, -0.9f), 1.0f, 0x10a60f30, gef::TJ_CENTRE, "Dash");
			}

			if (player_.getDoubleJumpActive() == true)
			{
				font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, 515.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_CENTRE, "Double Jump");
			}

			else
			{
				font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, 515.0f, -0.9f), 1.0f, 0x10a60f30, gef::TJ_CENTRE, "Double Jump");
			}

			if (player_.getResetWallActive() == true)
			{
				font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 125, 515.0f, -0.9f), 1.0f, 0xffa60f30, gef::TJ_CENTRE, "Double Jump Reset");
			}

			else
			{
				font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 125, 515.0f, -0.9f), 1.0f, 0x10a60f30, gef::TJ_CENTRE, "Double Jump Reset");
			}

		}
		
	}
}

void SceneApp::SetupLights()
{
	// grab the data for the default shader used for rendering 3D geometry
	gef::Default3DShaderData& default_shader_data = renderer_3d_->default_shader_data();

	// set the ambient light
	default_shader_data.set_ambient_light_colour(gef::Colour(0.25f, 0.25f, 0.25f, 1.0f));

	// add a point light that is almost white, but with a blue tinge
	// the position of the light is set far away so it acts light a directional light
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(-500.0f, 400.0f, 700.0f));
	default_shader_data.AddPointLight(default_point_light);
}

void SceneApp::UpdateSimulation(float frame_time)
{
	// update physics world
	float timeStep = 1.0f / 60.0f; // 60.0f

	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	world_->Step(timeStep, velocityIterations, positionIterations);

	// update object visuals from simulation data

	player_.UpdateFromSimulation(player_body_);

	for (int i = 0; i < groundEnemyVec.size(); i++)
	{
		groundEnemyVec[i].UpdateFromSimulation(groundEnemyVec[i].getBody());
	}

	for (int i = 0; i < MOVING_PLATFORM_NUM; i++)
	{
		movPlatformsVec[i].UpdateFromSimulation(movPlatform_bodies_vec[i]);
	}

	// don't have to update the ground visuals as it is static

	// collision detection
	// get the head of the contact list
	b2Contact* contact = world_->GetContactList();
	// get contact count
	int contact_count = world_->GetContactCount();

	//std::set<b2Body* > deletionList;

	for (int contact_num = 0; contact_num < contact_count; ++contact_num)
	{
		if (contact->IsTouching())
		{
			// get the colliding bodies
			b2Body* bodyA = contact->GetFixtureA()->GetBody();
			b2Body* bodyB = contact->GetFixtureB()->GetBody();

			// DO COLLISION RESPONSE HERE

			// seperate class objects
			// declared for collision

			Player* player = NULL;
			GroundEnemy* groundEnemy = NULL;
			Spike* spikeEnv = NULL;
			Collectable* theCollectable = NULL;

			// objects derived from game object class
			// declared for collision

			GameObject* stickyWall = NULL;
			GameObject* movingPlat = NULL;
			GameObject* dashAbility = NULL;
			GameObject* doubleJumpAbility = NULL;
			GameObject* resetWallAbility = NULL;

			GameObject* gameObjectA = NULL;
			GameObject* gameObjectB = NULL;

			gameObjectA = (GameObject*)bodyA->GetUserData();
			gameObjectB = (GameObject*)bodyB->GetUserData();

			// setting gameObjectA to correct object / object type

			if (gameObjectA)
			{
				if (gameObjectA->type() == PLAYER)
				{
					player = (Player*)bodyA->GetUserData();
				}

				if (gameObjectA->type() == GROUND_ENEMY)
				{
					groundEnemy = (GroundEnemy*)bodyA->GetUserData();
				}

				if (gameObjectA->type() == SPIKE)
				{
					spikeEnv = (Spike*)bodyA->GetUserData();
				}

				if (gameObjectA->type() == STICKY_WALL)
				{
					stickyWall = (GameObject*)bodyA->GetUserData();
				}

				if (gameObjectA->type() == MOVING)
				{
					movingPlat = (GameObject*)bodyA->GetUserData();
				}

				if (gameObjectA->type() == ABILITY_DASH)
				{
					dashAbility = (GameObject*)bodyA->GetUserData();
				}

				if (gameObjectA->type() == ABILITY_DJ)
				{
					doubleJumpAbility = (GameObject*)bodyA->GetUserData();
				}

				if (gameObjectA->type() == ABILITY_RW)
				{
					resetWallAbility = (GameObject*)bodyA->GetUserData();
				}

				if (gameObjectA->type() == COLLECTABLE)
				{
					theCollectable = (Collectable*)bodyA->GetUserData();
				}


			}


			// setting gameObjectB to correct object / object type

			if (gameObjectB)
			{
				if (gameObjectB->type() == PLAYER)
				{
					player = (Player*)bodyB->GetUserData();
				}

				if (gameObjectB->type() == GROUND_ENEMY)
				{
					groundEnemy = (GroundEnemy*)bodyB->GetUserData();
				}

				if (gameObjectB->type() == SPIKE)
				{
					spikeEnv = (Spike*)bodyB->GetUserData();
				}

				if (gameObjectB->type() == STICKY_WALL)
				{
					stickyWall = (GameObject*)bodyB->GetUserData();
				}

				if (gameObjectB->type() == MOVING)
				{
					movingPlat = (GameObject*)bodyB->GetUserData();
				}

				if (gameObjectB->type() == ABILITY_DASH)
				{
					dashAbility = (GameObject*)bodyB->GetUserData();
				}

				if (gameObjectB->type() == ABILITY_DJ)
				{
					doubleJumpAbility = (GameObject*)bodyB->GetUserData();
				}

				if (gameObjectB->type() == ABILITY_RW)
				{
					resetWallAbility = (GameObject*)bodyB->GetUserData();
				}

				if (gameObjectB->type() == COLLECTABLE)
				{
					theCollectable = (Collectable*)bodyB->GetUserData();
				}
			}


			// checks if player is colliding with a
			// Double Jump Reset wall --- stickyWall was original name of ability / object

			if (player && stickyWall)
			{

				// checks if player state is currently or just was on the wall
				// also checks if player has Double Jump Reset ability unlocked

				if (player->getPlayerState() != ON_WALL && player->getPlayerPreviousState() != ON_WALL && player->getResetWallActive() == true)
				{
					// if all true, change player state on collision
					player->setPlayerState(ON_WALL);
				}

			}


			// checks if player is colliding with a
			// moving platform

			if (player && movingPlat)
			{

				// checks if player state is currently or just was on a moving platform
				if (player->getPlayerState() != ON_MOVING_PLAT && player->getPlayerPreviousState() != ON_MOVING_PLAT)
				{
					// if true, change player state on collision
					player->setPlayerState(ON_MOVING_PLAT);
				}


			}


			// checks if player is colliding with a 
			// ground enemy

			if (player && groundEnemy)
			{
				// if true

				// carries out enemy's collision response to player
				groundEnemy->PlayerCollisionResponse(player, player_body_);

				// decrements player health
				player->DecrementHealth();

				// plays a hitting sound
				audio_manager->PlaySample(hit_sound);
			}


			// checks if player is colling with a
			// spike

			if (player && spikeEnv)
			{
				// if true

				// decrements player health
				player->DecrementHealth();

				// plays a hitting sound
				audio_manager->PlaySample(hit_sound);
			}


			// checks if player is colliding with
			// the Dash ability object

			if (player && dashAbility)
			{
				// if true

				// activates ability for player
				player->setDashActive(true);

				// plays ability pickup sound
				audio_manager->PlaySample(ability_pickup);

			}


			// checks if player is colliding with
			// the Double Jump ability object

			if (player && doubleJumpAbility)
			{
				// if true

				// activates ability for player
				player->setDoubleJumpActive(true);

				// plays ability pickup sound
				audio_manager->PlaySample(ability_pickup);

			}


			// checks if player is colliding with
			// the Double Jump Reset ability object

			if (player && resetWallAbility)
			{
				// if true

				// activates ability for player
				player->setResetWallActive(true);

				// plays ability pickup sound
				audio_manager->PlaySample(ability_pickup);

			}


			// checks if player is colliding with
			// the Morph Ball object

			if (player && theCollectable)
			{
				// if true

				// player wins, plays the end game sequence
				isCollectableUp = true;
			}

			
			// Get next contact point
			contact = contact->GetNext();
		}

		/*for (auto bodies : deletionList)
		{
			world_->DestroyBody(bodies);
		}*/
	}

}

void SceneApp::FrontendInit()
{
	// loads splash screen visual in
	// timer starts to end the frontend state after a specified time

	splash_screen_ = CreateTextureFromPNG("splash-screen.png", platform_);
	frontendTimer.Start();
}

void SceneApp::FrontendRelease()
{
	// releasing components of front end

	delete splash_screen_;
	splash_screen_ = NULL;
}

void SceneApp::FrontendUpdate(float frame_time)
{
	// pressing space changes app to menu state

	if (kb->IsKeyPressed(gef::Keyboard::KC_SPACE))
	{
		set_type_gamestate(MENU);
		MenuInit();
		FrontendRelease();
	}

	// waiting 5 seconds changes app to menu state

	if (frontendTimer.elapsedSeconds() > 5)
	{
		frontendTimer.Stop();
		set_type_gamestate(MENU);
		MenuInit();
		FrontendRelease();
	}
}

void SceneApp::FrontendRender()
{
	// renders front end components

	sprite_renderer_->Begin();

	gef::Sprite splashScreenSprite;
	splashScreenSprite.set_texture(splash_screen_);
	splashScreenSprite.set_position(gef::Vector4(platform_.width()*0.5f, platform_.height()*0.5f, 0.0f));
	splashScreenSprite.set_height(544);
	splashScreenSprite.set_width(960);
	sprite_renderer_->DrawSprite(splashScreenSprite);

	sprite_renderer_->End();
}

void SceneApp::InitAllGrounds()
{
	// initialises all objects that
	// would be considered platforms
	// for the player to stand and move on
	// from their respective functions

	InitBottomBorder();
	InitTopBorder();
	InitSmallPlatformsSA();
	InitMeduimPlatformsMA();
	InitBigPlatforms();
	InitMovingPlatforms();

}

void SceneApp::InitAllWalls()
{
	// initialises all objects that
	// would be considered walls
	// that the player can either hit and fall
	// from or attach itself to,
	// like the reset walls

	InitLeftBorder();
	InitRightBorder();
	InitVerySmallPlats();
	InitBlockingWalls();
	InitBiggerBlockingWalls();
	InitAreaWalls();
	InitResetWalls();
}

void SceneApp::GameInit()
{
	// create the renderer for draw 3D geometry
	renderer_3d_ = gef::Renderer3D::Create(platform_);

	// initialise primitive builder to make create some 3D geometry easier
	primitive_builder_ = new PrimitiveBuilder(platform_);

	// creates the lights within the scene
	SetupLights();

	// initialise the physics world
	b2Vec2 gravity(0.0f, -9.81f);
	world_ = new b2World(gravity);

	// calls all object initialisers
	// to be created once the level starts

	InitPlayer();
	InitCollectables();
	InitAllAbilities();
	InitAllSpikes();
	InitAllGrounds();
	InitAllWalls();

	// creating and pushing back ground enemies into a vector

	for (int i = 0; i < GROUND_ENEMY_NUM; i++)
	{
		groundEnemyVec.push_back(GroundEnemy());
	}

	// setting positions of ground enemies

	groundEnemyVec[0].setPosition(b2Vec2(45.0f, 137.0f));
	groundEnemyVec[1].setPosition(b2Vec2(125.0f, 137.0f));

	groundEnemyVec[2].setPosition(b2Vec2(107.0f, 187.0f));
	groundEnemyVec[3].setPosition(b2Vec2(63.0f, 187.0f));

	groundEnemyVec[4].setPosition(b2Vec2(60.0f, 284.0f));
	groundEnemyVec[5].setPosition(b2Vec2(20.0f, 284.0f));

	groundEnemyVec[6].setPosition(b2Vec2(-33.0f, 209.0f));
	groundEnemyVec[7].setPosition(b2Vec2(-115.0f, 209.0f));
	groundEnemyVec[8].setPosition(b2Vec2(-70.0f, 209.0f));

	groundEnemyVec[9].setPosition(b2Vec2(123.0f, 82.0f));
	groundEnemyVec[10].setPosition(b2Vec2(100.0f, 117.0f));


	// calling the init functions for all ground enemies within the vector
	// giving them their body settings, type etc
	for (int i = 0; i < groundEnemyVec.size(); i++)
	{
		groundEnemyVec[i].Init(world_, primitive_builder_);
	}
	
	// initialises the end game collectable as not collected
	isCollectableUp = false;

	// starts player in the camera looking at the player
	cameraSwitch = false;

	// checks for different sounds / music playing
	// stops any sounds / music that isn't related to the level

	if (menu_music_playing)
	{
		menu_music_playing = false;
		audio_manager->StopPlayingSampleVoice(menu_music);
	}

	if (game_over_playing)
	{
		game_over_playing = false;
	}

	// checks if level music is play
	// if not, play it and loop it

	if (!level_music_playing)
	{
		level_music_playing = true;
		audio_manager->PlaySample(level_music, true);
	}

	// loads in the background screens for each camera perspective 

	game_screen_ = CreateTextureFromPNG("game-background.png", platform_);
	game_screen_diff_camera = CreateTextureFromPNG("game-background2.png", platform_);

	// starts in game timer shown to player
	gameTimer.Start();
}

void SceneApp::GameRelease()
{
	// destroying the physics world also destroys all the objects within it
	delete world_;
	world_ = NULL;

	// releasing objects and meshs

	delete bottom_border_mesh_;
	bottom_border_mesh_ = NULL;

	delete top_border_mesh_;
	top_border_mesh_ = NULL;

	delete left_border_mesh_;
	left_border_mesh_ = NULL;

	delete right_border_mesh_;
	right_border_mesh_ = NULL;

	// clearing all vectors
	// so the game level may be started again
	// after completing, failing or generally going back
	// to main menu
	// without over filling the vectors

	small_platforms_SA.clear();
	medium_platforms_MA.clear();
	big_platforms_vec.clear();
	very_small_plat_vec.clear();
	blocking_wall_vec.clear();
	bigger_blocking_wall_vec.clear();
	area_walls_vec.clear();
	reset_walls_vec.clear();
	spikes_vec.clear();
	movPlatformsVec.clear();
	groundEnemyVec.clear();

	delete dashPickupMesh;
	dashPickupMesh = NULL;

	delete doubleJumpPickupMesh;
	doubleJumpPickupMesh = NULL;

	delete resetWallPickupMesh;
	resetWallPickupMesh = NULL;

	delete primitive_builder_;
	primitive_builder_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

}

void SceneApp::GameUpdate(float frame_time)
{
	// calls update simulation to update the physics engine

	UpdateSimulation(frame_time);

	// updates position of all moving platforms based on the function
	// call and the value entered inside them

	// all Y axis platforms
	movPlatformsVec[0].setPlatPosAndSpeedYaxis(movPlatform_bodies_vec[0], 53.0f, 100.0f, 4.0f);
	movPlatformsVec[2].setPlatPosAndSpeedYaxis(movPlatform_bodies_vec[2], 160.0f, 120.0f, 2.125f);
	movPlatformsVec[3].setPlatPosAndSpeedYaxis(movPlatform_bodies_vec[3], 90.0f, 130.0f, 2.125f);

	movPlatformsVec[4].setPlatPosAndSpeedYaxis(movPlatform_bodies_vec[4], 187.5f, 212.0f, 4.0f);

	movPlatformsVec[5].setPlatPosAndSpeedYaxis(movPlatform_bodies_vec[5], 243.0f, 223.0f, 4.25f);
	movPlatformsVec[6].setPlatPosAndSpeedYaxis(movPlatform_bodies_vec[6], 228.0f, 248.0f, 4.25f);
	movPlatformsVec[7].setPlatPosAndSpeedYaxis(movPlatform_bodies_vec[7], 253.0f, 233.0f, 4.25f);
	movPlatformsVec[8].setPlatPosAndSpeedYaxis(movPlatform_bodies_vec[8], 238.0f, 258.0f, 4.25f);
	

	movPlatformsVec[10].setPlatPosAndSpeedYaxis(movPlatform_bodies_vec[10], 187.5f, 215.0f, 4.15f);
	movPlatformsVec[12].setPlatPosAndSpeedYaxis(movPlatform_bodies_vec[12], 243.0f, 265.0f, 4.0f);


	// all X axis platforms
	movPlatformsVec[1].setPlatPosAndSpeedXaxis(movPlatform_bodies_vec[1], 25.0f, -50.0f, 4.0f);
	movPlatformsVec[9].setPlatPosAndSpeedXaxis(movPlatform_bodies_vec[9], 162.5f, 105.0f, 4.0f);
	movPlatformsVec[11].setPlatPosAndSpeedXaxis(movPlatform_bodies_vec[11], 55.0f, 35.0f, 4.0f);
	

	// updates position of all ground enemies based on
	// the values placed inside the function calls

	groundEnemyVec[0].Movement(45.0f, 80.0f, 4.0f);
	groundEnemyVec[1].Movement(125.0f, 90.0f, 4.0f);

	groundEnemyVec[2].Movement(109.0f, 125.0f, 4.5f);
	groundEnemyVec[3].Movement(61.0f, 45.0f, 4.5f);

	groundEnemyVec[4].Movement(60.0f, 25.0f, 7.0f);
	groundEnemyVec[5].Movement(20.0f, -10.0f, 7.0f);

	groundEnemyVec[6].Movement(-33.0f, -50.0f, 6.5f);
	groundEnemyVec[7].Movement(-115.0f, -85.0f, 6.5f);
	groundEnemyVec[8].Movement(-80.0f,-55.0f, 6.5f);
	

	groundEnemyVec[9].Movement(116.0f, 135.0f, 3.5f);
	groundEnemyVec[10].Movement(95.0f, 105.0f, 4.25f);


	// loops through all reset wall bodies
	// and sets the friction value to a certain value
	// based on whether or not the player has the Double Jump Reset
	// ability

	for (int i = 0; i < RESET_WALL_NUM; i++)
	{
		if (player_.getResetWallActive() == true)
		{
			reset_walls_bodies_vec[i]->GetFixtureList()->SetFriction(3.5f);

			// if player picks up Double Jump Reset ability
			// turn off the body of the ability object
			resetWallPickup_body_->SetActive(false);
		}

		else
		{
			reset_walls_bodies_vec[i]->GetFixtureList()->SetFriction(0.0f);
		}
	}

	// if player picks up Dash ability
	// turn off the body of the ability object

	if (player_.getDashActive() == true)
	{
		dashPickup_body_->SetActive(false);
	}

	// if player picks up Double Jump ability
	// turn off the body of the ability object

	if (player_.getDoubleJumpActive() == true)
	{
		doubleJumpPickup_body_->SetActive(false);
	}

	// plays sound queue if player is double jumping

	if (player_.getPlayerState() == DOUBLE_JUMPING)
	{
		audio_manager->PlaySample(jump_sound);
	}
	
	// plays sound queue if player is dashing

	if (player_.getPlayerState() == DASHING_LEFT || player_.getPlayerState() == DASHING_RIGHT)
	{
		audio_manager->PlaySample(dash_sound);
	}

	// debugs player position and state values

	gef::DebugOut("playerPos.y %f, playerPos.x %f, playerState %i, previousState %i, secondPreviousState %i\n",
		player_body_->GetPosition().y, player_body_->GetPosition().x, player_.getPlayerState(), player_.getPlayerPreviousState(), player_.getPlayerSecondPreviousState());
	
	// updates player functionality and variables

	player_.HandleInput(input_manager_, player_body_, frame_time);

	// if collectable object collides with the player
	// game changes to win state

	if (isCollectableUp)
	{
		set_type_gamestate(WIN);
		EndGameInit();
		GameRelease();
	}

	// if player's health drops below one
	// game changes to lose state

	if (player_.getHealth() < 1)
	{
		set_type_gamestate(LOSE);
		FailedInit();
		GameRelease();
		
	}


	
	if (kb)
	{
		// resets player position back to starting point
		// used for debugging and testing

		if (kb->IsKeyPressed(gef::Keyboard::KC_R) && gamestatetype == LEVEL1)
		{
			player_body_->SetTransform(b2Vec2(0.0f, 4.0f), 0);
			player_body_->SetAwake(false);
			player_body_->SetAwake(true);
		}

		// P key takes player back to menu 
		// player is shown this key in How To Play screen

		if (kb->IsKeyPressed(gef::Keyboard::KC_P))
		{
			set_type_gamestate(MENU);
			GameRelease();
			MenuInit();
			audio_manager->PlaySample(menu_button_sound);
		}
	}
	
}

void SceneApp::GameRender()
{
	// setup camera

	// projection
	float fov = gef::DegToRad(45.0f);
	float aspect_ratio = (float)platform_.width() / (float)platform_.height();
	gef::Matrix44 projection_matrix;
	projection_matrix = platform_.PerspectiveProjectionFov(fov, aspect_ratio, 0.1f, 500.0f);
	renderer_3d_->set_projection_matrix(projection_matrix);

	// view
	gef::Vector4 camera_eye;
	gef::Vector4 camera_lookat;
	gef::Vector4 camera_up(0.0f, 1.0f, 0.0f);

	// changes view to map view 
	if (cameraSwitch)
	{
		camera_eye = gef::Vector4(0.0f, 150.0f, 400.0f);
		camera_lookat = gef::Vector4(0.0f, 150.0f, 0.0f);
		
	}

	// standard view of the player 
	else
	{
		camera_eye = gef::Vector4(player_body_->GetPosition().x, player_body_->GetPosition().y + 20.0f, 70.0f);
		camera_lookat = gef::Vector4(player_body_->GetPosition().x, player_body_->GetPosition().y, 0.0f);
		
	}


	// pressing M key changes which camera is currently active

	if (kb->IsKeyPressed(gef::Keyboard::KC_M))
	{
		if (!cameraSwitch)
		{
			cameraSwitch = true;
		}

		else
		{
			cameraSwitch = false;
		}

		// plays sound signalling camera change
		audio_manager->PlaySample(menu_button_sound);
	}


	gef::Matrix44 view_matrix;
	view_matrix.LookAt(camera_eye, camera_lookat, camera_up);
	renderer_3d_->set_view_matrix(view_matrix);

	
	// draw 3d geometry
	renderer_3d_->Begin();

	// draw ground objects

	renderer_3d_->DrawMesh(bottom_border_);
	renderer_3d_->DrawMesh(top_border_);

	// draw all small platforms
	// changes the colour of them
	// depending on their position within the level

	for (int i = 0; i < SMALL_PLATFORM_NUM_SA; i++)
	{
		if (small_platforms_SA[i].transform().GetTranslation().y() < 70.0f)
		{
			renderer_3d_->set_override_material(&primitive_builder_->grass_green_material());
			renderer_3d_->DrawMesh(small_platforms_SA[i]);
			renderer_3d_->set_override_material(NULL);
		}

		else if (small_platforms_SA[i].transform().GetTranslation().y() > 70.0f && small_platforms_SA[i].transform().GetTranslation().y() < 170.0f)
		{
			renderer_3d_->set_override_material(&primitive_builder_->gray_material());
			renderer_3d_->DrawMesh(small_platforms_SA[i]);
			renderer_3d_->set_override_material(NULL);
		}

		else
		{
			renderer_3d_->set_override_material(&primitive_builder_->light_blue_material());
			renderer_3d_->DrawMesh(small_platforms_SA[i]);
			renderer_3d_->set_override_material(NULL);
		}

		
	}


	// draw all medium platforms
	for (int i = 0; i < MEDIUM_PLATFORM_NUM_MA; i++)
	{
		renderer_3d_->set_override_material(&primitive_builder_->light_pink_material());
		renderer_3d_->DrawMesh(medium_platforms_MA[i]);
		renderer_3d_->set_override_material(NULL);
	}

	// draw all moving platforms
	renderer_3d_->set_override_material(&primitive_builder_->yellow_material());
	for (int i = 0; i < MOVING_PLATFORM_NUM; i++)
	{
		renderer_3d_->DrawMesh(movPlatformsVec[i]);
	}
	renderer_3d_->set_override_material(NULL);


	// draw all big platforms
	for (int i = 0; i < BIG_PLATFORM_NUM; i++)
	{
		renderer_3d_->set_override_material(&primitive_builder_->red_material());
		renderer_3d_->DrawMesh(big_platforms_vec[i]);
		renderer_3d_->set_override_material(NULL);
	}

	
	// draw all v-small platforms
	for (int i = 0; i < VERY_SMALL_PLATFORM_NUM; i++)
	{
		renderer_3d_->set_override_material(&primitive_builder_->orange_material());
		renderer_3d_->DrawMesh(very_small_plat_vec[i]);
		renderer_3d_->set_override_material(NULL);
	}

	// draw wall objects
	renderer_3d_->DrawMesh(right_border_);
	renderer_3d_->DrawMesh(left_border_);

	// draw all blocking wall objects
	// changes the colour of them
	// depending on their position within the level
	for (int i = 0; i < BLOCKING_WALL_NUM; i++)
	{

		if (blocking_wall_vec[i].transform().GetTranslation().y() < 70.0f)
		{
			renderer_3d_->set_override_material(&primitive_builder_->brown_material());
			renderer_3d_->DrawMesh(blocking_wall_vec[i]);
			renderer_3d_->set_override_material(NULL);
		}

		else
		{
			renderer_3d_->set_override_material(&primitive_builder_->gray_material());
			renderer_3d_->DrawMesh(blocking_wall_vec[i]);
			renderer_3d_->set_override_material(NULL);
		}

		
	}


	// draw all bigger blocking walls
	for (int i = 0; i < BIGGER_BLOCKING_WALL_NUM; i++)
	{
		renderer_3d_->DrawMesh(bigger_blocking_wall_vec[i]);
	}


	// draw all area walls
	for (int i = 0; i < AREA_WALL_NUM; i++)
	{
		renderer_3d_->set_override_material(&primitive_builder_->red_material());
		renderer_3d_->DrawMesh(area_walls_vec[i]);
		renderer_3d_->set_override_material(NULL);
	}


	// draw all reset walls
	for (int i = 0; i < RESET_WALL_NUM; i++)
	{
		renderer_3d_->set_override_material(&primitive_builder_->blue_material());
		renderer_3d_->DrawMesh(reset_walls_vec[i]);
		renderer_3d_->set_override_material(NULL);
	}



	// draw player
	renderer_3d_->set_override_material(&primitive_builder_->purple_material());
	renderer_3d_->DrawMesh(player_);
	renderer_3d_->set_override_material(NULL);


	// only draws all ability objects
	// if the player has not picked up said abilities
	if (player_.getDashActive() == false)
	{
		renderer_3d_->set_override_material(&primitive_builder_->purple_material());
		renderer_3d_->DrawMesh(dashPickup_);
		renderer_3d_->set_override_material(NULL);
	}

	if (player_.getDoubleJumpActive() == false)
	{
		renderer_3d_->set_override_material(&primitive_builder_->green_material());
		renderer_3d_->DrawMesh(doubleJumpPickup_);
		renderer_3d_->set_override_material(NULL);
	}

	if (player_.getResetWallActive() == false)
	{
		renderer_3d_->set_override_material(&primitive_builder_->light_blue_material());
		renderer_3d_->DrawMesh(resetWallPickup_);
		renderer_3d_->set_override_material(NULL);
	}

	// draws all ground enemies
	for (int i = 0; i < groundEnemyVec.size(); i++)
	{
	//	renderer_3d_->set_override_material(&primitive_builder_->blue_material());
		groundEnemyVec[i].RenderGroundEnemy(renderer_3d_, primitive_builder_);
	//	renderer_3d_->set_override_material(NULL);
	}


	// draws all spikes
	for (int i = 0; i < SPIKE_NUM; i++)
	{
		renderer_3d_->set_override_material(&primitive_builder_->gray_material());
		renderer_3d_->DrawMesh(spikes_vec[i]);
		renderer_3d_->set_override_material(NULL);
	}
	
	// draws game collectable
	renderer_3d_->DrawMesh(collectable_);

	renderer_3d_->End();

	// start drawing sprites, but don't clear the frame buffer
	sprite_renderer_->Begin(false);
	
	// main camera background

	gameScreenSprite.set_texture(game_screen_);
	gameScreenSprite.set_position(gef::Vector4(platform_.width() / 2, platform_.height() / 2, 2.0f));
	gameScreenSprite.set_height(544);
	gameScreenSprite.set_width(960);

	// map camera background

	gameScreenSprite2.set_texture(game_screen_diff_camera);
	gameScreenSprite2.set_position(gef::Vector4(platform_.width() / 2, platform_.height() / 2, 2.0f));
	gameScreenSprite2.set_height(544);
	gameScreenSprite2.set_width(960);

	// renders background depending on camera selected
	if (!cameraSwitch)
	{
		sprite_renderer_->DrawSprite(gameScreenSprite2);
	}

	else
	{
		sprite_renderer_->DrawSprite(gameScreenSprite);
	}
	
	// draws hud details
	DrawHUD();
	sprite_renderer_->End();
}

void SceneApp::MenuInit()
{
	// initialises which state the menu starts on

	menuStateType = MENU_PLAY;

	// plays menu music if it isnt playing

	if (!menu_music_playing)
	{
		menu_music_playing = true;
		audio_manager->PlaySample(menu_music, true);
	}

	// stops all other sounds and music from playing
	// while the menu state is active

	if (level_music_playing)
	{
		level_music_playing = false;
		audio_manager->StopPlayingSampleVoice(level_music);
	}

	if (player_wins_playing)
	{
		player_wins_playing = false;
		audio_manager->StopPlayingSampleVoice(player_wins);
	}

	if (game_over_playing)
	{
		game_over_playing = false;
		audio_manager->StopPlayingSampleVoice(game_over);
	}

	// loads in menu background

	menu_screen_ = CreateTextureFromPNG("menu-backgroundv2.png",platform_);
}

void SceneApp::MenuUpdate(float frame_time)
{
	if (kb)
	{
		// changes menu state
		// to option above

		if (kb->IsKeyPressed(gef::Keyboard::KC_UP))
		{
			switch (menuStateType)
			{
			case MENU_PLAY:
				menuStateType = MENU_EXIT;
				break;
			case MENU_HTP:
				menuStateType = MENU_PLAY;
				break;
			case MENU_OPTIONS:
				menuStateType = MENU_HTP;
				break;
			case MENU_EXIT:
				menuStateType = MENU_OPTIONS;
				break;
			}

			// plays sound queue
			audio_manager->PlaySample(menu_button_sound);
		}

		// changes menu state
		// to option below

		if (kb->IsKeyPressed(gef::Keyboard::KC_DOWN))
		{
			switch (menuStateType)
			{
			case MENU_PLAY:
				menuStateType = MENU_HTP;
				break;
			case MENU_HTP:
				menuStateType = MENU_OPTIONS;
				break;
			case MENU_OPTIONS:
				menuStateType = MENU_EXIT;
				break;
			case MENU_EXIT:
				menuStateType = MENU_PLAY;
				break;
			}

			// plays sound queue
			audio_manager->PlaySample(menu_button_sound);
		}


		// selects whichever option is currently
		// highlighted, moves to different state

		if (kb->IsKeyPressed(gef::Keyboard::KC_RETURN))
		{
			switch (menuStateType)
			{
			case MENU_PLAY:
				set_type_gamestate(LEVEL1);
				GameInit();
				MenuRelease();
				break;
			case MENU_HTP:
				set_type_gamestate(HOW_TO_PLAY);
				HowToPlayInit();
				MenuRelease();
				break;
			case MENU_OPTIONS:
				set_type_gamestate(OPTIONS);
				OptionsInit();
				MenuRelease();
				break;
			case MENU_EXIT:
				isApplicationRunning = false;
				break;
			}

			// plays sound queue
			audio_manager->PlaySample(menu_button_sound);
		}
	}
}

void SceneApp::MenuRelease()
{
	delete menu_screen_;
	menu_screen_ = NULL;
}

void SceneApp::MenuRender()
{
	// starts drawing all sprites and text

	sprite_renderer_->Begin();

	// menu background

	gef::Sprite menuScreenSprite;
	menuScreenSprite.set_texture(menu_screen_);
	menuScreenSprite.set_position(gef::Vector4(platform_.width() / 2, platform_.height() / 2, 0.0f));
	menuScreenSprite.set_height(544);
	menuScreenSprite.set_width(960);
	sprite_renderer_->DrawSprite(menuScreenSprite);

	// displays which option is selected on menu
	// depending on current menu state

	switch (menuStateType)
	{
	case MENU_PLAY:
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 30, -0.9f), 1.2f, 0xff0000ff, gef::TJ_CENTRE, "Begin");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 60, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "How to Play");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 90, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "Options");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 120, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "Exit the game");
		break;
	case MENU_HTP:
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 30, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "Begin");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 60, -0.9f), 1.2f, 0xff0000ff, gef::TJ_CENTRE, "How to Play");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 90, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "Options");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 120, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "Exit the game");
		break;
	case MENU_OPTIONS:
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 30, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "Begin");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 60, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "How to Play");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 90, -0.9f), 1.2f, 0xff0000ff, gef::TJ_CENTRE, "Options");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 120, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "Exit the game");
		break;
	case MENU_EXIT:
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 30, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "Begin");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 60, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "How to Play");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 90, -0.9f), 1.2f, 0xffffffff, gef::TJ_CENTRE, "Options");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 120, -0.9f), 1.2f, 0xff0000ff, gef::TJ_CENTRE, "Exit the game");
		break;
	}
	

	sprite_renderer_->End();
}

void SceneApp::OptionsInit()
{
	// initialises which state the options start on
	optionsStateType = OPTIONS_DIFFICULTY;

	// loads in options background 
	options_background_ = CreateTextureFromPNG("options-background.png", platform_);
	
}

void SceneApp::OptionsUpdate(float frame_time)
{
	if (kb)
	{
		// changes state back to menu

		if (kb->IsKeyPressed(gef::Keyboard::KC_BACKSPACE))
		{
			set_type_gamestate(MENU);
			MenuInit();
			OptionsRelease();

			// plays sound queue
			audio_manager->PlaySample(menu_button_sound);
		}

		// changes selected option state 
		// to one above

		if (kb->IsKeyPressed(gef::Keyboard::KC_UP))
		{
			switch (optionsStateType)
			{
			case OPTIONS_DIFFICULTY:
				optionsStateType = OPTIONS_VOLUME;
				break;
			case OPTIONS_VOLUME:
				optionsStateType = OPTIONS_DIFFICULTY;
				break;
			}

			// plays sound queue
			audio_manager->PlaySample(menu_button_sound);
		}


		// changes selected option state 
		// to one below

		if (kb->IsKeyPressed(gef::Keyboard::KC_DOWN))
		{
			switch (optionsStateType)
			{
			case OPTIONS_DIFFICULTY:
				optionsStateType = OPTIONS_VOLUME;
				break;
			case OPTIONS_VOLUME:
				optionsStateType = OPTIONS_DIFFICULTY;
				break;
			}

			// plays sound queue
			audio_manager->PlaySample(menu_button_sound);
		}


		// based on which option state is currently active
		
		if (kb->IsKeyPressed(gef::Keyboard::KC_RIGHT))
		{
			switch (optionsStateType)
			{
			case OPTIONS_DIFFICULTY:
				// changes difficulty state to state to the right
				switch (difficulty)
				{
				case DIFF_EASY:
					difficulty = DIFF_NORMAL;
					break;
				case DIFF_NORMAL:
					difficulty = DIFF_HARD;
					break;
				case DIFF_HARD:
					difficulty = DIFF_ONESHOT;
					break;
				case DIFF_ONESHOT:
					difficulty = DIFF_EASY;
					break;
				}
				break;
			case OPTIONS_VOLUME:
				// changes volume state to state to the right
				switch (volume)
				{
				case ONE_HUNDRED:
					volume = ZERO;
					break;
				case SEVENTY_FIVE:
					volume = ONE_HUNDRED;
					break;
				case FIFTY:
					volume = SEVENTY_FIVE;
					break;
				case TWENTY_FIVE:
					volume = FIFTY;
					break;
				case ZERO:
					volume = TWENTY_FIVE;
					break;
				}
				break;
			}

			// plays sound queue
			audio_manager->PlaySample(menu_button_sound);
			
		}

		// based on which option state is currently active

		if (kb->IsKeyPressed(gef::Keyboard::KC_LEFT))
		{
			switch (optionsStateType)
			{
			case OPTIONS_DIFFICULTY:
				// changes difficulty state to state to the left
				switch (difficulty)
				{
				case DIFF_EASY:
					difficulty = DIFF_ONESHOT;
					break;
				case DIFF_NORMAL:
					difficulty = DIFF_EASY;
					break;
				case DIFF_HARD:
					difficulty = DIFF_NORMAL;
					break;
				case DIFF_ONESHOT:
					difficulty = DIFF_HARD;
					break;
				}
				break;
			case OPTIONS_VOLUME:
				// changes volume state to state to the left
				switch (volume)
				{
				case ONE_HUNDRED:
					volume = SEVENTY_FIVE;
					break;
				case SEVENTY_FIVE:
					volume = FIFTY;
					break;
				case FIFTY:
					volume = TWENTY_FIVE;
					break;
				case TWENTY_FIVE:
					volume = ZERO;
					break;
				case ZERO:
					volume = ONE_HUNDRED;
					break;
				}
				break;
			}
			// plays sound queue
			audio_manager->PlaySample(menu_button_sound);
			
		}
	}
}

void SceneApp::OptionsRelease()
{
	delete options_background_;
	options_background_ = NULL;
}

void SceneApp::OptionsRender()
{
	// starts drawing all sprites and text

	sprite_renderer_->Begin();

	// options background

	gef::Sprite optionsBackgroundSprite;
	optionsBackgroundSprite.set_texture(options_background_);
	optionsBackgroundSprite.set_position(gef::Vector4(platform_.width() / 2, platform_.height() / 2, 0.0f));
	optionsBackgroundSprite.set_height(544);
	optionsBackgroundSprite.set_width(960);
	sprite_renderer_->DrawSprite(optionsBackgroundSprite);

	// draws selected options states differently
	// depending on which one is active
	// will always draw which state is active
	// from each the difficulty state and volume state

	switch (optionsStateType)
	{
	case OPTIONS_DIFFICULTY:
		// draws difficulty state red
		// and volume white
		// with red being active

		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) - 180, -0.9f), 1.5f, 0xff0000ff, gef::TJ_CENTRE, "Difficulty");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Master Volume");

		switch (difficulty)
		{
		case DIFF_EASY:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 225, (platform_.height() / 2) - 140, -0.9f), 0.75f, 0xffded710, gef::TJ_CENTRE, "Easy");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Normal");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Hard");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "ONESHOT");
			break;
		case DIFF_NORMAL:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Easy");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffa60f30, gef::TJ_CENTRE, "Normal");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Hard");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "ONESHOT");
			break;
		case DIFF_HARD:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Easy");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Normal");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) - 140, -0.9f), 1.25f, 0xff5303bf, gef::TJ_CENTRE, "Hard");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "ONESHOT");
			break;
		case DIFF_ONESHOT:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Easy");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Normal");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Hard");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 225, (platform_.height() / 2) - 140, -0.9f), 1.5f, 0xff0f41a6, gef::TJ_CENTRE, "ONESHOT");
			break;
		}

		switch (volume)
		{
		case ONE_HUNDRED:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "0%% - Muted");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "25%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2), (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "50%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "75%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 200, (platform_.height() / 2) + 175, -0.9f), 1.5f, 0xff0f41a6, gef::TJ_CENTRE, "100%%");
			break;
		case SEVENTY_FIVE:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "0%% - Muted");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "25%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2), (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "50%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) + 175, -0.9f), 1.3f, 0xff5303bf, gef::TJ_CENTRE, "75%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "100%%");
			break;
		case FIFTY:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "0%% - Muted");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) +175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "25%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2), (platform_.height() / 2) + 175, -0.9f), 1.1f, 0xffa60f30, gef::TJ_CENTRE, "50%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "75%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "100%%");
			break;
		case TWENTY_FIVE:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "0%% - Muted");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffded710, gef::TJ_CENTRE, "25%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2), (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "50%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "75%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "100%%");
			break;
		case ZERO:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, (platform_.height() / 2) + 175, -0.9f), 0.8f, 0xff000000, gef::TJ_CENTRE, "0%% - Muted");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "25%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2), (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "50%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "75%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "100%%");
			break;
		}
		break;

	case OPTIONS_VOLUME:
		// draws volume state red
		// and difficulty white
		// with red being active

		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) - 180, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Difficulty");
		font_->RenderText(sprite_renderer_, gef::Vector4(platform_.width() / 2, (platform_.height() / 2) + 140, -0.9f), 1.5f, 0xff0000ff, gef::TJ_CENTRE, "Master Volume");

		switch (difficulty)
		{
		case DIFF_EASY:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 225, (platform_.height() / 2) - 140, -0.9f), 0.75f, 0xffded710, gef::TJ_CENTRE, "Easy");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Normal");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Hard");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "ONESHOT");
			break;
		case DIFF_NORMAL:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Easy");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffa60f30, gef::TJ_CENTRE, "Normal");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Hard");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "ONESHOT");
			break;
		case DIFF_HARD:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Easy");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Normal");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) - 140, -0.9f), 1.25f, 0xff5303bf, gef::TJ_CENTRE, "Hard");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "ONESHOT");
			break;
		case DIFF_ONESHOT:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 225, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Easy");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Normal");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) - 140, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "Hard");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 225, (platform_.height() / 2) - 140, -0.9f), 1.5f, 0xff0f41a6, gef::TJ_CENTRE, "ONESHOT");
			break;
		}

		switch (volume)
		{
		case ONE_HUNDRED:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "0%% - Muted");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "25%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2), (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "50%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "75%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 200, (platform_.height() / 2) + 175, -0.9f), 1.5f, 0xff0f41a6, gef::TJ_CENTRE, "100%%");
			break;
		case SEVENTY_FIVE:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "0%% - Muted");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "25%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2), (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "50%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) + 175, -0.9f), 1.3f, 0xff5303bf, gef::TJ_CENTRE, "75%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "100%%");
			break;
		case FIFTY:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "0%% - Muted");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "25%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2), (platform_.height() / 2) + 175, -0.9f), 1.1f, 0xffa60f30, gef::TJ_CENTRE, "50%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "75%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "100%%");
			break;
		case TWENTY_FIVE:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "0%% - Muted");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffded710, gef::TJ_CENTRE, "25%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2), (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "50%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "75%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "100%%");
			break;
		case ZERO:
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 200, (platform_.height() / 2) + 175, -0.9f), 0.8f, 0xff000000, gef::TJ_CENTRE, "0%% - Muted");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "25%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2), (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "50%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 75, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "75%%");
			font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) + 200, (platform_.height() / 2) + 175, -0.9f), 1.0f, 0xffffffff, gef::TJ_CENTRE, "100%%");
			break;
		}
		break;
	}



	sprite_renderer_->End();
}

void SceneApp::EndGameInit()
{
	// stops level timer in order
	// to receive final time to 
	// show player

	gameTimer.GetTimeStop();

	// stops all sounds / music
	// that isnt related to end game state

	if (level_music_playing)
	{
		level_music_playing = false;
		audio_manager->StopPlayingSampleVoice(level_music);
	}

	// plays end game sound / music
	if (!player_wins_playing)
	{
		player_wins_playing = true;
		audio_manager->PlaySample(player_wins);
	}

	// loads end game background

	win_game_background = CreateTextureFromPNG("win-background.png", platform_);
}

void SceneApp::EndGameUpdate(float frame_time)
{
	if (kb)
	{
		// changes state to menu

		if (kb->IsKeyPressed(gef::Keyboard::KC_BACKSPACE))
		{
			set_type_gamestate(MENU);
			MenuInit();
			EndGameRelease();

			// plays sound queue
			audio_manager->PlaySample(menu_button_sound);
		}
	}
}

void SceneApp::EndGameRelease()
{
	delete win_game_background;
	win_game_background = NULL;
}

void SceneApp::EndGameRender()
{
	// starts drawing all sprites and text

	sprite_renderer_->Begin();

	// end game background

	gef::Sprite wingameBackgroundSprite;
	wingameBackgroundSprite.set_texture(win_game_background);
	wingameBackgroundSprite.set_position(gef::Vector4(platform_.width() / 2, platform_.height() / 2, 0.0f));
	wingameBackgroundSprite.set_height(544);
	wingameBackgroundSprite.set_width(960);
	sprite_renderer_->DrawSprite(wingameBackgroundSprite);

	// renders final time 
	// player took to complete the level
	font_->RenderText(sprite_renderer_, gef::Vector4((platform_.width() / 2) - 15, (platform_.height() / 2) + 50 , -0.9f), 2.0f, 0xffa60f30, gef::TJ_CENTRE, "%.1f", gameTimer.elapsedSeconds());

	sprite_renderer_->End();
}

void SceneApp::FailedInit()
{
	// stops timer but wont
	// be displaying it as the player failed

	gameTimer.GetTimeStop();

	// stops all sounds / music
	// playing not related to failed state

	if (level_music_playing)
	{
		level_music_playing = false;
		audio_manager->StopPlayingSampleVoice(level_music);
	}

	// plays failed sound

	if (!game_over_playing)
	{
		game_over_playing = true;

		// plays sound queue
		audio_manager->PlaySample(game_over);
	}

	// loads in failed background

	game_over_screen_ = CreateTextureFromPNG("game-over-background.png", platform_);
}

void SceneApp::FailedUpdate(float frame_time)
{

	if (kb)
	{
		// changes state to menu state

		if (kb->IsKeyPressed(gef::Keyboard::KC_SPACE))
		{
			set_type_gamestate(MENU);
			MenuInit();
			FailedRelease();

			// plays sound queue
			audio_manager->PlaySample(menu_button_sound);

		}

		// changes state back to level state
		// allows the player to retry

		if (kb->IsKeyPressed(gef::Keyboard::KC_RETURN))
		{
			set_type_gamestate(LEVEL1);
			GameInit();
			FailedRelease();

			// plays sound queue
			audio_manager->PlaySample(menu_button_sound);

		}

		
	}
}

void SceneApp::FailedRelease()
{
	delete game_over_screen_;
	game_over_screen_ = NULL;
}

void SceneApp::FailedRender()
{
	// starts drawing all sprites and text

	sprite_renderer_->Begin();

	// game over background

	gef::Sprite gameoverScreenSprite;
	gameoverScreenSprite.set_texture(game_over_screen_);
	gameoverScreenSprite.set_position(gef::Vector4(platform_.width() / 2, platform_.height() / 2, 0.0f));
	gameoverScreenSprite.set_height(544);
	gameoverScreenSprite.set_width(960);
	sprite_renderer_->DrawSprite(gameoverScreenSprite);

	sprite_renderer_->End();
}

void SceneApp::HowToPlayInit()
{
	// loads in how to play background

	htp_background_ = CreateTextureFromPNG("how-to-play-background.png", platform_);
}

void SceneApp::HowToPlayRelease()
{
	delete htp_background_;
	htp_background_ = NULL;
}

void SceneApp::HowToPlayUpdate(float frame_time)
{
	// changes state to menu state

	if (kb->IsKeyPressed(gef::Keyboard::KC_BACKSPACE))
	{
		set_type_gamestate(MENU);
		MenuInit();
		HowToPlayRelease();
		audio_manager->PlaySample(menu_button_sound);
	}
}

void SceneApp::HowToPlayRender()
{
	// starts drawing all sprites and text

	sprite_renderer_->Begin();

	// how to play background

	gef::Sprite htpScreenSprite;
	htpScreenSprite.set_texture(htp_background_);
	htpScreenSprite.set_position(gef::Vector4(platform_.width() / 2, platform_.height() / 2, 0.0f));
	htpScreenSprite.set_height(544);
	htpScreenSprite.set_width(960);
	sprite_renderer_->DrawSprite(htpScreenSprite);

	sprite_renderer_->End();
}

// update state machine
// controls which state is currently updating

void SceneApp::UpdateGameStateMachine(float frame_time)
{
	switch (gamestatetype)
	{
	case INIT:
		FrontendUpdate(frame_time);
		break;
	case MENU:
		MenuUpdate(frame_time);
		break;
	case LEVEL1:
		GameUpdate(frame_time);
		break;
	case WIN:
		EndGameUpdate(frame_time);
		break;
	case OPTIONS:
		OptionsUpdate(frame_time);
		break;
	case HOW_TO_PLAY:
		HowToPlayUpdate(frame_time);
		break;
	case LOSE:
		FailedUpdate(frame_time);
		break;
	}
}


// render state machine
// controls which state is currently rendering

void SceneApp::RenderGameStateMachine()
{
	switch (gamestatetype)
	{
	case INIT:
		FrontendRender();
		break;
	case MENU:
		MenuRender();
		break;
	case LEVEL1:
		GameRender();
		break;
	case WIN:
		EndGameRender();
		break;
	case OPTIONS:
		OptionsRender();
		break;
	case HOW_TO_PLAY:
		HowToPlayRender();
		break;
	case LOSE:
		FailedRender();
		break;
	}
}


