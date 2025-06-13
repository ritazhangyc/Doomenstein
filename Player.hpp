#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Actor.hpp"


class Game;
class Controller;

enum class CameraMode
{
	CAMERAMODE_ACTOR,
	CAMERAMODE_FREEFLY,
	CAMERAMODE_COUNT
};

class Player: public Controller
{

public:
	Player();
	Player(Game* owner);
	virtual ~Player();

	void			Update();
	void			UpdateInput();
	void			UpdateKeyBoard();
	void			UpdateController();
	void			UpdateCamera();
	void			UpdateAudio();
	void			Render();

	void			SetCameraBaseOnCameraMode();
	void			SetPlayerIndex(int index);
	int				GetPlayerIndex() const;

public:
	Game*			m_game = nullptr;
	Vec3			m_position;
	Vec3			m_velocity;
	EulerAngles		m_orientation;
	Camera*			m_playerCamera;
	Camera*			m_worldCamera;
	CameraMode		m_cameraMode = CameraMode::CAMERAMODE_FREEFLY;
	
	bool			m_isSinglePlayerMode = true;
	int				m_controllerIndex = -1; // -1 means the keyboard and mouse 
	int				m_playerIndex;

private:
	float			m_speedFactor = 1.0f;
	bool			m_singlePlayerMode = true;
};


