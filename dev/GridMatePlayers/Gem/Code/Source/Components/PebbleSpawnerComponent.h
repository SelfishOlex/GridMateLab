#pragma once
#include <AzCore/Component/Component.h>
#include <GridMatePlayers/PlayerActionsBus.h>
#include <AzFramework/Network/NetBindable.h>

namespace GridMatePlayers
{
    class PebbleSpawnerComponent
        : public AZ::Component
        , public PlayerActionsBus::Handler
        , public AzFramework::NetBindable
    {
    public:
        AZ_COMPONENT(PebbleSpawnerComponent,
            "{A80FAC35-650C-42A9-8CC0-A3F1645AA7EA}",
            NetBindable);

        static void Reflect(AZ::ReflectContext* context);

    protected:
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // PlayerActionsBus interface implementation
        void PlayerFired(
            const AZ::Vector3& source) override;

        // NetBindable interface implementation
        GridMate::ReplicaChunkPtr GetNetworkBinding()
            override;
        void SetNetworkBinding(
            GridMate::ReplicaChunkPtr chunk) override;
        void UnbindFromNetwork() override;

        bool OnPlayerFire(
            AZ::Vector3 location,
            const GridMate::RpcContext& rc);

        void SpawnPebble(const AZ::Vector3& start);

    private:
        class Chunk;
        GridMate::ReplicaChunkPtr m_chunk;
    };
}