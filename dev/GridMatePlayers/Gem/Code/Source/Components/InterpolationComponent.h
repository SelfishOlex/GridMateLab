#pragma once
#include <AzCore/Component/Component.h>
#include "AzCore/Component/TickBus.h"
#include <GridMatePlayers/InterpolationBus.h>
#include <Utils/MovementHistory.h>
#include <AzFramework/Network/NetBindable.h>

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
        // Component
        void Activate() override;
        void Deactivate() override;

        // TickBus
        void OnTick(float, AZ::ScriptTimePoint) override;

        // InterpolationBus
        void SetWorldTranslation(
            const AZ::Vector3& desired, AZ::u32 time) override;
        AZ::Vector3 GetWorldTranslation() override;

    private:
        bool m_enabled = true;
        AZ::u32 m_delayForOthers = 500; // ms
        MovementHistory m_history{ 50 };
    };
}
