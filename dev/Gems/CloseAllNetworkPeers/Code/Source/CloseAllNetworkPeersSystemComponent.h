
#pragma once

#include <AzCore/Component/Component.h>
#include <CloseAllNetworkPeers/CloseAllNetworkPeersBus.h>
#include <GridMate/Session/Session.h>
#include <CrySystemBus.h>
#include <AzCore/Component/EntityBus.h>

namespace CloseAllNetworkPeers
{
    class CloseAllNetworkPeersSystemComponent
        : public AZ::Component
        , protected CloseAllNetworkPeersRequestBus::Handler
        , public GridMate::SessionEventBus::Handler
        , public CrySystemEventBus::Handler
        , public AZ::EntityBus::MultiHandler
    {
    public:
        AZ_COMPONENT(CloseAllNetworkPeersSystemComponent,
            "{B34B496A-264C-4D68-90C3-BCCBBA5E6E60}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(
            AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(
            AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(
            AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(
            AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        // CloseAllNetworkPeersRequestBus interface implementation
        void CloseAllNetworkPeers() override;

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // SessionEventBus
        void OnSessionHosted(
            GridMate::GridSession* session) override;
        void OnSessionDelete(
            GridMate::GridSession* session) override;

        // CrySystemEventBus
        void OnCrySystemInitialized(ISystem&,
            const SSystemInitParams&) override;
    private:
        GridMate::GridSession* m_session = nullptr;

        AZ::Entity* m_workerEntity = nullptr;
    };
}
