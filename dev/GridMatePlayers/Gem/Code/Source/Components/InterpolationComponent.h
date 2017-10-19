#pragma once
#include <AzCore/Component/Component.h>
#include "AzCore/Component/TickBus.h"
#include <GridMatePlayers/InterpolationBus.h>

namespace GridMatePlayers
{
    class InterpolationComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public InterpolationBus::Handler
    {
    public:
        AZ_COMPONENT(InterpolationComponent,
            "{B4B17EC8-E4AA-461A-B6D1-B74614EECE2F}");

        static void Reflect(AZ::ReflectContext* reflection);

    protected:
        void Activate() override;
        void Deactivate() override;

        // TickBus
        void OnTick(float deltaTime, AZ::ScriptTimePoint time)
            override;

        // InterpolationBus
        void SetInterpolationTarget(
            const AZ::Vector3& desired) override;

        AZ::Vector3 GetInterpolationTarget() override;

    private:
        bool m_enabled = true;

        bool m_hasDesired = false;
        AZ::Vector3 m_desired = AZ::Vector3::CreateZero();
        float m_leftTime = 0.f;
        float m_totalTime = 0.2f;
    };
}
