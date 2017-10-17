#pragma once
#include <AzCore/Component/Component.h>
#include <Utils/MovementHistory.h>
#include <GridMatePlayers/ServerPredictionRequestBus.h>
#include <AzCore/Component/TickBus.h>
#include <Utils/VectorInTime.h>
#include <AzCore/Component/TransformBus.h>

namespace GridMatePlayers
{
    class ServerPredictionComponent
        : public AZ::Component
        , public ServerPredictionRequestBus::Handler
    {
    public:
        AZ_COMPONENT(ServerPredictionComponent,
            "{EB45103C-4D70-448A-BFBC-BF8F00ED1A49}");

        static void Reflect(AZ::ReflectContext* reflection);

    protected:
        // Component
        void Activate() override;
        void Deactivate() override;

        // ServerPredictionRequestBus
        void OnCharacterMoveForward(
            const AZ::Vector3& speed,
            AZ::u32 clientTime,
            AZ::u32 serverTime) override;
        void OnCharacterStop(
            AZ::u32 clientTime,
            AZ::u32 serverTime) override;
        void OnCharacterMoved(
            const AZ::Vector3& world,
            AZ::u32 serverTime) override;

    private:
        MovementHistory m_history;
    };
}
