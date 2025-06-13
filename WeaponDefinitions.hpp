#pragma once
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include <map>

struct WeaponAnimation
{
public:
	std::string				m_weaponAnimationName;
	std::string				m_weaponAnimationShaderName;
	SpriteAnimDefinition*	m_weaponAnimation = nullptr;
	IntVec2					m_weaponAnimationCellCount;
	float					m_weaponAnimationSecondsPerFrame;
	int						m_weaponAnimationStartFrameIndex;
	int						m_weaponAnimationEndFrameIndex;
};

class WeaponDefinitions
{
public:

	std::string			m_weaponName;
	float				m_refireTime;
	int					m_rayCount;
	float				m_rayCone;
	float				m_rayRange;
	FloatRange			m_rayDamage;
	float				m_rayImpulse;

	int					m_projectileCount;
	std::string			m_projectileActorName;
	float				m_projectileCone;
	float				m_projectileSpeed;

	int					m_meleeCount;
	float				m_meleeArc;
	float				m_meleeRange;
	FloatRange			m_meleeDamage;
	float				m_meleeImpulse;

	std::string			m_hudWeaponShaderName;
	Texture*			m_baseTexture = nullptr;
	Texture*			m_hudReticleTexture = nullptr;
	IntVec2				m_hudReticleSize;
	IntVec2				m_hudSpriteSize;
	Vec2				m_hudSpritePivot;

	float				m_weaponMoveSpeed;
	bool				m_ammoLimit;
	int					m_ammoCount;
	float				m_reloadTime;

	std::map<std::string, WeaponAnimation>	m_weaponAnimations;
	std::map<std::string, std::string>	m_weaponSounds;
	static void			InitializeWeaponDefinitions();
	static std::map<std::string, WeaponDefinitions> s_weaponDefinitions;
};