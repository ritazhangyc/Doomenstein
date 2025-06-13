#include "Game/Weapon.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Game/AI.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"


Weapon::Weapon(WeaponDefinitions& weaponDefinition, ActorHandle ownerHandle, Map* map)
	:m_ownerHandle(ownerHandle),
	m_weaponDefinition(weaponDefinition),
	m_map(map)
{
	m_refireTimer = new Timer(weaponDefinition.m_refireTime, map->GetActorByHandle(ownerHandle)->GetGameClock());
	m_reloadingTimer = new Timer(weaponDefinition.m_reloadTime, map->GetActorByHandle(ownerHandle)->GetGameClock());

	m_weaponFireSound = g_theAudio->CreateOrGetSound(m_weaponDefinition.m_weaponSounds["Fire"].c_str(), false);
	if (weaponDefinition.m_ammoLimit)
	{
		m_currentAmmo = weaponDefinition.m_ammoCount;
	}
}

void Weapon::Fire()
{
	if (!m_isReloading)
	{
		if (m_refireTimer->IsStopped())
		{
			m_refireTimer->Start();
			m_animationName = "Attack";

			if (m_weaponDefinition.m_weaponName == "Pistol")
			{
				CastPistol();
			}

			if (m_weaponDefinition.m_weaponName == "PlasmaRifle")
			{
				CastPlasmaRifle();
			}

			if (m_weaponDefinition.m_weaponName == "MachineGun")
			{
				CastMachineGun();
			}

			if (m_weaponDefinition.m_weaponName == "Axes")
			{
				CastAxes();
			}

			if (m_weaponDefinition.m_weaponName == "Punch")
			{
				CastPunch();
			}

		}
	}
// 	if (m_refireTimer->IsStopped())
// 	{
// 		m_refireTimer->Start();
// 		m_animationName = "Attack";
// 		Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
// 		m_weaponFirePlayBackID = g_theAudio->StartSoundAt(m_weaponFireSound, m_owner->m_position);
// 
// 	}
// 	else 
// 	{
// 		if (m_refireTimer->DecrementPeriodIfElapsed())
// 		{
// 			m_refireTimer->Stop();
// 		}
// 		return;
// 	}
// 
// 	Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
// 	m_owner->PlayAnimation("Attack");
// 	if (m_weaponDefinition.m_weaponName == "Pistol")
// 	{
// 		RaycastResult3D raycastResult3D;
// 		ActorHandle hitActorHandle = ActorHandle::INVALID;
// 		raycastResult3D = m_owner->m_map->RaycastVsAll(m_owner->GetActorWeaponStartPosition(), m_owner->m_orientation.GetIBasis(), m_weaponDefinition.m_rayRange, m_owner->GetActorHandle(), hitActorHandle);
// 		if (raycastResult3D.m_didImpact)
// 		{
// 			if (hitActorHandle != ActorHandle::INVALID)
// 			{
// 				Actor* hitActor = m_owner->m_map->GetActorByHandle(hitActorHandle);
// 				float damageValue = m_owner->m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(m_weaponDefinition.m_rayDamage.m_min, m_weaponDefinition.m_rayDamage.m_max);
// 				hitActor->Damage(damageValue, m_owner);
// 				hitActor->AddImpulse(m_weaponDefinition.m_rayImpulse * m_owner->GetOrientationOnlyYaw().GetIBasis());
// 
// 				SpawnInfo bloodSplatter = SpawnInfo();
// 				bloodSplatter.m_faction = Faction::FACTION_NEUTRAL;
// 				bloodSplatter.m_actorName = "BloodSplatter";
// 				bloodSplatter.m_position = raycastResult3D.m_impactPosition;
// 				m_map->SpawnActor(bloodSplatter);
// 			}
// 			else
// 			{
// 				SpawnInfo bulletHitInfo = SpawnInfo();
// 				bulletHitInfo.m_faction = Faction::FACTION_NEUTRAL;
// 				bulletHitInfo.m_actorName = "BulletHit";
// 				bulletHitInfo.m_position = raycastResult3D.m_impactPosition + raycastResult3D.m_impactNormal * 0.05f;
// 				m_map->SpawnActor(bulletHitInfo);
// 			}
// 		}
// 	}
// 	else if (m_weaponDefinition.m_weaponName == "PlasmaRifle")
// 	{
// 		SpawnInfo spawnInfo;
// 		spawnInfo.m_actorName = "PlasmaProjectile";
// 		spawnInfo.m_position = m_owner->GetActorWeaponStartPosition();
// 		spawnInfo.m_orientation = EulerAngles(m_owner->m_orientation.m_yawDegrees, m_owner->m_orientation.m_pitchDegrees, 0.0f);
// 		Actor* projectile = m_owner->m_map->SpawnActor(spawnInfo);
// 		projectile->m_actorDefinition.m_actorFaction = Faction::FACTION_MARINEPROJECTILE;
// 		projectile->m_owner = m_owner;
// 		projectile->m_isStatic = false;
// 		projectile->AddImpulse(GetRandomDirectionInCone() * m_weaponDefinition.m_projectileSpeed);
// 	}
// 	else if (m_weaponDefinition.m_weaponName == "DemonMelee")
// 	{
// 		Actor* hitActor = m_map->GetActorByHandle(m_owner->m_AIController->m_targetActorHandle);
// 		float damageValue = m_owner->m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(m_weaponDefinition.m_meleeDamage.m_min, m_weaponDefinition.m_meleeDamage.m_max);
// 		hitActor->Damage(damageValue, m_owner);
// 		hitActor->AddImpulse(m_weaponDefinition.m_meleeImpulse * m_owner->GetOrientationOnlyYaw().GetIBasis());
// 	}
}

void Weapon::Fire(Actor* targetActor)
{
	if (!m_isReloading)
	{
		if (m_refireTimer->IsStopped())
		{
			m_refireTimer->Start();
			m_animationName = "Attack";

			if (m_weaponDefinition.m_weaponName == "DemonMelee")
			{
				CastDemonMelee(targetActor);
			}
			if (m_weaponDefinition.m_weaponName == "Demon2Melee")
			{
				CastDemon2Melee(targetActor);
			}
			if (m_weaponDefinition.m_weaponName == "Demon2Rifle")
			{
				CastDemon2Rifle(targetActor);
			}
			if (m_weaponDefinition.m_weaponName == "ZoombieWeapon")
			{
				CastZoombieWeapon(targetActor);
			}
		}
	}
}

void Weapon::Update()
{
	if (m_isReloading)
	{
		if (m_reloadingTimer->GetElapsedTime() > m_reloadingTimer->m_period)
		{
			m_reloadingTimer->Stop();
			m_currentAmmo = m_weaponDefinition.m_ammoCount;
			m_isReloading = false;
		}
	}

	if (!m_refireTimer->IsStopped())
	{
		if (m_refireTimer->DecrementPeriodIfElapsed())
		{
			m_refireTimer->Stop();
			m_animationName = "Idle";
		}
	}

	if (m_weaponFirePlayBackID != MISSING_SOUND_ID)
	{
		if (g_theAudio->IsPlaying(m_weaponFirePlayBackID))
		{
			Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
			g_theAudio->SetSoundPosition(m_weaponFirePlayBackID, m_owner->m_position);
		}
	}
}

void Weapon::Render(int playerNum /*= 1*/)
{
	std::vector<Vertex_PCU> hudVerts;
	std::vector<Vertex_PCU>	reticleVerts;
	std::vector<Vertex_PCU> gunVerts;

	if (m_isReloading)
	{
		std::vector<Vertex_PCU> reloadVerts;
		AABB2 reticleBox = AABB2();
		reticleBox.SetCenter(0.5f * WORLD_CAMERA_TOPRIGHT);
		reticleBox.SetDimensions(Vec2((float)m_weaponDefinition.m_hudReticleSize.x * 5.0f / (float)(playerNum), (float)m_weaponDefinition.m_hudReticleSize.y * 5.0f));
		WeaponAnimation const& animationDef = m_weaponDefinition.m_weaponAnimations["Reload"];
		SpriteDefinition const& def = animationDef.m_weaponAnimation->GetSpriteDefAtTime((float)m_reloadingTimer->GetElapsedTime());
		AddVertsForAABB2D(reloadVerts, reticleBox, Rgba8::WHITE, def.GetUVs().m_mins, def.GetUVs().m_maxs);
		g_theRenderer->BindTexture(&def.GetTexture());
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetLightConstants();
		g_theRenderer->DrawVertexArray((int)reloadVerts.size(), reloadVerts.data());
	}

	AABB2 reticleBox = AABB2();
	reticleBox.SetCenter(0.5f * WORLD_CAMERA_TOPRIGHT);
	reticleBox.SetDimensions(Vec2((float)m_weaponDefinition.m_hudReticleSize.x / (float)(playerNum), (float)m_weaponDefinition.m_hudReticleSize.y));

	AddVertsForAABB2D(hudVerts, AABB2(WORLD_CAMERA_BOTTOMLEFT, Vec2(1600.0f, m_weaponDefinition.m_baseTexture->GetAspectRatio() * 1600.0f)), Rgba8::WHITE);
	AddVertsForAABB2D(reticleVerts, reticleBox, Rgba8::WHITE);

	if (m_weaponDefinition.m_hudWeaponShaderName == "Default")
	{
		g_theRenderer->BindShader(nullptr);
	}
	else
	{
		Shader* shader = g_theRenderer->CreateShader(m_weaponDefinition.m_hudWeaponShaderName.c_str());
		g_theRenderer->BindShader(shader);
	}

	g_theRenderer->BindTexture(m_weaponDefinition.m_baseTexture);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetLightConstants();
	g_theRenderer->DrawVertexArray((int)hudVerts.size(), hudVerts.data());

	g_theRenderer->BindTexture(m_weaponDefinition.m_hudReticleTexture);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetLightConstants();
	g_theRenderer->DrawVertexArray((int)reticleVerts.size(), reticleVerts.data());



	Vec2 pivot = m_weaponDefinition.m_hudSpritePivot;
	IntVec2 sizeBox = m_weaponDefinition.m_hudSpriteSize;
	WeaponAnimation const& animationDef = m_weaponDefinition.m_weaponAnimations[m_animationName];

// 	if (m_refireTimer->GetElapsedTime() > animationDef.m_weaponAnimation->GetOneClipTotalLength() && m_animationName != "Idle")
// 	{
// 		m_animationName = "Idle";
// 	}

	SpriteDefinition const& def = animationDef.m_weaponAnimation->GetSpriteDefAtTime((float)m_refireTimer->GetElapsedFraction());
	AABB2 gunBox = AABB2();
	float centerX = Interpolate((float)sizeBox.x * 0.5f, 1600.0f - (float)sizeBox.x * 0.5f, pivot.x);
	float centerY = Interpolate((float)sizeBox.y * 0.5f, 800.0f - (float)sizeBox.y * 0.5f, pivot.y);
	gunBox.SetCenter(Vec2(centerX, centerY + m_weaponDefinition.m_baseTexture->GetAspectRatio() * 1600.0f));
	gunBox.SetDimensions(Vec2((float)sizeBox.x / (float)(playerNum), (float)sizeBox.y));
	AddVertsForAABB2D(gunVerts, gunBox, Rgba8::WHITE, def.GetUVs().m_mins, def.GetUVs().m_maxs);
	if (animationDef.m_weaponAnimationShaderName == "Default")
	{
		g_theRenderer->BindShader(nullptr);
	}
	else
	{
		Shader* shader = g_theRenderer->CreateShader(animationDef.m_weaponAnimationShaderName.c_str());
		g_theRenderer->BindShader(shader);
	}
	g_theRenderer->BindTexture(&def.GetTexture());
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetLightConstants();
	g_theRenderer->DrawVertexArray((int)gunVerts.size(), gunVerts.data());
}

Vec3 Weapon::GetRandomDirectionInCone()
{
	Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);

	float latitudeDegree = m_owner->m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(0.0f, m_weaponDefinition.m_projectileCone);
	float longitudeDegree = m_owner->m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(0.0f, m_weaponDefinition.m_projectileCone);

	Vec3 localDirection = m_owner->GetModelMatrix().GetIBasis3D();

	Mat44 matrix = m_owner->GetModelMatrix();

	matrix.AppendZRotation(longitudeDegree);
	matrix.AppendYRotation(latitudeDegree);

	return matrix.GetIBasis3D();
}

void Weapon::CastPistol()
{
	Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
	m_currentAmmo -= 1;
	m_owner->PlayAnimation("Attack");
	m_weaponFirePlayBackID = g_theAudio->StartSoundAt(m_weaponFireSound, m_owner->m_position, false, 0.5f);
	RaycastResult3D raycastResult3D;
	ActorHandle hitActorUID = ActorHandle::INVALID;
	raycastResult3D = m_owner->m_map->RaycastVsAll(m_owner->GetActorWeaponStartPosition(), m_owner->m_orientation.GetIBasis(), m_weaponDefinition.m_rayRange, m_owner->GetActorHandle(), hitActorUID);
	if (raycastResult3D.m_didImpact)
	{
		if (hitActorUID != ActorHandle::INVALID)
		{
			Actor* hitActor = m_owner->m_map->GetActorByHandle(hitActorUID);
			if (!hitActor->m_actorDefinition.m_staticObject && hitActor->m_isDead == false)
			{
				float damageValue = m_owner->m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(m_weaponDefinition.m_rayDamage.m_min, m_weaponDefinition.m_rayDamage.m_max);
				hitActor->Damage(damageValue, m_owner);
				hitActor->AddImpulse(m_weaponDefinition.m_rayImpulse * m_owner->GetOrientationOnlyYaw().GetIBasis());

				SpawnInfo bloodSplatter = SpawnInfo();
				bloodSplatter.m_faction = Faction::FACTION_NEUTRAL;
				bloodSplatter.m_actorName = "BloodSplatter";
				bloodSplatter.m_position = raycastResult3D.m_impactPosition;
				m_map->SpawnActor(bloodSplatter);
			}
			else
			{
				hitActor->Damage(0.0f, m_owner);

				SpawnInfo bulletHitInfo = SpawnInfo();
				bulletHitInfo.m_faction = Faction::FACTION_NEUTRAL;
				bulletHitInfo.m_actorName = "BulletHit";
				bulletHitInfo.m_position = raycastResult3D.m_impactPosition + raycastResult3D.m_impactNormal * 0.05f;
				m_map->SpawnActor(bulletHitInfo);
			}
		}
		else
		{
			SpawnInfo bulletHitInfo = SpawnInfo();
			bulletHitInfo.m_faction = Faction::FACTION_NEUTRAL;
			bulletHitInfo.m_actorName = "BulletHit";
			bulletHitInfo.m_position = raycastResult3D.m_impactPosition + raycastResult3D.m_impactNormal * 0.05f;
			m_map->SpawnActor(bulletHitInfo);
		}
	}

	if (m_currentAmmo <= 0)
	{
		Reload();
	}

}

void Weapon::CastPunch()
{
	Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
	m_owner->PlayAnimation("Attack");
	RaycastResult3D raycastResult3D;
	ActorHandle hitActorUID = ActorHandle::INVALID;
	raycastResult3D = m_owner->m_map->RaycastVsAll(m_owner->GetActorWeaponStartPosition(), m_owner->m_orientation.GetIBasis(), m_weaponDefinition.m_rayRange, m_owner->GetActorHandle(), hitActorUID);
	if (raycastResult3D.m_didImpact)
	{
		if (hitActorUID != ActorHandle::INVALID)
		{
			Actor* hitActor = m_owner->m_map->GetActorByHandle(hitActorUID);
			if (!hitActor->m_actorDefinition.m_staticObject)
			{
				float damageValue = m_owner->m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(m_weaponDefinition.m_rayDamage.m_min, m_weaponDefinition.m_rayDamage.m_max);
				m_weaponFirePlayBackID = g_theAudio->StartSoundAt(m_weaponFireSound, m_owner->m_position, false, 0.5f);
				hitActor->Damage(damageValue, m_owner);
				hitActor->AddImpulse(m_weaponDefinition.m_rayImpulse * m_owner->GetOrientationOnlyYaw().GetIBasis());

				SpawnInfo bloodSplatter = SpawnInfo();
				bloodSplatter.m_faction = Faction::FACTION_NEUTRAL;
				bloodSplatter.m_actorName = "BloodSplatter";
				bloodSplatter.m_position = raycastResult3D.m_impactPosition;
				m_map->SpawnActor(bloodSplatter);
			}
		}
	}
}

void Weapon::CastPlasmaRifle()
{
	Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
	m_owner->PlayAnimation("Attack");
	m_weaponFirePlayBackID = g_theAudio->StartSoundAt(m_weaponFireSound, m_owner->m_position, false, 0.5f);
	SpawnInfo projectile1;
	projectile1.m_actorName = "PlasmaProjectile";
	projectile1.m_position = m_owner->GetActorWeaponStartPosition();
	projectile1.m_orientation = EulerAngles(m_owner->m_orientation.m_yawDegrees - 5.0f, m_owner->m_orientation.m_pitchDegrees, 0.0f);
	Actor* projectileActor1 = m_owner->m_map->SpawnActor(projectile1);
	projectileActor1->m_actorDefinition.m_actorFaction = Faction::FACTION_MARINEPROJECTILE;
	projectileActor1->m_owner = m_owner;
	projectileActor1->m_isStatic = false;
	Vec3 randomDirection = GetRandomDirectionInCone() * m_weaponDefinition.m_projectileSpeed;
	projectileActor1->AddImpulse(randomDirection);

	SpawnInfo projectile2;
	projectile2.m_actorName = "PlasmaProjectile";
	projectile2.m_position = m_owner->GetActorWeaponStartPosition();
	projectile2.m_orientation = EulerAngles(m_owner->m_orientation.m_yawDegrees, m_owner->m_orientation.m_pitchDegrees, 0.0f);
	Actor* projectileActor2 = m_owner->m_map->SpawnActor(projectile2);
	projectileActor2->m_actorDefinition.m_actorFaction = Faction::FACTION_MARINEPROJECTILE;
	projectileActor2->m_owner = m_owner;
	projectileActor2->m_isStatic = false;
	Vec3 randomDirection2 = GetRandomDirectionInCone() * m_weaponDefinition.m_projectileSpeed;
	projectileActor2->AddImpulse(randomDirection2);

	SpawnInfo projectile3;
	projectile3.m_actorName = "PlasmaProjectile";
	projectile3.m_position = m_owner->GetActorWeaponStartPosition();
	projectile3.m_orientation = EulerAngles(m_owner->m_orientation.m_yawDegrees + 5.0f, m_owner->m_orientation.m_pitchDegrees, 0.0f);
	Actor* projectileActor3 = m_owner->m_map->SpawnActor(projectile3);
	projectileActor3->m_actorDefinition.m_actorFaction = Faction::FACTION_MARINEPROJECTILE;
	projectileActor3->m_owner = m_owner;
	projectileActor3->m_isStatic = false;
	Vec3 randomDirection3 = GetRandomDirectionInCone() * m_weaponDefinition.m_projectileSpeed;
	projectileActor3->AddImpulse(randomDirection3);

}

void Weapon::CastMachineGun()
{
	m_currentAmmo -= 1;
	Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
	m_owner->PlayAnimation("Attack");
	m_weaponFirePlayBackID = g_theAudio->StartSoundAt(m_weaponFireSound, m_owner->m_position, false, 0.1f);
	SpawnInfo spawnInfo;
	spawnInfo.m_actorName = "MachineGunProjectile";
	spawnInfo.m_position = m_owner->GetActorWeaponStartPosition();
	spawnInfo.m_orientation = EulerAngles(m_owner->m_orientation.m_yawDegrees, m_owner->m_orientation.m_pitchDegrees, 0.0f);
	Actor* projectile = m_owner->m_map->SpawnActor(spawnInfo);
	projectile->m_actorDefinition.m_actorFaction = Faction::FACTION_MARINEPROJECTILE;
	projectile->m_owner = m_owner;
	projectile->m_isStatic = false;
	projectile->AddImpulse(GetRandomDirectionInCone() * m_weaponDefinition.m_projectileSpeed);

	if (m_currentAmmo <= 0)
	{
		Reload();
	}
}

void Weapon::CastAxes()
{
	Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
	m_owner->PlayAnimation("Attack");
	RaycastResult3D raycastResult3D;
	ActorHandle hitActorUID = ActorHandle::INVALID;
	raycastResult3D = m_owner->m_map->RaycastVsAll(m_owner->GetActorWeaponStartPosition(), m_owner->m_orientation.GetIBasis(), m_weaponDefinition.m_rayRange, m_owner->GetActorHandle(), hitActorUID);
	if (raycastResult3D.m_didImpact)
	{
		if (hitActorUID != ActorHandle::INVALID)
		{
			Actor* hitActor = m_owner->m_map->GetActorByHandle(hitActorUID);
			if (!hitActor->m_actorDefinition.m_staticObject)
			{
				float damageValue = m_owner->m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(m_weaponDefinition.m_rayDamage.m_min, m_weaponDefinition.m_rayDamage.m_max);
				m_weaponFirePlayBackID = g_theAudio->StartSoundAt(m_weaponFireSound, m_owner->m_position, false, 0.5f);
				hitActor->Damage(damageValue, m_owner);
				hitActor->AddImpulse(m_weaponDefinition.m_rayImpulse * m_owner->GetOrientationOnlyYaw().GetIBasis());

				SpawnInfo bloodSplatter = SpawnInfo();
				bloodSplatter.m_faction = Faction::FACTION_NEUTRAL;
				bloodSplatter.m_actorName = "BloodSplatter";
				bloodSplatter.m_position = raycastResult3D.m_impactPosition;
				m_map->SpawnActor(bloodSplatter);
			}
		}
	}

}

void Weapon::CastDemonMelee(Actor* targetActor)
{
	Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
	m_weaponFirePlayBackID = g_theAudio->StartSoundAt(m_weaponFireSound, m_owner->m_position, false, 0.5f);
	if (m_owner->m_currentAnimName != "Attack")
	{
		m_owner->PlayAnimation("Attack");
	}
	float damageValue = m_owner->m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(m_weaponDefinition.m_meleeDamage.m_min, m_weaponDefinition.m_meleeDamage.m_max);
	targetActor->Damage(damageValue, m_owner);
	targetActor->AddImpulse(m_weaponDefinition.m_meleeImpulse * m_owner->GetOrientationOnlyYaw().GetIBasis());
}

void Weapon::CastZoombieWeapon(Actor* targetActor)
{
	Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
	m_weaponFirePlayBackID = g_theAudio->StartSoundAt(m_weaponFireSound, m_owner->m_position, false, 0.5f);
	m_owner->PlayAnimation("Attack");
	float damageValue = m_owner->m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(m_weaponDefinition.m_meleeDamage.m_min, m_weaponDefinition.m_meleeDamage.m_max);
	targetActor->Damage(damageValue, m_owner);
	targetActor->AddImpulse(m_weaponDefinition.m_meleeImpulse * m_owner->GetOrientationOnlyYaw().GetIBasis());
}

void Weapon::CastDemon2Melee(Actor* targetActor)
{
	Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
	m_weaponFirePlayBackID = g_theAudio->StartSoundAt(m_weaponFireSound, m_owner->m_position, false, 0.5f);
	if (m_owner->m_currentAnimName != "Attack")
	{
		m_owner->PlayAnimation("Attack");
	}
	float damageValue = m_owner->m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(m_weaponDefinition.m_meleeDamage.m_min, m_weaponDefinition.m_meleeDamage.m_max);
	targetActor->Damage(damageValue, m_owner);
	targetActor->AddImpulse(m_weaponDefinition.m_meleeImpulse * m_owner->GetOrientationOnlyYaw().GetIBasis());
}

void Weapon::CastDemon2Rifle(Actor* targetActor)
{
	Actor* m_owner = m_map->GetActorByHandle(m_ownerHandle);
	m_owner->PlayAnimation("Attack2");
	m_currentAmmo -= 1;
	m_weaponFirePlayBackID = g_theAudio->StartSoundAt(m_weaponFireSound, m_owner->m_position, false, 0.5f);
	SpawnInfo spawnInfo;
	spawnInfo.m_actorName = "PlasmaProjectile";
	spawnInfo.m_position = m_owner->m_position + m_owner->GetOrientationOnlyYaw().GetKBasis() * m_owner->m_actorDefinition.m_cameraEyeHeight * 0.7f + m_owner->m_orientation.GetIBasis() * m_owner->m_actorDefinition.m_collisionRadius;
	spawnInfo.m_orientation = EulerAngles(m_owner->m_orientation.m_yawDegrees, m_owner->m_orientation.m_pitchDegrees, 0.0f);
	Actor* projectile = m_owner->m_map->SpawnActor(spawnInfo);
	projectile->m_actorDefinition.m_actorFaction = Faction::FACTION_DEMONPROJECTILE;
	projectile->m_owner = m_owner;
	projectile->m_isStatic = false;
	projectile->AddImpulse((targetActor->m_position - m_owner->m_position).GetNormalized() * m_weaponDefinition.m_projectileSpeed);

	if (m_currentAmmo <= 0)
	{
		Reload();
	}
}

void Weapon::Reload()
{
	if (m_isReloading)
	{
		return;
	}
	m_isReloading = true;
	m_reloadingTimer->Start();
}
