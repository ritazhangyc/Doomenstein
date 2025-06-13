#include "Game/TileDefinition.hpp"
#include "Game/Tile.hpp"
#include "Game/Gamecommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"


std::map<std::string, TileDefinition>	TileDefinition::s_definitions;

void TileDefinition::InitializeTileDefs()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/TileDefinitions.xml");
	XmlElement* rootElement = doc.RootElement();
	XmlElement* childElementIterator = rootElement->FirstChildElement();

	while (childElementIterator)
	{
		NamedStrings tileDefinitionNameString = NamedStrings();
		tileDefinitionNameString.PopulateFromXmlElementAttributes(*childElementIterator);
		TileDefinition tileDefinition = TileDefinition();

		tileDefinition.m_name = tileDefinitionNameString.GetValue("name", DEFAULT_NAME);
		tileDefinition.m_isSolid = tileDefinitionNameString.GetValue("isSolid", false);
		tileDefinition.m_mapImagePixelColor = tileDefinitionNameString.GetValue("mapImagePixelColor", Rgba8::BLACK);
		tileDefinition.m_floorSpriteCoords = tileDefinitionNameString.GetValue("floorSpriteCoords", DEFAULT_SPRITE_COORDINATE);
		tileDefinition.m_ceilingSpriteCoords = tileDefinitionNameString.GetValue("ceilingSpriteCoords", DEFAULT_SPRITE_COORDINATE);
		tileDefinition.m_wallSpriteCoords = tileDefinitionNameString.GetValue("wallSpriteCoords", DEFAULT_SPRITE_COORDINATE);
		tileDefinition.m_startHeight = tileDefinitionNameString.GetValue("tileStartHeight", 0.0f);
		tileDefinition.m_tileHeight = tileDefinitionNameString.GetValue("tileHeight", 3.0f);

		if (tileDefinition.m_name != DEFAULT_NAME)
		{
			s_definitions[tileDefinition.m_name] = tileDefinition;
		}
		childElementIterator = childElementIterator->NextSiblingElement();
	}

}
