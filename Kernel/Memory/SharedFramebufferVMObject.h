/*
 * Copyright (c) 2022, Liav A. <liavalb@hotmail.co.il>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <Kernel/Forward.h>
#include <Kernel/Memory/AllocationStrategy.h>
#include <Kernel/Memory/AnonymousVMObject.h>
#include <Kernel/Memory/MemoryManager.h>
#include <Kernel/Memory/PageFaultResponse.h>
#include <Kernel/PhysicalAddress.h>

namespace Kernel::Memory {

class SharedFramebufferVMObject final : public VMObject {
public:
    class FakeWritesFramebufferVMObject final : public VMObject {
    public:
        static ErrorOr<NonnullRefPtr<FakeWritesFramebufferVMObject>> try_create(Badge<SharedFramebufferVMObject>, SharedFramebufferVMObject const& parent_object);

    private:
        FakeWritesFramebufferVMObject(SharedFramebufferVMObject const& parent_object, FixedArray<RefPtr<PhysicalPage>>&& new_physical_pages)
            : VMObject(move(new_physical_pages))
            , m_parent_object(parent_object)
        {
        }
        virtual StringView class_name() const override { return "FakeWritesFramebufferVMObject"sv; }
        virtual ErrorOr<NonnullRefPtr<VMObject>> try_clone() override { return Error::from_errno(ENOTIMPL); }
        virtual Span<RefPtr<PhysicalPage> const> physical_pages() const override { return m_parent_object->fake_sink_framebuffer_physical_pages(); }
        virtual Span<RefPtr<PhysicalPage>> physical_pages() override { return m_parent_object->fake_sink_framebuffer_physical_pages(); }
        NonnullRefPtr<SharedFramebufferVMObject> m_parent_object;
    };

    class RealWritesFramebufferVMObject final : public VMObject {
    public:
        static ErrorOr<NonnullRefPtr<RealWritesFramebufferVMObject>> try_create(Badge<SharedFramebufferVMObject>, SharedFramebufferVMObject const& parent_object);

    private:
        RealWritesFramebufferVMObject(SharedFramebufferVMObject const& parent_object, FixedArray<RefPtr<PhysicalPage>>&& new_physical_pages)
            : VMObject(move(new_physical_pages))
            , m_parent_object(parent_object)
        {
        }
        virtual StringView class_name() const override { return "RealWritesFramebufferVMObject"sv; }
        virtual ErrorOr<NonnullRefPtr<VMObject>> try_clone() override { return Error::from_errno(ENOTIMPL); }
        virtual Span<RefPtr<PhysicalPage> const> physical_pages() const override { return m_parent_object->real_framebuffer_physical_pages(); }
        virtual Span<RefPtr<PhysicalPage>> physical_pages() override { return m_parent_object->real_framebuffer_physical_pages(); }
        NonnullRefPtr<SharedFramebufferVMObject> m_parent_object;
    };

    virtual ~SharedFramebufferVMObject() override = default;

    static ErrorOr<NonnullRefPtr<SharedFramebufferVMObject>> try_create_for_physical_range(PhysicalAddress paddr, size_t size);
    static ErrorOr<NonnullRefPtr<SharedFramebufferVMObject>> try_create_at_arbitrary_physical_range(size_t size);
    virtual ErrorOr<NonnullRefPtr<VMObject>> try_clone() override { return Error::from_errno(ENOTIMPL); }

    void switch_to_fake_sink_framebuffer_writes(Badge<Kernel::DisplayConnector>);
    void switch_to_real_framebuffer_writes(Badge<Kernel::DisplayConnector>);

    virtual Span<RefPtr<PhysicalPage> const> physical_pages() const override;
    virtual Span<RefPtr<PhysicalPage>> physical_pages() override;

    Span<RefPtr<PhysicalPage>> fake_sink_framebuffer_physical_pages();
    Span<RefPtr<PhysicalPage> const> fake_sink_framebuffer_physical_pages() const;

    Span<RefPtr<PhysicalPage>> real_framebuffer_physical_pages();
    Span<RefPtr<PhysicalPage> const> real_framebuffer_physical_pages() const;

    FakeWritesFramebufferVMObject const& fake_writes_framebuffer_vmobject() const { return *m_fake_writes_framebuffer_vmobject; }
    FakeWritesFramebufferVMObject& fake_writes_framebuffer_vmobject() { return *m_fake_writes_framebuffer_vmobject; }

    RealWritesFramebufferVMObject const& real_writes_framebuffer_vmobject() const { return *m_real_writes_framebuffer_vmobject; }
    RealWritesFramebufferVMObject& real_writes_framebuffer_vmobject() { return *m_real_writes_framebuffer_vmobject; }

private:
    SharedFramebufferVMObject(FixedArray<RefPtr<PhysicalPage>>&& new_physical_pages, CommittedPhysicalPageSet, AnonymousVMObject& real_framebuffer_vmobject);

    virtual StringView class_name() const override { return "SharedFramebufferVMObject"sv; }

    ErrorOr<void> create_fake_writes_framebuffer_vm_object();
    ErrorOr<void> create_real_writes_framebuffer_vm_object();

    NonnullRefPtr<AnonymousVMObject> m_real_framebuffer_vmobject;
    RefPtr<FakeWritesFramebufferVMObject> m_fake_writes_framebuffer_vmobject;
    RefPtr<RealWritesFramebufferVMObject> m_real_writes_framebuffer_vmobject;
    bool m_writes_are_faked { false };
    mutable RecursiveSpinlock m_writes_state_lock { LockRank::None };
    CommittedPhysicalPageSet m_committed_pages;
};

}
