#pragma once
#include <AzCore/Component/Component.h>
#include <AzFramework/Network/NetBindable.h>
#include <AzCore/Component/TickBus.h>

namespace CloseAllNetworkPeers
{
    class CloseAllRequestInterface
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

        virtual void CloseAll() = 0;
    };

    using CloseAllRequestBus = AZ::EBus<CloseAllRequestInterface>;

    class CloseNetworkPeersComponent
        : public AZ::Component
        , public CloseAllRequestBus::Handler
        , public AzFramework::NetBindable
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(CloseNetworkPeersComponent,
            "{33C9FC43-8756-48F3-A925-C04A5BD4FF6A}",
            NetBindable);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(
            AZ::ComponentDescriptor::DependencyArrayType& list);
        static void GetIncompatibleServices(
            AZ::ComponentDescriptor::DependencyArrayType& list);
        static void GetRequiredServices(
            AZ::ComponentDescriptor::DependencyArrayType& list);
        static void GetDependentServices(
            AZ::ComponentDescriptor::DependencyArrayType& list);

    protected:
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // CloseNetworkPeersComponent
        void CloseAll() override;

        // NetBindable
        GridMate::ReplicaChunkPtr GetNetworkBinding() override;
        void SetNetworkBinding(
            GridMate::ReplicaChunkPtr chunk) override;
        void UnbindFromNetwork() override;

        bool OnCloseAll(const GridMate::RpcContext& rc);

        void OnTick(float, AZ::ScriptTimePoint time) override;

    private:
        class Chunk;
        GridMate::ReplicaChunkPtr m_chunk;

        bool m_isShuttinDown = false;
        AZ::u32 m_shutdownCountdown = 0;
    };
}
