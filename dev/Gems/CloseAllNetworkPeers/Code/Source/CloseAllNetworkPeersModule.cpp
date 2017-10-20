#include "StdAfx.h"
#include <platform_impl.h>
#include <AzCore/Memory/SystemAllocator.h>
#include "CloseAllNetworkPeersSystemComponent.h"
#include <IGem.h>
#include <ConsoleCommandCVars.h>
#include <CloseNetworkPeersComponent.h>

namespace CloseAllNetworkPeers
{
    class CloseAllNetworkPeersModule
        : public CryHooksModule
    {
    public:
        AZ_RTTI(CloseAllNetworkPeersModule,
            "{3318C61B-4F73-4B09-9D5E-2682A9B64BA3}",
            CryHooksModule);
        AZ_CLASS_ALLOCATOR(CloseAllNetworkPeersModule,
            AZ::SystemAllocator, 0);

        CloseAllNetworkPeersModule()
            : CryHooksModule()
        {
            m_descriptors.insert(m_descriptors.end(), {
                CloseAllPeersSystemComponent::CreateDescriptor(),
                CloseNetworkPeersComponent::CreateDescriptor()
            });
        }

        AZ::ComponentTypeList
        GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<CloseAllPeersSystemComponent>(),
            };
        }

        void OnCrySystemInitialized(ISystem& system,
            const SSystemInitParams& systemInitParams) override
        {
            CryHooksModule::OnCrySystemInitialized(
                system, systemInitParams);
            m_cvars.RegisterCVars();
        }

        void OnSystemEvent(ESystemEvent event,
            UINT_PTR, UINT_PTR) override
        {
            switch (event)
            {
            case ESYSTEM_EVENT_FULL_SHUTDOWN:
            case ESYSTEM_EVENT_FAST_SHUTDOWN:
                m_cvars.UnregisterCVars();
            default:
                AZ_UNUSED(event);
            }
        }

        ConsoleCommandCVars m_cvars;
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class
AZ_DECLARE_MODULE_CLASS(
    CloseAllNetworkPeers_2fe33254d36e4e7f9b0344a051833956,
    CloseAllNetworkPeers::CloseAllNetworkPeersModule)
