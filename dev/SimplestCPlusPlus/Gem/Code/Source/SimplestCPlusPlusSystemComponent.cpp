
#include "StdAfx.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include "SimplestCPlusPlusSystemComponent.h"

namespace SimplestCPlusPlus
{
    void SimplestCPlusPlusSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<SimplestCPlusPlusSystemComponent, AZ::Component>()
                ->Version(0)
                ->SerializerForEmptyClass();

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<SimplestCPlusPlusSystemComponent>("SimplestCPlusPlus", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void SimplestCPlusPlusSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("SimplestCPlusPlusService"));
    }

    void SimplestCPlusPlusSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("SimplestCPlusPlusService"));
    }

    void SimplestCPlusPlusSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        (void)required;
    }

    void SimplestCPlusPlusSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void SimplestCPlusPlusSystemComponent::Init()
    {
    }

    void SimplestCPlusPlusSystemComponent::Activate()
    {
        SimplestCPlusPlusRequestBus::Handler::BusConnect();
    }

    void SimplestCPlusPlusSystemComponent::Deactivate()
    {
        SimplestCPlusPlusRequestBus::Handler::BusDisconnect();
    }
}
