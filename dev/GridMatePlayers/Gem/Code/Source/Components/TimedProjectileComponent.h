#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

namespace GridMatePlayers
{
    class TimedProjectileComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(TimedProjectileComponent,
            "{30E90E50-DD3A-4F4C-AC4C-35553740EA40}");

        static void Reflect(AZ::ReflectContext* context);

    protected:
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // AZ::TickBus interface implementation
        void OnTick(float deltaTime,
                    AZ::ScriptTimePoint time) override;

    private:
        float m_lifetime = 0.f;
        float m_maxLifetime = 3.f;
        float m_impulse = 1000.f; // over one second
    };
}