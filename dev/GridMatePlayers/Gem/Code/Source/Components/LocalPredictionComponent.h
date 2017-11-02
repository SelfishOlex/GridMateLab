#pragma once
#include <AzCore/Component/Component.h>
#include "AzFramework/Network/NetBindable.h"
#include "Utils/MovementHistory.h"
#include "GridMatePlayers/LocalPredictionRequestBus.h"
#include "AzCore/Component/TickBus.h"
#include "AzCore/Component/TransformBus.h"
#include "Utils/VectorInTime.h"

namespace GridMatePlayers
{
    class LocalPredictionComponent
        : public AZ::Component
        , public AzFramework::NetBindable
        , public LocalPredictionRequestBus::Handler
        , public AZ::TransformNotificationBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(LocalPredictionComponent,
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

        void OnNewServerCheckpoint(const VectorInTime& value,
            const GridMate::TimeContext &tc);

        AZ::Vector3 GetPosition() const;
        AZ::u32 GetLocalTime() const;

    private:
        class Chunk;
        GridMate::ReplicaChunkPtr m_chunk;

        bool m_isActive = false;
        bool m_movingForward = false;
        float m_speed = 0.f;

        bool IsLocallyControlled() const;
        MovementHistory m_history{ 50 };
    };
}
