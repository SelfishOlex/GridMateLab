
#pragma once

#include <AzCore/Component/Component.h>

#include <GridMatePlayers/GridMatePlayersBus.h>

namespace GridMatePlayers
{
    class GridMatePlayersSystemComponent
        : public AZ::Component
        , protected GridMatePlayersRequestBus::Handler
    {
    public:
        AZ_COMPONENT(GridMatePlayersSystemComponent, "{27902E8B-0810-4C59-976E-C76A651CDD51}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // GridMatePlayersRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
