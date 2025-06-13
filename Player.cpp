#include "Game/Player.hpp"
#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"


Player::Player() :
	Controller()
{
	m_worldCamera = new Camera();
	m_playerCamera = new Camera();
	g_theAudio->SetNumListeners(m_playerIndex);
}

Player::Player(Game* owner)
	:m_game(owner)
{
	m_playerCamera = new Camera();
	m_worldCamera = new Camera();
// 	m_playerCamera->SetPerspectiveView(g_theWindow->GetAspect(), 60.0f, 0.1f, 100.0f);
// 	m_position = Vec3(2.5f, 8.5, 0.5f);
// 	m_orientation = EulerAngles(5.0f, 0.0f, 0.0f);
	g_theAudio->SetNumListeners(m_playerIndex);
}

Player::~Player()
{

}

void Player::Update()
{
// 	float deltaseconds = static_cast<float>(m_game->m_gameClock->GetDeltaSeconds());
	UpdateInput();
	UpdateCamera();
}

void Player::UpdateInput()
{
// 	float deltaseconds = static_cast<float>(m_game->m_gameClock->GetDeltaSeconds());
	UpdateKeyBoard();
	UpdateController();
}

void Player::UpdateKeyBoard()
{
	if (m_controllerIndex != -1)
	{
		return;
	}
	if (g_theInput->WasKeyJustPressed('F'))
	{
		if (m_singlePlayerMode)
		{
			if (m_cameraMode == CameraMode::CAMERAMODE_ACTOR)
			{
				UnPossess();
				m_cameraMode = CameraMode::CAMERAMODE_FREEFLY;
			}
			else
			{
				Possess(m_actorHandle);
				m_cameraMode = CameraMode::CAMERAMODE_ACTOR;
			}
			SetCameraBaseOnCameraMode();
		}
	}
	if (g_theInput->WasKeyJustPressed(' '))
	{
		m_game->m_map->LoadNextUIText();
	}

	switch (m_cameraMode)
	{
	Actor* actor;
	case CameraMode::CAMERAMODE_ACTOR:
		actor = GetActor();
		if (actor != nullptr)
		{
			if (actor->m_isDead)
			{
				return;
			}

			if (actor->m_actorDefinition.m_actorName == "Marine")
			{
				if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW))
				{
					actor->EquipPreviousWeapon();
				}

				if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW))
				{
					actor->EquipNextWeapon();
				}

				if (g_theInput->WasKeyJustPressed('1'))
				{
					actor->EquipWeapon(0);
				}

				if (g_theInput->WasKeyJustPressed('2'))
				{
					actor->EquipWeapon(1);
				}

				if (g_theInput->WasKeyJustPressed('3'))
				{
					actor->EquipWeapon(2);
				}

				if (g_theInput->WasKeyJustPressed('4'))
				{
					actor->EquipWeapon(3);
				}

				if (g_theInput->WasKeyJustPressed('5'))
				{
					actor->EquipWeapon(4);
				}

				if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
				{
					actor->Attack();
				}
			}

			Vec3 actorVelocity;
			float moveSpeed = actor->m_actorDefinition.m_physicsWalkSpeed;
			if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
			{
				moveSpeed = actor->m_actorDefinition.m_physicsRunSpeed;
			}

			//w d
			if (g_theInput->IsKeyDown('W'))
			{
				actorVelocity += actor->GetOrientationOnlyYaw().GetIBasis();
			}
			//s a
			if (g_theInput->IsKeyDown('S'))
			{
				actorVelocity -= actor->GetOrientationOnlyYaw().GetIBasis();
			}
			//a s
			if (g_theInput->IsKeyDown('A'))
			{
				actorVelocity += actor->GetOrientationOnlyYaw().GetJBasis();
			}
			//d w
			if (g_theInput->IsKeyDown('D'))
			{
				actorVelocity -= actor->GetOrientationOnlyYaw().GetJBasis();
			}

			if (g_theInput->WasKeyJustPressed('R'))
			{
				actor->GetEquippedWeapon()->Reload();
			}

			actorVelocity = actorVelocity.GetNormalized();
			actor->MoveInDirection(actorVelocity, moveSpeed);

			float cursorDeltaX = (float)g_theInput->GetCursorClientDelta().x;
			float clientDimensionX = 0.5f * (float)g_theWindow->GetClientDimensions().x;
			float ratioX = cursorDeltaX / clientDimensionX;

			float cursorDeltaY = (float)g_theInput->GetCursorClientDelta().y;
			float clientDimensionY = 0.5f * (float)g_theWindow->GetClientDimensions().y;
			float ratioY = cursorDeltaY / clientDimensionY;

			// wrong matrix
// 			actor->m_orientation.m_yawDegrees += 75.0f * ratioX;
//   			actor->m_orientation.m_rollDegrees += 60.0f * ratioY;
//  			actor->m_orientation.m_rollDegrees = GetClamped(actor->m_orientation.m_rollDegrees, -85.0f, 85.0f);

			actor->m_orientation.m_yawDegrees -= 80.0f * ratioX;
			actor->m_orientation.m_pitchDegrees += 80.0f * ratioY;
			actor->m_orientation.m_pitchDegrees = GetClamped(actor->m_orientation.m_pitchDegrees, -85.0f, 85.0f);
		
			if (actorVelocity != Vec3(0.f, 0.f, 0.f))
			{
				if (actor->m_currentAnimName == "Idle")
				{
					actor->PlayAnimation("Walk");
				}
			}
			else
			{
				if (actor->m_currentAnimName == "Walk")
				{
					actor->PlayAnimation("Idle");
				}
			}
		}
		break;
	case CameraMode::CAMERAMODE_FREEFLY:
		m_velocity = Vec3(0.0f, 0.0f, 0.0f);
		m_speedFactor = 1.0f;
		if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
		{
			m_speedFactor *= 15.0f;
		}
		float cursorDeltaX = (float)g_theInput->GetCursorClientDelta().x;
		float clientDimensionX = 0.5f * (float)g_theWindow->GetClientDimensions().x;
		float ratioX = cursorDeltaX / clientDimensionX;
		m_orientation.m_yawDegrees -= 80.f * ratioX;

		float cursorDeltaY = (float)g_theInput->GetCursorClientDelta().y;
		float clientDimensionY = 0.5f * (float)g_theWindow->GetClientDimensions().y;
		float ratioY = cursorDeltaY / clientDimensionY;
		m_orientation.m_pitchDegrees += 80.f * ratioY;

		if (g_theInput->IsKeyDown(KEYCODE_UPARROW))
		{
			m_orientation.m_pitchDegrees -= 90.0f * (float)Clock::GetSystemClock().GetDeltaSeconds();
		}
		if (g_theInput->IsKeyDown(KEYCODE_DOWNARROW))
		{
			m_orientation.m_pitchDegrees += 90.0f * (float)Clock::GetSystemClock().GetDeltaSeconds();
		}
		if (g_theInput->IsKeyDown(KEYCODE_LEFTARROW))
		{
			m_orientation.m_yawDegrees += 90.0f * (float)Clock::GetSystemClock().GetDeltaSeconds();
		}
		if (g_theInput->IsKeyDown(KEYCODE_RIGHTARROW))
		{
			m_orientation.m_yawDegrees -= 90.0f * (float)Clock::GetSystemClock().GetDeltaSeconds();
		}

		if (g_theInput->IsKeyDown('Q'))
		{
			m_orientation.m_rollDegrees -= 90.0f * (float)Clock::GetSystemClock().GetDeltaSeconds();
		}
		if (g_theInput->IsKeyDown('E'))
		{
			m_orientation.m_rollDegrees += 90.0f * (float)Clock::GetSystemClock().GetDeltaSeconds();
		}

		if (g_theInput->IsKeyDown('W'))
		{
			m_velocity += m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D();
		}
		if (g_theInput->IsKeyDown('S'))
		{
			m_velocity -= m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D();
		}
		if (g_theInput->IsKeyDown('A'))
		{
			m_velocity += m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetJBasis3D();
		}
		if (g_theInput->IsKeyDown('D'))
		{
			m_velocity -= m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetJBasis3D();
		}
		if (g_theInput->IsKeyDown('Z'))
		{
			m_velocity += m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetKBasis3D();
		}
		if (g_theInput->IsKeyDown('C'))
		{
			m_velocity -= m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetKBasis3D();
		}

		m_position += m_velocity * m_speedFactor * (float)Clock::GetSystemClock().GetDeltaSeconds();

		m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.0f, 85.0f);
		m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.0f, 45.0f);

		break;
	}
}

void Player::UpdateController()
{
	if (m_controllerIndex != 0)
	{
		return;
	}

	XboxController controller = g_theInput->GetController(m_controllerIndex);
	if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_DPAD_RIGHT) && !controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_DPAD_RIGHT))
	{
		m_game->m_map->LoadNextUIText();
	}
	switch (m_cameraMode)
	{
	Actor* actor;
	case CameraMode::CAMERAMODE_ACTOR:
		actor = GetActor();
		if (actor != nullptr)
		{
			if (actor->m_isDead)
			{
				return;
			}

			if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_DPAD_UP) && !controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_DPAD_UP))
			{
				actor->EquipPreviousWeapon();
			}

			if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_DPAD_DOWN) && !controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_DPAD_DOWN))
			{
				actor->EquipNextWeapon();
			}

// 			if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_X) && !controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_X))
// 			{
// 				actor->EquipWeapon(0);
// 			}
// 
// 			if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_Y) && !controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_Y))
// 			{
// 				actor->EquipWeapon(1);
// 			}

			if (controller.GetRightTrigger() > 0.0f)
			{
				actor->Attack();
			}

			Vec3 localVelocity;
			float moveSpeed = actor->m_actorDefinition.m_physicsWalkSpeed;
			if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_A))
			{
				moveSpeed = actor->m_actorDefinition.m_physicsRunSpeed;
			}
			localVelocity += actor->GetOrientationOnlyYaw().GetIBasis() * controller.GetLeftStick().GetPosition().y;
			localVelocity -= actor->GetOrientationOnlyYaw().GetJBasis() * controller.GetLeftStick().GetPosition().x;
			localVelocity = localVelocity.GetNormalized();

			actor->MoveInDirection(localVelocity, moveSpeed);

			float ratioX = (float)controller.GetRightStick().GetPosition().x;
			float ratioY = (float)controller.GetRightStick().GetPosition().y;

			actor->m_orientation.m_yawDegrees -= 1.5f * ratioX;
			actor->m_orientation.m_pitchDegrees -= 1.5f * ratioY;
			actor->m_orientation.m_pitchDegrees = GetClamped(actor->m_orientation.m_pitchDegrees, -85.0f, 85.0f);

			if (localVelocity != Vec3(0.f, 0.f, 0.f))
			{
				if (actor->m_currentAnimName != "Walk")
				{
					actor->PlayAnimation("Walk");
				}
			}
			else
			{
				if (actor->m_currentAnimName == "Walk")
				{
					actor->PlayAnimation("Idle");
				}
			}

		}
		break;
	case CameraMode::CAMERAMODE_FREEFLY:

		if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_A))
		{
			m_speedFactor = 10.f;
		}

		float ratioX = (float)controller.GetRightStick().GetPosition().x;
		float ratioY = (float)controller.GetRightStick().GetPosition().y;
		m_orientation.m_yawDegrees -= ratioX;
		m_orientation.m_pitchDegrees -= ratioY;
		// matrix wrong
		//m_orientation.m_rollDegrees = 90.f;

		m_velocity += m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D() * m_speedFactor * controller.GetLeftStick().GetPosition().y;
		m_velocity -= m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetJBasis3D() * m_speedFactor * controller.GetLeftStick().GetPosition().x;

		if (controller.GetLeftTrigger())
		{
			m_orientation.m_rollDegrees -= 90.0f * (float)Clock::GetSystemClock().GetDeltaSeconds();
		}
		if (controller.GetRightTrigger())
		{
			m_orientation.m_rollDegrees += 90.0f * (float)Clock::GetSystemClock().GetDeltaSeconds();
		}

		if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_LB))
		{
			m_velocity += m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetKBasis3D() * 2.0f;
		}
		if (controller.IsButtonDown(XboxButtonID::XBOX_BUTTON_RB))
		{
			m_velocity -= m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetKBasis3D() * 2.0f;
		}
		break;
	}
}

void Player::UpdateCamera()
{
	switch (m_cameraMode)
	{
	case CameraMode::CAMERAMODE_ACTOR:
		m_position.x = GetActor()->m_position.x;
		m_position.y = GetActor()->m_position.y;
		m_position.z = GetActor()->m_actorDefinition.m_cameraEyeHeight;
		m_orientation = GetActor()->m_orientation;
		m_worldCamera->SetPositionAndOrientation(m_position, m_orientation);
		break;
	case CameraMode::CAMERAMODE_FREEFLY:
		m_position += m_velocity * m_speedFactor * static_cast<float>(Clock::GetSystemClock().GetDeltaSeconds());
		m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.0f, 85.0f);
		m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.0f, 45.0f);
		m_worldCamera->SetPositionAndOrientation(m_position, m_orientation);
		break;
	}
}

void Player::UpdateAudio()
{
	g_theAudio->UpdateListener(m_playerIndex, m_position, m_orientation.GetIBasis(), m_orientation.GetKBasis());
}

void Player::Render()
{
	if (GetActor() != nullptr && GetActor()->m_playerPossessed && GetActor()->m_actorDefinition.m_actorFaction != Faction::FACTION_DEMON)
	{
		if (!GetActor()->m_isDead)
		{
			if (m_isSinglePlayerMode)
			{
				GetActor()->GetEquippedWeapon()->Render(1);
			}
			else
			{
				GetActor()->GetEquippedWeapon()->Render(2);
			}
			GetActor()->RenderOnUI();
		}
		else
		{
			std::vector<Vertex_PCU> deadVerts;
			AddVertsForAABB2D(deadVerts, AABB2(WORLD_CAMERA_BOTTOMLEFT, WORLD_CAMERA_TOPRIGHT), Rgba8(192, 192, 192, 50));
			g_theRenderer->BindShader(nullptr);
			g_theRenderer->BindTexture(nullptr);
			g_theRenderer->SetModelConstants();
			g_theRenderer->SetLightConstants();
			g_theRenderer->DrawVertexArray((int)deadVerts.size(), deadVerts.data());
		}
	}
}

void Player::SetCameraBaseOnCameraMode()
{
	if (m_cameraMode == CameraMode::CAMERAMODE_ACTOR)
	{
		if (m_isSinglePlayerMode)
		{
			m_worldCamera->m_cameraBox = AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f));
			m_playerCamera->m_cameraBox = AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f));
			m_playerCamera->SetOrthographicView(WORLD_CAMERA_BOTTOMLEFT, WORLD_CAMERA_TOPRIGHT);
			m_worldCamera->SetPerspectiveView(g_theWindow->GetAspect(), GetActor()->m_actorDefinition.m_cameraFOV, 0.1f, 100.0f);
		}
		else
		{
			if (m_playerIndex == 0)
			{
				m_worldCamera->m_cameraBox = AABB2(Vec2(0.0f, 0.5f), Vec2(1.0f, 1.0f));
				m_playerCamera->m_cameraBox = AABB2(Vec2(0.0f, 0.5f), Vec2(1.0f, 1.0f));
				m_playerCamera->SetOrthographicView(WORLD_CAMERA_BOTTOMLEFT, WORLD_CAMERA_TOPRIGHT);
				m_worldCamera->SetPerspectiveView(g_theWindow->GetAspect() * 2.0f, GetActor()->m_actorDefinition.m_cameraFOV, 0.1f, 100.0f);
			}
			else if (m_playerIndex == 1)
			{
				m_worldCamera->m_cameraBox = AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f, 0.5f));
				m_playerCamera->m_cameraBox = AABB2(Vec2(0.0f, 0.0f), Vec2(1.0f, 0.5f));
				m_playerCamera->SetOrthographicView(WORLD_CAMERA_BOTTOMLEFT, WORLD_CAMERA_TOPRIGHT);
				m_worldCamera->SetPerspectiveView(g_theWindow->GetAspect() * 2.0f, GetActor()->m_actorDefinition.m_cameraFOV, 0.1f, 100.0f);
			}
		}
	}
	else if (m_cameraMode == CameraMode::CAMERAMODE_FREEFLY)
	{
		m_worldCamera->SetPerspectiveView(g_theWindow->GetAspect(), 60.0f, 0.1f, 100.0f);
	}
}

void Player::SetPlayerIndex(int index)
{
	m_playerIndex = index;
}

int Player::GetPlayerIndex() const
{
	return m_playerIndex;
}

