#ifndef _SCENE_APP_H
#define _SCENE_APP_H

#include <system/application.h>
#include <maths/vector2.h>
#include "primitive_builder.h"
#include <graphics/mesh_instance.h>
#include <input/input_manager.h>
#include <input/sony_controller_input_manager.h>
#include <box2d/Box2D.h>
#include <graphics/sprite.h>
#include <graphics/scene.h>
#include "game_object.h"
#include "GroundEnemy.h"
#include "Spike.h"
#include <vector>
#include "audio/audio_manager.h"
#include "Collectable.h"


// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class InputManager;
	class Renderer3D;
	class AudioManager;
}

// gamestates used
// to control overall flow of game

enum GAMESTATE
{
	INIT,
	MENU,
	LEVEL1,
	HOW_TO_PLAY,
	WIN,
	OPTIONS,
	LOSE

};

// menu states 
// control menu options

enum MENU_STATE
{
	MENU_PLAY,
	MENU_HTP,
	MENU_OPTIONS,
	MENU_EXIT

};

// option states
// control states for changing difficulty
// and changing volume

enum OPTIONS_STATE
{
	OPTIONS_DIFFICULTY,
	OPTIONS_VOLUME

};

// volume states
// used for changing volume values

enum VOLUME_SETTING
{
	ONE_HUNDRED,
	SEVENTY_FIVE,
	FIFTY,
	TWENTY_FIVE,
	ZERO

};

// difficulty states
// used for changing difficulty 

enum DIFFICULTY_SETTING
{
	DIFF_EASY,
	DIFF_NORMAL,
	DIFF_HARD,
	DIFF_ONESHOT

};

class SceneApp : public gef::Application
{
public:
	SceneApp(gef::Platform& platform);
	void Init();
	void CleanUp();
	bool Update(float frame_time);
	void Render();
	inline void set_type_gamestate(GAMESTATE type) { gamestatetype = type; }
	inline GAMESTATE gamestateType() { return gamestatetype; }

private:
	
	// Initialisor functions for all
	// objects used within the game

	void InitPlayer();

	void InitAllSpikes();

	void InitCollectables();
	void InitAllAbilities();

	// ground inits
	void InitAllGrounds();
	void InitBottomBorder();
	void InitTopBorder();
	void InitSmallPlatformsSA();
	void InitMeduimPlatformsMA();
	void InitBigPlatforms();
	void InitMovingPlatforms();
	void InitVerySmallPlats();

	// wall inits
	void InitAllWalls();
	void InitRightBorder();
	void InitLeftBorder();
	void InitBlockingWalls();
	void InitBiggerBlockingWalls();
	void InitAreaWalls();
	void InitResetWalls();

	// font functions

	void InitFont();
	void CleanUpFont();

	// hud and lights functions

	void DrawHUD();
	void SetupLights();

	// Update simulation functions
	// used for collision detection and updating
	// the box2d engine

	void UpdateSimulation(float frame_time);

	// update and render state machine functions
	// used within the game

	void UpdateGameStateMachine(float frame_time);
	void RenderGameStateMachine();

	// Asset functions
	// loading and getting the asset loaded in

	gef::Scene* LoadSceneAssets(gef::Platform& platform, const char* filename);
	gef::Mesh* GetMeshFromSceneAssets(gef::Scene* scene);

	// all enum state declarations


	GAMESTATE gamestatetype;
	PLAYER_STATE playerStateType;
	MENU_STATE menuStateType;
	DIFFICULTY_SETTING difficulty;
	OPTIONS_STATE optionsStateType;
	VOLUME_SETTING volume;
    

	//
	// FRONTEND DECLARATIONS
	//

	// Texture, sprite and scene variables
	// for loading in png files and 3d object files

	gef::Texture* splash_screen_;
	gef::Texture* menu_screen_;
	gef::Texture* game_screen_;
	gef::Texture* game_screen_diff_camera;
	gef::Sprite gameScreenSprite, gameScreenSprite2;

	gef::Texture* game_over_screen_;
	gef::Texture* options_background_;
	gef::Texture* htp_background_;
	gef::Texture* win_game_background;

	gef::Scene* scene_assets_;

	//
	// GAME DECLARATIONS
	//

	// declaring gef variables
	// used within the game

	gef::SpriteRenderer* sprite_renderer_;
	gef::Font* font_;
	gef::InputManager* input_manager_;
	gef::AudioManager* audio_manager;

	gef::Renderer3D* renderer_3d_;
	PrimitiveBuilder* primitive_builder_;

	// create the physics world
	b2World* world_;

	// player variables
	Player player_;
	b2Body* player_body_;

	// enemy variables;

	std::vector<GroundEnemy> groundEnemyVec;

	// spike variables
	gef::Mesh* spikeMesh_;
	Spike spikeObj;
	b2Body* spike_body_;

	std::vector<Spike> spikes_vec;
	std::vector<b2Body*> spikes_bodies_vec;

	// collectable / ability pickup variables

	gef::Mesh* collectableMesh;
	Collectable collectable_;
	b2Body* collectable_body_;
	bool isCollectableUp;

	gef::Mesh* dashPickupMesh;
	GameObject dashPickup_;
	b2Body* dashPickup_body_;

	gef::Mesh* doubleJumpPickupMesh;
	GameObject doubleJumpPickup_;
	b2Body* doubleJumpPickup_body_;

	gef::Mesh* resetWallPickupMesh;
	GameObject resetWallPickup_;
	b2Body* resetWallPickup_body_;


	// ground variables - solid ground, platforms, out of bounds

	gef::Mesh* bottom_border_mesh_;
	GameObject bottom_border_;
	b2Body* bottom_border_body;

	gef::Mesh* top_border_mesh_;
	GameObject top_border_;
	b2Body* top_border_body;

	// platforms - starting area

	GameObject small_platform;
	b2Body* small_platform_body;
	gef::Mesh* small_platform_mesh_SA;

	std::vector<GameObject> small_platforms_SA;
	std::vector<b2Body*> small_platform_bodies_SA;

	// platforms - middle area

	// use small platforms from SA inside MA, save creating another vector

	GameObject medium_platform;
	b2Body* medium_platform_body;
	gef::Mesh* medium_platform_mesh_MA;

	std::vector<GameObject> medium_platforms_MA;
	std::vector<b2Body*> medium_platform_bodies_MA;

	// big platforms

	GameObject big_platform;
	b2Body* big_platform_body;
	gef::Mesh* big_platform_mesh_;

	std::vector<GameObject> big_platforms_vec;
	std::vector<b2Body*> big_platform_bodies_vec;
	
	// moving platforms - all areas

	MovingPlatform movPlatform;
	b2Body* movPlatform_body;
	gef::Mesh* movPlatform_mesh;

	std::vector<MovingPlatform> movPlatformsVec;
	std::vector<b2Body*> movPlatform_bodies_vec;


	// wall variables

	// border variables
	gef::Mesh* right_border_mesh_;
	GameObject right_border_;
	b2Body* right_border_body_;

	gef::Mesh* left_border_mesh_;
	GameObject left_border_;
	b2Body* left_border_body_;

	// hard jump shortcut variables - very small wall plats

	gef::Mesh* very_small_plat_mesh_;
	GameObject very_small_plat_;
	b2Body* very_small_plat_body_;

	std::vector<GameObject> very_small_plat_vec;
	std::vector<b2Body*> very_small_plat_bodies_vec;

	// blocking walls

	gef::Mesh* blocking_wall_mesh_;
	GameObject blocking_wall_;
	b2Body* blocking_wall_body_;

	std::vector<GameObject> blocking_wall_vec;
	std::vector<b2Body*> blocking_wall_bodies_vec;

	// bigger blocking walls

	gef::Mesh* bigger_blocking_wall_mesh_;
	GameObject bigger_blocking_wall_;
	b2Body* bigger_blocking_wall_body_;

	std::vector<GameObject> bigger_blocking_wall_vec;
	std::vector<b2Body*> bigger_blocking_wall_bodies_vec;

	// area walls

	gef::Mesh* area_wall_mesh_;
	GameObject area_wall_;
	b2Body* area_wall_body_;

	std::vector<GameObject> area_walls_vec;
	std::vector<b2Body*> area_walls_bodies_vec;

	// reset wall variables

	gef::Mesh* reset_wall_mesh_;
	GameObject reset_wall_;
	b2Body* reset_wall_body_;

	std::vector<GameObject> reset_walls_vec;
	std::vector<b2Body*> reset_walls_bodies_vec;

	// control variables
	const gef::Keyboard* kb;
	const gef::SonyController* controller;

	// camera bool
	bool cameraSwitch;

	// audio variables
	
	bool menu_music_playing;
	int menu_music;

	bool level_music_playing;
	int level_music;

	bool game_over_playing;
	int game_over;

	bool player_wins_playing;
	int player_wins;

	bool ability_pickup_playing;
	int ability_pickup;

	int jump_sound;
	int dash_sound;
	int hit_sound;
	int menu_button_sound;

	// hud variable

	float fps_;

	// bool check used within the 
	// overall update function

	bool isApplicationRunning;

	// timers

	Timer frontendTimer;
	Timer gameTimer;

	// splash screen functions

	void FrontendInit();
	void FrontendRelease();
	void FrontendUpdate(float frame_time);
	void FrontendRender();

	// menu functions

	void MenuInit();
	void MenuRelease();
	void MenuUpdate(float frame_time);
	void MenuRender();

	// in-game functions

	void GameInit();
	void GameRelease();
	void GameUpdate(float frame_time);
	void GameRender();

	// end-game functions

	void EndGameInit();
	void EndGameRelease();
	void EndGameUpdate(float frame_time);
	void EndGameRender();

	// options functions

	void OptionsInit();
	void OptionsRelease();
	void OptionsUpdate(float frame_time);
	void OptionsRender();

	// how to play functions

	void HowToPlayInit();
	void HowToPlayRelease();
	void HowToPlayUpdate(float frame_time);
	void HowToPlayRender();


	// failed functions
	
	void FailedInit();
	void FailedRelease();
	void FailedUpdate(float frame_time);
	void FailedRender();


};

#endif // _SCENE_APP_H
