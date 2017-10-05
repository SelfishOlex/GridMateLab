
#include "StdAfx.h"
#include <platform_impl.h>

#include <AzCore/Memory/SystemAllocator.h>

#include "StraightLineMoverSystemComponent.h"

#include <IGem.h>

namespace StraightLineMover
{
    class StraightLineMoverModule
        : public CryHooksModule
    {
    public:
        AZ_RTTI(StraightLineMoverModule, "{1F7767DE-5F0C-4A45-8346-F3D6041E894E}", CryHooksModule);
        AZ_CLASS_ALLOCATOR(StraightLineMoverModule, AZ::SystemAllocator, 0);

        StraightLineMoverModule()
            : CryHooksModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                StraightLineMoverSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<StraightLineMoverSystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(StraightLineMover_4e10cafdabe1412cb9b47b0e81fcdbd3, StraightLineMover::StraightLineMoverModule)
