
#pragma once

#include <AzCore/Component/Component.h>

#include <SimplestCPlusPlus/SimplestCPlusPlusBus.h>

namespace SimplestCPlusPlus
{
    class SimplestCPlusPlusSystemComponent
        : public AZ::Component
        , protected SimplestCPlusPlusRequestBus::Handler
    {
    public:
        AZ_COMPONENT(SimplestCPlusPlusSystemComponent, "{BF85BF20-F7C4-4FD1-A5D5-22F8E16F755A}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // SimplestCPlusPlusRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
