#include "Game/ActorHandle.hpp"


const ActorHandle ActorHandle::INVALID = ActorHandle(MAX_ACTOR_UID, MAX_ACTOR_INDEX);

ActorHandle::ActorHandle()
{
	m_data = INVALID.m_data;
}

ActorHandle::ActorHandle(unsigned int uid, unsigned int index)
{
	unsigned int salt = (uid & MAX_ACTOR_UID);
	m_data = (salt << 16) | index & MAX_ACTOR_INDEX;
}

bool ActorHandle::IsValid() const
{
	return m_data != ActorHandle::INVALID.m_data;
}

unsigned int ActorHandle::GetIndex() const
{
	return MAX_ACTOR_INDEX & m_data;
}

bool ActorHandle::operator==(const ActorHandle& other) const
{
	return m_data == other.m_data;
}

bool ActorHandle::operator!=(const ActorHandle& other) const
{ 
	return m_data != other.m_data;
}
