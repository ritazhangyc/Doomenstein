#pragma once
#include "Game/GameCommon.hpp"
#include "Game/ActorDefinitions.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <map>


class Image;
class Shader;
class Texture;
struct IntVec2;


struct SpawnInfo
{
	std::string		m_actorName;
	Vec3			m_velocity;
	Vec3			m_position;
	EulerAngles		m_orientation;
	Faction			m_faction;
	float			m_timeOffset;
};

struct EnemyInfo
{
	int											m_roundIndex;
	std::vector<SpawnInfo>						m_enemyInfo;
};

class MapDefinition
{
public:
	std::string			m_name;
	Image*				m_image;
	Shader*				m_shader;
	Texture*			m_spriteSheetTexture;
	IntVec2				m_spriteSheetCellCount;

	std::string			m_skyBoxTexturePath;
	std::string			m_playerIconTexture;
	std::string			m_welcomeTexture;
	AABB2				m_mapBounds;

	std::vector<EnemyInfo>	m_allDemonSpawnInfo;
	std::vector<SpawnInfo>	m_allSpawnPoints;

	static void										InitializeMapDefs();
	static std::map<std::string, MapDefinition>		s_mapDefinitions;
};
