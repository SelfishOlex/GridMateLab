#pragma once
#include <AzCore/Component/Component.h>
#include "AzFramework/Network/NetBindable.h"
#include "GridMatePlayers/CharacterMovementNotificationBus.h"
#include "Utils/MovementTrack.h"
#include "GridMatePlayers/CharacterMovementRequestBus.h"
#include "AzCore/Component/TickBus.h"
#include "AzCore/Component/TransformBus.h"
#include "Utils/VectorInTime.h"

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

    protected:
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

        void OnNewCheckpoint(const VectorInTime& value,
            const GridMate::TimeContext &tc);

        AZ::Vector3 GetPosition() const;
        AZ::u32 GetTime() const;

    private:
        class Chunk;
        GridMate::ReplicaChunkPtr m_chunk;

        bool m_isActive = false;
        bool m_movingForward = false;
        float m_speed = 0.f;
        float m_allowedDeviation = 0.1f;

        MovementTrack m_movePoints;
    };
}
