#pragma once
#include <AzCore/Component/Component.h>
#include "AzCore/Component/TickBus.h"
#include "AzFramework/Network/NetBindable.h"
#include "GridMatePlayers/PlayerControlsBus.h"

namespace GridMatePlayers
{
    class ServerPlayerControlsComponent
        : public AZ::Component
        , public PlayerControlsBus::Handler
        , public AZ::TickBus::Handler
        , public AzFramework::NetBindable
    {
    public:
        AZ_COMPONENT(ServerPlayerControlsComponent,
            "{8381b4f3-e7de-4dfc-845b-e52fd7bd2394}",
            NetBindable);

        ~ServerPlayerControlsComponent() override = default;

        static void Reflect(AZ::ReflectContext* reflection);

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // PlayerControlsBus interface
        void ForwardKeyReleased() override;
        void ForwardKeyPressed() override;
        void FireKeyReleased() override;

        // AZ::TickBus interface implementation
        void OnTick(float deltaTime,
            AZ::ScriptTimePoint time) override;

        // RPC callbacks
        bool OnStartForward(AZ::u32 time, const GridMate::RpcContext& rc);
        bool OnStopForward(AZ::u32 time, const GridMate::RpcContext& rc);
        bool OnFireCommand(const GridMate::RpcContext& rc);

        // NetBindable
        GridMate::ReplicaChunkPtr GetNetworkBinding() override;
        void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
            override;
        void UnbindFromNetwork() override;

    private:
        bool m_movingForward = false;
        float m_speed = 1.f;

        class Chunk;
        GridMate::ReplicaChunkPtr m_chunk;

        AZ::u32 GetLocalTime() const;

        class PlayerActionInTime
        {
        public:
            bool operator<(const PlayerActionInTime& right) const
            {
                // Sort with smaller values on top of priority q
                return (m_time > right.m_time);
            }

            enum class ActionType
            {
                Stop,
                MoveForward
            };

            PlayerActionInTime(ActionType action, AZ::u32 time)
                : m_action(action),
                  m_time(time) {}

            ActionType m_action;
            AZ::u32 m_time; // milliseconds
        };

        AZStd::priority_queue<PlayerActionInTime> m_futureActions;

        void PerformAction(const PlayerActionInTime& action);
    };
}
