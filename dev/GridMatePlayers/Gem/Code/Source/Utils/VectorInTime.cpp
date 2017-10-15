#include "StdAfx.h"
#include "VectorInTime.h"
#include <GridMate/Serialize/MathMarshal.h>
#include "GridMate/Serialize/Buffer.h"

using namespace AZ;
using namespace GridMate;
using namespace GridMatePlayers;

void VectorInTime::Marshaler::Marshal(
    GridMate::WriteBuffer& wb, const VectorInTime& value) const
{
    GridMate::Marshaler<Vector3> mv;
    mv.Marshal(wb, value.m_vector);

    GridMate::Marshaler<AZ::u32> mu32;
    mu32.Marshal(wb, value.m_time);
}

void VectorInTime::Marshaler::Unmarshal(
    VectorInTime& value, GridMate::ReadBuffer& rb) const
{
    GridMate::Marshaler<Vector3> mv;
    mv.Unmarshal(value.m_vector, rb);

    GridMate::Marshaler<AZ::u32> mu32;
    mu32.Unmarshal(value.m_time, rb);
}

bool VectorInTime::Throttler::WithinThreshold(
    const VectorInTime& cur) const
{
    if (cur.m_time != m_baseline.m_time)
        return false;

    const auto diff = m_baseline.m_vector - cur.m_vector;
    return diff.GetLengthSq() < 0.1f;
}

void VectorInTime::Throttler::UpdateBaseline(
    const VectorInTime& baseline)
{
    m_baseline = baseline;
}

VectorInTime::VectorInTime(const AZ::Vector3& v, AZ::u32 time)
    : m_vector(v), m_time(time) {}

bool VectorInTime::operator==(const VectorInTime& other) const
{
    return (m_time == other.m_time) &&
           (m_vector == other.m_vector);
}
