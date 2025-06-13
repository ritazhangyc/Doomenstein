#include "Game/Controller.hpp"
#include "Game/Map.hpp"
#include "Game/Actor.hpp"

Controller::Controller(ActorHandle const& actorHandle, Map* mapInstance)
{
	m_actorHandle = actorHandle;
	m_map = mapInstance;
}

Controller::Controller()
{

}

Controller::~Controller()
{
}

void Controller::Possess(ActorHandle const& possessActorHandle)
{
	UnPossess();
	if (m_map->GetActorByHandle(possessActorHandle) != nullptr)
	{
		if ((m_map->GetActorByHandle(possessActorHandle)->m_actorDefinition.m_canBePossessed))
		{
			m_actorHandle = possessActorHandle;
			GetActor()->OnPossessed(this);
		}
	}
}

void Controller::UnPossess()
{
	Actor* m_currentActor = GetActor();
	if (m_currentActor != nullptr)
	{
		GetActor()->OnUnpossessed(this);
	}
}


Actor* Controller::GetActor() const
{
	return m_map->GetActorByHandle(m_actorHandle);
}

void Controller::SetCurrentMap(Map* map)
{
	m_map = map;
}
