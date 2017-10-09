#pragma once
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

namespace SimplestCPlusPlus
{
    class StraightLineMoverComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(StraightLineMoverComponent,
            "{F68C1110-F69D-4452-B56E-166B5458C95A}");

        static void Reflect(AZ::ReflectContext* context);

        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;

        // AZ::TickBus interface implementation
        void OnTick(float deltaTime,
            AZ::ScriptTimePoint time) override;

    private:
        float m_speed = 0.3f;
        bool m_isAuthoritative = false;
    };
}