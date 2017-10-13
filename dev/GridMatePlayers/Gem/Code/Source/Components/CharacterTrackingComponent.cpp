#include "StdAfx.h"
#include "CharacterTrackingComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include "GridMate/Replica/ReplicaFunctions.h"
#include "AzFramework/Network/NetBindingHandlerBus.h"
#include "AzCore/Component/TransformBus.h"
#include "GridMate/Replica/ReplicaMgr.h"
#include "LmbrCentral/Physics/CryCharacterPhysicsBus.h"

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

class CharacterTrackingComponent::Chunk
    : public ReplicaChunk
{
public:
    GM_CLASS_ALLOCATOR(Chunk);
    Chunk()
        : m_serverCheckpoint("Server Checkpoint") {}

    bool IsReplicaMigratable() override { return true; }

    static const char* GetChunkName()
    {
        return "CharacterTrackingComponent::Chunk";
    }

    DataSetVectorInTime::BindInterface<
            CharacterTrackingComponent,
            &CharacterTrackingComponent::OnNewCheckpoint>
        m_serverCheckpoint;
};

void CharacterTrackingComponent::Reflect(ReflectContext* reflect)
{
    if (auto sc = azrtti_cast<SerializeContext*>(reflect))
    {
        sc->Class<CharacterTrackingComponent, Component>()
            ->Version(1);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<CharacterTrackingComponent>(
                "Character Tracking", "[Description]")
                ->ClassElement(
                    Edit::ClassElements::EditorData, "")
                ->Attribute(Edit::Attributes::Category,
                    "GridMate Players")
                ->Attribute(Edit::Attributes::
                    AppearsInAddComponentMenu,
                    AZ_CRC("Game"));
        }
    }

    auto& descTable = ReplicaChunkDescriptorTable::Get();
    if (!descTable.FindReplicaChunkDescriptor(
        ReplicaChunkClassId(Chunk::GetChunkName())))
    {
        descTable.RegisterChunkType<Chunk>();
    }
}

void CharacterTrackingComponent::Activate()
{
    const auto self = GetEntityId();
    CharacterMovementRequestBus::Handler::BusConnect(self);
    CharacterMovementNotificationBus::Handler::BusConnect(self);

    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
        TransformNotificationBus::Handler::BusConnect(self);
    else
        TickBus::Handler::BusConnect();

    m_isActive = true;
}

void CharacterTrackingComponent::Deactivate()
{
    m_isActive = false;

    CharacterMovementRequestBus::Handler::BusDisconnect();
    CharacterMovementNotificationBus::Handler::BusDisconnect();

    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
        TransformNotificationBus::Handler::BusDisconnect();
    else
        TickBus::Handler::BusDisconnect();
}

ReplicaChunkPtr CharacterTrackingComponent::GetNetworkBinding()
{
    m_chunk = GridMate::CreateReplicaChunk<Chunk>();
    m_chunk->SetHandler(this);
    return m_chunk;
}

void CharacterTrackingComponent::SetNetworkBinding(
    ReplicaChunkPtr chunk)
{
    m_chunk = chunk;
    m_chunk->SetHandler(this);
}

void CharacterTrackingComponent::UnbindFromNetwork()
{
    m_chunk->SetHandler(nullptr);
    m_chunk = nullptr;
}

void CharacterTrackingComponent::OnCharacterMoveUpdate(
    const Vector3& serverPosition, u32 timestamp)
{
    if (m_chunk && m_chunk->IsProxy())
    {
        const auto local = m_movePoints.GetPositionAt(timestamp);

        const auto diff = local - serverPosition;
        if (diff.GetLengthSq() > m_allowedDeviation)
        {
            AZ_Printf("Book", "6. checking (-- %f --) @ %d vs (-- %f --) @ %d",
                //static_cast<float>(serverPosition.GetX()),
                static_cast<float>(serverPosition.GetY()),
                //static_cast<float>(serverPosition.GetZ()),
                timestamp,
                //static_cast<float>(local.GetX()),
                static_cast<float>(local.GetY()),
                //static_cast<float>(local.GetZ())
                GetTime()
            );

            AZ_Printf("Book", "7. correcting");

            EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
                SetWorldTM, Transform::CreateTranslation(serverPosition));

            m_movePoints.DeleteAfter(timestamp);
            m_movePoints.AddDataPoint(serverPosition, timestamp);
        }
        else
        {
            //AZ_Printf("Book", "7a. history agrees");
        }
    }
}

void CharacterTrackingComponent::OnCharacterMoveForward(
    float speed, u32 time)
{
    m_movingForward = true;
    m_speed = speed;
    AZ_Printf("Book", "2. add movement point");
    m_movePoints.AddDataPoint(GetPosition(), time);
}

void CharacterTrackingComponent::OnCharacterStop(u32 time)
{
    m_movingForward = false;
    m_speed = 0;
    m_movePoints.AddDataPoint(GetPosition(), time);
}

void CharacterTrackingComponent::OnTransformChanged(
    const AZ::Transform&, const AZ::Transform& world)
{
    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
        {
            const auto diff = chunk->m_serverCheckpoint.Get().m_vector - world.GetTranslation();
            if (diff.GetLengthSq() < m_allowedDeviation) return;

            AZ_Printf("Book", "5. server movement check point (-- %f --) @ %d",
                //static_cast<float>(world.GetTranslation().GetX()),
                static_cast<float>(world.GetTranslation().GetY()),
                //static_cast<float>(world.GetTranslation().GetZ()),
                GetTime());

            chunk->m_serverCheckpoint.Set(
                { world.GetTranslation(), GetTime() });
        }
    }
}

void CharacterTrackingComponent::OnTick(float deltaTime,
    AZ::ScriptTimePoint time)
{
    EBUS_EVENT_ID(GetEntityId(),
        LmbrCentral::CryCharacterPhysicsRequestBus,
        RequestVelocity,
        Vector3::CreateAxisY(m_speed), 0);
}

Vector3 CharacterTrackingComponent::GetPosition() const
{
    auto t = AZ::Transform::CreateIdentity();
    EBUS_EVENT_ID_RESULT(t, GetEntityId(), AZ::TransformBus,
        GetWorldTM);
    return t.GetTranslation();
}

AZ::u32 CharacterTrackingComponent::GetTime() const
{
    if (!m_chunk) return 0;
    return m_chunk->GetReplicaManager()->GetTime().m_localTime;
}

void CharacterTrackingComponent::OnNewCheckpoint(
    const VectorInTime& value, const GridMate::TimeContext& tc)
{
    if (m_isActive &&
        !NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        AZ_Printf("Book", "6. check history");
        OnCharacterMoveUpdate(value.m_vector, value.m_time);
    }
}
