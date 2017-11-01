#include "StdAfx.h"
#include "LocalClientComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Transform.h>
#include <ISystem.h>
#include <INetwork.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

void LocalClientComponent::Reflect(AZ::ReflectContext* context)
{
    if (auto sc = azrtti_cast<SerializeContext*>(context))
    {
        sc->Class<LocalClientComponent, Component>()
          ->Version(1)
          ->Field("Camera Offset",
                  &LocalClientComponent::m_cameraOffset);

        if (EditContext* ec = sc->GetEditContext())
        {
            ec->Class<LocalClientComponent>(
                  "Local Client",
                  "Handles session logic")
              ->ClassElement(
                  Edit::ClassElements::EditorData, "")
              ->Attribute(Edit::Attributes::Category,
                          "GridMate Players")
              ->Attribute(
                  Edit::Attributes::
                  AppearsInAddComponentMenu,
                  AZ_CRC("Game"))
              ->DataElement(nullptr,
                            &LocalClientComponent::m_cameraOffset,
                            "Camera Offset", "");
        }
    }
}

void LocalClientComponent::Activate()
{
#if !defined(DEDICATED_SERVER)
    LocalClientBus::Handler::BusConnect();

    ISystem* system = nullptr;
    EBUS_EVENT_RESULT(system, CrySystemRequestBus, GetCrySystem);
    if (system)
    {
        SessionEventBus::Handler::BusConnect(
            system->GetINetwork()->GetGridMate());
    }
#endif
}

void LocalClientComponent::Deactivate()
{
#if !defined(DEDICATED_SERVER)
    LocalClientBus::Handler::BusDisconnect();
    if (SessionEventBus::Handler::BusIsConnected())
    {
        SessionEventBus::Handler::BusDisconnect();
    }
#endif
}

void LocalClientComponent::AttachToBody(
    GridMate::MemberIDCompact playerId,
    const AZ::EntityId& body)
{
    if (m_selfId == 0 || m_selfId != playerId) return;

    Transform t = Transform::CreateTranslation(m_cameraOffset);
    EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
        SetLocalTM, t);

    EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
        SetParentRelative, body);
}

void LocalClientComponent::OnMemberJoined(
    GridSession* session, GridMember* member)
{
    m_selfId = session->GetMyMember()->GetIdCompact();
}