#include "Game/WeaponDefinitions.hpp"
#include "Game/Gamecommon.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Renderer/Renderer.hpp"


std::map<std::string, WeaponDefinitions> WeaponDefinitions::s_weaponDefinitions;

void WeaponDefinitions::InitializeWeaponDefinitions()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/WeaponDefinitions.xml");
	if (doc.ErrorID() != 0)
	{
		ERROR_AND_DIE("CAN NOT FIND THE WEAPON DEFINITIONS FILE");
	}
	XmlElement* rootElement = doc.RootElement();
	XmlElement* childElementIterator = rootElement->FirstChildElement();
	while (childElementIterator)
	{
		NamedStrings weaponAttributeNamedString;
		weaponAttributeNamedString.PopulateFromXmlElementAttributes(*childElementIterator);
		WeaponDefinitions weaponDefinition;

		weaponDefinition.m_weaponName = weaponAttributeNamedString.GetValue("name", DEFAULT_NAME);
		weaponDefinition.m_refireTime = weaponAttributeNamedString.GetValue("refireTime", -1.0f);
		weaponDefinition.m_rayCount = weaponAttributeNamedString.GetValue("rayCount", -1);
		weaponDefinition.m_rayCone = weaponAttributeNamedString.GetValue("rayCone", -1.0f);
		weaponDefinition.m_rayRange = weaponAttributeNamedString.GetValue("rayRange", -1.0f);
		weaponDefinition.m_rayDamage = weaponAttributeNamedString.GetValue("rayDamage", FloatRange());
		weaponDefinition.m_rayImpulse = weaponAttributeNamedString.GetValue("rayImpulse", -1.0f);
		
		weaponDefinition.m_projectileCount = weaponAttributeNamedString.GetValue("projectileCount", -1);
		weaponDefinition.m_projectileActorName = weaponAttributeNamedString.GetValue("projectileActor", DEFAULT_NAME);
		weaponDefinition.m_projectileCone = weaponAttributeNamedString.GetValue("projectileCone", -1.0f);
		weaponDefinition.m_projectileSpeed = weaponAttributeNamedString.GetValue("projectileSpeed", -1.0f);
		weaponDefinition.m_meleeCount = weaponAttributeNamedString.GetValue("meleeCount", -1);
		weaponDefinition.m_meleeArc = weaponAttributeNamedString.GetValue("meleeArc", -1.0f);
		weaponDefinition.m_meleeRange = weaponAttributeNamedString.GetValue("meleeRange", -1.0f);
		weaponDefinition.m_meleeDamage = weaponAttributeNamedString.GetValue("meleeDamage", FloatRange());
		weaponDefinition.m_meleeImpulse = weaponAttributeNamedString.GetValue("meleeImpulse", -1.0f);
		
		weaponDefinition.m_weaponMoveSpeed = weaponAttributeNamedString.GetValue("weaponMoveSpeed", 1.0f);
		weaponDefinition.m_ammoLimit = weaponAttributeNamedString.GetValue("ammoLimit", false);
		weaponDefinition.m_ammoCount = weaponAttributeNamedString.GetValue("ammoCount", -1);
		weaponDefinition.m_reloadTime = weaponAttributeNamedString.GetValue("reloadTime", -1.0f);

		XmlElement* hudElementOfChildElementIterator = childElementIterator->FirstChildElement("HUD");
		XmlElement* soundElementOfChildElementIterator = childElementIterator->FirstChildElement("Sounds");

		if (hudElementOfChildElementIterator)
		{
			NamedStrings weaponHUDNamedStrings;
			weaponHUDNamedStrings.PopulateFromXmlElementAttributes(*hudElementOfChildElementIterator);

			weaponDefinition.m_hudReticleSize = weaponHUDNamedStrings.GetValue("reticleSize", DEFAULT_SPRITE_COORDINATE);
			weaponDefinition.m_hudSpriteSize = weaponHUDNamedStrings.GetValue("spriteSize", DEFAULT_SPRITE_COORDINATE);
			weaponDefinition.m_hudSpritePivot = weaponHUDNamedStrings.GetValue("spritePivot", Vec2(-1.0f, -1.0f));

			std::string baseTextureName = weaponHUDNamedStrings.GetValue("baseTexture", DEFAULT_NAME);
			std::string reticleTextureName = weaponHUDNamedStrings.GetValue("reticleTexture", DEFAULT_NAME);
			if (baseTextureName != DEFAULT_NAME)
			{
				weaponDefinition.m_baseTexture = g_theRenderer->CreateOrGetTextureFromFile(baseTextureName.c_str());
				weaponDefinition.m_hudWeaponShaderName = weaponHUDNamedStrings.GetValue("shader", DEFAULT_NAME);
			}
			if (reticleTextureName != DEFAULT_NAME)
			{
				weaponDefinition.m_hudReticleTexture = g_theRenderer->CreateOrGetTextureFromFile(reticleTextureName.c_str());
			}

			XmlElement* animationElement = hudElementOfChildElementIterator->FirstChildElement("Animation");

			while (animationElement)
			{
				WeaponAnimation animationInfo;
				NamedStrings animationInfoNamedStrings;
				animationInfoNamedStrings.PopulateFromXmlElementAttributes(*animationElement);
				animationInfo.m_weaponAnimationName = animationInfoNamedStrings.GetValue("name", DEFAULT_NAME);

				if (animationInfo.m_weaponAnimationName != DEFAULT_NAME)
				{
					animationInfo.m_weaponAnimationShaderName = animationInfoNamedStrings.GetValue("shader", DEFAULT_NAME);
					animationInfo.m_weaponAnimationCellCount = animationInfoNamedStrings.GetValue("cellCount", DEFAULT_SPRITE_COORDINATE);
					animationInfo.m_weaponAnimationSecondsPerFrame = animationInfoNamedStrings.GetValue("secondsPerFrame", -1.0f);
					animationInfo.m_weaponAnimationStartFrameIndex = animationInfoNamedStrings.GetValue("startFrame", -1);
					animationInfo.m_weaponAnimationEndFrameIndex = animationInfoNamedStrings.GetValue("endFrame", -1);

					std::string spriteAnimationPath = animationInfoNamedStrings.GetValue("spriteSheet", DEFAULT_NAME);
					Texture* animationTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteAnimationPath.c_str());
					SpriteSheet* animationSpriteSheet = new SpriteSheet(*animationTexture, animationInfo.m_weaponAnimationCellCount);
					animationInfo.m_weaponAnimation = new SpriteAnimDefinition
					(*animationSpriteSheet,
						animationInfo.m_weaponAnimationStartFrameIndex,
						animationInfo.m_weaponAnimationEndFrameIndex,
						animationInfo.m_weaponAnimationSecondsPerFrame,
						ConvertNameToSpriteAnimbackType(animationInfoNamedStrings.GetValue("LoopMode", "Once"))
					);

					weaponDefinition.m_weaponAnimations[animationInfo.m_weaponAnimationName] = animationInfo;
				}
				animationElement = animationElement->NextSiblingElement();
			}
		}

		if (soundElementOfChildElementIterator)
		{
			XmlElement* sound = soundElementOfChildElementIterator->FirstChildElement();
			while (sound)
			{
				NamedStrings soundStrings = NamedStrings();
				soundStrings.PopulateFromXmlElementAttributes(*sound);
				weaponDefinition.m_weaponSounds[soundStrings.GetValue("sound", DEFAULT_NAME)] = soundStrings.GetValue("name", DEFAULT_NAME);
				sound = sound->NextSiblingElement();
			}
		}

		s_weaponDefinitions[weaponDefinition.m_weaponName] = weaponDefinition;
		childElementIterator = childElementIterator->NextSiblingElement();
	}
}

