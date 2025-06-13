#pragma once
#include "Game/ActorDefinitions.hpp"
#include "Game/ActorHandle.hpp"
#include "Game/AI.hpp"
#include "Game/Weapon.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Audio/AudioSystem.hpp"


class Map;
class Controller;

class Actor
{
public:
	Actor();
	//Actor(Map* owner, Vec3 const& startPosition, EulerAngles const& startOrientation, Rgba8 startColor, float startPhysicalHeight, float startPhysicalRadius, bool isStatic);
	Actor(Map* owner, ActorDefinition actorDefinition, ActorHandle const& actorHandle);
	~Actor();

	void			StartUp();
	void			Update();
	void			UpdateAIController();
	void			UpdatePhysics();
	void			UpdateDestroyedStatus();
	void			UpdateAnimation();
	void			UpdateAudio();

	void			Render(Camera const& camera, int playerIndex);
	void			RenderOnUI() const;

	void			Damage(float damage, Actor* source);
	void			Dead(Actor* source);

	void			AddForce(const Vec3& force);
	void			AddImpulse(const Vec3& impulse);
	void			OnCollide(Actor* other);

	void			OnPossessed(Controller* controller);
	void			OnUnpossessed(Controller* controller);
	void			MoveInDirection(Vec3 direction, float speed);
	void			TurnInDirection(float degrees, float maxAngle);

	void			EquipWeapon(int weaponIndex);
	void			EquipNextWeapon();
	void			EquipPreviousWeapon();
	void			Attack();
	void			Attack(Actor* targetActor);


	void			PlayAnimation(std::string animationName);

	Vec2			GetActorPositionXY() const;
	FloatRange		GetActorZRange() const;

	ActorHandle		GetActorHandle() const;
	Controller*		GetCurrentController();
	Weapon*			GetEquippedWeapon();
	Clock*			GetGameClock() const;
	EulerAngles		GetOrientation() const;
	EulerAngles		GetOrientationOnlyYaw() const;
	Mat44			GetModelMatrix() const;
	Vec3			GetActorWeaponStartPosition() const;
	float			GetRandomDamageValue() const;

public:
	Vec3			m_position = Vec3(0.f, 0.f, 0.f);
	Vec3			m_velocity = Vec3(0.f, 0.f, 0.f);
	Vec3			m_acceleration = Vec3(0.f, 0.f, 0.f);
	EulerAngles		m_orientation;
	Rgba8			m_color;
	float			m_health;
	float			m_damageValue = -1.0f; 


	Map*			m_map;
	Timer*			m_lifetimeTimer;
	Timer*			m_animationTimer;
	AI*				m_AIController = nullptr;
	Actor*			m_owner = nullptr;
	Controller*		m_currentController = nullptr;
	RandomNumberGenerator* m_rng = nullptr;

	ActorHandle		m_actorHandle = ActorHandle::INVALID;
	ActorDefinition	m_actorDefinition;
	std::vector<Vertex_PCU>	m_vertex;
	std::vector<Weapon*>	m_weapons;
	Weapon*			m_currentWeapon;
	SoundInfo*		m_hurtSound;
	SpriteSheet*	m_playerIconSpriteSheet;


// 	float			m_physicalHeight;
// 	float			m_physicalRadius;
	bool			m_isStatic;
	bool			m_isDestroyed = false;
	bool			m_isDead = false;
	bool			m_playerPossessed = false;

	std::vector<Vertex_PCUTBN>	m_vertexes;
	std::vector<unsigned int>	m_indexes;
	std::string					m_currentAnimName = "Idle";

	float						m_hurtImpulseTimer = 0.0f;
	bool						m_isIncreasing = true;
};




