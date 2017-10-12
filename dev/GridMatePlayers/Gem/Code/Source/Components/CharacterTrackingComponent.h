#pragma once
#include <AzCore/Component/Component.h>
#include "AzFramework/Network/NetBindable.h"
#include "GridMatePlayers/CharacterMovementNotificationBus.h"
#include "Utils/MovementTrack.h"
#include "GridMatePlayers/CharacterMovementRequestBus.h"
#include "AzCore/Component/TickBus.h"

namespace GridMatePlayers
{
    class CharacterTrackingComponent
        : public AZ::Component
        , public AzFramework::NetBindable
        , public CharacterMovementNotificationBus::Handler
        , public CharacterMovementRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(CharacterTrackingComponent,
            "{03B4BDF1-930F-4955-9054-D352D4BEB5D5}",
            NetBindable);

        static void Reflect(AZ::ReflectContext* reflection);

        void Activate() override;
        void Deactivate() override;

        // NetBindable
        GridMate::ReplicaChunkPtr GetNetworkBinding() override;
        void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
            override;
        void UnbindFromNetwork() override;

        // CharacterMovementNotificationBus
        void OnCharacterMoveUpdate(const AZ::Transform& t,
            AZ::u32 timestamp) override;

        // CharacterMovementRequestBus
        void OnCharacterMoveForward(AZ::u32 time) override;
        void OnCharacterStop(AZ::u32 time) override;

        // TickBus
        void OnTick(float dt, AZ::ScriptTimePoint) override;

    private:
        class Chunk;
        GridMate::ReplicaChunkPtr m_chunk;

        MovementTrack m_movePoints;
        float m_speed = 1.f; // use from ServerPlayerControls?

        AZ::Transform GetLocalTM();
        AZ::u32 GetTime();
        bool Compare(const AZ::Transform& one,
            const AZ::Transform& another) const;

        class TransformWithTime
        {
        public:
            TransformWithTime() {}
            TransformWithTime(const AZ::Transform& t, AZ::u32 time)
                : m_transform(t), m_time(time) {}

            AZ::Transform m_transform;
            AZ::u32       m_time = 0;

            struct Marshaler
            {
                void Marshal(GridMate::WriteBuffer& wb,
                    const TransformWithTime& value) const
                {
                    wb.Write(value.m_transform);
                    wb.Write(value.m_time);
                }

                void Unmarshal(TransformWithTime& value,
                               GridMate::ReadBuffer& rb) const
                {
                    rb.Read(value.m_transform);
                    rb.Read(value.m_time);
                }
            };

            bool operator==(const TransformWithTime& another) const;
        };

        void OnNewCheckpoint(const TransformWithTime& value,
            const GridMate::TimeContext &tc);
    };
}
