#pragma once
#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
// #include "Engine/Math/IntVec2.hpp"
// #include "Engine/Core/Rgba8.hpp"
// #include "Engine/Renderer/Renderer.hpp"
// #include <map>


// class TileDefinition
// {
// public:
// 
// 	std::string										m_name;
// 	bool											m_isSolid;
// 	Rgba8											m_mapImagePixelColor;
// 	IntVec2											m_floorSpriteCoords;
// 	IntVec2											m_ceilingSpriteCoords;
// 	IntVec2											m_wallSpriteCoords;
// 
// 	static void										InitializeTileDefs();
// 	static std::map<std::string, TileDefinition>	s_definitions;
// };

class Tile
{

public:

	Tile(IntVec2 const& tilePosition, TileDefinition tileDef);
	Tile(const Tile& copyFrom);

	void			AddVertsForTiles(std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indexes, SpriteSheet* spriteSheet);
	TileDefinition 	GetTileDef() const;
	void			SetTileDef(TileDefinition& tileDef);

	AABB3			m_bounds;
	IntVec2			m_tileCoords;
	TileDefinition 	m_tileDef;
};

