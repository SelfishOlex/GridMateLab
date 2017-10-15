#pragma once
#include "AzCore/Math/Vector3.h"
#include "GridMate/Replica/DataSet.h"

namespace GridMatePlayers
{
    class VectorInTime
    {
    public:
        VectorInTime() = default;
        VectorInTime(const AZ::Vector3& v, AZ::u32 time);

        AZ::Vector3 m_vector;
        AZ::u32     m_time = 0;

        struct Marshaler;
        struct Throttler;

        bool operator==(const VectorInTime& other) const;
    };

    struct VectorInTime::Marshaler
    {
        void Marshal(GridMate::WriteBuffer& wb,
                     const VectorInTime& value) const;
        void Unmarshal(VectorInTime& value,
                       GridMate::ReadBuffer& rb) const;
    };

    struct VectorInTime::Throttler
    {
        bool WithinThreshold(const VectorInTime& cur) const;
        void UpdateBaseline(const VectorInTime& baseline);
        VectorInTime m_baseline;
    };

    using DataSetVectorInTime = GridMate::DataSet<VectorInTime,
        VectorInTime::Marshaler, VectorInTime::Throttler>;
}
