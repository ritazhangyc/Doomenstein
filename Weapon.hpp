#pragma once
#include "Game/GameCommon.hpp"
#include "Game/ActorHandle.hpp"
#include "Game/WeaponDefinitions.hpp"
#include "Engine/Core/Timer.hpp"

class Actor;
class Map;
class Weapon 
{
public:
	Weapon(WeaponDefinitions& weaponDefinition, ActorHandle ownerHandle, Map* map);
	void				Fire();
	void				Fire(Actor* targetActor);

	void				Update();
	void				Render(int playerNum = 1);
	Vec3				GetRandomDirectionInCone();

	void				CastPistol();
	void				CastPunch();
	void				CastPlasmaRifle();
	void				CastMachineGun();
	void				CastAxes();

	void				CastDemonMelee(Actor* targetActor);
	void				CastZoombieWeapon(Actor* targetActor);
	void				CastDemon2Melee(Actor* targetActor);
	void				CastDemon2Rifle(Actor* targetActor);

	void				Reload();


public:
	std::string			m_animationName = "Idle";
	SoundID				m_weaponFireSound = MISSING_SOUND_ID;
	SoundPlaybackID		m_weaponFirePlayBackID = MISSING_SOUND_ID;
	WeaponDefinitions	m_weaponDefinition;
	ActorHandle			m_ownerHandle;
	Map*				m_map;
	Timer*				m_refireTimer;
	Timer*				m_reloadingTimer;


	int					m_currentAmmo = -1;
	bool				m_isReloading = false;
};