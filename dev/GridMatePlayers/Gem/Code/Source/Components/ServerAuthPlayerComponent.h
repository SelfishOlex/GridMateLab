#pragma once
#include <AzCore/Component/Component.h>
#include <GridMate/Replica/ReplicaCommon.h>
#include <AzFramework/Network/NetBindable.h>
#include <GridMatePlayers/ServerPlayerBodyBus.h>

namespace GridMatePlayers
{
    class ServerAuthPlayerComponent
        : public AZ::Component
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

        // ServerPlayerBodyBus interface
        void SetAssociatedPlayerId(
            const GridMate::MemberIDCompact& player) override;

    private:
        class Chunk;
        GridMate::ReplicaChunkPtr m_chunk;
    };
}
