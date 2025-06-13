#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Gamecommon.hpp"
#include "Game/Player.hpp"
#include "Game/Tile.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/WeaponDefinitions.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

Game::Game()
{
	m_player = new Player();
	m_gameClock = new Clock();
	m_gameTimer = new Timer(0.5f, m_gameClock);
	m_attractCamera = new Camera();
	m_screenCamera = new Camera();
}


Game::~Game()
{
}

void Game::StartUp()
{
	m_gameTimer->Start();
	m_currentFPS = 1.0f / (float)m_gameClock->GetDeltaSeconds();

	m_attractCamera->SetOrthographicView(WORLD_CAMERA_BOTTOMLEFT, WORLD_CAMERA_TOPRIGHT);
// 	m_screenCamera->SetOrthographicView(WORLD_CAMERA_BOTTOMLEFT, WORLD_CAMERA_TOPRIGHT);

	// 	IntVec2 clientDimension = g_theWindow->GetClientDimensions();
	// 	m_screenCamera->SetOrthographicView(Vec2(0.0f, 0.0f), Vec2(static_cast<float>(clientDimension.x), static_cast<float>(clientDimension.y)));

	float cameraAspect = g_theWindow->GetAspect();
	m_screenCamera->SetOrthographicView(Vec2(0.0f, 0.0f), Vec2(100.0f * cameraAspect, 100.0f));

	m_diffuseShader = g_theRenderer->CreateShader("Data/Shaders/Diffuse", VertexType::Vertex_PCUTBN);

	TileDefinition::InitializeTileDefs();
	ActorDefinition::IntializeProjectileActorDefs();
	WeaponDefinitions::InitializeWeaponDefinitions();
	ActorDefinition::InitializeActorDefs();
	MapDefinition::InitializeMapDefs();
	InitializeGameConfig();

	EnterState(GameState::ATTRACT);
}

void Game::InitializeGameConfig()
{
	XmlDocument doc;
	doc.LoadFile("Data/GameConfig.xml");
	XmlElement* rootElement = doc.RootElement();

	NamedStrings gameConfigNameString = NamedStrings();
	gameConfigNameString.PopulateFromXmlElementAttributes(*rootElement);
	m_config.m_defaultMap = gameConfigNameString.GetValue("defaultMap", DEFAULT_NAME);

	SoundInfo* mainMenuSound = new SoundInfo();
	mainMenuSound->m_soundName = "mainMenuMusic";
	mainMenuSound->m_soundID = g_theAudio->CreateOrGetSound(gameConfigNameString.GetValue("mainMenuMusic", DEFAULT_NAME));
	mainMenuSound->m_soundPlayBackID = g_theAudio->StartSound(mainMenuSound->m_soundID, true, 1.0f, 0.0f, 1.0f, true);
	m_soundsList.push_back(mainMenuSound);

	SoundInfo* buttonClickSound = new SoundInfo();
	buttonClickSound->m_soundName = "buttonClickSound";
	buttonClickSound->m_soundID = g_theAudio->CreateOrGetSound(gameConfigNameString.GetValue("buttonClickSound", DEFAULT_NAME));
	m_soundsList.push_back(buttonClickSound);

	SoundInfo* gameMusic = new SoundInfo();
	gameMusic->m_soundName = "gameMusic";
	gameMusic->m_soundID = g_theAudio->CreateOrGetSound(gameConfigNameString.GetValue("gameMusic", DEFAULT_NAME));
	m_soundsList.push_back(gameMusic);
}

void Game::ShutDown()
{
	g_theApp->ClearGameInstance();
	delete this;
}

void Game::Update()
{
	AdjustForPauseAndTimeDistortion();
	//     UpdateCameras();

	switch (m_currentState)
	{
	case GameState::ATTRACT:
		UpdateAttract();
		break;
	case GameState::PLAYING:
		UpdatePlaying();
		break;
	}
}

void Game::UpdateAttract()
{
	XboxController controller = g_theInput->GetController(0);
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		SoundID clickSound = GetSoundIDFromList("buttonClickSound");
		g_theAudio->StartSound(clickSound);
		if ((int)m_players.size() == 0)
		{
			JoinPlayer();
		}
		else if ((int)m_players.size() == 1)
		{
			if (m_players[0] != nullptr)
			{
				if (m_players[0]->m_controllerIndex == -1)
				{
					ExitState(GameState::ATTRACT);
					EnterState(GameState::PLAYING);
				}
				else
				{
					JoinPlayer();
				}
			}
			else
			{
				JoinPlayer();
			}
		}
		else if ((int)m_players.size() == 2)
		{
			Player* player1 = m_players[0];
			Player* player2 = m_players[1];
			if (player1 && player2)
			{
				ExitState(GameState::ATTRACT);
				EnterState(GameState::PLAYING);
			}
			else
			{
				JoinPlayer();
			}
		}
	}

	if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_START) && !controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_START))
	{
		SoundID clickSound = GetSoundIDFromList("buttonClickSound");
		g_theAudio->StartSound(clickSound);
		if ((int)m_players.size() == 0)
		{
			JoinPlayer(0);
		}
		else if ((int)m_players.size() == 1)
		{
			if (m_players[0] != nullptr)
			{
				if (m_players[0]->m_controllerIndex == 0)
				{
					ExitState(GameState::ATTRACT);
					EnterState(GameState::PLAYING);
				}
				else
				{
					JoinPlayer(0);
				}
			}
			else
			{
				JoinPlayer(0);
			}
		}
		else if ((int)m_players.size() == 2)
		{
			Player* player1 = m_players[0];
			Player* player2 = m_players[1];
			if (player1 && player2)
			{
				ExitState(GameState::ATTRACT);
				EnterState(GameState::PLAYING);
			}
			else
			{
				JoinPlayer(0);
			}
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		if ((int)m_players.size() == 0)
		{
			ExitState(GameState::ATTRACT);
			g_theApp->HandleQuitRequested();
			return;
		}
		else
		{
			bool isEmpty = true;

			for (int i = 0; i < (int)m_players.size(); i++)
			{
				if (m_players[i] != nullptr)
				{
					isEmpty = false;
					if (m_players[i]->m_controllerIndex == -1)
					{
						delete m_players[i];
						m_players[i] = nullptr;
						return;
					}
				}
			}
			if (isEmpty)
			{
				ExitState(GameState::ATTRACT);
				g_theApp->HandleQuitRequested();
				return;
			}
		}
	}

	if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_BACK) && !controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
	{
		SoundID clickSound = GetSoundIDFromList("buttonClickSound");
		g_theAudio->StartSound(clickSound);
		if ((int)m_players.size() == 0)
		{
			ExitState(GameState::ATTRACT);
			g_theApp->HandleQuitRequested();
			return;
		}
		else
		{
			bool isempty = true;

			for (int i = 0; i < (int)m_players.size(); i++)
			{
				if (m_players[i] != nullptr)
				{
					isempty = false;
					if (m_players[i]->m_controllerIndex == 0)
					{
						delete m_players[i];
						m_players[i] = nullptr;
						return;
					}
				}
			}
			if (isempty)
			{
				ExitState(GameState::ATTRACT);
				g_theApp->HandleQuitRequested();
				return;
			}
		}
	}
}

void Game::UpdatePlaying()
{
	// 	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	// 	{
	// 		ExitState(GameState::PLAYING);
	// 		g_theApp->ReloadGame();
	// 		return;
	// 	}
	// 
	// 	UpdateGameInfo();
	// 
	// 	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	// 	{
	// 		m_isCameraMode = !m_isCameraMode;
	// 	}
	// 
	// 	if (m_isCameraMode)
	// 	{
	// 		//float delatseconds = static_cast<float>(Clock::GetSystemClock().GetDeltaSeconds());
	// 		m_player->Update();
	// 	}
	// 	else
	// 	{
	// 		UpdateMode();
	// 	}
	// 	m_map->Update();
	UpdateInput();
	UpdatePlayer();
	UpdateMap();
	UpdateCamera();
}

void Game::Render()
{
//	g_theRenderer->BindShader(nullptr);
	switch (m_currentState)
	{
	case GameState::ATTRACT:
		RenderAttractMode();
		break;
	case GameState::PLAYING:
		RenderPlayingMode();
		break;
	}
}

void Game::UpdateInput()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		SoundID clickSound = GetSoundIDFromList("buttonClickSound");
		g_theAudio->StartSound(clickSound);
		ExitState(GameState::PLAYING);
		g_theApp->ReloadGame();
		return;
	}
	XboxController controller = g_theInput->GetController(0);

	if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_BACK))
	{
		SoundID clickSound = GetSoundIDFromList("buttonClickSound");
		g_theAudio->StartSound(clickSound);
		ExitState(GameState::PLAYING);
		g_theApp->ReloadGame();
		return;
	}
	UpdateGameInfo();
}

void Game::UpdatePlayer()
{
	if (m_currentState != GameState::PLAYING)
	{
		return;
	}
// 	m_player->Update();
// 	for (int i = 0; i < (int)m_players.size(); i++)
// 	{
// 		if (m_players[i] != nullptr)
// 		{
// 			m_players[i]->UpdateInput();
// 			m_players[i]->UpdateAudio();
// 		}
// 	}
	for (int i = 0; i < (int)m_players.size(); i++)
	{
		if (m_players[i] != nullptr)
		{
			m_players[i]->UpdateInput();
		}
	}
	for (int i = 0; i < (int)m_players.size(); i++)
	{
		if (m_players[i] != nullptr)
		{
			m_players[i]->UpdateAudio();
		}
	}
}

void Game::UpdateMap()
{
	if (m_currentState != GameState::PLAYING)
	{
		return;
	}
	m_map->Update();
}

void Game::UpdateCamera()
{
	if (m_currentState != GameState::PLAYING)
	{
		return;
	}
	Mat44 CamToRender;
 	CamToRender.SetIJK3D(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
// 	m_player->m_playerCamera->SetCameraToRenderTransform(CamToRender);

	for (int i = 0; i < (int)m_players.size(); i++)
	{
		if (m_players[i] != nullptr)
		{
			m_players[i]->UpdateCamera();
			m_players[i]->m_worldCamera->SetCameraToRenderTransform(CamToRender);
			//m_players[i]->m_playerCamera->SetCameraToRenderTransform(CamToRender);

		}
	}
// 	m_player->UpdateCamera();
}

void Game::RenderPlayingMode() const
{
// 	g_theRenderer->BeginCamera(*m_player->m_playerCamera);
// 	g_theRenderer->ClearScreen(Rgba8(100, 100, 100, 200));
// 	m_map->Render();
// 
// 	DebugRenderWorld(*m_player->m_playerCamera);
// 	g_theRenderer->EndCamera(*m_player->m_playerCamera);

  	g_theRenderer->ClearScreen(Rgba8::BLACK);

	for (int i = 0; i < (int)m_players.size(); i++)
	{
		if (m_players[i] != nullptr)
		{
			g_theRenderer->BeginCamera(*m_players[i]->m_worldCamera);
			m_map->Render(*m_players[i]->m_worldCamera, i);
			DebugRenderWorld(*m_players[i]->m_worldCamera);
			g_theRenderer->EndCamera(*m_players[i]->m_worldCamera);


			g_theRenderer->BeginCamera(*m_players[i]->m_playerCamera);
			m_players[i]->Render();
			m_map->RenderMapUIStuff();
			g_theRenderer->EndCamera(*m_players[i]->m_playerCamera);
		}
	}

	g_theRenderer->BeginCamera(*m_screenCamera);
	DebugRenderScreen(*m_screenCamera);
	g_theRenderer->EndCamera(*m_screenCamera);
}

void Game::EnterState(GameState state)
{
	switch (state)
	{
	case GameState::ATTRACT:
		m_currentState = GameState::ATTRACT;
		m_nextState = GameState::PLAYING;
		EnterAttractGamestate();
		break;
	case GameState::PLAYING:
		m_currentState = GameState::PLAYING;
		m_nextState = GameState::ATTRACT;
		EnterPlayingGamestate();
		break;
	}
}

void Game::ExitState(GameState state)
{
	switch (state)
	{
	case GameState::ATTRACT:
		m_currentState = GameState::PLAYING;
		m_nextState = GameState::ATTRACT;
		break;
	case GameState::PLAYING:
		m_currentState = GameState::ATTRACT;
		m_nextState = GameState::PLAYING;
		ExitPlayingGamestate();
		break;
	}
}

Player* Game::GetCurrentPlayer() const
{
	return m_player;
}

Clock* Game::GetGameClock() const
{
	return m_gameClock;
}

SoundInfo* Game::GetSoundInfoFromList(std::string name)
{
	for (SoundInfo* sound : m_soundsList)
	{
		if (sound != nullptr)
		{
			if (sound->m_soundName == name)
			{
				return sound;
			}
		}
	}
	return nullptr;
}

SoundPlaybackID Game::GetSoundFromList(std::string name)
{
	for (SoundInfo* sound : m_soundsList)
	{
		if (sound != nullptr)
		{
			if (sound->m_soundName == name)
			{
				return sound->m_soundPlayBackID;
			}
		}
	}
	return MISSING_SOUND_ID;
}

SoundID Game::GetSoundIDFromList(std::string name) const
{
	for (SoundInfo* sound : m_soundsList)
	{
		if (sound != nullptr)
		{
			if (sound->m_soundName == name)
			{
				return sound->m_soundID;
			}
		}
	}
	return MISSING_SOUND_ID;
}

GameState Game::GetCurrentGameState() const
{
	return m_currentState;
}

void Game::JoinPlayer(int controllerIndex /*= -1*/)
{
	for (int i = 0; i < (int)m_players.size(); i++)
	{
		if (m_players[i] == nullptr)
		{
			Player* player = new Player();
			player->m_game = this;
			int playerIndex = i;
			player->m_controllerIndex = controllerIndex;
			player->SetPlayerIndex(playerIndex);
			m_players[i] = player;
			if (i > 0)
			{
				m_isSinglePlayerMode = false;
			}
			return;
		}
	}

	Player* player = new Player();
	player->m_game = this;
	int playerIndex = (int)m_players.size();
	player->m_controllerIndex = controllerIndex;
	if (playerIndex > 0)
	{
		m_isSinglePlayerMode = false;
	}
	player->SetPlayerIndex(playerIndex);
	m_players.push_back(player);
}

int Game::GetPlayerListNum() const
{
	return (int)m_players.size();

}

Player* Game::GetPlayerByIndex(int index /*= 0*/) const
{
	return m_players[index];
}

void Game::EnterPlayingGamestate()
{
	SoundPlaybackID mainMenuMusic = GetSoundFromList("mainMenuMusic");
	g_theAudio->SetSoundPlaybackPaused(mainMenuMusic, true);
	SoundID	gamemusicID = GetSoundIDFromList("gameMusic");
 	GetSoundInfoFromList("gameMusic")->m_soundPlayBackID = g_theAudio->StartSound(gamemusicID, true);
	
	auto mapdef = MapDefinition::s_mapDefinitions.find(m_config.m_defaultMap);

// 	MapDefinition& mapDefinition = MapDefinition::s_mapDefinitions[m_config.m_defaultMap];
	if (mapdef == MapDefinition::s_mapDefinitions.end())
	{
		ERROR_AND_DIE("NO MAP DEFINITION CALLED THIS!");
	}
	m_map = new Map(mapdef->second, this);
	m_map->StartUp();

// 	m_player = new Player(this);
// 	m_player->SetCurrentMap(m_map);
	for (int i = 0; i < (int)m_players.size(); i++)
	{
		Player* player = m_players[i];
		if (player)
		{
			player->m_isSinglePlayerMode = m_isSinglePlayerMode;
			player->SetCurrentMap(m_map);
			m_map->SpawnActorForPlayer(player);
		}
	}

}

void Game::EnterAttractGamestate()
{
	SoundPlaybackID mainMenuMusic = GetSoundFromList("mainMenuMusic");
	g_theAudio->SetSoundPlaybackPaused(mainMenuMusic, false);
}

void Game::ExitPlayingGamestate()
{
 	g_theAudio->StopSound(GetSoundInfoFromList("gameMusic")->m_soundPlayBackID);
	delete m_map;
	m_map = nullptr;

// 	delete m_player;
// 	m_player = nullptr;
	for (int i = 0; i < (int)m_players.size(); i++)
	{
		if (m_players[i] != nullptr)
		{
			delete m_players[i];
		}
	}
	m_players.clear();
}

void Game::AdjustForPauseAndTimeDistortion()
{
	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_gameClock->TogglePause();
	}

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_gameClock->StepSingleFrame();
	}

// 	if (g_theInput->WasKeyJustPressed('T'))
// 	{
// 		if (m_gameClock->GetTimeScale() == 0.1f)
// 		{
// 			m_gameClock->SetTimeScale(1.0f);
// 		}
// 		else
// 		{
// 			m_gameClock->SetTimeScale(0.1f);
// 		}
// 	}
}

void Game::UpdateAttractMode()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		ExitState(GameState::ATTRACT);
		EnterState(GameState::PLAYING);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		ExitState(GameState::ATTRACT);
		g_theApp->HandleQuitRequested();
		return;
	}

}

// void Game::UpdateCameras()
// {
// 	//     Mat44 CamToRender;
// 	//     CamToRender.SetIJK3D(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
// 	//     m_player->m_playerCamera->SetCameraToRenderTransform(CamToRender);
// 	if (m_currentState != GameState::PLAYING)
// 	{
// 		return;
// 	}
// 	m_player->UpdateCamera();
// }

void Game::RenderAttractMode()
{
	g_theRenderer->BeginCamera(*m_attractCamera);
	g_theRenderer->ClearScreen(Rgba8::BLACK);
	std::vector<Vertex_PCU> verts;

	std::vector<Vertex_PCU> inGameUI;
	inGameUI.reserve(60);
	AABB2 mapUIBox = AABB2(Vec2(0.f, 0.f), Vec2(1600.0f, 800.0f));
	AddVertsForAABB2D(inGameUI, mapUIBox, Rgba8::WHITE);
	Texture* dialogTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Doom.png");
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(dialogTexture);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetLightConstants();
	g_theRenderer->DrawVertexArray((int)inGameUI.size(), inGameUI.data());

	if ((int)m_players.size() == 0)
	{
		const static AABB2 box1 = AABB2(Vec2(400.0f, 120.0f), Vec2(1200.0f, 240.0f));
		const static AABB2 box2 = AABB2(Vec2(400.0f, 100.0f), Vec2(1200.0f, 200.0f));
		const static AABB2 box3 = AABB2(Vec2(400.0f, 80.0f), Vec2(1200.0f, 160.0f));
		g_theFont->AddVertsForTextInBox2D(verts, "Press Space to join with mouse and Keyboard", box1, 15.0f);
		g_theFont->AddVertsForTextInBox2D(verts, "Press Start to join with Controller", box2, 15.0f);
		g_theFont->AddVertsForTextInBox2D(verts, "Press Esc or Back to exit", box3, 15.0f);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(&g_theFont->GetTexture());
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetLightConstants();
		g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
	}
	if ((int)m_players.size() == 1)
	{
		if (m_players[0] != nullptr)
		{
			AABB2 titleBox1 = AABB2(Vec2(400.0f, 200.0f), Vec2(1200.0f, 400.0f));
			AABB2 titleBox2 = AABB2(Vec2(400.0f, 180.0f), Vec2(1200.0f, 320.0f));
			AABB2 box2 = AABB2(Vec2(400.0f, 160.0f), Vec2(1200.0f, 200.0f));
			AABB2 box3 = AABB2(Vec2(400.0f, 120.0f), Vec2(1200.0f, 160.0f));
			AABB2 box4 = AABB2(Vec2(400.0f, 80.0f), Vec2(1200.0f, 120.0f));
			std::string mainTitle1;
			std::string mainTitle2;
			std::string subTitle1;
			std::string subTitle2;
			std::string subTitle3;

			if (m_players[0]->m_controllerIndex == -1)
			{
				mainTitle1 = "Player1";
				mainTitle2 = "Mouse and Keyboard";
				subTitle1 = "Press SPACE to start game";
				subTitle2 = "Press ESC to leave game";
				subTitle3 = "Press START to join player";
			}
			else
			{
				mainTitle1 = "Player1";
				mainTitle2 = "Controller";
				subTitle1 = "Press START to start game";
				subTitle2 = "Press BACK to leave game";
				subTitle3 = "Press SPACE to join player";
			}
			g_theFont->AddVertsForTextInBox2D(verts, mainTitle1, titleBox1, 40.0f);
			g_theFont->AddVertsForTextInBox2D(verts, mainTitle2, titleBox2, 15.0f);
			g_theFont->AddVertsForTextInBox2D(verts, subTitle1, box2, 20.0f);
			g_theFont->AddVertsForTextInBox2D(verts, subTitle2, box3, 20.0f);
			g_theFont->AddVertsForTextInBox2D(verts, subTitle3, box4, 20.0f);
			g_theRenderer->BindShader(nullptr);
			g_theRenderer->BindTexture(&g_theFont->GetTexture());
			g_theRenderer->SetModelConstants();
			g_theRenderer->SetLightConstants();
			g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
		}
		else
		{
			AABB2 box1 = AABB2(Vec2(400.0f, 200.0f), Vec2(1200.0f, 240.0f));
			AABB2 box2 = AABB2(Vec2(400.0f, 160.0f), Vec2(1200.0f, 200.0f));
			AABB2 box3 = AABB2(Vec2(400.0f, 120.0f), Vec2(1200.0f, 160.0f));
			g_theFont->AddVertsForTextInBox2D(verts, "Press Space to join with Keyboard and Mouse", box1, 20.0f);
			g_theFont->AddVertsForTextInBox2D(verts, "Press Start to join with Controller", box2, 20.0f);
			g_theFont->AddVertsForTextInBox2D(verts, "Press Esc or Back to exit", box3, 20.0f);
			g_theRenderer->BindShader(nullptr);
			g_theRenderer->BindTexture(&g_theFont->GetTexture());
			g_theRenderer->SetModelConstants();
			g_theRenderer->SetLightConstants();
			g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
		}
	}
	if ((int)m_players.size() == 2)
	{
		Player* player1 = m_players[0];
		Player* player2 = m_players[1];

		if (player1 || player2)
		{
			if (player1 && player2)
			{
				AABB2 box1 = AABB2(Vec2(400.0f, 600.0f), Vec2(1200.0f, 800.0f));
				AABB2 box2 = AABB2(Vec2(400.0f, 560.0f), Vec2(1200.0f, 600.0f));
				AABB2 box3 = AABB2(Vec2(400.0f, 520.0f), Vec2(1200.0f, 560.0f));
				AABB2 box4 = AABB2(Vec2(400.0f, 480.0f), Vec2(1200.0f, 520.0f));

				AABB2 box5 = AABB2(Vec2(400.0f, 200.0f), Vec2(1200.0f, 400.0f));
				AABB2 box6 = AABB2(Vec2(400.0f, 160.0f), Vec2(1200.0f, 200.0f));
				AABB2 box7 = AABB2(Vec2(400.0f, 120.0f), Vec2(1200.0f, 160.0f));
				AABB2 box8 = AABB2(Vec2(400.0f, 80.0f), Vec2(1200.0f, 120.0f));

				std::string mainTitle1;
				std::string mainTitle2;
				std::string subTitle1;
				std::string subTitle2;
				std::string subTitle4;
				std::string subTitle5;

				if (player1->m_controllerIndex == -1)
				{
					mainTitle1 = "Player1: Mouse and Keyboard";
					subTitle1 = "Press SPACE to start game";
					subTitle2 = "Press ESC to leave game";

					mainTitle2 = "Player2: Controller";
					subTitle4 = "Press START to start game";
					subTitle5 = "Press BACK to leave game";

					g_theFont->AddVertsForTextInBox2D(verts, mainTitle1, box1, 25.0f);
					g_theFont->AddVertsForTextInBox2D(verts, subTitle1, box2, 15.0f);
					g_theFont->AddVertsForTextInBox2D(verts, subTitle2, box3, 15.0f);

					g_theFont->AddVertsForTextInBox2D(verts, mainTitle2, box5, 25.0f);
					g_theFont->AddVertsForTextInBox2D(verts, subTitle4, box6, 15.0f);
					g_theFont->AddVertsForTextInBox2D(verts, subTitle5, box7, 15.0f);

					g_theRenderer->BindShader(nullptr);
					g_theRenderer->BindTexture(&g_theFont->GetTexture());
					g_theRenderer->SetModelConstants();
					g_theRenderer->SetLightConstants();
					g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());

				}
				else
				{
					mainTitle1 = "Player2: Mouse and Keyboard";
					subTitle1 = "Press SPACE to start game";
					subTitle2 = "Press ESC to leave game";

					mainTitle2 = "Player1: Controller";
					subTitle4 = "Press START to start game";
					subTitle5 = "Press BACK to leave game";

					g_theFont->AddVertsForTextInBox2D(verts, mainTitle2, box1, 25.0f);
					g_theFont->AddVertsForTextInBox2D(verts, subTitle4, box2, 15.0f);
					g_theFont->AddVertsForTextInBox2D(verts, subTitle5, box3, 15.0f);

					g_theFont->AddVertsForTextInBox2D(verts, mainTitle1, box5, 25.0f);
					g_theFont->AddVertsForTextInBox2D(verts, subTitle1, box6, 15.0f);
					g_theFont->AddVertsForTextInBox2D(verts, subTitle2, box7, 15.0f);

					g_theRenderer->BindShader(nullptr);
					g_theRenderer->BindTexture(&g_theFont->GetTexture());
					g_theRenderer->SetModelConstants();
					g_theRenderer->SetLightConstants();
					g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());


				}

			}
			else
			{
				AABB2 box1 = AABB2(Vec2(400.0f, 200.0f), Vec2(1200.0f, 400.0f));
				AABB2 box2 = AABB2(Vec2(400.0f, 160.0f), Vec2(1200.0f, 200.0f));
				AABB2 box3 = AABB2(Vec2(400.0f, 120.0f), Vec2(1200.0f, 160.0f));
				AABB2 box4 = AABB2(Vec2(400.0f, 80.0f), Vec2(1200.0f, 120.0f));
				std::string mainTitle1;
				std::string subTitle1;
				std::string subTitle2;
				std::string subTitle3;
				if (player1 != nullptr)
				{
					if (player1->m_controllerIndex == -1)
					{
						mainTitle1 = "Player1: Mouse and Keyboard";
						subTitle1 = "Press SPACE to start game";
						subTitle2 = "Press ESC to leave game";
						subTitle3 = "Press START to join player";
					}
					else
					{
						mainTitle1 = "Player1: Controller";
						subTitle1 = "Press START to start game";
						subTitle2 = "Press BACK to leave game";
						subTitle3 = "Press SPACE to join player";
					}
				}

				if (player2 != nullptr)
				{
					if (player2->m_controllerIndex == -1)
					{
						mainTitle1 = "Player2: Mouse and Keyboard";
						subTitle1 = "Press SPACE to start game";
						subTitle2 = "Press ESC to leave game";
						subTitle3 = "Press START to join player";
					}
					else
					{
						mainTitle1 = "Player2: Controller";
						subTitle1 = "Press START to start game";
						subTitle2 = "Press BACK to leave game";
						subTitle3 = "Press SPACE to join player";
					}

				}

				g_theFont->AddVertsForTextInBox2D(verts, mainTitle1, box1, 40.0f);
				g_theFont->AddVertsForTextInBox2D(verts, subTitle1, box2, 20.0f);
				g_theFont->AddVertsForTextInBox2D(verts, subTitle2, box3, 20.0f);
				g_theFont->AddVertsForTextInBox2D(verts, subTitle3, box4, 20.0f);
				g_theRenderer->BindShader(nullptr);
				g_theRenderer->BindTexture(&g_theFont->GetTexture());
				g_theRenderer->SetModelConstants();
				g_theRenderer->SetLightConstants();
				g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());

			}
		}
		else
		{
			const static AABB2 box1 = AABB2(Vec2(400.0f, 200.0f), Vec2(1200.0f, 240.0f));
			const static AABB2 box2 = AABB2(Vec2(400.0f, 160.0f), Vec2(1200.0f, 200.0f));
			const static AABB2 box3 = AABB2(Vec2(400.0f, 120.0f), Vec2(1200.0f, 160.0f));
			g_theFont->AddVertsForTextInBox2D(verts, "Press Space to join with Keyboard and Mouse", box1, 20.0f);
			g_theFont->AddVertsForTextInBox2D(verts, "Press Start to join with Controller", box2, 20.0f);
			g_theFont->AddVertsForTextInBox2D(verts, "Press Esc or Back to exit", box3, 20.0f);
			g_theRenderer->BindShader(nullptr);
			g_theRenderer->BindTexture(&g_theFont->GetTexture());
			g_theRenderer->SetModelConstants();
			g_theRenderer->SetLightConstants();
			g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
		}
	}
	g_theRenderer->EndCamera(*m_attractCamera);
}


void Game::DebugRender() const
{
	if (!g_isDebugDraw)
	{
		return;
	}
}

void Game::UpdateGameInfo()
{
// 	IntVec2 clientDimension = g_theWindow->GetClientDimensions();
// 	float clientDimensionX = static_cast<float>(clientDimension.x);
// 	float clientDimensionY = static_cast<float>(clientDimension.y);

	// 	std::string playerInfo = Stringf("Player position: %.2f, %.2f, %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z);
	// 	DebugAddScreenText(playerInfo, Vec2(20.f, clientDimensionY - 30.f), 15.f, Vec2(0.5, 0.5f), 0);

	if (m_gameTimer->DecrementPeriodIfElapsed())
	{
		m_currentFPS = 1.0f / (float)m_gameClock->GetDeltaSeconds();
	}

	std::string gameInfo = Stringf("Time: %.2f Scale: %.2f FPS: %.f ", (float)m_gameClock->GetTotalSeconds(), 1.f, m_currentFPS);
	DebugAddScreenText(gameInfo, Vec2(130.f, 95.f), 2.f, Vec2(0.5, 0.5f), 0);

	// debug info
// 	std::string playerInfo = Stringf("PositionX: %.2f PositionY: %.2f Yaw: %.2f Pitch: %.2f Roll: %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_orientation.m_yawDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees);
// 	DebugAddScreenText(playerInfo, Vec2(50.f, 40.f), 2.f, Vec2(0.5, 0.5f), 0);
}
