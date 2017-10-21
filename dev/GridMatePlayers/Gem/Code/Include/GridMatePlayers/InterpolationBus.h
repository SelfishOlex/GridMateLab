#pragma once
#include <AzCore/EBus/EBus.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Math/Vector3.h>

namespace GridMatePlayers
{
    class InterpolationInterface
        : public AZ::EBusTraits
    {
    public:
        virtual ~InterpolationInterface() = default;

        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy =
            AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy =
            AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        virtual void SetWorldTranslation(
            const AZ::Vector3& desired, AZ::u32 time) = 0;

        virtual AZ::Vector3 GetWorldTranslation() = 0;
    };

    using InterpolationBus = AZ::EBus<InterpolationInterface>;
} // namespace GridMatePlayers