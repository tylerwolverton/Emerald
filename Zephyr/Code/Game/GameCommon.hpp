#pragma once
#include <string>

class Window;
class App;
class InputSystem;
class AudioSystem;
class RenderContext;
class Game;
class PerformanceTracker;
class SpriteSheet;
struct Vec2;
struct Rgba8;


//-----------------------------------------------------------------------------------------------
// External variable declarations
//
extern Window* g_window;
extern App* g_app;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audioSystem;
extern RenderContext* g_renderer;
extern Game* g_game;
extern PerformanceTracker* g_performanceTracker;


//-----------------------------------------------------------------------------------------------
// System typedefs
//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
class PhysicsSystem;
class GJK2DCollision;

typedef PhysicsSystem<GJK2DCollision> PhysicsSystem2D;


//-----------------------------------------------------------------------------------------------
// Global Functions
//
void PopulateGameConfig();


//-----------------------------------------------------------------------------------------------
// Game Constants
//
constexpr float CLIENT_ASPECT = 16.f / 9.f;

constexpr float SQRT_2_OVER_2 = 0.70710678f;

constexpr float WINDOW_WIDTH = 16.f;
constexpr float WINDOW_HEIGHT = 9.f;
constexpr float WINDOW_WIDTH_PIXELS = 1920.f;
constexpr float WINDOW_HEIGHT_PIXELS = 1080.f;

constexpr float TILE_SIZE = 1.f;

constexpr float PLAYER_PHYSICS_RADIUS = .25f;
constexpr float PLAYER_COSMETIC_RADIUS = .4f;
constexpr float PLAYER_SPEED = .25f;
constexpr float PLAYER_MAX_SPEED = 1.f;

constexpr float ACTOR_WANDER_DIRECTION_CHANGE_COOLDOWN_SECONDS = 3.f;

constexpr float PHYSICS_FRICTION_FRACTION = .2f;

constexpr float DEBUG_LINE_THICKNESS = 0.02f;

constexpr float MAX_CAMERA_SHAKE_DIST = 5.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 1.f;


//-----------------------------------------------------------------------------------------------
enum eCollisionLayer : unsigned int
{
	NONE,
	STATIC_ENVIRONMENT,
	PLAYER,
	PLAYER_PROJECTILE,
	ENEMY,
	ENEMY_PROJECTILE,
	PORTAL,
	PICKUP,
	NPC,
};

eCollisionLayer GetCollisionLayerFromString( const std::string& layerStr );
