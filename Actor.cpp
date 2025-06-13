#include "Game/Actor.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/Tile.hpp"
#include "Game/ActorDefinitions.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/VertexUtils.hpp"

#include <vector>
#include <Engine/Renderer/DebugRenderSystem.hpp>


Actor::Actor()
{
}

// Actor::Actor(Map* owner, Vec3 const& startPosition, EulerAngles const& startOrientation, Rgba8 startColor, float startPhysicalHeight, float startPhysicalRadius, bool isStatic)
// 	: m_map(owner)
// 	, m_position(startPosition)
// 	, m_orientation(startOrientation)
// 	, m_color(startColor)
// 	, m_physicalHeight(startPhysicalHeight)
// 	, m_physicalRadius(startPhysicalRadius)
// 	, m_isStatic(isStatic)
// {
// }

Actor::Actor(Map* owner, ActorDefinition actorDefinition, ActorHandle const& actorHandle)
	: m_map(owner)
	, m_actorDefinition(actorDefinition)
	, m_actorHandle(actorHandle)
{
}

Actor::~Actor()
{
}


void Actor::StartUp()
{
	if (m_actorDefinition.m_canBePossessed)
	{
		m_AIController = new AI();
		m_AIController->m_map = m_map;
		m_AIController->m_actorHandle = m_actorHandle;

		if (m_actorDefinition.m_actorFaction == Faction::FACTION_MARINE)
		{
			m_AIController->m_isPlayerAI = true;
			m_playerIconSpriteSheet = new SpriteSheet(*g_theRenderer->CreateOrGetTextureFromFile("Data/Images/FaceHud.png"), IntVec2(3, 4));
		}
		else if (m_actorDefinition.m_actorFaction == Faction::FACTION_DEMON)
		{
			m_AIController->m_isPlayerAI = false;
			m_currentController = m_AIController;
		}

		m_hurtSound = new SoundInfo();
		m_hurtSound->m_soundName = "Hurt";
		m_hurtSound->m_soundID = g_theAudio->CreateOrGetSound(m_actorDefinition.m_soundsMap["Hurt"], false);
	}

	m_isStatic = m_actorDefinition.m_staticObject;
	m_health = m_actorDefinition.m_actorHealth;
	m_lifetimeTimer = new Timer(m_actorDefinition.m_corpseLifetime, GetGameClock());
	m_animationTimer = new Timer(0.0f, GetGameClock());

	for (int i = 0; i < (int)m_actorDefinition.m_inventoryWeaponNames.size(); i++)
	{
		WeaponDefinitions weaponDef = WeaponDefinitions::s_weaponDefinitions[m_actorDefinition.m_inventoryWeaponNames[i]];
		Weapon* weapon = new Weapon(weaponDef, m_actorHandle, m_map);
		m_weapons.push_back(weapon);
	}

	if (!m_weapons.empty())
	{
		m_currentWeapon = m_weapons[0];
	}

	if (m_actorDefinition.m_dieOnSpawn)
	{
		Dead(nullptr);
	}
}

void Actor::Update()
{
	if (m_health < m_actorDefinition.m_actorHealth * 0.2f && !m_isDead)
	{
		if (m_hurtImpulseTimer > 1.0f)
		{
			m_hurtImpulseTimer = 1.0f;
			m_isIncreasing = false;
		}
		if (m_hurtImpulseTimer < 0.0f)
		{
			m_hurtImpulseTimer = 0.0f;
			m_isIncreasing = true;
		}

		if (m_isIncreasing)
		{
			m_hurtImpulseTimer += (float)GetGameClock()->GetDeltaSeconds() * 3.0f;
		}
		else
		{
			m_hurtImpulseTimer -= (float)GetGameClock()->GetDeltaSeconds() * 3.0f;
		}
		m_color = Rgba8::Interpolate(Rgba8::WHITE, Rgba8(255, 68, 68), m_hurtImpulseTimer);
	}
	else
	{
		m_hurtImpulseTimer = 0.0f;
		m_isIncreasing = true;
	}
	UpdateAIController();
	UpdatePhysics();
	UpdateDestroyedStatus();
	UpdateAnimation();
	UpdateAudio();
}

void Actor::UpdateAIController()
{
	if (m_AIController == nullptr)
	{
		return;
	}
	if (m_currentController == m_AIController)
	{
		m_AIController->Update();
	}
}

void Actor::UpdatePhysics()
{
	if (m_isDead)
	{
		if (m_playerPossessed)
		{
			m_actorDefinition.m_cameraEyeHeight -= 1.5f * (float)GetGameClock()->GetDeltaSeconds();
		}
		return;
	}
	if (m_actorDefinition.m_physicsSimulated)
	{
		AddForce(-m_actorDefinition.m_physicsDrag * m_velocity);
		if (m_actorDefinition.m_gravitySimulated)
		{
			AddForce(-19.8f * GetOrientationOnlyYaw().GetKBasis());
		}
		m_velocity += m_acceleration * (float)m_map->GetGameInstance()->GetGameClock()->GetDeltaSeconds();
		m_position += m_velocity * (float)m_map->GetGameInstance()->GetGameClock()->GetDeltaSeconds();
		m_acceleration = Vec3(0.f, 0.f, 0.f);
	}
}

void Actor::UpdateDestroyedStatus()
{
	if (m_lifetimeTimer->HasPeriodElapsed())
	{
		m_isDestroyed = true;
	}
}

void Actor::UpdateAnimation()
{
	if (m_currentAnimName == "Hurt")
	{
		if (m_animationTimer->GetElapsedTime() > m_animationTimer->m_period)
		{
			PlayAnimation("Idle");
		}
	}
	if (m_currentAnimName == "Attack")
	{
		if (m_animationTimer->GetElapsedTime() > m_animationTimer->m_period)
		{
			PlayAnimation("Idle");
		}
	}
}

void Actor::UpdateAudio()
{
	if (m_currentWeapon != nullptr)
	{
		m_currentWeapon->Update();
	}
}

void Actor::Render(Camera const& camera, int playerIndex)
{
	if (m_playerPossessed)
	{
		if (((Player*)m_currentController)->GetPlayerIndex() == playerIndex)
		{
			return;
		}
	}

// 	m_vertex.clear();
// 	Vec3 cylinderStartPosition = m_position;
// 	Vec3 cylinderEndPosition = m_position + GetOrientationOnlyYaw().GetKBasis() * m_actorDefinition.m_collisionHeight;
// 	AddVertsForCylinder3D(m_vertex, cylinderStartPosition, cylinderEndPosition, m_actorDefinition.m_collisionRadius, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16);

// 	m_vertexes.clear();
// 	m_indexes.clear();
	Rgba8 actorColor = Rgba8::WHITE;
	float currentLargestDotProduct = -999.0f;
	int	currentLargestDotProductIndex = -999;

	Vec2 billboardSize = m_actorDefinition.m_visualSize;
	Vec2 alignment = m_actorDefinition.m_visualPivot;
	Vec3 leftBottomPoint = -alignment.x * Vec3(0.0f, billboardSize.x, 0.0f) - alignment.y * Vec3(0.0f, 0.0f, billboardSize.y);
	Vec3 rightBottomPoint = leftBottomPoint + Vec3(0.0f, billboardSize.x, 0.0f);
	Vec3 rightTopPoint = rightBottomPoint + Vec3(0.0f, 0.0f, billboardSize.y);
	Vec3 leftTopPoint = leftBottomPoint + Vec3(0.0f, 0.0f, billboardSize.y);

	Mat44 worldToLocalMatrix = GetModelMatrix().GetOrthonormalInverse();
	Vec3 cameraToActor = m_position - camera.GetModelToWorldTransform().GetTranslation3D();
	cameraToActor = worldToLocalMatrix.TransformVectorQuantity3D(cameraToActor);
	cameraToActor.z = 0.0f;
	cameraToActor = cameraToActor.GetNormalized();

	if (m_actorDefinition.m_visualSpriteSheet == nullptr)
	{
		m_color = Rgba8(255, 65, 65);
		std::vector<Vertex_PCU> verts;
		g_theFont->AddVertsForText3DAtOriginXForward(verts, 0.08f, Stringf("%.f", m_damageValue));
		Mat44 modelMatrix;
		modelMatrix.AppendTranslation3D(m_position);
		modelMatrix.Append(GetBillboardTransform(m_actorDefinition.m_visualBillboardType, camera.GetModelToWorldTransform(), m_position, Vec2(1.f, 1.f)));
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(&g_theFont->GetTexture());
		g_theRenderer->SetLightConstants();
		g_theRenderer->SetModelConstants(modelMatrix, m_color);
		g_theRenderer->SetDepthMode(DepthMode::DISABLED);
		g_theRenderer->DrawVertexArray((int)verts.size(), verts.data());
		g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);	
	}
	else
	{
		m_vertexes.clear();
		m_indexes.clear();

		ActorAnimation actorAnimation = m_actorDefinition.m_actorAnimations[0];
		for (int i = 0; i < (int)m_actorDefinition.m_actorAnimations.size(); i++)
		{
			if (m_actorDefinition.m_actorAnimations[i].m_animationName == m_currentAnimName)
			{
				actorAnimation = m_actorDefinition.m_actorAnimations[i];
			}
		}


		for (int i = 0; i < (int)actorAnimation.m_animationDirectionals.size(); i++)
		{
			Vec3 localDirectional = actorAnimation.m_animationDirectionals[i].m_lookDirection.GetNormalized();
			float dotValue = DotProduct3D(cameraToActor, localDirectional);

			if (dotValue > currentLargestDotProduct)
			{
				currentLargestDotProduct = dotValue;
				currentLargestDotProductIndex = i;
			}
		}

		SpriteAnimDefinition* spriteAnim = actorAnimation.m_animationDirectionals[currentLargestDotProductIndex].m_spriteAnimation;

		float speedRatio = 1.0f;
		if (actorAnimation.m_animationScaledBySpeed)
		{
			speedRatio = m_velocity.GetLength() / m_actorDefinition.m_physicsRunSpeed;
		}
		AABB2 uvBox = spriteAnim->GetSpriteDefAtTime((float)m_animationTimer->GetElapsedTime() * speedRatio).GetUVs();
		AddVertsForQuad3D(m_vertexes, m_indexes, leftBottomPoint, rightBottomPoint, rightTopPoint, leftTopPoint, Rgba8::WHITE, uvBox);

		Shader* shader = g_theRenderer->CreateShader(m_actorDefinition.m_visualShaderPath.c_str(), VertexType::Vertex_PCUTBN);
		Mat44 modelMatrix;
		modelMatrix.AppendTranslation3D(m_position);
		modelMatrix.Append(GetBillboardTransform(m_actorDefinition.m_visualBillboardType, camera.GetModelToWorldTransform(), m_position, Vec2(1.f, 1.f)));
		g_theRenderer->BindShader(shader);
		g_theRenderer->BindTexture(&m_actorDefinition.m_visualSpriteSheet->GetTexture());
		g_theRenderer->SetLightConstants();
		g_theRenderer->SetModelConstants(modelMatrix, m_color);
		g_theRenderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data(), (int)m_indexes.size(), m_indexes.data());
	}


// 
// 	Mat44 worldToLocalMatrix = GetModelMatrix().GetOrthonormalInverse();
// 	Vec3 cameraToActor = m_position - camera.GetModelToWorldTransform().GetTranslation3D();
// 	cameraToActor = worldToLocalMatrix.TransformVectorQuantity3D(cameraToActor);
// 	cameraToActor.z = 0.0f;
// 	cameraToActor = cameraToActor.GetNormalized();
// 
// 	float currentLargestDotProduct = -999.0f;
// 	int	currentLargestDotProductIndex = -999;
// 	ActorAnimation actorAnimation = m_actorDefinition.m_actorAnimations[0];
// 	for (int i = 0; i < (int)m_actorDefinition.m_actorAnimations.size(); i++)
// 	{
// 		if (m_actorDefinition.m_actorAnimations[i].m_animationName == m_currentAnimName)
// 		{
// 			actorAnimation = m_actorDefinition.m_actorAnimations[i];
// 		}
// 	}
// 
// 	for (int i = 0; i < (int)actorAnimation.m_animationDirectionals.size(); i++)
// 	{
// 		Vec3 localDirectional = actorAnimation.m_animationDirectionals[i].m_lookDirection.GetNormalized();
// 		float dotValue = DotProduct3D(cameraToActor, localDirectional);
// 
// 		if (dotValue > currentLargestDotProduct)
// 		{
// 			currentLargestDotProduct = dotValue;
// 			currentLargestDotProductIndex = i;
// 		}
// 	}
// 
// 	SpriteAnimDefinition* spriteAnim = actorAnimation.m_animationDirectionals[currentLargestDotProductIndex].m_spriteAnimation;
// 
// 	float speedRatio = 1.0f;
// 	if (actorAnimation.m_animationScaledBySpeed)
// 	{
// 		speedRatio = m_velocity.GetLength() / m_actorDefinition.m_physicsRunSpeed;
// 	}
// 	AABB2 uvBox = spriteAnim->GetSpriteDefAtTime((float)m_animationTimer->GetElapsedTime() * speedRatio).GetUVs();
// 	AddVertsForQuad3D(m_vertexes, m_indexes, leftBottomPoint, rightBottomPoint, rightTopPoint, leftTopPoint, Rgba8::WHITE, uvBox);
// 
// 	Shader* shader = g_theRenderer->CreateShader(m_actorDefinition.m_visualShaderPath.c_str(), VertexType::Vertex_PCUTBN);
// 	Mat44 modelMatrix;
// 	modelMatrix.AppendTranslation3D(m_position);
// 	modelMatrix.Append(GetBillboardTransform(m_actorDefinition.m_visualBillboardType, camera.GetModelToWorldTransform(), m_position, Vec2(1.0f,1.0f)));
// 	g_theRenderer->BindShader(shader);
// 	g_theRenderer->BindTexture(&m_actorDefinition.m_visualSpriteSheet->GetTexture());
// 	g_theRenderer->SetLightConstants();
// 	g_theRenderer->SetModelConstants(modelMatrix, actorColor);
// 	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
// 	g_theRenderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data(), (int)m_indexes.size(), m_indexes.data());


// 	g_theRenderer->BindShader(nullptr);
// 	g_theRenderer->BindTexture(nullptr);
// 	g_theRenderer->SetModelConstants(Mat44(), actorColor);
// 	g_theRenderer->SetLightConstants();
// 	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
// 	g_theRenderer->DrawVertexArray((int)m_vertex.size(), m_vertex.data());
// 	g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_NONE);
// 	g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
// 	g_theRenderer->DrawVertexArray((int)m_vertex.size(), m_vertex.data());
// 
// 	if (m_actorDefinition.m_actorFaction != Faction::FACTION_MARINEPROJECTILE)
// 	{
// 		m_vertex.clear();
// 		Vec3 coneStartPosition = m_position + GetOrientationOnlyYaw().GetKBasis() * m_actorDefinition.m_cameraEyeHeight * 0.9f + GetOrientationOnlyYaw().GetIBasis() * m_actorDefinition.m_collisionRadius;
// 		Vec3 coneEndPosition = coneStartPosition + GetOrientationOnlyYaw().GetIBasis() * 0.1f;
// 		AddVertsForCone3D(m_vertex, coneStartPosition, coneEndPosition, 0.15f * m_actorDefinition.m_collisionHeight, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 16);
// 
// 		g_theRenderer->BindShader(nullptr);
// 		g_theRenderer->BindTexture(nullptr);
// 		g_theRenderer->SetLightConstants();
// 		g_theRenderer->SetModelConstants(Mat44(), actorColor);
// 		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
// 		g_theRenderer->DrawVertexArray((int)m_vertex.size(), m_vertex.data());
// 
// 		g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_NONE);
// 		g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
// 		g_theRenderer->DrawVertexArray((int)m_vertex.size(), m_vertex.data());
// 	}

}

void Actor::RenderOnUI() const
{
	std::vector<Vertex_PCU> playerUIVerts;
	playerUIVerts.reserve(60);

	AABB2 healthTextBox = AABB2(Vec2(380.0f, 30.0f), Vec2(600, 100.0f));
	g_theFont->AddVertsForTextInBox2D(playerUIVerts, Stringf("%.f", m_health), healthTextBox, 50.0f, Rgba8::WHITE, 1.0f);

	AABB2 killEnemiesTextBox = AABB2(Vec2(0.0f, 30.0f), Vec2(200, 100.0f));
	g_theFont->AddVertsForTextInBox2D(playerUIVerts, Stringf("%i", m_currentController->m_killedEnemiesNum), killEnemiesTextBox, 50.0f, Rgba8::WHITE, 1.0f);

	AABB2 deathTextBox = AABB2(Vec2(1400.0f, 30.0f), Vec2(1600, 100.0f));
	g_theFont->AddVertsForTextInBox2D(playerUIVerts, Stringf("%i", m_currentController->m_deathTimes), deathTextBox, 50.0f, Rgba8::WHITE, 1.0f);

	AABB2 ammoTextBox = AABB2(Vec2(200.0f, 30.0f), Vec2(400.f, 100.0f));
	g_theFont->AddVertsForTextInBox2D(playerUIVerts, Stringf("%i", m_currentWeapon->m_currentAmmo), ammoTextBox, 50.0f, Rgba8::WHITE, 1.0f);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&g_theFont->GetTexture());
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetLightConstants();
	g_theRenderer->DrawVertexArray((int)playerUIVerts.size(), playerUIVerts.data());

	playerUIVerts.clear();
	AABB2 playerIconBox = AABB2(Vec2(738.0f, 0.0f), Vec2(865.0f, 110.0f));
	int currentHealthIcon = 4 - static_cast<int>(m_health / 25.0f);
	currentHealthIcon >= 4 ? currentHealthIcon = 3 : currentHealthIcon;
	AddVertsForAABB2D(playerUIVerts, playerIconBox, Rgba8::WHITE, m_playerIconSpriteSheet->GetSpriteUVsByIntPos(IntVec2(2, currentHealthIcon)).m_mins, m_playerIconSpriteSheet->GetSpriteUVsByIntPos(IntVec2(2, currentHealthIcon)).m_maxs);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&m_playerIconSpriteSheet->GetTexture());
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetLightConstants();
	g_theRenderer->DrawVertexArray((int)playerUIVerts.size(), playerUIVerts.data());

}


void Actor::Damage(float damage, Actor* source)
{
	m_health -= damage;
	if (m_AIController != nullptr)
	{
		if (m_currentController == m_AIController)
		{
			m_AIController->DamagedBy(source->GetActorHandle());
		}
	}
	if (m_health <= 0.0f)
	{
		m_health = 0.0f;
		Dead(source);
	}
	else
	{
		PlayAnimation("Hurt");
		g_theAudio->StartSoundAt(m_hurtSound->m_soundID, m_position, false, 3.0f);
	}
}

void Actor::Dead(Actor* source)
{
	if (source)
	{
		if (source->m_currentController != nullptr)
		{
			source->m_currentController->m_killedEnemiesNum += 1;
		}
	}
	if (!m_isDead)
	{
// 		if (m_currentController != nullptr)
// 		{
// 			m_currentController->m_deathTimes += 1;
// 		}
// 		m_isDead = true;
// 		m_lifetimeTimer->Start();
// 		PlayAnimation("Death");
		if (m_actorDefinition.m_actorName == "Demon2")
		{
			int scorNum = m_map->GetGameInstance()->m_rng->RollRandomIntInRange(3, 6);

			for (int i = 0; i < scorNum; i++)
			{
				SpawnInfo info;
				float xDiretion = m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(0.0f, 1.0f);
				float yDiretion = m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(0.0f, 1.0f);
				float radius = m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(1.0f, 3.0f);
				Vec3 spawnDirection = Vec3(xDiretion, yDiretion, m_position.z + 0.1f).GetNormalized();

				info.m_actorName = "Scorpion";
				info.m_faction = Faction::FACTION_DEMON;
				info.m_position = m_position + radius * spawnDirection;
				info.m_orientation = EulerAngles(m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(0.0f, 360.0f), 0.0f, 0.0f);

				Actor* scorActor = m_map->SpawnActor(info);
				scorActor->m_AIController->m_targetActorHandle = source->GetActorHandle();
			}
		}
		if (m_currentController != nullptr)
		{
			m_currentController->m_deathTimes += 1;
		}
		m_isDead = true;
		m_lifetimeTimer->Start();
		PlayAnimation("Death");
	}
}

void Actor::AddForce(const Vec3& force)
{
	m_acceleration += force;
}

void Actor::AddImpulse(const Vec3& impulse)
{
	m_velocity += impulse;
}

void Actor::OnCollide(Actor* other)
{
	UNUSED(other);
}

void Actor::OnPossessed(Controller* controller)
{
	m_playerPossessed = true;
	m_currentController = controller;
}

void Actor::OnUnpossessed(Controller* controller)
{
	UNUSED(controller);
	m_playerPossessed = false;
	m_currentController = m_AIController;
}

void Actor::MoveInDirection(Vec3 direction, float speed)
{
	speed *= m_currentWeapon->m_weaponDefinition.m_weaponMoveSpeed;
	AddForce(direction * speed * m_actorDefinition.m_physicsDrag);
}

void Actor::TurnInDirection(float degrees, float maxAngle)
{
	m_orientation.m_yawDegrees = GetTurnedTowardDegrees(m_orientation.m_yawDegrees, degrees, maxAngle);
}

void Actor::EquipWeapon(int weaponIndex)
{
	if (weaponIndex >= 0 && weaponIndex < (int)m_weapons.size())
	{
		if (m_weapons[weaponIndex] != nullptr)
		{
			m_currentWeapon = m_weapons[weaponIndex];
		}
	}
}

void Actor::EquipNextWeapon()
{
	for (int i = 0; i < (int)m_weapons.size(); i++)
	{
		if (m_currentWeapon == m_weapons[i])
		{
			if (i == (int)m_weapons.size() - 1)
			{
				m_currentWeapon = m_weapons[0];
			}
			else
			{
				m_currentWeapon = m_weapons[i + 1];
			}
			return;
		}
	}
}

void Actor::EquipPreviousWeapon()
{
	for (int i = 0; i < (int)m_weapons.size(); i++)
	{
		if (m_currentWeapon == m_weapons[i])
		{
			if (i == 0)
			{
				m_currentWeapon = m_weapons[(int)m_weapons.size() - 1];
			}
			else
			{
				m_currentWeapon = m_weapons[i - 1];
			}
			return;
		}
	}
}

void Actor::Attack()
{
	m_currentWeapon->Fire();
}

void Actor::Attack(Actor* targetActor)
{
	m_currentWeapon->Fire(targetActor);
}

void Actor::PlayAnimation(std::string animationName)
{
	if (animationName == "Idle")
	{
		m_currentAnimName = animationName;
		m_animationTimer->Stop();
		return;
	}

	for (int i = 0; i < (int)m_actorDefinition.m_actorAnimations.size(); i++)
	{
		if (m_actorDefinition.m_actorAnimations[i].m_animationName == animationName)
		{
			m_currentAnimName = animationName;
			ActorAnimation const& actorAnimation = m_actorDefinition.m_actorAnimations[i];
			float totalSeconds = actorAnimation.m_animationDirectionals[0].m_spriteAnimation->GetOneClipTotalLength();
			m_animationTimer->m_period = totalSeconds;
			m_animationTimer->Start();
		}
	}
}

Vec2 Actor::GetActorPositionXY() const
{
	return Vec2(m_position.x, m_position.y);
}

FloatRange Actor::GetActorZRange() const
{
	return FloatRange(m_position.z, m_position.z + m_actorDefinition.m_collisionHeight);
}

ActorHandle Actor::GetActorHandle() const
{
	return m_actorHandle;
}

Controller* Actor::GetCurrentController()
{
	return m_currentController;
}

Weapon* Actor::GetEquippedWeapon()
{
	return m_currentWeapon;
}

Clock* Actor::GetGameClock() const
{
	return m_map->GetGameInstance()->GetGameClock();
}

EulerAngles Actor::GetOrientation() const
{
	return m_orientation;
}

EulerAngles Actor::GetOrientationOnlyYaw() const
{
	return EulerAngles(m_orientation.m_yawDegrees, 0.f, 0.f);
}

Mat44 Actor::GetModelMatrix() const
{
	Mat44 matrix;

	matrix.SetIJKT3D(m_orientation.GetIBasis(), m_orientation.GetJBasis(), m_orientation.GetKBasis(), m_position);

	return matrix;
}

Vec3 Actor::GetActorWeaponStartPosition() const
{
	return  m_position + GetOrientationOnlyYaw().GetKBasis() * m_actorDefinition.m_cameraEyeHeight + m_orientation.GetIBasis() * m_actorDefinition.m_collisionRadius;
}

float Actor::GetRandomDamageValue() const
{
	return m_map->GetGameInstance()->m_rng->RollRandomFloatInRange(m_actorDefinition.m_damageOnCollide.m_min, m_actorDefinition.m_damageOnCollide.m_max);
}



