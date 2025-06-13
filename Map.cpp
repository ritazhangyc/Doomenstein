#include "Game/Tile.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorHandle.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Controller.hpp"
#include "Game/AI.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"


Map::Map(MapDefinition const& mapdefition, Game* game)
	:m_mapDefinition(mapdefition)
	, m_game(game)
{
}

Map::~Map()
{

}


void Map::StartUp()
{
	m_dimensions = m_mapDefinition.m_image->GetDimensions();
	m_shader = m_mapDefinition.m_shader;
	m_spriteSheet = new SpriteSheet(*m_mapDefinition.m_spriteSheetTexture, m_mapDefinition.m_spriteSheetCellCount);
	m_heatMap = new TileHeatMap(m_dimensions);
	m_mapDialogTimer = new Timer(1.5f, m_game->GetGameClock());
	m_enemySpawnTimer = new Timer(1.0f, m_game->GetGameClock());

	SetSkyBox();
	StartUpTiles();
// 	InitializeActor();
// 	SpawnActorBesidesPlayer();
	StartUpDialogTexts();
}

void Map::SetSkyBox()
{
	if (m_mapDefinition.m_skyBoxTexturePath == DEFAULT_NAME)
	{
		return;
	}

	AABB3 bounds = AABB3();
	bounds.m_mins = Vec3(-0.0f, -0.0f, -1.0f);
	bounds.m_maxs = bounds.m_mins + Vec3((float)m_dimensions.x, (float)m_dimensions.y, 10.0f);

	Vec3 min = bounds.m_mins;
	Vec3 max = bounds.m_maxs;

	AABB2 uvForFrontFace = AABB2(Vec2(0.25f, 1.f / 3.f), Vec2(0.5f, 2.f / 3.f));
	AABB2 uvForBackFace = AABB2(Vec2(0.75f, 1.f / 3.f), Vec2(1.0f, 2.f / 3.f));
	AABB2 uvForLeftFace = AABB2(Vec2(0.0f, 1.f / 3.f), Vec2(0.25f, 2.f / 3.f));
	AABB2 uvForRightFace = AABB2(Vec2(0.5f, 1.f / 3.f), Vec2(0.75f, 2.f / 3.f));
	AABB2 uvForTopFace = AABB2(Vec2(0.25f, 2.f / 3.f), Vec2(0.5f, 1.0f));
	AABB2 uvForBottomFace = AABB2(Vec2(0.25f, 0.0f), Vec2(0.5f, 1.f / 3.f));

	AddVertsForQuad3D(m_skyboxVertes, m_skyBoxIndexes, Vec3(max.x, min.y, min.z), min, Vec3(min.x, min.y, max.z), Vec3(max.x, min.y, max.z), Rgba8::WHITE, uvForFrontFace);
	AddVertsForQuad3D(m_skyboxVertes, m_skyBoxIndexes, Vec3(max.x, max.y, min.z), Vec3(max.x, min.y, min.z), Vec3(max.x, min.y, max.z), max, Rgba8::WHITE, uvForLeftFace);
	AddVertsForQuad3D(m_skyboxVertes, m_skyBoxIndexes, Vec3(min.x, max.y, min.z), Vec3(max.x, max.y, min.z), max, Vec3(min.x, max.y, max.z), Rgba8::WHITE, uvForBackFace);
	AddVertsForQuad3D(m_skyboxVertes, m_skyBoxIndexes, min, Vec3(min.x, max.y, min.z), Vec3(min.x, max.y, max.z), Vec3(min.x, min.y, max.z), Rgba8::WHITE, uvForRightFace);
	AddVertsForQuad3D(m_skyboxVertes, m_skyBoxIndexes, Vec3(min.x, min.y, max.z), Vec3(min.x, max.y, max.z), max, Vec3(max.x, min.y, max.z), Rgba8::WHITE, uvForTopFace);

}

void Map::StartUpTiles()
{
	m_tiles.reserve(1024);
	for (int i = 0; i < m_dimensions.x; i++)
	{
		for (int j = 0; j < m_dimensions.y; j++)
		{
			IntVec2 spawnPosition = IntVec2(i, j);

			if (m_mapDefinition.m_image != nullptr)
			{
				Rgba8 texelColor = m_mapDefinition.m_image->GetTexelColor(spawnPosition);

				for (auto it = TileDefinition::s_definitions.begin(); it != TileDefinition::s_definitions.end(); ++it)
				{
					if (texelColor == it->second.m_mapImagePixelColor)
					{
						Tile* tile = new Tile(spawnPosition, it->second);
						tile->AddVertsForTiles(m_vertexes, m_indexes, m_spriteSheet);
						m_tiles.push_back(tile);
						break;
					}
				}
			}
		}
	}
}

void Map::InitializeActor()
{
	int spawnPointIndex = m_game->m_rng->RollRandomIntLessThan((int)m_mapDefinition.m_allSpawnPoints.size());
	SpawnInfo playerInfo = m_mapDefinition.m_allSpawnPoints[spawnPointIndex];
	playerInfo.m_actorName = ActorDefinition::s_actorDefinitions["Marine"].m_actorName;
	playerInfo.m_faction = ActorDefinition::s_actorDefinitions["Marine"].m_actorFaction;
	
	Actor* playerActor = SpawnActor(playerInfo);

	m_game->GetCurrentPlayer()->m_cameraMode = CameraMode::CAMERAMODE_ACTOR;
	m_game->GetCurrentPlayer()->Possess(playerActor->GetActorHandle());

// 	for (int spawnInfoIndex = 0; spawnInfoIndex < (int)m_mapDefinition.m_allDemonSpawnInfo.size(); spawnInfoIndex++)
// 	{
// 		SpawnActor(m_mapDefinition.m_allDemonSpawnInfo[spawnInfoIndex]);
// 	}

	m_game->GetCurrentPlayer()->SetCameraBaseOnCameraMode();

}

void Map::StartUpDialogTexts()
{
	std::string string1 = "Welcome to Hell";
	std::string string2 = "Kill them ALL!";

	m_mapDialogText.push_back(string1);
	m_mapDialogText.push_back(string2);
}

void Map::Update()
{
	if (m_mapDialogTimer->IsStopped() == false)
	{
		if (m_mapDialogTimer->GetElapsedTime() > m_mapDialogTimer->m_period)
		{
			m_showRoundImage = false;
		}
		else
		{
			m_showRoundImage = true;
		}
	}

	for (int i = 0; i < m_game->m_players.size(); i++)
	{
		if (m_game->m_players[i]->GetActor() != nullptr)
		{
			if (m_game->m_players[i]->GetActor()->m_position.x < m_mapDefinition.m_mapBounds.m_mins.x + m_game->m_players[i]->GetActor()->m_actorDefinition.m_collisionRadius)
			{
				m_game->m_players[i]->GetActor()->m_position.x = m_mapDefinition.m_mapBounds.m_mins.x + m_game->m_players[i]->GetActor()->m_actorDefinition.m_collisionRadius;
			}
			if (m_game->m_players[i]->GetActor()->m_position.x > m_mapDefinition.m_mapBounds.m_maxs.x - m_game->m_players[i]->GetActor()->m_actorDefinition.m_collisionRadius)
			{
				m_game->m_players[i]->GetActor()->m_position.x = m_mapDefinition.m_mapBounds.m_maxs.x - m_game->m_players[i]->GetActor()->m_actorDefinition.m_collisionRadius;
			}

			if (m_game->m_players[i]->GetActor()->m_position.y < m_mapDefinition.m_mapBounds.m_mins.y + m_game->m_players[i]->GetActor()->m_actorDefinition.m_collisionRadius)
			{
				m_game->m_players[i]->GetActor()->m_position.y = m_mapDefinition.m_mapBounds.m_mins.y + m_game->m_players[i]->GetActor()->m_actorDefinition.m_collisionRadius;
			}
			if (m_game->m_players[i]->GetActor()->m_position.y > m_mapDefinition.m_mapBounds.m_maxs.y - m_game->m_players[i]->GetActor()->m_actorDefinition.m_collisionRadius)
			{
				m_game->m_players[i]->GetActor()->m_position.y = m_mapDefinition.m_mapBounds.m_maxs.y - m_game->m_players[i]->GetActor()->m_actorDefinition.m_collisionRadius;
			}
		}
	}
	HandleKeyBoardInput();
	UpdateActors();
	CheckCollisionActorWithEachOther();
	CheckCollisionActorWithWorld();
	DeleteDestroyedActors();
	SpawnPlayers();
}

void Map::HandleKeyBoardInput()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_sunDirection.x -= 1.0f;
		std::string currentXComponent = Stringf("Sun Direction X: %.2f", m_sunDirection.x);
		DebugAddMessage(currentXComponent, 3.0f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_sunDirection.x += 1.0f;
		std::string currentXComponent = Stringf("Sun Direction X: %.2f", m_sunDirection.x);
		DebugAddMessage(currentXComponent, 3.0f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_sunDirection.y -= 1.0f;
		std::string currentYComponent = Stringf("Sun Direction Y: %.2f", m_sunDirection.y);
		DebugAddMessage(currentYComponent, 3.0f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		m_sunDirection.y += 1.0f;
		std::string currentYComponent = Stringf("Sun Direction Y: %.2f", m_sunDirection.y);
		DebugAddMessage(currentYComponent, 3.0f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_sunIntensity -= 0.05f;
		m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
		std::string currentSunIntensity = Stringf("Sun Intensity: %.2f", m_sunIntensity);
		DebugAddMessage(currentSunIntensity, 3.0f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_sunIntensity += 0.05f;
		m_sunIntensity = GetClampedZeroToOne(m_sunIntensity);
		std::string currentSunIntensity = Stringf("Sun Intensity: %.2f", m_sunIntensity);
		DebugAddMessage(currentSunIntensity, 3.0f);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_ambientIntensity -= 0.05f;
		m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
		std::string currentSunIntensity = Stringf("Ambient Intensity: %.2f", m_ambientIntensity);
		DebugAddMessage(currentSunIntensity, 3.0f);
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		m_ambientIntensity += 0.05f;
		m_ambientIntensity = GetClampedZeroToOne(m_ambientIntensity);
		std::string currentSunIntensity = Stringf("Ambient Intensity: %.2f", m_ambientIntensity);
		DebugAddMessage(currentSunIntensity, 3.0f);
	}

	// 	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	// 	{
	// 		DebugAddWorldLine(m_game->GetCurrentPlayer()->m_position, 10.0f * m_game->GetCurrentPlayer()->m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D() + m_game->GetCurrentPlayer()->m_position, 0.01f, 10.0f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
	// 		RaycastResult3D raycastResult = RaycastVsAll(m_game->GetCurrentPlayer()->m_position, m_game->GetCurrentPlayer()->m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D(), 10.0f);
	// 		if (raycastResult.m_didImpact)
	// 		{
	// 			DebugAddWorldPoint(raycastResult.m_impactPosition, 0.06f, 10.0f, Rgba8::WHITE, Rgba8::WHITE);
	// 			DebugAddWorldArrow(raycastResult.m_impactPosition, raycastResult.m_impactPosition + 0.3f * raycastResult.m_impactNormal, 0.03f, 10.0f, Rgba8::BLUE, Rgba8::BLUE);
	// 		}
	// 	}																																												
	// 																																																																																						
	// 	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))																			
	// 	{																																												
	// 		DebugAddWorldLine(m_game->GetCurrentPlayer()->m_position, 0.25f * m_game->GetCurrentPlayer()->m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D() + m_game->GetCurrentPlayer()->m_position, 0.01f, 10.0f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
	// 		RaycastResult3D raycastResult = RaycastVsAll(m_game->GetCurrentPlayer()->m_position, m_game->GetCurrentPlayer()->m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D(), 0.25f);
	// 		if (raycastResult.m_didImpact)
	// 		{
	// 			DebugAddWorldPoint(raycastResult.m_impactPosition, 0.06f, 10.0f, Rgba8::WHITE, Rgba8::WHITE);
	// 			DebugAddWorldArrow(raycastResult.m_impactPosition, raycastResult.m_impactPosition + 0.3f * raycastResult.m_impactNormal, 0.03f, 10.0f, Rgba8::BLUE, Rgba8::BLUE);
	// 		}
	// 	}
	if (g_theInput->WasKeyJustPressed('N'))
	{
		PossessNextActor();
	}
}

void Map::UpdateActors()
{
	EnemyInfo info = m_mapDefinition.m_allDemonSpawnInfo[m_currentRoundIndex];
	if (m_roundEnemies.size() == info.m_enemyInfo.size())
	{
		m_enemySpawnTimer->Stop();
		m_lastFrameEnemySpawnTimer = 0.0f;

		bool allEnemiesDead = true;
		for (int i = 0; i < m_roundEnemies.size(); ++i)
		{
			if (m_roundEnemies[i] != nullptr)
			{
				allEnemiesDead = false;
				break;
			}
		}
		if (allEnemiesDead == true)
		{
			if (m_currentRoundIndex + 1 < m_mapDefinition.m_allDemonSpawnInfo.size())
			{
				m_roundEnemies.clear();
				m_currentRoundIndex += 1;
				m_mapDialogTimer->Start();
				m_enemySpawnTimer->Start();
			}
			else
			{
				m_showUI = true;
				std::string string1 = "You Survived!";
				m_mapDialogText.clear();
				m_mapDialogText.push_back(string1);
				m_isend = true;
			}
		}
	}
	else
	{
		for (int i = 0; i < info.m_enemyInfo.size(); i++)
		{
			SpawnInfo& spawnInfo = info.m_enemyInfo[i];

			if ((float)m_enemySpawnTimer->GetElapsedTime() > spawnInfo.m_timeOffset &&
				m_lastFrameEnemySpawnTimer < spawnInfo.m_timeOffset)
			{
				Actor* actor = SpawnActor(info.m_enemyInfo[i]);
				m_roundEnemies.push_back(actor);
			}
		}

		m_lastFrameEnemySpawnTimer = (float)m_enemySpawnTimer->GetElapsedTime();
	}

	for (int actorNum = 0; actorNum < (int)m_actors.size(); actorNum++)
	{
		if (m_actors[actorNum] != nullptr)
		{
			m_actors[actorNum]->Update();
		}
	}
}

void Map::CheckCollisionActorWithEachOther()
{
	for (int actorNum = 0; actorNum < (int)m_actors.size(); actorNum++)
	{
		if (m_actors[actorNum] != nullptr && !m_actors[actorNum]->m_isDead)
		{
			for (int actorAfterIndex = actorNum + 1; actorAfterIndex < (int)m_actors.size(); actorAfterIndex++)
			{
				if (m_actors[actorAfterIndex] != nullptr && !m_actors[actorAfterIndex]->m_isDead)
				{
					Actor* actor1 = m_actors[actorNum];
					Actor* actor2 = m_actors[actorAfterIndex];
					if (actor1->m_owner == actor2 || actor2->m_owner == actor1)
					{
						continue;
					}

					FloatRange actor1ZRange = actor1->GetActorZRange();
					FloatRange actor2ZRange = actor2->GetActorZRange();
					if (!actor1ZRange.IsOnRange(actor2ZRange.m_min) && !actor1ZRange.IsOnRange(actor2ZRange.m_max))
					{
						continue;
					}
					if (actor1->m_actorDefinition.m_actorFaction == Faction::FACTION_MARINEPROJECTILE &&
						actor2->m_actorDefinition.m_actorFaction == Faction::FACTION_MARINEPROJECTILE)
					{
						continue;
					}
					if (actor1->m_actorDefinition.m_actorFaction == Faction::FACTION_MARINEPROJECTILE &&
						actor2->m_actorDefinition.m_actorFaction == Faction::FACTION_DEMON)
					{
						if (IsDiscsOverlap(actor1->GetActorPositionXY(), actor1->m_actorDefinition.m_collisionRadius, actor2->GetActorPositionXY(), actor2->m_actorDefinition.m_collisionRadius))
						{
							actor1->Dead(nullptr);
							actor2->Damage(actor1->GetRandomDamageValue(), actor1->m_owner);
							actor2->AddImpulse(actor1->m_actorDefinition.m_impulseOnCollide * actor1->GetOrientationOnlyYaw().GetIBasis());
							continue;
						}
					}
					if (actor2->m_actorDefinition.m_actorFaction == Faction::FACTION_MARINEPROJECTILE &&
						actor1->m_actorDefinition.m_actorFaction == Faction::FACTION_DEMON)
					{
						if (IsDiscsOverlap(actor1->GetActorPositionXY(), actor1->m_actorDefinition.m_collisionRadius, actor2->GetActorPositionXY(), actor2->m_actorDefinition.m_collisionRadius))
						{
							actor1->Damage(actor2->GetRandomDamageValue(), actor2->m_owner);
							actor1->AddImpulse(actor2->m_actorDefinition.m_impulseOnCollide * actor2->GetOrientationOnlyYaw().GetIBasis());
							actor2->Dead(nullptr);
							continue;
						}
					}


					Vec2 actor1Location = Vec2(actor1->m_position.x, actor1->m_position.y);
					Vec2 actor2Location = Vec2(actor2->m_position.x, actor2->m_position.y);
					if (actor1->m_isStatic == false && actor2->m_isStatic == false)
					{
						PushDiscsOutOfEachOther2D(actor1Location, actor1->m_actorDefinition.m_collisionRadius, actor2Location, actor2->m_actorDefinition.m_collisionRadius);
					}
					if (actor1->m_isStatic == false && actor2->m_isStatic == true)
					{
						PushDiscOutOfDisc2D(actor1Location, actor1->m_actorDefinition.m_collisionRadius, actor2Location, actor2->m_actorDefinition.m_collisionRadius);
					}
					if (actor1->m_isStatic == true && actor2->m_isStatic == false)
					{
						PushDiscOutOfDisc2D(actor2Location, actor2->m_actorDefinition.m_collisionRadius, actor1Location, actor1->m_actorDefinition.m_collisionRadius);
					}
					actor1->m_position.x = actor1Location.x;
					actor1->m_position.y = actor1Location.y;
					actor2->m_position.x = actor2Location.x;
					actor2->m_position.y = actor2Location.y;

				}
			}
		}
	}
}
void Map::CheckCollisionActorWithWorld()
{
	for (int actorNum = 0; actorNum < (int)m_actors.size(); actorNum++)
	{
		if (m_actors[actorNum] == nullptr || m_actors[actorNum]->m_isDead)
		{
			continue;
		}

		// Push out the cylinder from other tiles
		std::vector<Tile*> aroundTiles = GetAllTilesAround(m_actors[actorNum]->m_position);

		for (int i = 0; i < (int)aroundTiles.size(); i++)
		{
			if (aroundTiles[i] != nullptr && aroundTiles[i]->m_tileDef.m_isSolid)
			{
				Vec2 mins = Vec2(aroundTiles[i]->m_bounds.m_mins.x, aroundTiles[i]->m_bounds.m_mins.y);
				Vec2 maxs = Vec2(aroundTiles[i]->m_bounds.m_maxs.x, aroundTiles[i]->m_bounds.m_maxs.y);

				AABB2 tileTopView = AABB2(mins, maxs);

				Vec2 actorLocation = Vec2(m_actors[actorNum]->m_position.x, m_actors[actorNum]->m_position.y);

				if (IsPointInsideDisc2D(GetNearestPointOnAABB2D(actorLocation, tileTopView), actorLocation, m_actors[actorNum]->m_actorDefinition.m_collisionRadius))
				{
					if (m_actors[actorNum]->m_position.z <= aroundTiles[i]->GetTileDef().m_tileHeight)
					{
						if (m_actors[actorNum]->m_actorDefinition.m_actorFaction == Faction::FACTION_MARINEPROJECTILE || m_actors[actorNum]->m_actorDefinition.m_actorFaction == Faction::FACTION_DEMONPROJECTILE)
						{
							m_actors[actorNum]->Dead(nullptr);
						}

						PushDiscOutOfAABB2D(actorLocation, m_actors[actorNum]->m_actorDefinition.m_collisionRadius, tileTopView);
						m_actors[actorNum]->m_position.x = actorLocation.x;
						m_actors[actorNum]->m_position.y = actorLocation.y;

					}
				}
			}
		}

		// Push out the cylinder out of floor and ceiling

		if (m_actors[actorNum]->m_position.z + m_actors[actorNum]->m_actorDefinition.m_collisionHeight > m_tiles[0]->GetTileDef().m_tileHeight)
		{
			m_actors[actorNum]->m_position.z = m_tiles[0]->GetTileDef().m_tileHeight - m_actors[actorNum]->m_actorDefinition.m_collisionHeight;
			if (m_actors[actorNum]->m_actorDefinition.m_actorFaction == Faction::FACTION_MARINEPROJECTILE || m_actors[actorNum]->m_actorDefinition.m_actorFaction == Faction::FACTION_DEMONPROJECTILE)
			{
				m_actors[actorNum]->Dead(nullptr);
				continue;
			}
		}

		if (m_actors[actorNum]->m_position.z < 0.0f)
		{
			m_actors[actorNum]->m_position.z = 0.0f;
			if (m_actors[actorNum]->m_actorDefinition.m_actorFaction == Faction::FACTION_MARINEPROJECTILE || m_actors[actorNum]->m_actorDefinition.m_actorFaction == Faction::FACTION_DEMONPROJECTILE)
			{
				m_actors[actorNum]->Dead(nullptr);
				continue;
			}
		}
	}

}



void Map::DeleteDestroyedActors()
{
	for (unsigned int i = 0; i < (int)m_actors.size(); ++i)
	{
		if (m_actors[i] != nullptr)
		{
			if (m_actors[i]->m_isDestroyed)
			{
				delete m_actors[i];
				m_actors[i] = nullptr;
			}
		}
	}

	for (unsigned int i = 0; i < (int)m_roundEnemies.size(); ++i)
	{
		if (m_roundEnemies[i] != nullptr)
		{
			if (m_roundEnemies[i]->m_isDestroyed)
			{
				m_roundEnemies[i] = nullptr;
			}
		}
	}
}

void Map::SpawnPlayers()
{
	for (int i = 0; i < (int)m_game->GetPlayerListNum(); i++)
	{
		if (m_game->GetPlayerByIndex(i) != nullptr)
		{
			if (m_game->GetPlayerByIndex(i)->GetActor() == nullptr)
			{
				RespawnPlayerActor(m_game->GetPlayerByIndex(i));
			}
		}
	}
}

void Map::SpawnActorForPlayer(Player* player)
{
	int spawnPointIndex = player->GetPlayerIndex();

	SpawnInfo playerInfo = m_mapDefinition.m_allSpawnPoints[spawnPointIndex];

	playerInfo.m_actorName = ActorDefinition::s_actorDefinitions["Marine"].m_actorName;
	playerInfo.m_faction = ActorDefinition::s_actorDefinitions["Marine"].m_actorFaction;

	Actor* playerActor = SpawnActor(playerInfo);

	player->m_cameraMode = CameraMode::CAMERAMODE_ACTOR;
	player->Possess(playerActor->GetActorHandle());
	player->SetCameraBaseOnCameraMode();
}

// void Map::SpawnActorBesidesPlayer()
// {
// 	for (int spawnInfoIndex = 0; spawnInfoIndex < (int)m_mapDefinition.m_allDemonSpawnInfo.size(); spawnInfoIndex++)
// 	{
// 		SpawnActor(m_mapDefinition.m_allDemonSpawnInfo[spawnInfoIndex]);
// 	}
// }

RaycastResult3D Map::RaycastVsAll(Vec3 const& startPosition, Vec3 const& rayDirection, float maxDistance, ActorHandle ownerHandle, ActorHandle& hitActorHandle)
{
	std::vector<RaycastResult3D> allResult;
	RaycastResult3D resultAll;
	
	RaycastResult3D resultVsAllActors;
	hitActorHandle = ActorHandle::INVALID;
	resultVsAllActors = RaycastVsAllActors(startPosition, rayDirection, maxDistance, ownerHandle, hitActorHandle);

	RaycastResult3D resultVsWorldGrids;
	resultVsWorldGrids = RaycastVsWorldGrids(startPosition, rayDirection, maxDistance);

	RaycastResult3D	resultVsWorldFloorAndCeiling;
	resultVsWorldFloorAndCeiling = RaycastVsWorldFloorAndCeiling(startPosition, rayDirection, maxDistance);


	allResult.push_back(resultVsAllActors);
	allResult.push_back(resultVsWorldGrids);
	allResult.push_back(resultVsWorldFloorAndCeiling);

	float currentNearestDistance = FLT_MAX;
	for (int i = 0; i < (int)allResult.size(); i++)
	{
		if (allResult[i].m_didImpact == false)
		{
			continue;
		}
		if (allResult[i].m_impactDistance < currentNearestDistance)
		{
			currentNearestDistance = allResult[i].m_impactDistance;
			resultAll = allResult[i];
		}
	}
	if (resultAll.m_impactDistance != resultVsAllActors.m_impactDistance)
	{
		hitActorHandle == ActorHandle::INVALID;
	}
	return resultAll;
}

RaycastResult3D Map::RaycastVsAllActors(Vec3 const& startPosition, Vec3 const& rayDirection, float maxDistance, ActorHandle ownerHandle, ActorHandle& hitActorHandle)
{
	RaycastResult3D resultVsAllActors;
	float currentNearestLength = FLT_MAX;

	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i] != nullptr)
		{
			if (m_actors[i]->GetActorHandle() == ownerHandle)
			{
				continue;
			}
			if (m_actors[i]->m_actorDefinition.m_actorFaction == Faction::FACTION_MARINEPROJECTILE)
			{
				continue;
			}
			Vec2 actorPositionXY = m_actors[i]->GetActorPositionXY();
			FloatRange minMaxZ = m_actors[i]->GetActorZRange();
			RaycastResult3D newResult = RaycastVsCylinderZ3D(startPosition, rayDirection, maxDistance, actorPositionXY, minMaxZ, m_actors[i]->m_actorDefinition.m_collisionRadius);
			
			if (newResult.m_didImpact)
			{
				if (newResult.m_impactDistance < currentNearestLength)
				{
					currentNearestLength = newResult.m_impactDistance;
					resultVsAllActors = newResult;
					hitActorHandle = m_actors[i]->GetActorHandle();
				}
			}
		}
	}
	return resultVsAllActors;
}

RaycastResult3D Map::RaycastVsWorldGrids(Vec3 const& startPosition, Vec3 const& rayDirection, float maxDistance)
{
	RaycastResult3D resultVsWorldGrids;
	IntVec2 tileCoords = GetTileCoordsXYBaseOnPosition(startPosition);

	if (IsPositionInsideSolidTile(startPosition))
	{
		resultVsWorldGrids.m_didImpact = true;
		resultVsWorldGrids.m_impactDistance = 0.0f;
		resultVsWorldGrids.m_impactPosition = startPosition;
		resultVsWorldGrids.m_impactNormal = -rayDirection;
		return resultVsWorldGrids;
	}

	int currentStepX = tileCoords.x;
	int currentStepY = tileCoords.y;
	float fwdDistPerXCrossing = 1.0f / (rayDirection.x * maxDistance);
	float fwdDistPerYCrossing = 1.0f / (rayDirection.y * maxDistance);
	int tileStepDirectionX = rayDirection.x > 0 ? 1 : -1;
	int tileStepDirectionY = rayDirection.y > 0 ? 1 : -1;

	float xAtNextXCrossing;
	float yAtNextYCrossing;
	float distanceAlongXAxis;
	float distanceAlongYAxis;
	float tValueAtNextXCrossing = 0.0f;
	float tValueAtNextYCrossing = 0.0f;
	Vec3 tmpImpactPosition = Vec3(-999.0f, -999.0f, -999.0f);

	while (!IsPositionInsideSolidTile(IntVec2(currentStepX, currentStepY), tmpImpactPosition.z))
	{
		xAtNextXCrossing = static_cast<float>(currentStepX + (tileStepDirectionX + 1) / 2.0f);
		yAtNextYCrossing = static_cast<float>(currentStepY + (tileStepDirectionY + 1) / 2.0f);
		distanceAlongXAxis = xAtNextXCrossing - (float)startPosition.x;
		distanceAlongYAxis = yAtNextYCrossing - (float)startPosition.y;
		tValueAtNextXCrossing = distanceAlongXAxis * fwdDistPerXCrossing;
		tValueAtNextYCrossing = distanceAlongYAxis * fwdDistPerYCrossing;

		if (tValueAtNextXCrossing < tValueAtNextYCrossing)
		{
			if (tValueAtNextXCrossing > 1.0f)
			{
				return resultVsWorldGrids;
			}
			currentStepX += tileStepDirectionX;
			tmpImpactPosition = startPosition + (rayDirection * tValueAtNextXCrossing * maxDistance);
		}
		else
		{
			if (tValueAtNextYCrossing > 1.0f)
			{
				return resultVsWorldGrids;
			}
			currentStepY += tileStepDirectionY;
			tmpImpactPosition = startPosition + (rayDirection * tValueAtNextYCrossing * maxDistance);
		}
	}

	resultVsWorldGrids.m_didImpact = true;
	if (tValueAtNextXCrossing < tValueAtNextYCrossing)
	{
		resultVsWorldGrids.m_impactPosition = startPosition + (rayDirection * tValueAtNextXCrossing * maxDistance);
		if (rayDirection.x > 0)
		{
			resultVsWorldGrids.m_impactNormal = Vec3(-1.0f, 0.0f, 0.0f);
		}
		else
		{
			resultVsWorldGrids.m_impactNormal = Vec3(1.0f, 0.0f, 0.0f);
		}
		resultVsWorldGrids.m_impactDistance = tValueAtNextXCrossing * maxDistance;
	}
	else
	{
		resultVsWorldGrids.m_impactPosition = startPosition + (rayDirection * tValueAtNextYCrossing * maxDistance);
		if (rayDirection.y > 0)
		{
			resultVsWorldGrids.m_impactNormal = Vec3(0.0f, -1.0f, 0.0f);
		}
		else
		{
			resultVsWorldGrids.m_impactNormal = Vec3(0.0f, 1.0f, 0.0f);
		}
		resultVsWorldGrids.m_impactDistance = tValueAtNextYCrossing * maxDistance;
	}
	return resultVsWorldGrids;
}

RaycastResult3D Map::RaycastVsWorldFloorAndCeiling(Vec3 const& startPosition, Vec3 const& rayDirection, float maxDistance)
{
	RaycastResult3D resultVsFloorAndCeiling;

	if (rayDirection.z > 0)
	{
		float tValueForCeiling = (1.0f - startPosition.z) / (rayDirection * maxDistance).z;
		if (tValueForCeiling > 0.0f && tValueForCeiling < 1.0f)
		{
			resultVsFloorAndCeiling.m_didImpact = true;
			resultVsFloorAndCeiling.m_impactDistance = tValueForCeiling * maxDistance;
			resultVsFloorAndCeiling.m_impactPosition = startPosition + resultVsFloorAndCeiling.m_impactDistance * rayDirection;
			if (!IsPositionInsideWorldBounds(resultVsFloorAndCeiling.m_impactPosition))
			{
				return RaycastResult3D();
			}
			resultVsFloorAndCeiling.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f);
		}
		else
		{
			return resultVsFloorAndCeiling;
		}
	}
	else
	{
		float tValueForCeiling = (0.0f - startPosition.z) / (rayDirection * maxDistance).z;
		if (tValueForCeiling > 0.0f && tValueForCeiling < 1.0f)
		{
			resultVsFloorAndCeiling.m_didImpact = true;
			resultVsFloorAndCeiling.m_impactDistance = tValueForCeiling * maxDistance;
			resultVsFloorAndCeiling.m_impactPosition = startPosition + resultVsFloorAndCeiling.m_impactDistance * rayDirection;
			if (!IsPositionInsideWorldBounds(resultVsFloorAndCeiling.m_impactPosition))
			{
				return RaycastResult3D();
			}
			resultVsFloorAndCeiling.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
		}
		else
		{
			return resultVsFloorAndCeiling;
		}
	}

	return resultVsFloorAndCeiling;
}


void Map::Render(Camera const& camera, int playerIndex) const
{
	g_theRenderer->BindShader(m_game->m_diffuseShader);
	g_theRenderer->BindTexture(&m_spriteSheet->GetTexture());
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetLightConstants(m_sunDirection, m_sunIntensity, m_ambientIntensity);
	g_theRenderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data(), (int)m_indexes.size(), m_indexes.data());
	if (m_mapDefinition.m_skyBoxTexturePath != DEFAULT_NAME)
	{
		Texture* skyBoxTexture = g_theRenderer->CreateOrGetTextureFromFile(m_mapDefinition.m_skyBoxTexturePath.c_str());
		g_theRenderer->BindTexture(skyBoxTexture);
		g_theRenderer->SetLightConstants(m_sunDirection, 0.2f, 0.5f);
		g_theRenderer->DrawVertexArray((int)m_skyboxVertes.size(), m_skyboxVertes.data(), (int)m_skyBoxIndexes.size(), m_skyBoxIndexes.data());
	}
	RenderActors(camera, playerIndex);
}

void Map::RenderActors(Camera const& camera, int playerIndex) const
{
	for (int actorNum = 0; actorNum < (int)m_actors.size(); actorNum++)
	{
		if (m_actors[actorNum] != nullptr)
		{
			m_actors[actorNum]->Render(camera, playerIndex);
		}
	}
}

void Map::RenderMapUIStuff() const
{
	if (m_showUI)
	{
		std::vector<Vertex_PCU> inGameUI;
		inGameUI.reserve(60);
		AABB2 mapUIBox = AABB2(Vec2(0.f, 0.f), Vec2(1600.0f, 800.0f));
		AddVertsForAABB2D(inGameUI, mapUIBox, Rgba8::WHITE);
		Texture* dialogTexture = g_theRenderer->CreateOrGetTextureFromFile(m_mapDefinition.m_welcomeTexture.c_str());
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(dialogTexture);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetLightConstants();
		g_theRenderer->DrawVertexArray((int)inGameUI.size(), inGameUI.data());

		inGameUI.clear();
		AABB2 textBox = AABB2(Vec2(550.0f, 450.0f), Vec2(1000.0f, 850.0f));
		g_theFont->AddVertsForTextInBox2D(inGameUI, m_mapDialogText[m_dialogIndex], textBox, 80.0f, Rgba8::WHITE, 0.5f);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(&g_theFont->GetTexture());
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetLightConstants();
		g_theRenderer->DrawVertexArray((int)inGameUI.size(), inGameUI.data());
	}

	if (m_showRoundImage)
	{
		std::vector<Vertex_PCU> RoundUIVerts;
		RoundUIVerts.reserve(6);

		AABB2 roundVertsBox = AABB2(Vec2(950.0f, 200.f ), Vec2(1600.f, 680.0f));
		AddVertsForAABB2D(RoundUIVerts, roundVertsBox, Rgba8::WHITE);
		Texture* roundTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/ImComing.png");
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(roundTexture);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetLightConstants();
		g_theRenderer->DrawVertexArray((int)RoundUIVerts.size(), RoundUIVerts.data());

		RoundUIVerts.clear();
		AABB2 textBox = AABB2(Vec2(400.0f, 310.0f), Vec2(1050.0f, 610.0f));
		g_theFont->AddVertsForTextInBox2D(RoundUIVerts, Stringf("Wave %i Start!", m_currentRoundIndex + 1), textBox, 100.0f, Rgba8::RED, 1.0f, Vec2(0.0f, 0.5f));
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(&g_theFont->GetTexture());
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetLightConstants();
		g_theRenderer->DrawVertexArray((int)RoundUIVerts.size(), RoundUIVerts.data());

	}
}

IntVec2 Map::GetTileCoordsXYBaseOnPosition(Vec3 const& position)
{
	return IntVec2(RoundDownToInt(position.x), RoundDownToInt(position.y));
}

Tile* Map::GetTileBaseOnPosition(Vec3 const& position)
{
	if (position.x < 0.0f || position.x >(float)m_dimensions.x)
	{
		return nullptr;
	}
	if (position.y < 0.0f || position.y >(float)m_dimensions.y)
	{
		return nullptr;
	}
	IntVec2 tileDimension = IntVec2(RoundDownToInt(position.x), RoundDownToInt(position.y));
	size_t tileIndex = static_cast<size_t>(m_dimensions.y * tileDimension.x + (tileDimension.y));
	return m_tiles[tileIndex];
}

Tile* Map::GetTileBaseOnTilePosition(IntVec2 const& tilePos)
{
	size_t tileIndex = static_cast<size_t>(m_dimensions.y * (tilePos.x) + (tilePos.y));
	if (tileIndex<0 || tileIndex>m_tiles.size() - 1)
	{
		return nullptr;
	}
	return m_tiles[tileIndex];
}

bool Map::IsTileSolid(int tileCoordX, int tileCoordY)
{
	if (tileCoordX < 0 || tileCoordX>m_dimensions.x - 1 || tileCoordY < 0 || tileCoordY>m_dimensions.y - 1)
	{
		return false;
	}
	Tile* tileAtPos = GetTileBaseOnTilePosition(IntVec2(tileCoordX, tileCoordY));
	return tileAtPos->m_tileDef.m_isSolid;
}


bool Map::IsPositionInsideSolidTile(Vec3 const& position)
{
	Tile* tile = GetTileBaseOnPosition(position);
	if (tile == nullptr)
	{
		return false;
	}
	bool isPositionZInRange = position.z >= tile->m_bounds.m_mins.z && position.z <= tile->m_bounds.m_maxs.z;
	return isPositionZInRange && IsTileSolid(RoundDownToInt(position.x), RoundDownToInt(position.y));
}


bool Map::IsPositionInsideSolidTile(IntVec2 const& tileCoords, float zValue)
{
	Tile* tile = GetTileBaseOnTilePosition(tileCoords);
	if (tile == nullptr)
	{
		return false;
	}
	bool isPositionZInRange = zValue >= tile->m_bounds.m_mins.z && zValue <= tile->m_bounds.m_maxs.z;
	return isPositionZInRange && IsTileSolid(tileCoords.x, tileCoords.y);
}

bool Map::IsPositionInsideWorldBounds(Vec3 const& position)
{
	FloatRange worldDimensionX = FloatRange(0.0f, (float)m_dimensions.x);
	FloatRange worldDimensionY = FloatRange(0.0f, (float)m_dimensions.y);
	return worldDimensionX.IsOnRange(position.x) && worldDimensionY.IsOnRange(position.y);
}

bool Map::IsPositionInsideBound(Vec3 const& position)
{
	return(position.x > m_mapDefinition.m_mapBounds.m_mins.x &&
		position.y > m_mapDefinition.m_mapBounds.m_mins.y &&
		position.x < m_mapDefinition.m_mapBounds.m_maxs.x &&
		position.y < m_mapDefinition.m_mapBounds.m_maxs.y);
}

bool Map::IsPlayerActorAccessable(Vec3 const& targetPosition, Vec3 const& actorPosition)
{
	IntVec2 actorIntCoords = IntVec2(RoundDownToInt(actorPosition.x), RoundDownToInt(actorPosition.y));
	PopulateDistanceField(m_heatMap, actorIntCoords);
	Tile* targetTile = GetTileBaseOnPosition(targetPosition);
	int tileIndex = GetTileIndex(targetTile);

	if (m_heatMap->GetHeatValueByIndex(tileIndex) == 999.0f)
	{
		return false;
	}
	if (m_heatMap->GetHeatValueByIndex(tileIndex) == -1.0f)
	{
		return false;
	}
	return true;
}

std::vector<Tile*> Map::GetAllTilesAround(Vec3 const& position)
{
// 	if (position.x<0.0f || position.x>(float)m_dimensions.x)
// 	{
// 		ERROR_AND_DIE("Position is outside of the x range of the map")
// 	}
// 	if (position.y<0.0f || position.y>(float)m_dimensions.y)
// 	{
// 		ERROR_AND_DIE("Position is outside of the y range of the map");
// 	}

	int positionXInt;
	int positionYInt;

	if (position.x < -0.99f)
	{
		positionXInt = 0;
	}
	else if (position.x > (float)m_dimensions.x + 0.99f)
	{
		positionXInt = m_dimensions.x;
	}
	else
	{
		positionXInt = RoundDownToInt(position.x);
	}

	if (position.y < -0.99f)
	{
		positionYInt = 0;
	}
	else if (position.y > (float)m_dimensions.y + 0.99f)
	{
		positionYInt = m_dimensions.y;
	}
	else 
	{
		positionYInt = RoundDownToInt(position.y);
	}

	IntVec2 tileDimension = IntVec2(positionXInt, positionYInt);
	std::vector<Tile*>	allTilesAround;
	allTilesAround.reserve(8);
	allTilesAround.push_back(GetTileBaseOnTilePosition(IntVec2(tileDimension.x - 1, tileDimension.y)));
	allTilesAround.push_back(GetTileBaseOnTilePosition(IntVec2(tileDimension.x + 1, tileDimension.y)));
	allTilesAround.push_back(GetTileBaseOnTilePosition(IntVec2(tileDimension.x, tileDimension.y - 1)));
	allTilesAround.push_back(GetTileBaseOnTilePosition(IntVec2(tileDimension.x, tileDimension.y + 1)));
	allTilesAround.push_back(GetTileBaseOnTilePosition(IntVec2(tileDimension.x - 1, tileDimension.y - 1)));
	allTilesAround.push_back(GetTileBaseOnTilePosition(IntVec2(tileDimension.x - 1, tileDimension.y + 1)));
	allTilesAround.push_back(GetTileBaseOnTilePosition(IntVec2(tileDimension.x + 1, tileDimension.y - 1)));
	allTilesAround.push_back(GetTileBaseOnTilePosition(IntVec2(tileDimension.x + 1, tileDimension.y + 1)));
	return allTilesAround;
}

void Map::UpdateControlMode(Vec3 const& velocity)
{
	float m_speed = 1.0f;

	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		m_speed = 15.0f;
	}

	m_actors[3]->m_position += velocity * m_speed * static_cast<float>(Clock::GetSystemClock().GetDeltaSeconds());

}

void Map::DebugPossessNext()
{
	ActorHandle currentPlayerPossessedActorUID = m_game->GetCurrentPlayer()->m_actorHandle;
	if (currentPlayerPossessedActorUID == ActorHandle::INVALID)
	{
		return;
	}

	Actor* nextActorToPossess = nullptr;

	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i]->m_actorHandle == currentPlayerPossessedActorUID)
		{
			if (i == m_actors.size() - 1)
			{
				nextActorToPossess = m_actors[0];
				break;
			}
			nextActorToPossess = m_actors[(i + 1)];
			break;
		}
	}

	if (nextActorToPossess != nullptr)
	{
		if (nextActorToPossess->m_actorDefinition.m_canBePossessed)
		{
			m_game->GetCurrentPlayer()->m_cameraMode = CameraMode::CAMERAMODE_ACTOR;
			m_game->GetCurrentPlayer()->Possess(nextActorToPossess->GetActorHandle());
		}
	}
}

void Map::PossessNextActor()
{
	if (m_game->m_isSinglePlayerMode == false)
	{
		return;
	}
	ActorHandle currentPlayerPossessedActorUID = m_game->GetPlayerByIndex()->m_actorHandle;
	if (currentPlayerPossessedActorUID == ActorHandle::INVALID)
	{
		return;
	}

	Actor* nextActorToPossess = nullptr;
	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i]->m_actorHandle == currentPlayerPossessedActorUID)
		{
			if (i == m_actors.size() - 1)
			{
				nextActorToPossess = m_actors[0];
				break;
			}
			nextActorToPossess = m_actors[(i + 1)];
			break;
		}
	}

	if (nextActorToPossess != nullptr)
	{
		if (nextActorToPossess->m_actorDefinition.m_canBePossessed)
		{
			m_game->GetPlayerByIndex()->m_cameraMode = CameraMode::CAMERAMODE_ACTOR;
			m_game->GetPlayerByIndex()->Possess(nextActorToPossess->GetActorHandle());
		}
	}
}

void Map::RespawnPlayerActor(Player* playerController)
{
	int spawnPointIndex = m_game->m_rng->RollRandomIntLessThan((int)m_mapDefinition.m_allSpawnPoints.size());

	SpawnInfo playerInfo = m_mapDefinition.m_allSpawnPoints[spawnPointIndex];

	playerInfo.m_actorName = ActorDefinition::s_actorDefinitions["Marine"].m_actorName;
	playerInfo.m_faction = ActorDefinition::s_actorDefinitions["Marine"].m_actorFaction;


	for (int i = 0; i < m_actors.size(); i++)
	{
		if (m_actors[i] == nullptr)
		{
			continue;
		}
		if (m_actors[i]->m_actorHandle == playerController->m_actorHandle)
		{
			delete m_actors[i];
			m_actors[i] = nullptr;
			continue;
		}

		if (m_actors[i]->m_actorDefinition.m_actorFaction == Faction::FACTION_DEMON)
		{
			if (m_actors[i]->m_AIController != nullptr)
			{
				if (m_actors[i]->m_AIController->m_targetActorHandle == playerController->m_actorHandle)
				{
					m_actors[i]->m_AIController->m_targetActorHandle == ActorHandle::INVALID;
				}
			}
		}
	}

	Actor* playerActor = SpawnActor(playerInfo);
	playerController->m_cameraMode = CameraMode::CAMERAMODE_ACTOR;
	playerController->Possess(playerActor->GetActorHandle());
	playerController->SetCameraBaseOnCameraMode();
}

Actor* Map::SpawnActor(const SpawnInfo& spawnInfo)
{
	for (unsigned int i = 0; i < (int)m_actors.size(); ++i)
	{
		if (m_actors[i] == nullptr)
		{
			ActorHandle actorHandle = ActorHandle(m_actorSalt, i);

			Actor* actor = new Actor(this, ActorDefinition::s_actorDefinitions[spawnInfo.m_actorName], actorHandle);
			m_actors[i] = actor;

			actor->m_position = spawnInfo.m_position;
			actor->m_orientation = spawnInfo.m_orientation;
			actor->m_map = this;

			actor->StartUp();

			return actor;
		}
	}

	ActorHandle actorHandle = ActorHandle(m_actorSalt, (unsigned int)m_actors.size());

	Actor* actor = new Actor(this, ActorDefinition::s_actorDefinitions[spawnInfo.m_actorName], actorHandle);
	m_actors.push_back(actor);

	actor->m_position = spawnInfo.m_position;
	actor->m_orientation = spawnInfo.m_orientation;
	actor->m_map = this;
	actor->StartUp();

	return actor;
}

Actor* Map::GetActorByHandle(const ActorHandle& handle) const
{
	// 	unsigned int actorIndex = handle.GetIndex();
	// 	if ((size_t)actorIndex > m_actors.size())
	// 	{
	// 		return nullptr;
	// 	}
	// 	if (m_actors[(size_t)actorIndex] == nullptr)
	// 	{
	// 		return nullptr;
	// 	}
	// 	if (m_actors[(size_t)actorIndex]->m_actorHandle != handle)
	// 	{
	// 		return nullptr;
	// 	}
	// 	return m_actors[(size_t)actorIndex];

	unsigned int actorIndex = handle.GetIndex();

	if ((size_t)actorIndex > m_actors.size())
	{
		return nullptr;
	}

	if (m_actors[(size_t)actorIndex] == nullptr)
	{
		return nullptr;
	}

	if (m_actors[(size_t)actorIndex]->m_actorHandle != handle)
	{
		return nullptr;
	}

	return m_actors[(size_t)actorIndex];
}

Actor* Map::GetClosestVisibleEnemy(Actor* currentActor)
{
	Actor* currentNearestOne = nullptr;
	float currentNearestDistance = currentActor->m_actorDefinition.m_aiSightRadius;

	for (int i = 0; i < (int)m_actors.size(); i++)
	{
		if (m_actors[i] != nullptr)
		{
			if (m_actors[i]->m_actorDefinition.m_visible)
			{
				if (m_actors[i]->m_actorDefinition.m_actorFaction != currentActor->m_actorDefinition.m_actorFaction &&
					m_actors[i]->m_actorDefinition.m_canBePossessed)
				{
					float distanceTwoActors = GetDistance2D(currentActor->GetActorPositionXY(), m_actors[i]->GetActorPositionXY());
					float angleTwoActors = GetAngleDegreesBetweenVectors2D(
						Vec2(currentActor->GetOrientationOnlyYaw().GetIBasis().x,
							currentActor->GetOrientationOnlyYaw().GetIBasis().y),
						m_actors[i]->GetActorPositionXY() - currentActor->GetActorPositionXY());

					if (distanceTwoActors < currentNearestDistance && angleTwoActors < currentActor->m_actorDefinition.m_aiSightAngle * 0.5f)
					{
						currentNearestDistance = distanceTwoActors;
						currentNearestOne = m_actors[i];
					}
				}
			}
		}
	}
	return currentNearestOne;
}

Game* Map::GetGameInstance() const
{
	return m_game;
}

void Map::LoadNextUIText()
{
	if (m_isend)
	{
		m_game->ExitState(GameState::PLAYING);
	}
	if (m_dialogIndex < m_mapDialogText.size() - 1)
	{
		m_dialogIndex += 1;
	}
	else
	{
		m_dialogIndex = 0;
		m_showUI = false;
		m_mapDialogTimer->Start();
		m_enemySpawnTimer->Start();
	}
}

bool Map::PopulateDistanceField(TileHeatMap* out_distanceField, IntVec2 startCoords, float maxConst /*= 999.0f*/)
{
	Tile* currentTile = GetTileBaseOnTilePosition(startCoords);

	if (currentTile == nullptr)
	{
		return false;
	}

	if (currentTile->GetTileDef().m_isSolid)
	{
		return false;
	}

	out_distanceField->SetAllValues(maxConst);

	out_distanceField->SetValueByIndex(GetTileIndex(currentTile), 0.0f);

	for (int valueIndex = 0; valueIndex < out_distanceField->GetValuesSize(); ++valueIndex)
	{
		if (m_tiles[valueIndex]->GetTileDef().m_isSolid)
		{
			out_distanceField->SetValueByIndex(valueIndex, -1.0f);
		}
	}


	bool mapBeenChange = true;

	float currentLowestValue = 0.0f;

	while (mapBeenChange && currentLowestValue < maxConst)
	{
		mapBeenChange = false;

		for (int i = 0; i < out_distanceField->GetValuesSize(); i++)
		{
			if (out_distanceField->GetHeatValueByIndex(i) == currentLowestValue)
			{
				Tile* tile = m_tiles[i];

				Tile* leftTile = GetTileBaseOnTilePosition(IntVec2(tile->m_tileCoords.x - 1, tile->m_tileCoords.y));
				Tile* rightTile = GetTileBaseOnTilePosition(IntVec2(tile->m_tileCoords.x + 1, tile->m_tileCoords.y));
				Tile* downTile = GetTileBaseOnTilePosition(IntVec2(tile->m_tileCoords.x, tile->m_tileCoords.y - 1));
				Tile* upTile = GetTileBaseOnTilePosition(IntVec2(tile->m_tileCoords.x, tile->m_tileCoords.y + 1));

				if (leftTile != nullptr)
				{
					if (leftTile->GetTileDef().m_isSolid == false)
					{
						float tileHeatValue = out_distanceField->GetHeatValueByIndex(GetTileIndex(leftTile));
						if (tileHeatValue > currentLowestValue)
						{
							out_distanceField->SetValueByIndex(GetTileIndex(leftTile), currentLowestValue + 1.0f);
							mapBeenChange = true;
						}
					}
				}

				if (rightTile != nullptr)
				{
					if (rightTile->GetTileDef().m_isSolid == false)
					{
						float tileHeatValue = out_distanceField->GetHeatValueByIndex(GetTileIndex(rightTile));
						if (tileHeatValue > currentLowestValue)
						{
							out_distanceField->SetValueByIndex(GetTileIndex(rightTile), currentLowestValue + 1.0f);
							mapBeenChange = true;
						}
					}
				}


				if (downTile != nullptr)
				{
					if (downTile->GetTileDef().m_isSolid == false)
					{
						float tileHeatValue = out_distanceField->GetHeatValueByIndex(GetTileIndex(downTile));
						if (tileHeatValue > currentLowestValue)
						{
							out_distanceField->SetValueByIndex(GetTileIndex(downTile), currentLowestValue + 1.0f);
							mapBeenChange = true;
						}
					}
				}


				if (upTile != nullptr)
				{
					if (upTile->GetTileDef().m_isSolid == false)
					{
						float tileHeatValue = out_distanceField->GetHeatValueByIndex(GetTileIndex(upTile));
						if (tileHeatValue > currentLowestValue)
						{
							out_distanceField->SetValueByIndex(GetTileIndex(upTile), currentLowestValue + 1.0f);
							mapBeenChange = true;
						}
					}
				}

			}
		}
		if (mapBeenChange)
		{
			currentLowestValue += 1.0f;
		}
	}

	return true;
}

int Map::GetTileIndex(Tile* tile)
{
	return m_dimensions.y * (tile->m_tileCoords.x) + (tile->m_tileCoords.y);
}

