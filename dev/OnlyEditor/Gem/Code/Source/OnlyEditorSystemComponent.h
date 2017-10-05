
#pragma once

#include <AzCore/Component/Component.h>

#include <OnlyEditor/OnlyEditorBus.h>

namespace OnlyEditor
{
    class OnlyEditorSystemComponent
        : public AZ::Component
        , protected OnlyEditorRequestBus::Handler
    {
    public:
        AZ_COMPONENT(OnlyEditorSystemComponent, "{E2470612-A0FA-4F44-8C43-4DD0FF88C90B}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // OnlyEditorRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
