#include "StdAfx.h"
#include <platform_impl.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <IGem.h>
#include <WindowPositionCVars.h>

namespace WindowPosition
{
    class WindowPositionModule
        : public CryHooksModule
    {
    public:
        AZ_RTTI(WindowPositionModule,
            "{636206AD-29E1-4A8A-9575-1F18DAC2BE94}",
            CryHooksModule);
        AZ_CLASS_ALLOCATOR(WindowPositionModule,
            AZ::SystemAllocator, 0);

        WindowPositionModule() : CryHooksModule() {}

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

        WindowPositionCVars m_cvars;
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name
//     of the class above
AZ_DECLARE_MODULE_CLASS(
    WindowPosition_081922bcf58844ef887a1b6b3574240a,
    WindowPosition::WindowPositionModule)
