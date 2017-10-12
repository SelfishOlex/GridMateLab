#pragma once
#include "AzCore/std/containers/ring_buffer.h"
#include "AzCore/Math/Transform.h"

namespace GridMatePlayers
{
    class MovementTrack
    {
    public:
        void AddDataPoint(const AZ::Transform& t, AZ::u32 time);
        AZ::Transform GetTransformAt(AZ::u32 time);

    private:
        class DataPoint
        {
        public:
            AZ::Transform m_position;
            AZ::u32       m_time;
        };

        AZStd::ring_buffer<DataPoint> m_points;
    };
}
