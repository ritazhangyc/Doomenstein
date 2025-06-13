#pragma once
#include "Game/MapDefinition.hpp"
#include "Game/ActorHandle.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/TileHeatMap.hpp"

class Tile;
class Game;
class Actor;
class Player;

class Map
{
public:

	Map(MapDefinition const& mapdefition, Game* game);
	~Map();

	void						StartUp();
	void						SetSkyBox();
	void						StartUpTiles();
	void						InitializeActor();
	void						StartUpDialogTexts();


	void						Update();
	void						HandleKeyBoardInput();
	void						UpdateActors();
	void						CheckCollisionActorWithEachOther();
	void						CheckCollisionActorWithWorld();
	void						DeleteDestroyedActors();
	void						SpawnPlayers();
	void						SpawnActorForPlayer(Player* player);
// 	void						SpawnActorBesidesPlayer();

	RaycastResult3D				RaycastVsAll(Vec3 const& startPosition, Vec3 const& rayDirection, float maxDistance, ActorHandle ownerHandle, ActorHandle& hitActorHandle);
	RaycastResult3D				RaycastVsAllActors(Vec3 const& startPosition, Vec3 const& rayDirection, float maxDistance, ActorHandle ownerHandle, ActorHandle& hitActorHandle);
	RaycastResult3D				RaycastVsWorldGrids(Vec3 const& startPosition, Vec3 const& rayDirection, float maxDistance);
	RaycastResult3D				RaycastVsWorldFloorAndCeiling(Vec3 const& startPosition, Vec3 const& rayDirection, float maxDistance);

	void						Render(Camera const& camera, int playerIndex = 0) const;
	void						RenderActors(Camera const& camera, int playerIndex = 0) const;
	void						RenderMapUIStuff() const;

	IntVec2						GetTileCoordsXYBaseOnPosition(Vec3 const& position);
	Tile*						GetTileBaseOnPosition(Vec3 const& position);
	Tile*						GetTileBaseOnTilePosition(IntVec2 const& tilePos);
	bool						IsTileSolid(int tileCoordX, int tileCoordY);
	bool						IsPositionInsideSolidTile(Vec3 const& position);
	bool						IsPositionInsideSolidTile(IntVec2 const& tileCoords, float zValue);
	bool						IsPositionInsideWorldBounds(Vec3 const& position);
	bool						IsPositionInsideBound(Vec3 const& position);
	bool						IsPlayerActorAccessable(Vec3 const& targetPosition, Vec3 const& actorPosition);

	std::vector<Tile*>			GetAllTilesAround(Vec3 const& position);

	void						UpdateControlMode(Vec3 const& velocity);
	void						DebugPossessNext();
	void						PossessNextActor();
	void						RespawnPlayerActor(Player* playerController);

	Actor*						SpawnActor(const SpawnInfo& spawnInfo);
	Actor*						GetActorByHandle(const ActorHandle& handle) const;
	Actor*						GetClosestVisibleEnemy(Actor* currentActor);
	Game*						GetGameInstance() const;

	void						LoadNextUIText();

	bool						PopulateDistanceField(TileHeatMap* out_distanceField, IntVec2 startCoords, float maxConst = 999.0f);
	int							GetTileIndex(Tile* tile);


private:
	Game*						m_game;
	VertexBuffer*				m_vertexBuffer;
	IndexBuffer*				m_indexBuffer;
	SpriteSheet*				m_spriteSheet;
	MapDefinition 				m_mapDefinition;
	std::vector<Tile*>			m_tiles;
	IntVec2						m_dimensions;
	std::vector<Vertex_PCUTBN>	m_vertexes;
	std::vector<unsigned int>	m_indexes;
	Shader*						m_shader;
	Vec3						m_sunDirection = Vec3(2.0f, 1.0f, -1.0f);
	float						m_sunIntensity = 0.85f;
	float						m_ambientIntensity = 0.35f;
	std::vector<Actor*>			m_actors;
	unsigned int				m_actorSalt = 0x00000000;

	std::vector<Vertex_PCUTBN>	m_skyboxVertes;
	std::vector<unsigned int>	m_skyBoxIndexes;
	TileHeatMap*				m_heatMap;
	bool						m_showUI = true;
	bool						m_showRoundImage = false;

	Timer*						m_mapDialogTimer;
	Timer*						m_enemySpawnTimer;

	float						m_lastFrameEnemySpawnTimer = 0.0f;
	std::vector<std::string>	m_mapDialogText;
	std::vector<Actor*>			m_roundEnemies;
	int							m_currentRoundIndex = 0;
	int							m_dialogIndex;
	bool						m_isend = false;
};




