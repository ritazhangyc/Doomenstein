#pragma once
#include "Game/Controller.hpp"

class Game;

class AI : public Controller
{
public:
	void		DamagedBy(ActorHandle actorHandle);
	void		Update();

	void		CastEnemyAI();
	void		CastDemon2();

public:
	ActorHandle	m_targetActorHandle = ActorHandle::INVALID;
	bool		m_isPlayerAI = false;
};
