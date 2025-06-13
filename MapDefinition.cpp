#include "Game/MapDefinition.hpp"
#include "Game/Gamecommon.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <map>

std::map<std::string, MapDefinition> MapDefinition::s_mapDefinitions;

void MapDefinition::InitializeMapDefs()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/MapDefinitions.xml");
	if (doc.ErrorID() != 0)
	{
		ERROR_AND_DIE("CAN NOT FIND THE MAP DEFINITIONS FILE");
	}
	XmlElement* rootElement = doc.RootElement();
	XmlElement* childElementIterator = rootElement->FirstChildElement();

	while (childElementIterator)
	{
		NamedStrings mapDefinitionNameString = NamedStrings();
		mapDefinitionNameString.PopulateFromXmlElementAttributes(*childElementIterator);
		MapDefinition mapDefinition = MapDefinition();
		mapDefinition.m_name = mapDefinitionNameString.GetValue("name", DEFAULT_NAME);
		mapDefinition.m_mapBounds = AABB2();
		mapDefinition.m_mapBounds.m_mins = mapDefinitionNameString.GetValue("minBounds", Vec2(-1.0f, -1.0f));
		mapDefinition.m_mapBounds.m_maxs = mapDefinitionNameString.GetValue("maxBounds", Vec2(-1.0f, -1.0f));

		if (mapDefinition.m_name != DEFAULT_NAME)
		{
			mapDefinition.m_skyBoxTexturePath = mapDefinitionNameString.GetValue("skyBoxTexture", DEFAULT_NAME);
			std::string imagePath = mapDefinitionNameString.GetValue("image", DEFAULT_NAME);
			mapDefinition.m_image = new Image(imagePath.c_str());
			std::string shaderPath = mapDefinitionNameString.GetValue("shader", DEFAULT_NAME);
			mapDefinition.m_shader = g_theRenderer->CreateShader(shaderPath.c_str(), VertexType::Vertex_PCUTBN);
			std::string	spriteSheetTexture = mapDefinitionNameString.GetValue("spriteSheetTexture", DEFAULT_NAME);
			mapDefinition.m_spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetTexture.c_str());
			mapDefinition.m_spriteSheetCellCount = mapDefinitionNameString.GetValue("spriteSheetCellCount", DEFAULT_SPRITE_COORDINATE);

// 			XmlElement* spawnInfoLevelChildren = childElementIterator->FirstChildElement()->FirstChildElement();
// 			mapDefinition.m_allDemonSpawnInfo.reserve(10);
// 			mapDefinition.m_allSpawnPoints.reserve(10);
// 			while (spawnInfoLevelChildren)
// 			{
// 				NamedStrings spawnInfoString = NamedStrings();
// 				spawnInfoString.PopulateFromXmlElementAttributes(*spawnInfoLevelChildren);
// 
// 				SpawnInfo info = SpawnInfo();
// 				info.m_actorName = spawnInfoString.GetValue("actor", DEFAULT_NAME);
// 				info.m_position = spawnInfoString.GetValue("position", DEFAULT_SPAWN_POSITION);
// 				info.m_orientation = spawnInfoString.GetValue("orientation", DEFAULT_ORIENTATION);
// 				if (info.m_actorName == "SpawnPoint")
// 				{
// 					mapDefinition.m_allSpawnPoints.push_back(info);
// 				}
// 				else if (info.m_actorName == "Demon")
// 				{
// 					mapDefinition.m_allDemonSpawnInfo.push_back(info);
// 				}
// 				spawnInfoLevelChildren = spawnInfoLevelChildren->NextSiblingElement();
// 			}b

			XmlElement* mapUIInfo = childElementIterator->FirstChildElement("MapUI");

			if (mapUIInfo)
			{
				NamedStrings mapUIInfoString = NamedStrings();
				mapUIInfoString.PopulateFromXmlElementAttributes(*mapUIInfo);
				mapDefinition.m_playerIconTexture = mapUIInfoString.GetValue("missionIcon", DEFAULT_NAME);
				mapDefinition.m_welcomeTexture = mapUIInfoString.GetValue("welcomePoster", DEFAULT_NAME);
			}

			XmlElement* spawnInfoLevelChildren = childElementIterator->FirstChildElement("SpawnInfos")->FirstChildElement("SpawnInfo");
			mapDefinition.m_allSpawnPoints.reserve(10);

			while (spawnInfoLevelChildren)
			{
				NamedStrings spawnInfoString = NamedStrings();
				spawnInfoString.PopulateFromXmlElementAttributes(*spawnInfoLevelChildren);

				SpawnInfo info = SpawnInfo();

				info.m_actorName = spawnInfoString.GetValue("actor", DEFAULT_NAME);
				info.m_position = spawnInfoString.GetValue("position", DEFAULT_SPAWN_POSITION);
				info.m_orientation = spawnInfoString.GetValue("orientation", DEFAULT_ORIENTATION);
				if (info.m_actorName == "SpawnPoint")
				{
					mapDefinition.m_allSpawnPoints.push_back(info);
				}
				spawnInfoLevelChildren = spawnInfoLevelChildren->NextSiblingElement();
			}

			XmlElement* enemySpawnInfos = childElementIterator->FirstChildElement("EnemyInfos");

			if (enemySpawnInfos)
			{
				XmlElement* enemyInfo = enemySpawnInfos->FirstChildElement();

				while (enemyInfo)
				{
					NamedStrings enemyInfoString = NamedStrings();
					enemyInfoString.PopulateFromXmlElementAttributes(*enemyInfo);

					EnemyInfo info = EnemyInfo();
					info.m_roundIndex = enemyInfoString.GetValue("roundIndex", -1);

					XmlElement* enemySpawnInfo = enemyInfo->FirstChildElement("SpawnInfo");
					while (enemySpawnInfo)
					{
						NamedStrings spawnInfoString = NamedStrings();
						spawnInfoString.PopulateFromXmlElementAttributes(*enemySpawnInfo);

						SpawnInfo sinfo = SpawnInfo();
						sinfo.m_actorName = spawnInfoString.GetValue("actor", DEFAULT_NAME);
						sinfo.m_position = spawnInfoString.GetValue("position", DEFAULT_SPAWN_POSITION);
						sinfo.m_orientation = spawnInfoString.GetValue("orientation", DEFAULT_ORIENTATION);
						sinfo.m_timeOffset = spawnInfoString.GetValue("TimeOffset", -1.0f);

						info.m_enemyInfo.push_back(sinfo);

						enemySpawnInfo = enemySpawnInfo->NextSiblingElement();
					}

					mapDefinition.m_allDemonSpawnInfo.push_back(info);
					enemyInfo = enemyInfo->NextSiblingElement();
				}
			}

			s_mapDefinitions[mapDefinition.m_name] = mapDefinition;
		}
		childElementIterator = childElementIterator->NextSiblingElement();
	}
}
