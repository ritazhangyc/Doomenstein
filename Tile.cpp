#include "Game/Tile.hpp"
#include "Game/Game.hpp"
#include "Game/Gamecommon.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/VertexUtils.hpp"
// #include "Engine/Core/XmlUtils.hpp"
// #include "Engine/Core/NamedStrings.hpp"

// std::map<std::string, TileDefinition>	TileDefinition::s_definitions;

Tile::Tile(IntVec2 const& tilePosition, TileDefinition tileDef)
	:m_tileCoords(tilePosition),
	m_tileDef(tileDef),
	m_bounds(AABB3())
{
	Vec3 bottomLeftPosition = Vec3((float)m_tileCoords.x, (float)m_tileCoords.y, 0.0f);
	Vec3 topRightPosition = bottomLeftPosition + Vec3(1.0f, 1.0f, 1.0f);

	m_bounds.m_mins = bottomLeftPosition;
	m_bounds.m_maxs = topRightPosition;
}

Tile::Tile(const Tile& copyFrom)
	:m_tileCoords(copyFrom.m_tileCoords),
	m_tileDef(copyFrom.m_tileDef)
{
}

void Tile::AddVertsForTiles(std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indexes, SpriteSheet* spriteSheet)
{
	// add floor sprites
	if (m_tileDef.m_floorSpriteCoords != DEFAULT_SPRITE_COORDINATE)
	{
		AABB2 UVs = spriteSheet->GetSpriteUVsByIntPos(m_tileDef.m_floorSpriteCoords);
		AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 0.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 0.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 0.f), Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 0.f), Rgba8::WHITE, UVs);
		//AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 0.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 0.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 0.f), Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 0.f), Rgba8::WHITE, UVs);
	}

	// add ceil sprites
	if (m_tileDef.m_ceilingSpriteCoords != DEFAULT_SPRITE_COORDINATE)
	{
		AABB2 UVs = spriteSheet->GetSpriteUVsByIntPos(m_tileDef.m_ceilingSpriteCoords);
		AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 1.f), Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 1.f), Rgba8::WHITE, UVs);
		//AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 1.f), Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 1.f), Rgba8::WHITE, UVs);
	}

	// add wall sprites
	if (m_tileDef.m_wallSpriteCoords != DEFAULT_SPRITE_COORDINATE)
	{
		AABB2 UVs = spriteSheet->GetSpriteUVsByIntPos(m_tileDef.m_wallSpriteCoords);
		AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 0.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 0.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 1.f), Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 1.f), Rgba8::WHITE, UVs);
		AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 0.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 0.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 1.f), Rgba8::WHITE, UVs);
		AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 0.f), Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 0.f), Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 1.f), Rgba8::WHITE, UVs);
		AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 0.f), Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 0.f), Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 1.f), Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 1.f), Rgba8::WHITE, UVs);
	
		AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 1.f), Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 1.f), Rgba8::WHITE, UVs);

// 		AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 0.f), Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 0.f), Rgba8::WHITE, UVs);
// 		AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 1.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 0.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, 0.f), Rgba8::WHITE, UVs);
// 		AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 1.f), Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 1.f), Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 0.f), Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, 0.f), Rgba8::WHITE, UVs);
// 		AddVertsForQuad3D(vertexes, indexes, Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 1.f), Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 1.f), Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, 0.f), Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, 0.f), Rgba8::WHITE, UVs);

	
	}
}

TileDefinition Tile::GetTileDef() const
{
	return m_tileDef;
}

void Tile::SetTileDef(TileDefinition& tileDef)
{
	m_tileDef = tileDef;
}

// void TileDefinition::InitializeTileDefs()
// {
// 	XmlDocument doc;
// 	doc.LoadFile("Data/Definitions/TileDefinitions.xml");
// 	XmlElement* rootElement = doc.RootElement();
// 	XmlElement* childElementIterator = rootElement->FirstChildElement();
// 
// 	std::string	defaultName = "NO_NAME";
// 	IntVec2	defaultCoords = IntVec2(-1, -1);
// 
// 	while (childElementIterator)
// 	{
// 		NamedStrings tileDefinitionNameString = NamedStrings();
// 		tileDefinitionNameString.PopulateFromXmlElementAttributes(*childElementIterator);
// 		TileDefinition tileDefinition = TileDefinition();
// 
// 		tileDefinition.m_name = tileDefinitionNameString.GetValue("name", defaultName);
// 		tileDefinition.m_isSolid = tileDefinitionNameString.GetValue("isSolid", false);
// 		tileDefinition.m_mapImagePixelColor = tileDefinitionNameString.GetValue("mapImagePixelColor", Rgba8::BLACK);
// 		tileDefinition.m_floorSpriteCoords = tileDefinitionNameString.GetValue("floorSpriteCoords", defaultCoords);
// 		tileDefinition.m_ceilingSpriteCoords = tileDefinitionNameString.GetValue("ceilingSpriteCoords", defaultCoords);
// 		tileDefinition.m_wallSpriteCoords = tileDefinitionNameString.GetValue("wallSpriteCoords", defaultCoords);
// 
// 		if (tileDefinition.m_name != defaultName)
// 		{
// 			s_definitions[tileDefinition.m_name] = tileDefinition;
// 		}
// 		childElementIterator = childElementIterator->NextSiblingElement();
// 	}
// 
// }
