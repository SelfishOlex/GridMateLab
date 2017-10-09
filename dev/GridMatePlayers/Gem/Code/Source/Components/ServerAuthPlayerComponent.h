#pragma once
#include <AzCore/Component/Component.h>
#include <GridMatePlayers/PlayerControlsBus.h>
#include <AzCore/Component/TickBus.h>
#include <GridMate/Replica/ReplicaCommon.h>
#include <AzFramework/Network/NetBindable.h>
#include <GridMatePlayers/ServerPlayerBodyBus.h>

namespace GridMatePlayers
{
    class ServerAuthPlayerComponent
        : public AZ::Component
          , public PlayerControlsBus::Handler
          , public AZ::TickBus::Handler
          , public AzFramework::NetBindable
          , public ServerPlayerBodyBus::Handler
    {
    public:
        AZ_COMPONENT(ServerAuthPlayerComponent,
            "{3097BBE1-CCAA-46B5-B027-121A4588C036}",
            NetBindable);

        static void Reflect(AZ::ReflectContext* context);

        GridMate::ReplicaChunkPtr GetNetworkBinding() override;
        void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
        override;
        void UnbindFromNetwork() override;

    protected:
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // PlayerControlsBus interface
        void ForwardKeyUp() override;
        void ForwardKeyDown() override;
        void FireKeyUp() override;

        // AZ::TickBus interface implementation
        void OnTick(float deltaTime,
                    AZ::ScriptTimePoint time) override;

        // ServerPlayerBodyBus interface
        void SetAssociatedPlayerId(
            const GridMate::MemberIDCompact& player) override;

        // RPC callbacks
        bool OnStartForward(const GridMate::RpcContext& rc);
        bool OnStopForward(const GridMate::RpcContext& rc);
        bool OnFireCommand(const GridMate::RpcContext& rc);

        // DataSet callback
        void OnOwningPlayerChanged(
            const GridMate::MemberIDCompact& value,
            const GridMate::TimeContext& tc);

    private:
        bool m_movingForward = false;
        float m_speed = 1.f;

        class Chunk;
        GridMate::ReplicaChunkPtr m_chunk;

        void BroadcastNewBody();
        bool m_readyToConnectToBody = false;
    };
}