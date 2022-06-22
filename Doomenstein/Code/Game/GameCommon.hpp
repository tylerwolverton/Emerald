#pragma once
#include <string>

class Window;
class App;
class InputSystem;
class AudioSystem;
class RenderContext;
class NetworkingSystem;
class PerformanceTracker;
class PhysicsConfig;
class Game;
class SpriteSheet;
class Camera;
struct Vec2;
struct Vec3;
struct Mat44;
struct OBB2;
struct OBB3;
struct Rgba8;


//-----------------------------------------------------------------------------------------------
// External variable declarations
//
extern Window* g_window;
extern App* g_app;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audioSystem;
extern RenderContext* g_renderer;
extern NetworkingSystem* g_networkingSystem;
extern Game* g_game;
extern PerformanceTracker* g_performanceTracker;


//-----------------------------------------------------------------------------------------------
// System typedefs
//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
class PhysicsSystem;
class Simple3DCollision;

typedef PhysicsSystem<Simple3DCollision> PhysicsSystem3D;


//-----------------------------------------------------------------------------------------------
// Game Constants
//-----------------------------------------------------------------------------------------------
constexpr float CLIENT_ASPECT = 16.f / 9.f;

constexpr float SQRT_2_OVER_2 = 0.70710678f;

constexpr float WINDOW_WIDTH = 16.f;
constexpr float WINDOW_HEIGHT = 9.f;
constexpr float WINDOW_WIDTH_PIXELS = 1920.f;
constexpr float WINDOW_HEIGHT_PIXELS = 1080.f;

constexpr float TILE_SIZE = 1.f;

constexpr float DEBUG_LINE_THICKNESS = 0.02f;

constexpr float MAX_CAMERA_SHAKE_DIST = 5.f;
constexpr float SCREEN_SHAKE_ABLATION_PER_SECOND = 1.f;


//-----------------------------------------------------------------------------------------------
// Config
//-----------------------------------------------------------------------------------------------
void PopulateGameConfig();


//-----------------------------------------------------------------------------------------------
// Billboarding
//-----------------------------------------------------------------------------------------------
enum class eBillboardStyle
{
	CAMERA_FACING_INVALID,
	CAMERA_FACING_XY,
	CAMERA_OPPOSING_XY,
	CAMERA_FACING_XYZ,
	CAMERA_OPPOSING_XYZ,
};

eBillboardStyle GetBillboardStyleFromString( const std::string& billboardStyleStr );

//-----------------------------------------------------------------------------------------------
void BillboardSpriteCameraFacingXY( const Vec3& pos, const Vec2& dimensions, const Camera& camera, Vec3* out_fourCorners );
void BillboardSpriteCameraOpposingXY( const Vec3& pos, const Vec2& dimensions, const Camera& camera, Vec3* out_fourCorners );
void BillboardSpriteCameraFacingXYZ( const Vec3& pos, const Vec2& dimensions, const Camera& camera, Vec3* out_fourCorners );
void BillboardSpriteCameraOpposingXYZ( const Vec3& pos, const Vec2& dimensions, const Camera& camera, Vec3* out_fourCorners );
