#pragma once
#include <AzCore/Component/Component.h>
#include <CloseAllNetworkPeers/CloseAllNetworkPeersRequestBus.h>
#include <AzCore/Component/EntityBus.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <GridMate/Session/Session.h>
#include <CrySystemBus.h>

namespace CloseAllNetworkPeers
{
    class CloseAllPeersSystemComponent
        : public AZ::Component
        , protected CloseAllNetworkPeersRequestBus::Handler
        , public AZ::EntityBus::MultiHandler
        , AzFramework::GameEntityContextEventBus::Handler
        , public GridMate::SessionEventBus::Handler
        , public CrySystemEventBus::Handler
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

        // SessionEventBus
        void OnSessionHosted(
            GridMate::GridSession* session) override;
        void OnSessionDelete(GridMate::GridSession* session) override;
        void OnSessionEnd(GridMate::GridSession* session) override;

        // CrySystemEventBus
        void OnCrySystemInitialized(ISystem&,
            const SSystemInitParams&) override;

        void CreateWorkerEntity();

    private:
        AZ::Entity* m_workerEntity = nullptr;
        bool m_isHost = false;
        bool m_isMapLoaded = false;
    };
}
