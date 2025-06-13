#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"


Renderer* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
XboxController* g_xbox = nullptr;
Window* g_theWindow = nullptr;
App* g_theApp = nullptr;
AudioSystem* g_theAudio = nullptr;
BitmapFont* g_theFont = nullptr;

App::App()
{
}

App::~App()
{
}

void App::Startup()
{
	// create all Engine subsystems
	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_font = std::string("Data/Fonts/SquirrelFixedFont");
	devConsoleConfig.m_numLines = 35;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	InputSystemConfig inputConfig;
	g_theInput = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_aspectRatio = 2.0f;
	windowConfig.m_inputSystem = g_theInput;
	windowConfig.m_windowTitle = "Doomenstein";
	g_theWindow = new Window(windowConfig);

	RendererConfig rendererConfig;
	rendererConfig.m_window = g_theWindow;
	g_theRenderer = new Renderer(rendererConfig);

	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_theRenderer;

	AudioSystemConfig audioConfig;
	g_theAudio = new AudioSystem(audioConfig);

	// start up all Engine subsystems
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_theDevConsole->Startup();
	g_theEventSystem->StartUp();
	g_theAudio->Startup();
	DebugRenderSyetemStartup(debugRenderConfig);

	g_theEventSystem->SubscribeEventCallbackFunction("quit", App::QuitGame);
	g_theFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	
	// create and start the game
	m_game = new Game();
	m_game->StartUp();

	g_theDevConsole->AddLine(DevConsole::WARNING, "Controls");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "~ - Open Dev Console");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Mouse - Aim");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Left Mouse - Fire");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "ESC - quit");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Shift	- Sprint");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Space - Start game");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "W / S	- Move forward or back");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "A / D	- Move left or right");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Z / C	- Move down or up");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Q / E	- Roll");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "1		- Punch");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "2		- Plasma Rifle");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "3		- MachineGun");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "4		- Axe");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "5		- Pistol");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "F		- Toggle free fly mode");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "N		- Possess next actor");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "P		- Pause");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "F2/F3	- Decrease/Increase sun direction x-component by 1");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "F4/F5	- Decrease/Increase sun direction y-component by 1");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "F6/F7	- Decrease/Increase sun intensity by 0.05 ");
	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "F8/F9	- Decrease/Increase the ambient intensity by 0.05 ");
}

void App::RunMainLoop()
{
	while (!IsQuitting())
	{
		RunFrame();
	}
}

void App::Shutdown()
{
	delete m_game;
	m_game = nullptr;

	DebugRenderSystemShutdown();
	g_theRenderer->ShutDown();
	g_theWindow->ShutDown();
	g_theInput->ShutDown();
	g_theDevConsole->Shutdown();
	g_theEventSystem->Shutdown();
	g_theAudio->ShutDown();

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theWindow;
	g_theWindow = nullptr;

	delete g_theInput;
	g_theInput = nullptr;

	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	delete g_theEventSystem;
	g_theEventSystem = nullptr;

	delete g_theAudio;
	g_theAudio = nullptr;
}

void App::RunFrame()
{
	BeginFrame();
	Update();
	Render();
	EndFrame();
}

void App::HandleQuitRequested()
{
	m_isQuitting = true;
}

void App::BeginFrame()
{
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_theEventSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theAudio->BeginFrame();
	DebugRenderBeginFrame();

	if (g_theInput->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_theDevConsole->ToggleMode();
	}
}

void App::Update()
{
	if (!g_theWindow->GetWindowFocus() || g_theDevConsole->GetMode() == DevConsoleMode::APPEAR || m_game->GetCurrentGameState() == GameState::ATTRACT)
	{
		g_theInput->SetCursorMode(CursorMode::POINTER);
	}
	else
	{
		g_theInput->SetCursorMode(CursorMode::FPS);
		g_theInput->GetCursorClientDelta();
	}
	m_game->Update();
}

void App::Render() const
{
	if (m_isQuitting)
	{
		return;
	}
	m_game->Render();
	RenderDevConsole();
}

void App::RenderDevConsole() const
{
	float cameraAspect = g_theWindow->GetAspect();
	Camera devConsoleCamera = Camera();
	devConsoleCamera.SetOrthographicView(Vec2(0.0f, 0.0f), Vec2(100.0f * cameraAspect, 100.0f));
	g_theRenderer->BeginCamera(devConsoleCamera);
	AABB2 box = AABB2(Vec2(0.0f, 0.0f), Vec2(100.0f * cameraAspect, 100.0f));
	g_theDevConsole->Render(box, g_theRenderer);
	g_theRenderer->EndCamera(devConsoleCamera);
}

void App::EndFrame()
{
	DebugRenderEndFrame();
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theDevConsole->EndFrame();
	g_theAudio->EndFrame();
}

bool App::QuitGame(EventArgs& args)
{
	UNUSED(args);
	g_theApp->HandleQuitRequested();
	return false;
}

void App::ClearGameInstance()
{
	m_game = nullptr;
}

void App::ReloadGame()
{
	m_game->ShutDown();
	m_game = new Game();
	m_game->StartUp();
}
