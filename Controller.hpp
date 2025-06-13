#pragma once
#include "Game/Map.hpp"
#include "Game/ActorHandle.hpp"

class Map;
class Actor;

class Controller
{
public:
	Controller();
	Controller(ActorHandle const& actorHandle, Map* mapInstance);
	~Controller();

	void				Possess(ActorHandle const& possessActorHandle);
	void				UnPossess();
	Actor*				GetActor() const;
	void				SetCurrentMap(Map* map);

	int					m_deathTimes = 0;
	int					m_killedEnemiesNum = 0;
	
	Map*				m_map = nullptr;
	ActorHandle			m_actorHandle = ActorHandle::INVALID;
};