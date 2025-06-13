#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class Renderer;
class Window;
class InputSystem;
class App;

struct Vec2;
struct Vec3;
struct IntVec2;
struct Rgba8;

extern Renderer*		g_theRenderer; 
extern Window*			g_theWindow;
extern InputSystem*		g_theInput;
extern EventSystem*		g_theEventSystem;
extern AudioSystem*		g_theAudio;
extern App*				g_theApp;
extern BitmapFont*		g_theFont;

extern bool				g_isDebugDraw;

constexpr int		NUM_LINE_TRIS = 2;
constexpr int		NUM_LINE_VERTS = 3 * NUM_LINE_TRIS;

constexpr float		SCREEN_SIZE_X = 1600.f;
constexpr float		SCREEN_SIZE_Y = 800.f;

const Vec2	WORLD_CAMERA_BOTTOMLEFT = Vec2(0.f, 0.f);
const Vec2	WORLD_CAMERA_TOPRIGHT = Vec2(1600.f, 800.f);

const std::string DEFAULT_NAME = "NO_NAME";
const IntVec2 DEFAULT_SPRITE_COORDINATE = IntVec2(-1, -1);
const Vec3 DEFAULT_SPAWN_POSITION = Vec3(-100.0f, -100.0f, -100.0f);
const EulerAngles DEFAULT_ORIENTATION = EulerAngles(-100.0f, -100.0f, -100.0f);

void DebugDrawLine(Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
void DebugDrawRing(Vec2 const& center, float radius, float thickness, Rgba8 const& color);

BillboardType	ConvertNameToBillboardType(std::string name);
SpriteAnimPlaybackType	ConvertNameToSpriteAnimbackType(std::string name);