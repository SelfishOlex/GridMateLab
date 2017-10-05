
#include "StdAfx.h"
#include <platform_impl.h>

#include <AzCore/Memory/SystemAllocator.h>

#include "OnlyEditorSystemComponent.h"

#include <IGem.h>

namespace OnlyEditor
{
    class OnlyEditorModule
        : public CryHooksModule
    {
    public:
        AZ_RTTI(OnlyEditorModule, "{18EC867B-2A19-485A-8BF2-250F34B4F89A}", CryHooksModule);
        AZ_CLASS_ALLOCATOR(OnlyEditorModule, AZ::SystemAllocator, 0);

        OnlyEditorModule()
            : CryHooksModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                OnlyEditorSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<OnlyEditorSystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(OnlyEditor_c5444f59d21a406a9f3a7ca1ef5cccba, OnlyEditor::OnlyEditorModule)
