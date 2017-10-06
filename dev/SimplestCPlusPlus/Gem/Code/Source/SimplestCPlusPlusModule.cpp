
#include "StdAfx.h"
#include <platform_impl.h>

#include <AzCore/Memory/SystemAllocator.h>

#include "SimplestCPlusPlusSystemComponent.h"

#include <IGem.h>
#include <Components/StraightLineMoverComponent.h>
#include <Components/NetSyncComponent.h>

namespace SimplestCPlusPlus
{
    class SimplestCPlusPlusModule
        : public CryHooksModule
    {
    public:
        AZ_RTTI(SimplestCPlusPlusModule, "{55EBBAEF-9027-4AFE-8F12-89EB64A25F0D}", CryHooksModule);
        AZ_CLASS_ALLOCATOR(SimplestCPlusPlusModule, AZ::SystemAllocator, 0);

        SimplestCPlusPlusModule()
            : CryHooksModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                SimplestCPlusPlusSystemComponent::CreateDescriptor(),
                StraightLineMoverComponent::CreateDescriptor(),
                NetSyncComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<SimplestCPlusPlusSystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(SimplestCPlusPlus_f5d179c7ce264ee19b16f62bcb59a4dd, SimplestCPlusPlus::SimplestCPlusPlusModule)
