#pragma once
#include "Game/Gamecommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class Player;
class Prop;
class Map;
class BitmapFont;

enum class GameState
{
	NONE,
	ATTRACT,
	LOBBY,
	PLAYING,
	COUNT
};


struct GameConfig
{
	std::string			m_defaultMap;
	float				m_musicVolume;
	std::string			m_mainMenuMusic;
	std::string			m_gameMusic;
	std::string			m_buttonClickSound;
	float				m_windowAspect;
};


class Game
{
public:
	Game();
	~Game();

	void StartUp();
	void ShutDown();
	void Update();
	void Render();

	void UpdateInput();
	void UpdatePlayer();
	void UpdateMap();
	void UpdateCamera();

	void EnterState(GameState state);
	void ExitState(GameState state);

	Player* GetCurrentPlayer() const;
	Clock*  GetGameClock() const;
	GameState GetCurrentGameState() const;

	void		JoinPlayer(int controllerIndex = -1);
	int			GetPlayerListNum() const;
	Player*		GetPlayerByIndex(int index = 0) const;
	SoundInfo*	GetSoundInfoFromList(std::string name);
	SoundPlaybackID		GetSoundFromList(std::string name);
	SoundID				GetSoundIDFromList(std::string name) const;


private:
	void AdjustForPauseAndTimeDistortion();
	void UpdateAttractMode();
/*	void UpdateCameras();*/
	void InitializeGameConfig();

	void RenderAttractMode();
	void RenderPlayingMode() const;
	void DebugRender() const;
	void UpdateGameInfo();

	void EnterPlayingGamestate();
	void EnterAttractGamestate();
	void ExitPlayingGamestate();
	void UpdateAttract();
	void UpdatePlaying();

public:
	bool		m_isAttractMode = true;
	bool		g_isDebugDraw = false;
	Shader*		m_diffuseShader;
	Map*		m_map;
	Player*		m_player;
	GameState	m_currentState = GameState::ATTRACT;
	GameState	m_nextState = GameState::ATTRACT;
	GameConfig	m_config;
	Clock*		m_gameClock;
	RandomNumberGenerator* m_rng = nullptr;
	bool		m_isSinglePlayerMode = true;
	std::vector<Player*> m_players;

private:
	Camera*		m_attractCamera;
	Camera*		m_screenCamera;
	Timer*		m_gameTimer;
	bool		m_isPausedAfterNextUpdate = false;
	bool		m_isPaused = false;
	bool		m_isSlowMo = false;
	bool		m_isCameraMode = true;
	float		m_varyTime = 0.f;
	float		m_currentFPS;
	Timer*		m_colorChangeTimer;
	BitmapFont*	m_font = nullptr;
	std::vector<SoundInfo*>	m_soundsList;

	SoundID		m_gameMusic;
	SoundPlaybackID m_gamePlayback;
};




