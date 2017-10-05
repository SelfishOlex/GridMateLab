
#include "StdAfx.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include "StraightLineMoverSystemComponent.h"

namespace StraightLineMover
{
    void StraightLineMoverSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<StraightLineMoverSystemComponent, AZ::Component>()
                ->Version(0)
                ->SerializerForEmptyClass();

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<StraightLineMoverSystemComponent>("StraightLineMover", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void StraightLineMoverSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("StraightLineMoverService"));
    }

    void StraightLineMoverSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("StraightLineMoverService"));
    }

    void StraightLineMoverSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        (void)required;
    }

    void StraightLineMoverSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void StraightLineMoverSystemComponent::Init()
    {
    }

    void StraightLineMoverSystemComponent::Activate()
    {
        StraightLineMoverRequestBus::Handler::BusConnect();
    }

    void StraightLineMoverSystemComponent::Deactivate()
    {
        StraightLineMoverRequestBus::Handler::BusDisconnect();
    }
}
