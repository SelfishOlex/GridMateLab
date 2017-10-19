#pragma once
#include <AzCore/Component/Component.h>
#include <GridMatePlayers/NetworkTimeRequestBus.h>
#include <GridMate/Session/Session.h>
#include <CrySystemBus.h>

namespace GridMatePlayers
{
    class NetworkTimeSystemComponent
        : public AZ::Component
        , protected NetworkTimeRequestBus::Handler
        , public GridMate::SessionEventBus::Handler
        , public CrySystemEventBus::Handler
    {
    public:
        AZ_COMPONENT(NetworkTimeSystemComponent,
            "{6ADBBF4F-1895-4580-BE77-166614E22B9C}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(
            AZ::ComponentDescriptor::DependencyArrayType& list)
        {
            list.push_back(AZ_CRC("NetworkTimeService"));
        }

    protected:
        // GridMatePlayersRequestBus interface implementation
        AZ::u32 GetLocalTime() override;

        // SessionEventBus
        void OnSessionHosted(
            GridMate::GridSession* session) override;
        void OnSessionJoined(
            GridMate::GridSession* session) override;
        void OnSessionDelete(
            GridMate::GridSession* session) override;

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // CrySystemEventBus
        void OnCrySystemInitialized(ISystem&,
            const SSystemInitParams&) override;
    private:
        GridMate::GridSession* m_session = nullptr;
        AZ::u32 m_serverLag = 200; // milliseconds
    };
}
