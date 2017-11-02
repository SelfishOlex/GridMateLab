#pragma once
#include "AzCore/std/containers/ring_buffer.h"
#include "AzCore/Math/Transform.h"

namespace GridMatePlayers
{
    class MovementHistory
    {
    public:
        explicit MovementHistory(AZ::u32 size);

        void AddDataPoint(const AZ::Vector3& t, AZ::u32 time);
        AZ::Vector3 GetPositionAt(AZ::u32 time);
        void DeleteAfter(AZ::u32 time);
        void DeleteAll();
        bool HasHistory() const;

    private:
        class DataPoint
        {
        public:
            DataPoint(const AZ::Vector3& position, AZ::u32 time)
                : m_position(position)
                , m_time(time) {}

            DataPoint() = default;

            AZ::Vector3 m_position;
            AZ::u32     m_time = 0;
        };

        AZStd::ring_buffer<DataPoint> m_points;
    };
}
