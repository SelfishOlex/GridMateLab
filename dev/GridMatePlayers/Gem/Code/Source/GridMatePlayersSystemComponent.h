#pragma once
#include <AzCore/Component/Component.h>
#include <GridMatePlayers/GridMatePlayersBus.h>
#include <GridMate/Session/Session.h>
#include <CrySystemBus.h>

namespace GridMatePlayers
{
    class GridMatePlayersSystemComponent
        : public AZ::Component
        , protected GridMatePlayersRequestBus::Handler
        , public GridMate::SessionEventBus::Handler
        , public CrySystemEventBus::Handler
    {
    public:
        AZ_COMPONENT(GridMatePlayersSystemComponent,
            "{27902E8B-0810-4C59-976E-C76A651CDD51}");

        static void Reflect(AZ::ReflectContext* context);

    protected:
        // GridMatePlayersRequestBus interface implementation
        AZ::u32 GetLocalTime() override;

        // SessionEventBus
        void OnSessionHosted(
            GridMate::GridSession* session) override;

        void OnSessionJoined(
            GridMate::GridSession* session) override;

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // CrySystemEventBus
        void OnCrySystemInitialized(ISystem&,
            const SSystemInitParams&) override;
    private:
        GridMate::GridSession* m_session = nullptr;

        AZ::u32 m_serverBehindTime = 200;
    };
}
