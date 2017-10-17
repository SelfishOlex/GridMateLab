#include "StdAfx.h"
#include "MovementHistory.h"
#include "AzCore/Casting/numeric_cast.h"

using namespace AZ;
using namespace GridMatePlayers;

MovementHistory::MovementHistory()
{
    m_points.set_capacity(50);
}

void MovementHistory::AddDataPoint(const Vector3& t, u32 time)
{
    m_points.push_back(DataPoint{ t, time });
}

Vector3 MovementHistory::GetPositionAt(u32 time)
{
    if (m_points.size() == 0)
        return Vector3::CreateZero();

    auto after = m_points[m_points.size() - 1];
    auto before = m_points[0];
    for (auto it = m_points.rbegin(); it != m_points.rend(); ++it)
    {
        if (it->m_time <= time)
        {
            before = *it;
            break;
        }

        after = *it;
    }

    auto bPos = before.m_position;
    if (after.m_time != before.m_time)
    {
        const auto portion = (time - before.m_time) /
            aznumeric_cast<float>(after.m_time - before.m_time);
        return bPos.Lerp(after.m_position, portion);
    }

    return bPos;
}

void MovementHistory::DeleteAfter(AZ::u32 time)
{
    while(m_points.size() > 0)
    {
        if (m_points.back().m_time > time)
            m_points.pop_back();
        else
            break;
    }
}

void MovementHistory::DeleteAll()
{
    m_points.clear();
}

bool MovementHistory::HasHistory() const
{
    return m_points.size() > 0;
}
