#include "Game/AI.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"
#include "Game/Tile.hpp"
#include "Game/Weapon.hpp"
#include "Game/WeaponDefinitions.hpp"
#include "Engine/Core/Clock.hpp"

void AI::DamagedBy(ActorHandle actorHandle)
{
	m_targetActorHandle = actorHandle;
}

void AI::Update()
{
	Actor* currentActor = m_map->GetActorByHandle(m_actorHandle);
	if (currentActor->m_isDead)
	{
		return;
	}
	if (currentActor == nullptr)
	{
		return;
	}
	if (currentActor != nullptr && currentActor->m_actorDefinition.m_actorFaction == Faction::FACTION_MARINE)
	{
		return;
	}
	if (m_isPlayerAI)
	{
		if (currentActor->m_actorDefinition.m_actorFaction != Faction::FACTION_MARINE)
		{
			return;
		}
	}
	else
	{
		if (currentActor->m_actorDefinition.m_actorFaction != Faction::FACTION_DEMON)
		{
			return;
		}
		if (currentActor->m_actorDefinition.m_actorName == "Demon2")
		{
			CastDemon2();
		}
		else
		{
			CastEnemyAI();
		}
	}

}
void AI::CastEnemyAI()
{
	Actor* currentActor = m_map->GetActorByHandle(m_actorHandle);

	if (currentActor->m_isDead == false)
	{
		Actor* targetActor = m_map->GetActorByHandle(m_targetActorHandle);

		if (targetActor != nullptr && targetActor->m_isDead == false)
		{
			Vec2 targetActorVector = Vec2(targetActor->m_position.x - currentActor->m_position.x, targetActor->m_position.y - currentActor->m_position.y);
			float targetDegree = targetActorVector.GetOrientationDegrees();
			currentActor->TurnInDirection(targetDegree, currentActor->m_actorDefinition.m_physicsTurnSpeed * (float)currentActor->GetGameClock()->GetDeltaSeconds());
			float radiusPlusAmount = targetActor->m_actorDefinition.m_collisionRadius + currentActor->m_actorDefinition.m_collisionRadius + currentActor->GetEquippedWeapon()->m_weaponDefinition.m_meleeRange;
			if (targetActorVector.GetLengthSquared() > radiusPlusAmount * radiusPlusAmount)
			{
				currentActor->MoveInDirection(currentActor->GetOrientationOnlyYaw().GetIBasis(), currentActor->m_actorDefinition.m_physicsRunSpeed);
				if (currentActor->m_currentAnimName != "Walk")
				{
					currentActor->PlayAnimation("Walk");
				}
			}
			else
			{
				currentActor->Attack(targetActor);
			}
		}
		else
		{
			if (currentActor->m_currentAnimName != "Walk")
			{
				currentActor->PlayAnimation("Walk");
			}
			currentActor->MoveInDirection(currentActor->GetOrientationOnlyYaw().GetIBasis(), currentActor->m_actorDefinition.m_physicsWalkSpeed);
			if (currentActor->m_isDead == false)
			{
				Actor* targetEnemyActor = currentActor->m_map->GetClosestVisibleEnemy(currentActor);

				if (targetEnemyActor != nullptr && targetEnemyActor->m_isDead == false)
				{
					m_targetActorHandle = targetEnemyActor->m_actorHandle;
				}
			}
		}
	}

}

void AI::CastDemon2()
{
	Actor* currentActor = m_map->GetActorByHandle(m_actorHandle);

	if (currentActor->m_isDead == false)
	{
		Actor* targetActor = m_map->GetActorByHandle(m_targetActorHandle);
		if (targetActor != nullptr && targetActor->m_isDead == false)
		{
			Vec2 targetActorVector = Vec2(targetActor->m_position.x - currentActor->m_position.x, targetActor->m_position.y - currentActor->m_position.y);
			float targetDegree = targetActorVector.GetOrientationDegrees();
			currentActor->TurnInDirection(targetDegree, currentActor->m_actorDefinition.m_physicsTurnSpeed * (float)currentActor->GetGameClock()->GetDeltaSeconds());

			float radiusPlusAmount = targetActor->m_actorDefinition.m_collisionRadius + currentActor->m_actorDefinition.m_collisionRadius + currentActor->GetEquippedWeapon()->m_weaponDefinition.m_meleeRange;
			float speed = currentActor->m_actorDefinition.m_physicsWalkSpeed;
			if (targetActorVector.GetLengthSquared() > 20.0f)
			{
				if (currentActor->m_currentWeapon != currentActor->m_weapons[1])
				{
					currentActor->m_currentWeapon = currentActor->m_weapons[1];
				}
				currentActor->Attack(targetActor);
			}
			else if (targetActorVector.GetLengthSquared() >= radiusPlusAmount * radiusPlusAmount + 0.5f && targetActorVector.GetLengthSquared() <= 20.0f)
			{
				if (currentActor->m_currentAnimName != "Walk")
				{
					currentActor->PlayAnimation("Walk");
				}
				speed = currentActor->m_actorDefinition.m_physicsRunSpeed;
			}
			else if (targetActorVector.GetLengthSquared() < radiusPlusAmount * radiusPlusAmount + 0.5f)
			{
				if (currentActor->m_currentWeapon != currentActor->m_weapons[0])
				{
					currentActor->m_currentWeapon = currentActor->m_weapons[0];
				}
				currentActor->Attack(targetActor);
			}
			currentActor->MoveInDirection(currentActor->GetOrientationOnlyYaw().GetIBasis(), speed);
		}
		else
		{
			if (currentActor->m_currentAnimName != "Walk")
			{
				currentActor->PlayAnimation("Walk");
			}
			currentActor->MoveInDirection(currentActor->GetOrientationOnlyYaw().GetIBasis(), currentActor->m_actorDefinition.m_physicsWalkSpeed);
			if (currentActor->m_isDead == false)
			{
				Actor* targetEnemyActor = currentActor->m_map->GetClosestVisibleEnemy(currentActor);

				if (targetEnemyActor != nullptr && targetEnemyActor->m_isDead == false)
				{
					m_targetActorHandle = targetEnemyActor->m_actorHandle;
				}
			}
		}
	}
}

