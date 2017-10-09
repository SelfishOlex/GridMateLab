#pragma once
#include <AzCore/Component/Component.h>
#include <GridMatePlayers/PlayerControlsBus.h>
#include <AzCore/Component/TickBus.h>

namespace GridMatePlayers
{
    class PlayerControlsComponent
        : public AZ::Component
        , public PlayerControlsBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(PlayerControlsComponent,
            "{2F2D22D1-2214-4A77-B36D-B14D1DB0A75F}");

        static void Reflect(AZ::ReflectContext* context);

    protected:
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // GridMatePlayers::PlayerControlsBus
        void ForwardKeyUp() override;
        void ForwardKeyDown() override;
        void FireKeyUp() override;

        // AZ::TickBus interface implementation
        void OnTick(float deltaTime,
            AZ::ScriptTimePoint time) override;

    private:
        bool m_movingForward = false;
        float m_movingSpeedPerSecond = 1.f;
    };
}