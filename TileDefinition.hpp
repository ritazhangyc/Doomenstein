#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include <map>


class TileDefinition
{
public:

	std::string										m_name;
	bool											m_isSolid;
	Rgba8											m_mapImagePixelColor;
	IntVec2											m_floorSpriteCoords;
	IntVec2											m_ceilingSpriteCoords;
	IntVec2											m_wallSpriteCoords;
	float											m_tileHeight;
	float											m_startHeight = 0.0f;

	static void										InitializeTileDefs();
	static std::map<std::string, TileDefinition>	s_definitions;
};