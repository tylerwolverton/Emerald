#pragma once
#include "Engine/Core/Rgba8.hpp"


//------------------------------------------------------------------------
struct Vec2;
struct Vec3;
struct Vec4;
struct Mat44;
struct AABB2;
struct AABB3;
struct OBB3;
class Clock;
class Camera;
class Texture;
class EventSystem;
class RenderContext;


/************************************************************************/
/*                                                                      */
/* TYPES                                                                */
/*                                                                      */
/************************************************************************/

// Only effects "world" mode stuff; 
enum eDebugRenderMode
{
	DEBUG_RENDER_ALWAYS,          // what is rendered always shows up
	DEBUG_RENDER_USE_DEPTH,       // respect the depth buffer
	DEBUG_RENDER_XRAY,            // renders twice - once darker when it should be hidden, and once more saturated when it should appear
};


/************************************************************************/
/*                                                                      */
/* FUNCTION PROTOTYPES                                                  */
/*                                                                      */
/************************************************************************/
//------------------------------------------------------------------------
//  System
//------------------------------------------------------------------------
// setup
void DebugRenderSystemStartup( RenderContext* context, EventSystem* eventSystem );    // may be used to allocate resources to the system
void DebugRenderSystemShutdown();   // cleans up the system to prevent leaks.

// control
void EnableDebugRendering();
void DisableDebugRendering();
void ClearDebugRendering();

// output
void DebugRenderBeginFrame();                   // Does nothing, here for completeness.
void DebugRenderWorldToCamera( Camera* camera );   // Draws all world objects to this camera 
void DebugRenderScreenTo( Texture* output );    // Draws all screen objects onto this texture (screen coordinate system is up to you.  I like a 1080p default)
void DebugRenderEndFrame();                     // Clean up dead objects

//------------------------------------------------------------------------
// World Rendering
//------------------------------------------------------------------------
// points
void DebugAddWorldPoint( const Vec3& pos, float size, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldPoint( const Vec3& pos, float size, const Rgba8& color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldPoint( const Vec3& pos, const Rgba8& color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldPoint( const Vec2& pos, const Rgba8& color, float duration = 0.f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// lines
void DebugAddWorldLine( const Vec3& p0, const Rgba8& p0_color, const Rgba8& p1_color,
						const Vec3& p1, const Rgba8& start_tint, const Rgba8& end_tint,
						float duration, float thickness,
						eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

void DebugAddWorldLine( const Vec3& p0, const Rgba8& p0_color, const Rgba8& p1_color,
						const Vec3& p1, const Rgba8& start_tint, const Rgba8& end_tint,
						float duration,
						eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );


void DebugAddWorldLine( const Vec3& start, const Vec3& end, const Rgba8& color, float thickness = 0.01f, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

//// line strip [extra]
//void DebugAddWorldLineStrip( uint count, vec3 const* positions,
//							 rgba start_p0_color, rgba start_pf_color,    // color of first/end point at the beginning of duration
//							 rgba end_p0_color, rgba end_pf_color,        // color of first/end point at the end of duration
//							 float duration,
//							 eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// arrows
void DebugAddWorldArrow( const Vec3& p0, const Rgba8& p0_color, const Rgba8& p1_color,
						 const Vec3& p1, const Rgba8& start_tint, const Rgba8& end_tint,
						 float duration,
						 eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldArrow( const Vec3& start, const Vec3& end, const Rgba8& color, float duration = 0.f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldArrow( const Vec2& start, const Vec2& end, const Rgba8& color, float duration = 0.f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// Quads
void DebugAddWorldQuad( const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const AABB2& uvs, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// bounds
void DebugAddWorldWireBounds( const OBB3& bounds, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldWireBounds( const OBB3& bounds, const Rgba8& color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldWireBounds( const AABB3& bounds, const Rgba8& color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

void DebugAddWorldWireSphere( const Vec3& pos, float radius, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldWireSphere( const Vec3& pos, float radius, const Rgba8& color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

void DebugAddWorldWireCylinder( const Vec3& pos0, const Vec3& pos1, float radius, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldWireCylinder( const Vec3& pos0, const Vec3& pos1, float radius, const Rgba8& color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// basis
void DebugAddWorldBasis( const Mat44& basis, float arrowLength, float duration = 0.0f, const Rgba8 & start_tint = Rgba8::WHITE, const Rgba8& end_tint = Rgba8::WHITE, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldBasis( const Mat44& basis, const Rgba8& start_tint, const Rgba8& end_tint, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldBasis( const Mat44& basis, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );

// text
// non-billboarded will be oriented in the world based on the passed in basis matrix 
void DebugAddWorldText( const Mat44& basis, const Vec2& pivot, const Rgba8& start_color, const Rgba8& end_color, float duration, float textHeight, eDebugRenderMode mode, char const* text );
void DebugAddWorldText( const Mat44& basis, const Vec2& pivot, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode, char const* text );
void DebugAddWorldTextf( const Mat44& basis, const Vec2& pivot, const Rgba8& color, float duration, float textHeight, eDebugRenderMode mode, char const* format, ... );
void DebugAddWorldTextf( const Mat44& basis, const Vec2& pivot, const Rgba8& color, char const* format, ... ); // assume DEBUG_RENDER_USE_DEPTH

// this text will always orient itself to the current camera
void DebugAddWorldBillboardText( const Vec3& origin, const Vec2& pivot, float textHeight, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode, char const* text );
void DebugAddWorldBillboardText( const Vec3& origin, const Vec2& pivot, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode, char const* text );
void DebugAddWorldBillboardTextf( const Vec3& origin, const Vec2& pivot, const Rgba8& color, float duration, eDebugRenderMode mode, char const* format, ... );
void DebugAddWorldBillboardTextf( const Vec3& origin, const Vec2& pivot, const Rgba8& color, char const* format, ... );

//// grid [extra] 
//void DebugAddWorldGrid( vec3 origin,
//						vec3 i, float iMin, float iMax, float iMinorSegment, iMajorSegment, rgba iMinorColor, rgba iMajorColor, rgba iOriginColor,
//						vec3 j, float jMin, float jMax, float jMinorSegment, jMajorSegment, rgba jMinorcolor, rgba jMajorColor, rgba jOriginColor,
//						rgba axisColor );
//void DebugAddWorldXYGrid();
//void DebugAddWorldXZGrid();
//void DebugAddWorldYZGrid();
//
//// mesh [extra]
//void DebugAddWireMeshToWorld( mat44 transform, GPUMesh* mesh, rgba start_tint, rgba end_tint, float duration, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
//void DebugAddWireMeshToWorld( mat44 transform, GPUMesh* mesh, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );


//------------------------------------------------------------------------
// Screen Rendering
//------------------------------------------------------------------------
void DebugRenderSetScreenHeight( float height ); // default to 1080.0f when system starts up.  Meaning (0,0) should always be bottom left, (aspect * height, height) is top right
AABB2 DebugGetScreenBounds();                    // useful if you want to align to top right for something

// points
void DebugAddScreenPoint( const Vec2& pos, float size, const Rgba8& start_color, const Rgba8& end_color, float duration );
void DebugAddScreenPoint( const Vec2& pos, float size, const Rgba8& color, float duration = 0.0f );
void DebugAddScreenPoint( const Vec2& pos, const Rgba8& color ); // assumed size;

// lines
void DebugAddScreenLine( const Vec2& p0, const Rgba8& p0_start_color,
						 const Vec2& p1, const Rgba8& p1_start_color,
						 const Rgba8& start_tint, const Rgba8& end_tint,
						 float duration );
void DebugAddScreenLine( const Vec2& p0, const Vec2& p1, const Rgba8& color, float duration = 0.0f );

// arrows
void DebugAddScreenArrow( const Vec2& p0, const Rgba8& p0_start_color,
						  const Vec2& p1, const Rgba8& p1_start_color,
						  const Rgba8& start_tint, const Rgba8& end_tint,
						  float duration );
void DebugAddScreenArrow( const Vec2& p0, const Vec2& p1, const Rgba8& color, float duration = 0.0f );

// quad
void DebugAddScreenQuad( const AABB2& bounds, const Rgba8& start_color, const Rgba8& end_color, float duration );
void DebugAddScreenQuad( const AABB2& bounds, const Rgba8& color, float duration = 0.0f );

// texture
void DebugAddScreenTexturedQuad( const AABB2& bounds, Texture* tex, const AABB2& uvs, const Rgba8& start_tint, const Rgba8& end_tint, float duration = 0.0f );
void DebugAddScreenTexturedQuad( const AABB2& bounds, Texture* tex, const AABB2& uvs, const Rgba8& tint, float duration = 0.0f );
void DebugAddScreenTexturedQuad( const AABB2& bounds, Texture* tex, const Rgba8& tint = Rgba8::WHITE, float duration = 0.0f ); // assume UVs are full texture

// text
void DebugAddScreenText( const Vec4& pos, const Vec2& pivot, float size, const Rgba8& start_color, const Rgba8& end_color, float duration, char const* text );
void DebugAddScreenTextf( const Vec4& pos, const Vec2& pivot, float size, const Rgba8& start_color, const Rgba8& end_color, float duration, char const* format, ... );
void DebugAddScreenTextf( const Vec4& pos, const Vec2& pivot, float size, const Rgba8& color, float duration, char const* format, ... );
void DebugAddScreenTextf( const Vec4& pos, const Vec2& pivot, float size, const Rgba8& color, char const* format, ... );
void DebugAddScreenTextf( const Vec4& pos, const Vec2& pivot, const Rgba8& color, char const* format, ... );

//// screen basis [extra]
//void DebugAddScreenBasis( vec2 screen_origin_location, mat44 basis_to_render, rgba start_tint, rgba end_tint, float duration );
//void DebugAddScreenBasis( vec2 screen_origin_location, mat44 basis_to_render, rgba tint = rgba::WHITE, float duration = 0.0f );
//
//// message log system [extra]
//void DebugAddMessage( float duration, rgba color, char const* format, ... );
//void DebugAddMessage( rgba color, char const* format, ... );
