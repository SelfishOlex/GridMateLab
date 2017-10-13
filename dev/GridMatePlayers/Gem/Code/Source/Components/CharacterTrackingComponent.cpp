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

class CharacterTrackingComponent::Chunk : public ReplicaChunk
{
public:
    GM_CLASS_ALLOCATOR(Chunk);
    Chunk() : m_serverCheckpoint("Server Checkpoint") {}
    bool IsReplicaMigratable() override { return true; }

    static const char* GetChunkName()
    {
        return "CharacterTrackingComponent::Chunk";
    }

    DataSetVectorInTime::BindInterface<
            CharacterTrackingComponent,
            &CharacterTrackingComponent::OnNewServerCheckpoint>
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

void CharacterTrackingComponent::OnCharacterMoveForward(
    float speed, u32 time)
{
    m_movingForward = true;
    m_speed = speed;
    AZ_Printf("Book", "2. forwad, local (-- %f --) @ %d",
        static_cast<float>(GetPosition().GetY()), time);
    m_history.AddDataPoint(GetPosition(), time);
}

void CharacterTrackingComponent::OnCharacterStop(u32 time)
{
    m_movingForward = false;
    m_speed = 0;
    AZ_Printf("Book", "2. stopped, local (-- %f --) @ %d",
        static_cast<float>(GetPosition().GetY()), time);
    m_history.AddDataPoint(GetPosition(), time);
}

void CharacterTrackingComponent::OnTransformChanged(
    const AZ::Transform&, const AZ::Transform& world)
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        const auto diff =
            chunk->m_serverCheckpoint.Get().m_vector -
            world.GetTranslation();
        if (diff.GetLengthSq() < m_allowedDeviation) return;

        AZ_Printf("Book", "5. server (-- %f --) @ %d",
            static_cast<float>(world.GetTranslation().GetY()),
            GetTime());

        chunk->m_serverCheckpoint.Set(
            { world.GetTranslation(), GetTime() });
    }
}

void CharacterTrackingComponent::OnTick(float, ScriptTimePoint)
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

void CharacterTrackingComponent::OnNewServerCheckpoint(
    const VectorInTime& value, const GridMate::TimeContext&)
{
    if (m_isActive &&
        !NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        AZ_Printf("Book", "6. check history");
        const auto& serverPosition = value.m_vector;
        const auto local = m_history.GetPositionAt(value.m_time);
        const auto diff = local - serverPosition;

        if (diff.GetLengthSq() > m_allowedDeviation)
        {
            EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
                SetWorldTM,
                Transform::CreateTranslation(serverPosition));

            m_history.DeleteAfter(value.m_time);
            m_history.AddDataPoint(serverPosition, value.m_time);
        }
    }
}
