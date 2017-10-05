
#pragma once

#include <AzCore/Component/Component.h>

#include <StraightLineMover/StraightLineMoverBus.h>

namespace StraightLineMover
{
    class StraightLineMoverSystemComponent
        : public AZ::Component
        , protected StraightLineMoverRequestBus::Handler
    {
    public:
        AZ_COMPONENT(StraightLineMoverSystemComponent, "{8C8F6CC8-BC10-453E-8F08-92F8018A8706}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // StraightLineMoverRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
