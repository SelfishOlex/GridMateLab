#pragma once
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <AzFramework/Network/NetBindable.h>

namespace SimplestCPlusPlus
{
    class NetSyncComponent
        : public AZ::Component
        , public AZ::TransformNotificationBus::Handler
        , public AzFramework::NetBindable
    {
    public:
        AZ_COMPONENT(NetSyncComponent,
            "{0EC069DF-A5B1-4EC7-8779-59AD3D24C005}",
            NetBindable);

        static void Reflect(AZ::ReflectContext* context);

        // AZ::Component interface implementation
        void Init() override {}
        void Activate() override;
        void Deactivate() override;

        // Listen for changes on master
        void OnTransformChanged(
            const AZ::Transform& local,
            const AZ::Transform& world) override;

        // Update transform on client
        void OnNewTransform(
            const AZ::Vector3 &v,
            const GridMate::TimeContext &tc);

        // GridMate::ReplicaChunk implementation
        GridMate::ReplicaChunkPtr GetNetworkBinding() override;
        void SetNetworkBinding(
            GridMate::ReplicaChunkPtr chunk) override;
        void UnbindFromNetwork() override;

    private:
        bool m_isAuthoritative = false;

        class Chunk;
        GridMate::ReplicaChunkPtr m_chunk;
    };
}