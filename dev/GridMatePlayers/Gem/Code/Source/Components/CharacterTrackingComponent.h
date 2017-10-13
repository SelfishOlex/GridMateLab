#pragma once
#include <AzCore/Component/Component.h>
#include "AzFramework/Network/NetBindable.h"
#include "GridMatePlayers/CharacterMovementNotificationBus.h"
#include "Utils/MovementTrack.h"
#include "GridMatePlayers/CharacterMovementRequestBus.h"
#include "AzCore/Component/TickBus.h"
#include "AzCore/Component/TransformBus.h"

namespace GridMatePlayers
{
    class CharacterTrackingComponent
        : public AZ::Component
        , public AzFramework::NetBindable
        , public CharacterMovementNotificationBus::Handler
        , public CharacterMovementRequestBus::Handler
        , public AZ::TransformNotificationBus::Handler
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
        void OnCharacterMoveUpdate(const AZ::Vector3& serverPos,
            AZ::u32 time) override;

        // CharacterMovementRequestBus
        void OnCharacterMoveForward(float speed,
            AZ::u32 time) override;
        void OnCharacterStop(AZ::u32 time) override;

        // TransformNotificationBus
        void OnTransformChanged(const AZ::Transform& local,
            const AZ::Transform& world) override;

        // TickBus
        void OnTick(float deltaTime, AZ::ScriptTimePoint time)
            override;

    private:
        class Chunk;
        GridMate::ReplicaChunkPtr m_chunk;

        bool m_movingForward = false;
        float m_speed = 0.f;

        bool m_isActive = false;

        AZ::Transform GetLocalTM();
        AZ::u32 GetTime();
        bool IsClose(const AZ::Vector3& one,
            const AZ::Vector3& another) const;

        MovementTrack m_movePoints;

        class PositionInTime
        {
        public:
            PositionInTime() {}

            PositionInTime(
                const AZ::Vector3& t, AZ::u32 time)
                : m_position(t), m_time(time) {}

            AZ::Vector3 m_position;
            AZ::u32     m_time = 0;

            struct Marshaler
            {
                void Marshal(GridMate::WriteBuffer& wb,
                    const PositionInTime& value) const
                {
                    wb.Write(value.m_position);
                    wb.Write(value.m_time);
                }

                void Unmarshal(PositionInTime& value,
                    GridMate::ReadBuffer& rb) const
                {
                    rb.Read(value.m_position);
                    rb.Read(value.m_time);
                }
            };

            struct Throttler;

            bool operator==(const PositionInTime& other) const;
        };

        void OnNewCheckpoint(const PositionInTime& value,
            const GridMate::TimeContext &tc);
    };
}
