#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"

class Game;

class App
{
public:
	App();
	~App();

	void Startup();
	void RunMainLoop();
	void Render() const;
	void RunFrame();
	void BeginFrame();
	void Update();
	void EndFrame();
	void Shutdown();
		
	void HandleQuitRequested();
	bool IsQuitting() const { return m_isQuitting; }
	void RenderDevConsole() const;
	static bool	QuitGame(EventArgs& args);
	void ClearGameInstance();
	void ReloadGame();

private:
	Game*	m_game = nullptr;
	float	m_timeLastFrameStart = 0.0f;
	bool	m_isQuitting = false;
};

