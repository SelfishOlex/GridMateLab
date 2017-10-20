#pragma once
#include <AzCore/Component/Component.h>
#include <CloseAllNetworkPeers/CloseAllNetworkPeersRequestBus.h>
#include <AzCore/Component/EntityBus.h>
#include <AzFramework/Entity/GameEntityContextBus.h>

namespace CloseAllNetworkPeers
{
    class CloseAllPeersSystemComponent
        : public AZ::Component
        , protected CloseAllNetworkPeersRequestBus::Handler
        , public AZ::EntityBus::MultiHandler
        , AzFramework::GameEntityContextEventBus::Handler
    {
    public:
        AZ_COMPONENT(CloseAllPeersSystemComponent,
            "{B34B496A-264C-4D68-90C3-BCCBBA5E6E60}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(
            AZ::ComponentDescriptor::DependencyArrayType&);
        static void GetIncompatibleServices(
            AZ::ComponentDescriptor::DependencyArrayType&);
        static void GetRequiredServices(
            AZ::ComponentDescriptor::DependencyArrayType&);
        static void GetDependentServices(
            AZ::ComponentDescriptor::DependencyArrayType&);

    protected:
        // CloseAllNetworkPeersRequestBus interface implementation
        void CloseAllNetworkPeers() override;

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // GameEntityContextEventBus
        void OnGameEntitiesStarted() override;
        void OnGameEntitiesReset() override;

    private:
        AZ::Entity* m_workerEntity = nullptr;
    };
}
