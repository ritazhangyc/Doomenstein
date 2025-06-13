#pragma once
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include <map>


enum class Faction
{
	FACTION_DEMON,
	FACTION_MARINE,
	FACTION_MARINEPROJECTILE,
	FACTION_NEUTRAL,
	FACTION_DEMONPROJECTILE,
	FACTION_STATICOBJECT,
	FACTION_ACCOUNT
};

struct DirectionAnimation
{
	Vec3	m_lookDirection;
	SpriteAnimDefinition* m_spriteAnimation;
};

struct ActorAnimation
{
	std::string		m_animationName;
	bool			m_animationScaledBySpeed;
	float			m_animationSecondPerFrame;
	SpriteAnimPlaybackType m_animationPlaybackType;
	std::vector<DirectionAnimation> m_animationDirectionals;
};

class ActorDefinition
{
public:

	std::string		m_actorName;
	Faction			m_actorFaction;
	float			m_actorHealth;
	bool			m_canBePossessed;
	float			m_corpseLifetime;
	bool			m_visible;
	bool			m_dieOnSpawn;

	float			m_collisionRadius;
	float			m_collisionHeight;
	bool			m_collisionCollidesWithWorld;
	bool			m_collisionCollidesWithActor;

	bool			m_physicsSimulated;
	float			m_physicsWalkSpeed;
	float			m_physicsRunSpeed;
	float			m_physicsTurnSpeed;
	bool			m_physicsFlying;
	float			m_physicsDrag;

	float			m_cameraEyeHeight;
	float			m_cameraFOV;

	bool			m_aiEnabled;
	float			m_aiSightRadius;
	float			m_aiSightAngle;

	bool			m_staticObject;
	FloatRange		m_nocollisionHeightRange;


	Vec2			m_visualSize;
	Vec2			m_visualPivot;
	BillboardType	m_visualBillboardType;
	bool			m_visualLit;
	bool			m_visualRenderGrounded;
	std::string		m_visualShaderPath;
	std::string		m_visualTexture;
	SpriteSheet*	m_visualSpriteSheet;
	IntVec2			m_visualCellCount;

	std::vector<ActorAnimation> m_actorAnimations;
	std::vector<std::string> m_inventoryWeaponNames;
	std::map<std::string, std::string> m_soundsMap;

	FloatRange		m_damageOnCollide;
	float			m_impulseOnCollide;
	bool			m_dieOnCollide;
	bool			m_gravitySimulated;

	static void		InitializeActorDefs();
	static void		IntializeProjectileActorDefs();

	static std::map<std::string, ActorDefinition>		s_actorDefinitions;
};