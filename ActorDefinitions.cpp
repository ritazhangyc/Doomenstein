#include "Game/ActorDefinitions.hpp"
#include "Game/Gamecommon.hpp"
#include "Engine/Renderer/Renderer.hpp"

std::map<std::string, ActorDefinition> ActorDefinition::s_actorDefinitions;

void ActorDefinition::InitializeActorDefs()
 {
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/ActorDefinitions.xml");
	if (doc.ErrorID() != 0)
	{
		ERROR_AND_DIE("CAN NOT FIND THE ACTOR DEFINITIONS FILE");
	}
	XmlElement* rootElement = doc.RootElement();
	XmlElement* childElementIterator = rootElement->FirstChildElement();

	while (childElementIterator)
	{
		NamedStrings actorDefinitionNameString = NamedStrings();
		actorDefinitionNameString.PopulateFromXmlElementAttributes(*childElementIterator);
		ActorDefinition actorDefinition;
		actorDefinition.m_actorName = actorDefinitionNameString.GetValue("name", DEFAULT_NAME);
		
		if (actorDefinitionNameString.GetValue("faction", DEFAULT_NAME) == "Marine")
		{
			actorDefinition.m_actorFaction = Faction::FACTION_MARINE;
		}
		else if (actorDefinitionNameString.GetValue("faction", DEFAULT_NAME) == "Demon")
		{
			actorDefinition.m_actorFaction = Faction::FACTION_DEMON;
		}
		else if (actorDefinition.m_staticObject)
		{
			actorDefinition.m_actorFaction = Faction::FACTION_STATICOBJECT;
		}
		else
		{
			actorDefinition.m_actorFaction = Faction::FACTION_NEUTRAL;
		}

		actorDefinition.m_actorHealth = actorDefinitionNameString.GetValue("health", -1.0f);
		actorDefinition.m_canBePossessed = actorDefinitionNameString.GetValue("canBePossessed", false);
		actorDefinition.m_corpseLifetime = actorDefinitionNameString.GetValue("corpseLifetime", -100.0f);
		actorDefinition.m_visible = actorDefinitionNameString.GetValue("visible", false);
		actorDefinition.m_dieOnSpawn = actorDefinitionNameString.GetValue("dieOnSpawn", false);
		actorDefinition.m_gravitySimulated = actorDefinitionNameString.GetValue("gravityEnabled", false);
		actorDefinition.m_staticObject = actorDefinitionNameString.GetValue("staticObject", false);

		XmlElement* collisionElementOfChildElementIterator = childElementIterator->FirstChildElement("Collision");
		XmlElement* physicsElementOfChildElementIterator = childElementIterator->FirstChildElement("Physics");
		XmlElement* cameraElementOfChildElementIterator = childElementIterator->FirstChildElement("Camera");
		XmlElement* aiElementOfChildElementIterator = childElementIterator->FirstChildElement("AI");
		XmlElement* visualsElementOfChildElementIterator = childElementIterator->FirstChildElement("Visuals");
		XmlElement* soundsElementOfChildElementIterator = childElementIterator->FirstChildElement("Sounds");
		XmlElement* inventoryElementOfChildElementIterator = childElementIterator->FirstChildElement("Inventory");

		if (collisionElementOfChildElementIterator)
		{
			NamedStrings collisionDefNamedStrings = NamedStrings();
			collisionDefNamedStrings.PopulateFromXmlElementAttributes(*collisionElementOfChildElementIterator);

			actorDefinition.m_collisionRadius = collisionDefNamedStrings.GetValue("radius", -100.0f);
			actorDefinition.m_collisionHeight = collisionDefNamedStrings.GetValue("height", -100.0f);
			actorDefinition.m_collisionCollidesWithWorld = collisionDefNamedStrings.GetValue("collidesWithWorld", false);
			actorDefinition.m_collisionCollidesWithActor = collisionDefNamedStrings.GetValue("collidesWithActors", false);

			actorDefinition.m_nocollisionHeightRange = collisionDefNamedStrings.GetValue("nocollisionHeightRange", FloatRange());
		}

		if (physicsElementOfChildElementIterator)
		{
			NamedStrings physicsDefNamedStrings = NamedStrings();
			physicsDefNamedStrings.PopulateFromXmlElementAttributes(*physicsElementOfChildElementIterator);
			actorDefinition.m_physicsSimulated = physicsDefNamedStrings.GetValue("simulated", false);
			actorDefinition.m_physicsWalkSpeed = physicsDefNamedStrings.GetValue("walkSpeed", -100.0f);
			actorDefinition.m_physicsRunSpeed = physicsDefNamedStrings.GetValue("runSpeed", -100.0f);
			actorDefinition.m_physicsTurnSpeed = physicsDefNamedStrings.GetValue("turnSpeed", -100.0f);
			actorDefinition.m_physicsFlying = physicsDefNamedStrings.GetValue("flying", false);
			actorDefinition.m_physicsDrag = physicsDefNamedStrings.GetValue("drag", -100.0f);
		}

		if (cameraElementOfChildElementIterator)
		{
			NamedStrings cameraDefNamedStrings = NamedStrings();
			cameraDefNamedStrings.PopulateFromXmlElementAttributes(*cameraElementOfChildElementIterator);
			actorDefinition.m_cameraEyeHeight = cameraDefNamedStrings.GetValue("eyeHeight", -100.0f);
			actorDefinition.m_cameraFOV = cameraDefNamedStrings.GetValue("cameraFOV", -100.0f);
		}

		if (aiElementOfChildElementIterator)
		{
			NamedStrings aiDefNamedStrings = NamedStrings();
			aiDefNamedStrings.PopulateFromXmlElementAttributes(*aiElementOfChildElementIterator);
			actorDefinition.m_aiEnabled = aiDefNamedStrings.GetValue("aiEnabled", false);
			actorDefinition.m_aiSightAngle = aiDefNamedStrings.GetValue("sightRadius", -100.0f);
			actorDefinition.m_aiSightRadius = aiDefNamedStrings.GetValue("sightAngle", -100.0f);
		}

		if (inventoryElementOfChildElementIterator)
		{
			XmlElement* childIterator = inventoryElementOfChildElementIterator->FirstChildElement();
			while (childIterator)
			{
				NamedStrings weaponDefNamedStrings = NamedStrings();
				weaponDefNamedStrings.PopulateFromXmlElementAttributes(*childIterator);
				actorDefinition.m_inventoryWeaponNames.push_back(weaponDefNamedStrings.GetValue("name", DEFAULT_NAME));
				childIterator = childIterator->NextSiblingElement();
			}
		}

		if (visualsElementOfChildElementIterator)
		{
			NamedStrings visualDefNamedStrings = NamedStrings();
			visualDefNamedStrings.PopulateFromXmlElementAttributes(*visualsElementOfChildElementIterator);

			actorDefinition.m_visualSize = visualDefNamedStrings.GetValue("size", Vec2(0.f, 0.f));
			actorDefinition.m_visualPivot = visualDefNamedStrings.GetValue("pivot", Vec2(0.f, 0.f));
			actorDefinition.m_visualLit = visualDefNamedStrings.GetValue("renderLit", false);
			actorDefinition.m_visualRenderGrounded = visualDefNamedStrings.GetValue("renderRounded", false);
			actorDefinition.m_visualShaderPath = visualDefNamedStrings.GetValue("shader", DEFAULT_NAME);
			actorDefinition.m_visualCellCount = visualDefNamedStrings.GetValue("cellCount", DEFAULT_SPRITE_COORDINATE);

			std::string visualSpriteSheetPath = visualDefNamedStrings.GetValue("spriteSheet", DEFAULT_NAME);
			std::string visualBillboardType = visualDefNamedStrings.GetValue("billboardType", DEFAULT_NAME);
// 			Texture* spriteTexture = g_theRenderer->CreateOrGetTextureFromFile(visualSpriteSheetPath.c_str());

			actorDefinition.m_visualBillboardType = ConvertNameToBillboardType(visualBillboardType);
// 			actorDefinition.m_visualSpriteSheet = new SpriteSheet(*spriteTexture, actorDefinition.m_visualCellCount);

			if (visualSpriteSheetPath != DEFAULT_NAME)
			{
				Texture* spriteTexture = g_theRenderer->CreateOrGetTextureFromFile(visualSpriteSheetPath.c_str());
				actorDefinition.m_visualSpriteSheet = new SpriteSheet(*spriteTexture, actorDefinition.m_visualCellCount);
				XmlElement* childIterator = visualsElementOfChildElementIterator->FirstChildElement();
				while (childIterator)
				{
					NamedStrings AnimationDefNamedStrings = NamedStrings();
					AnimationDefNamedStrings.PopulateFromXmlElementAttributes(*childIterator);
					ActorAnimation animationInfo;
					animationInfo.m_animationName = AnimationDefNamedStrings.GetValue("name", DEFAULT_NAME);
					animationInfo.m_animationScaledBySpeed = AnimationDefNamedStrings.GetValue("scaleBySpeed", false);
					animationInfo.m_animationSecondPerFrame = AnimationDefNamedStrings.GetValue("secondsPerFrame", -1.0f);
					animationInfo.m_animationPlaybackType = ConvertNameToSpriteAnimbackType(AnimationDefNamedStrings.GetValue("playbackMode", DEFAULT_NAME));

					XmlElement* directionIterator = childIterator->FirstChildElement();
					while (directionIterator)
					{
						NamedStrings directionNamedStrings = NamedStrings();
						directionNamedStrings.PopulateFromXmlElementAttributes(*directionIterator);
						DirectionAnimation anim = DirectionAnimation();
						anim.m_lookDirection = directionNamedStrings.GetValue("vector", DEFAULT_SPAWN_POSITION);
						NamedStrings animationGroup = NamedStrings();
						XmlElement* animationNode = directionIterator->FirstChildElement("Animation");
						animationGroup.PopulateFromXmlElementAttributes(*animationNode);

						int startFrameIndex = animationGroup.GetValue("startFrame", -1);
						int endFrameIndex = animationGroup.GetValue("endFrame", -1);

						anim.m_spriteAnimation = new SpriteAnimDefinition(*actorDefinition.m_visualSpriteSheet, startFrameIndex, endFrameIndex, animationInfo.m_animationSecondPerFrame, animationInfo.m_animationPlaybackType);
						animationInfo.m_animationDirectionals.push_back(anim);
						directionIterator = directionIterator->NextSiblingElement();
					}
					actorDefinition.m_actorAnimations.push_back(animationInfo);
					childIterator = childIterator->NextSiblingElement();
				}
			}
// 			XmlElement* childIterator = visualsElementOfChildElementIterator->FirstChildElement();
// 			while (childIterator)
// 			{
// 				NamedStrings AnimationDefNamedStrings = NamedStrings();
// 				AnimationDefNamedStrings.PopulateFromXmlElementAttributes(*childIterator);
// 
// 				ActorAnimation animationInfo;
// 
// 				animationInfo.m_animationName = AnimationDefNamedStrings.GetValue("name", DEFAULT_NAME);
// 				animationInfo.m_animationScaledBySpeed = AnimationDefNamedStrings.GetValue("scaleBySpeed", false);
// 				animationInfo.m_animationSecondPerFrame = AnimationDefNamedStrings.GetValue("secondsPerFrame", -1.0f);
// 				animationInfo.m_animationPlaybackType = ConvertNameToSpriteAnimbackType(AnimationDefNamedStrings.GetValue("playbackMode", DEFAULT_NAME));
// 
// 				XmlElement* directionIterator = childIterator->FirstChildElement();
// 				while (directionIterator)
// 				{
// 					NamedStrings directionNamedStrings = NamedStrings();
// 					directionNamedStrings.PopulateFromXmlElementAttributes(*directionIterator);
// 
// 					DirectionAnimation anim = DirectionAnimation();
// 
// 					anim.m_lookDirection = directionNamedStrings.GetValue("vector", DEFAULT_SPAWN_POSITION);
// 
// 					NamedStrings animationGroup = NamedStrings();
// 					XmlElement* animationNode = directionIterator->FirstChildElement("Animation");
// 					animationGroup.PopulateFromXmlElementAttributes(*animationNode);
// 
// 					int startFrameIndex = animationGroup.GetValue("startFrame", -1);
// 					int endFrameIndex = animationGroup.GetValue("endFrame", -1);
// 
// 					anim.m_spriteAnimation = new SpriteAnimDefinition(*actorDefinition.m_visualSpriteSheet, startFrameIndex, endFrameIndex, animationInfo.m_animationSecondPerFrame, animationInfo.m_animationPlaybackType);
// 
// 					animationInfo.m_animationDirectionals.push_back(anim);
// 
// 					directionIterator = directionIterator->NextSiblingElement();
// 				}
// 				actorDefinition.m_actorAnimations.push_back(animationInfo);
// 				childIterator = childIterator->NextSiblingElement();
// 			}
		}

		if (soundsElementOfChildElementIterator)
		{
			XmlElement* sound = soundsElementOfChildElementIterator->FirstChildElement();
			while (sound)
			{
				NamedStrings soundStrings = NamedStrings();
				soundStrings.PopulateFromXmlElementAttributes(*sound);
				actorDefinition.m_soundsMap[soundStrings.GetValue("sound", DEFAULT_NAME)] = soundStrings.GetValue("name", DEFAULT_NAME);
				sound = sound->NextSiblingElement();
			}
		}

		s_actorDefinitions[actorDefinition.m_actorName] = actorDefinition;
		childElementIterator = childElementIterator->NextSiblingElement();
	}
}

void ActorDefinition::IntializeProjectileActorDefs()
{
	XmlDocument doc;
	doc.LoadFile("Data/Definitions/ProjectileActorDefinitions.xml");
	if (doc.ErrorID() != 0)
	{
		ERROR_AND_DIE("CAN NOT FIND THE PROJECTILE ACTOR DEFINITIONS FILE");
	}
	XmlElement* rootElement = doc.RootElement();
	XmlElement* childElementIterator = rootElement->FirstChildElement();

	while (childElementIterator)
	{
		NamedStrings actorDefinitionNameString = NamedStrings();
		actorDefinitionNameString.PopulateFromXmlElementAttributes(*childElementIterator);
		ActorDefinition actorDefinition;

		actorDefinition.m_actorName = actorDefinitionNameString.GetValue("name", DEFAULT_NAME);
		actorDefinition.m_canBePossessed = actorDefinitionNameString.GetValue("canBePossessed", false);
		actorDefinition.m_corpseLifetime = actorDefinitionNameString.GetValue("corpseLifetime", -100.0f);
		actorDefinition.m_visible = actorDefinitionNameString.GetValue("visible", false);
		actorDefinition.m_gravitySimulated = actorDefinitionNameString.GetValue("gravityEnabled", false);
		
		XmlElement* childIteratorCollision = childElementIterator->FirstChildElement("Collision");
		XmlElement* childIteratorPhysic = childElementIterator->FirstChildElement("Physics");
		XmlElement* childIteratorVisual = childElementIterator->FirstChildElement("Visuals");

		if (childIteratorCollision)
		{
			NamedStrings collisionDefNamedStrings = NamedStrings();
			collisionDefNamedStrings.PopulateFromXmlElementAttributes(*childIteratorCollision);

			actorDefinition.m_collisionRadius = collisionDefNamedStrings.GetValue("radius", -100.0f);
			actorDefinition.m_collisionHeight = collisionDefNamedStrings.GetValue("height", -100.0f);
			actorDefinition.m_collisionCollidesWithWorld = collisionDefNamedStrings.GetValue("collidesWithWorld", false);
			actorDefinition.m_collisionCollidesWithActor = collisionDefNamedStrings.GetValue("collidesWithActors", false);
			actorDefinition.m_damageOnCollide = collisionDefNamedStrings.GetValue("damageOnCollide", FloatRange());
			actorDefinition.m_impulseOnCollide = collisionDefNamedStrings.GetValue("impulseOnCollide", -100.0f);
			actorDefinition.m_dieOnCollide = collisionDefNamedStrings.GetValue("collidesWithActors", false);
		}

		if (childIteratorPhysic)
		{
			NamedStrings physicsDefNamedStrings = NamedStrings();
			physicsDefNamedStrings.PopulateFromXmlElementAttributes(*childIteratorPhysic);

			actorDefinition.m_physicsSimulated = physicsDefNamedStrings.GetValue("simulated", false);
			actorDefinition.m_physicsTurnSpeed = physicsDefNamedStrings.GetValue("turnSpeed", -100.0f);
			actorDefinition.m_physicsDrag = physicsDefNamedStrings.GetValue("drag", -100.0f);
			actorDefinition.m_physicsFlying = physicsDefNamedStrings.GetValue("flying", false);
		}

		if (childIteratorVisual)
		{
			NamedStrings visualDefNamedStrings = NamedStrings();
			visualDefNamedStrings.PopulateFromXmlElementAttributes(*childIteratorVisual);

			actorDefinition.m_visualSize = visualDefNamedStrings.GetValue("size", Vec2(0.f, 0.f));
			actorDefinition.m_visualPivot = visualDefNamedStrings.GetValue("pivot", Vec2(0.f, 0.f));
			actorDefinition.m_visualBillboardType = ConvertNameToBillboardType(visualDefNamedStrings.GetValue("billboardType", DEFAULT_NAME));
			actorDefinition.m_visualLit = visualDefNamedStrings.GetValue("renderLit", false);
			actorDefinition.m_visualShaderPath = visualDefNamedStrings.GetValue("shader", DEFAULT_NAME);
			actorDefinition.m_visualCellCount = visualDefNamedStrings.GetValue("cellCount", DEFAULT_SPRITE_COORDINATE);

			Texture* spriteTexture = g_theRenderer->CreateOrGetTextureFromFile(visualDefNamedStrings.GetValue("spriteSheet", DEFAULT_NAME).c_str());
			actorDefinition.m_visualSpriteSheet = new SpriteSheet(*spriteTexture, actorDefinition.m_visualCellCount);
			XmlElement* animationIterator = childIteratorVisual->FirstChildElement();

			while (animationIterator)
			{
				NamedStrings animationDefNamedStrings = NamedStrings();
				animationDefNamedStrings.PopulateFromXmlElementAttributes(*animationIterator);
				ActorAnimation animInfo;

				animInfo.m_animationName = animationDefNamedStrings.GetValue("name", DEFAULT_NAME);
				animInfo.m_animationSecondPerFrame = animationDefNamedStrings.GetValue("secondsPerFrame", -1.0f);
				animInfo.m_animationScaledBySpeed = animationDefNamedStrings.GetValue("scaleBySpeed", false);
				animInfo.m_animationPlaybackType = ConvertNameToSpriteAnimbackType(animationDefNamedStrings.GetValue("playbackMode", DEFAULT_NAME));
				XmlElement* directionIterator = animationIterator->FirstChildElement();
				while (directionIterator)
				{
					NamedStrings directionNamedStrings = NamedStrings();
					directionNamedStrings.PopulateFromXmlElementAttributes(*directionIterator);
					DirectionAnimation anim = DirectionAnimation();
					anim.m_lookDirection = directionNamedStrings.GetValue("vector", DEFAULT_SPAWN_POSITION);
					NamedStrings animationGroup = NamedStrings();
					XmlElement* animationNode = directionIterator->FirstChildElement("Animation");
					animationGroup.PopulateFromXmlElementAttributes(*animationNode);

					int startFrameIndex = animationGroup.GetValue("startFrame", -1);
					int endFrameIndex = animationGroup.GetValue("endFrame", -1);

					anim.m_spriteAnimation = new SpriteAnimDefinition(*actorDefinition.m_visualSpriteSheet, startFrameIndex, endFrameIndex, animInfo.m_animationSecondPerFrame, animInfo.m_animationPlaybackType);
					animInfo.m_animationDirectionals.push_back(anim);
					directionIterator = directionIterator->NextSiblingElement();
				}
				actorDefinition.m_actorAnimations.push_back(animInfo);
				animationIterator = animationIterator->NextSiblingElement();
			}
		}

		s_actorDefinitions[actorDefinition.m_actorName] = actorDefinition;
		childElementIterator = childElementIterator->NextSiblingElement();
	}
}

