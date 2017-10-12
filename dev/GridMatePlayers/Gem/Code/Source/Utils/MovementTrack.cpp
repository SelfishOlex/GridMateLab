#include "StdAfx.h"
#include "MovementTrack.h"

using namespace GridMatePlayers;

void MovementTrack::AddDataPoint(const AZ::Transform& t,
    AZ::u32 time)
{
}

AZ::Transform MovementTrack::GetTransformAt(AZ::u32 time)
{
    return AZ::Transform::CreateIdentity();
}
