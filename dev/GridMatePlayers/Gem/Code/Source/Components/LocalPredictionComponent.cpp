#include "StdAfx.h"
#include "LocalPredictionComponent.h"
#include <AzCore/Serialization/EditContext.h>
#include "GridMate/Replica/ReplicaFunctions.h"
#include "AzFramework/Network/NetBindingHandlerBus.h"
#include "AzCore/Component/TransformBus.h"
#include "GridMate/Replica/ReplicaMgr.h"
#include "LmbrCentral/Physics/CryCharacterPhysicsBus.h"
#include <GridMatePlayers/ServerPredictionRequestBus.h>

using namespace AZ;
using namespace AzFramework;
using namespace GridMate;
using namespace GridMatePlayers;

class LocalPredictionComponent::Chunk : public ReplicaChunk
{
public:
    GM_CLASS_ALLOCATOR(Chunk);
    Chunk() : m_serverCheckpoint("Server Checkpoint") {}
    bool IsReplicaMigratable() override { return true; }

    static const char* GetChunkName()
    {
        return "LocalPredictionComponent::Chunk";
    }

    DataSetVectorInTime::BindInterface<
            LocalPredictionComponent,
            &LocalPredictionComponent::OnNewServerCheckpoint>
        m_serverCheckpoint;
};

void LocalPredictionComponent::Reflect(ReflectContext* reflect)
{
    if (auto sc = azrtti_cast<SerializeContext*>(reflect))
    {
        sc->Class<LocalPredictionComponent, Component>()
            ->Version(1)
            ->Field("Allowed Deviation",
                &LocalPredictionComponent::m_allowedDeviation)
            ->Field("Rollback Ratio",
                &LocalPredictionComponent::m_rollbackRatio);

        if (auto ec = sc->GetEditContext())
        {
            ec->Class<LocalPredictionComponent>(
                "Local Prediction",
                "[Guesses how server will move the character]")
                ->ClassElement(
                    Edit::ClassElements::EditorData, "")
                ->Attribute(Edit::Attributes::Category,
                    "GridMate Players")
                ->Attribute(Edit::Attributes::
                    AppearsInAddComponentMenu,
                    AZ_CRC("Game"))
                ->DataElement(nullptr,
                    &LocalPredictionComponent::m_allowedDeviation,
                    "Allowed Deviation", "")
                ->DataElement(nullptr,
                    &LocalPredictionComponent::m_rollbackRatio,
                    "Rollback Ratio", "")
            ;
        }
    }

    auto& descTable = ReplicaChunkDescriptorTable::Get();
    if (!descTable.FindReplicaChunkDescriptor(
        ReplicaChunkClassId(Chunk::GetChunkName())))
    {
        descTable.RegisterChunkType<Chunk>();
    }
}

void LocalPredictionComponent::Activate()
{
    const auto self = GetEntityId();
    LocalPredictionRequestBus::Handler::BusConnect(self);

    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
        TransformNotificationBus::Handler::BusConnect(self);
    else
        TickBus::Handler::BusConnect();

    m_isActive = true;
}

void LocalPredictionComponent::Deactivate()
{
    m_isActive = false;
    LocalPredictionRequestBus::Handler::BusDisconnect();

    if (NetQuery::IsEntityAuthoritative(GetEntityId()))
        TransformNotificationBus::Handler::BusDisconnect();
    else
        TickBus::Handler::BusDisconnect();
}

ReplicaChunkPtr LocalPredictionComponent::GetNetworkBinding()
{
    m_chunk = GridMate::CreateReplicaChunk<Chunk>();
    m_chunk->SetHandler(this);
    return m_chunk;
}

void LocalPredictionComponent::SetNetworkBinding(
    ReplicaChunkPtr chunk)
{
    m_chunk = chunk;
    m_chunk->SetHandler(this);
}

void LocalPredictionComponent::UnbindFromNetwork()
{
    m_chunk->SetHandler(nullptr);
    m_chunk = nullptr;
}

void LocalPredictionComponent::OnCharacterMoveForward(
    float speed, u32 time)
{
    m_movingForward = true;
    m_speed = speed;
    AZ_Printf("Book", "forward, local (-- %f --) @ %d",
        static_cast<float>(GetPosition().GetY()), time);
    m_history.AddDataPoint(GetPosition(), time);
}

void LocalPredictionComponent::OnCharacterStop(u32 time)
{
    m_movingForward = false;
    m_speed = 0;
    AZ_Printf("Book", "stopped, local (-- %f --) @ %d",
        static_cast<float>(GetPosition().GetY()), time);
    m_history.AddDataPoint(GetPosition(), time);
}

void LocalPredictionComponent::OnTransformChanged(
    const AZ::Transform&, const AZ::Transform& world)
{
    if (auto chunk = static_cast<Chunk*>(m_chunk.get()))
    {
        const auto diff =
            chunk->m_serverCheckpoint.Get().m_vector -
            world.GetTranslation();
        if (float(diff.GetLength()) < m_allowedDeviation) return;

        AZ_Printf("Book", "server (-- %f --) @ %d",
            static_cast<float>(world.GetTranslation().GetY()),
            GetTime());

        chunk->m_serverCheckpoint.Set(
            { world.GetTranslation(), GetTime() });

        EBUS_EVENT_ID(GetEntityId(), ServerPredictionRequestBus,
            OnCharacterMoved, world.GetTranslation(), GetTime());
    }
}

void LocalPredictionComponent::OnTick(float, ScriptTimePoint)
{
    EBUS_EVENT_ID(GetEntityId(),
        LmbrCentral::CryCharacterPhysicsRequestBus,
        RequestVelocity,
        Vector3::CreateAxisY(m_speed), 0);
}

Vector3 LocalPredictionComponent::GetPosition() const
{
    auto t = AZ::Transform::CreateIdentity();
    EBUS_EVENT_ID_RESULT(t, GetEntityId(), AZ::TransformBus,
        GetWorldTM);
    return t.GetTranslation();
}

AZ::u32 LocalPredictionComponent::GetTime() const
{
    if (!m_chunk) return 0;
    return m_chunk->GetReplicaManager()->GetTime().m_localTime;
}

void LocalPredictionComponent::OnNewServerCheckpoint(
    const VectorInTime& value, const GridMate::TimeContext&)
{
    if (m_isActive &&
        !NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        if (m_history.HasHistory())
        {
            const auto& serverPos = value.m_vector;
            const auto serverTime = value.m_time;
            const auto backThen =
                m_history.GetPositionAt(serverTime);
            const float diff = (backThen - serverPos).GetLength();

            if (diff > m_allowedDeviation)
            {
                auto now = AZ::Transform::CreateIdentity();
                EBUS_EVENT_ID_RESULT(now, GetEntityId(),
                    AZ::TransformBus, GetWorldTM);

                m_history.DeleteAfter(value.m_time);
                m_history.AddDataPoint(serverPos, value.m_time);

                // adjust current position based on the difference
                const auto adjusted = now.GetTranslation() -
                    (backThen - serverPos) * m_rollbackRatio;

                m_history.AddDataPoint(adjusted, GetTime());

                EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
                    SetWorldTM,
                    Transform::CreateTranslation(adjusted));

                AZ_Printf("Book", "rollback (-- %f --) @%d; dv %f"
                    "so now (-- %f --) @%d",
                    static_cast<float>(value.m_vector.GetY()),
                    value.m_time,
                    diff, static_cast<float>(adjusted.GetY()),
                    GetTime());
            }
        }
        else
        {
            m_history.AddDataPoint(value.m_vector, value.m_time);
            EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus,
                SetWorldTM,
                Transform::CreateTranslation(value.m_vector));
        }
    }
}
