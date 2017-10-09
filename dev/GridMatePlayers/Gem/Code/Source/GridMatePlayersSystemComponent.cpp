
#include "StdAfx.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include "GridMatePlayersSystemComponent.h"

namespace GridMatePlayers
{
    void GridMatePlayersSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<GridMatePlayersSystemComponent, AZ::Component>()
                ->Version(0)
                ->SerializerForEmptyClass();

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<GridMatePlayersSystemComponent>("GridMatePlayers", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void GridMatePlayersSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("GridMatePlayersService"));
    }

    void GridMatePlayersSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("GridMatePlayersService"));
    }

    void GridMatePlayersSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        (void)required;
    }

    void GridMatePlayersSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void GridMatePlayersSystemComponent::Init()
    {
    }

    void GridMatePlayersSystemComponent::Activate()
    {
        GridMatePlayersRequestBus::Handler::BusConnect();
    }

    void GridMatePlayersSystemComponent::Deactivate()
    {
        GridMatePlayersRequestBus::Handler::BusDisconnect();
    }
}
