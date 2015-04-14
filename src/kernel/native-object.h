// Copyright 2014 Runtime.JS project authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <kernel/kernel.h>
#include <kernel/object-wrapper.h>
#include <kernel/v8utils.h>
#include <kernel/template-cache.h>
#include <acpi.h>
#include <common/utils.h>
#include <kernel/handle.h>
#include <kernel/pipes.h>

using common::Range;
using common::MemoryBlock;

namespace rt {

class AcpiManager;

class NativesObject : public JsObjectWrapper<NativesObject,
        NativeTypeId::TYPEID_NATIVES> {
public:
    NativesObject() : JsObjectWrapper() {}

    DECLARE_NATIVE(CallHandler);
    DECLARE_NATIVE(SyncRPC);
    DECLARE_NATIVE(SetTimeout);
    DECLARE_NATIVE(SetInterval);
    DECLARE_NATIVE(ClearTimer);
    DECLARE_NATIVE(KernelLog);
    DECLARE_NATIVE(InitrdText);
    DECLARE_NATIVE(KernelLoaderCallback);
    DECLARE_NATIVE(Resources);
    DECLARE_NATIVE(Version);
    DECLARE_NATIVE(Exit);
    DECLARE_NATIVE(Eval);
    DECLARE_NATIVE(Args);
    DECLARE_NATIVE(InstallInternals);
    DECLARE_NATIVE(CallResult);
    DECLARE_NATIVE(Reboot);
    DECLARE_NATIVE(Debug);
    DECLARE_NATIVE(StopVideoLog);
    DECLARE_NATIVE(BufferAddress);
    DECLARE_NATIVE(CreatePipe);
    DECLARE_NATIVE(NetChecksum);
    DECLARE_NATIVE(BufferSliceInplace);

    /**
     * Kernel utils
     */
    DECLARE_NATIVE(StartProfiling);
    DECLARE_NATIVE(StopProfiling);
    DECLARE_NATIVE(GetCommandLine);

    /**
     * Super basic encoder/decoder support
     * https://encoding.spec.whatwg.org/
     */
    DECLARE_NATIVE(TextEncoder);
    DECLARE_NATIVE(TextEncoderEncode);
    DECLARE_NATIVE(TextDecoder);
    DECLARE_NATIVE(TextDecoderDecode);

    /**
     * Get unique index of an handle object within the handle pool
     */
    DECLARE_NATIVE(HandleIndex);

    /**
     * Handle method call handler
     */
    DECLARE_NATIVE(HandleMethodCall);

    /**
     * performance.now()
     */
    DECLARE_NATIVE(PerformanceNow);

    /**
     * List running isolates and stat info
     */
    DECLARE_NATIVE(IsolatesInfo);

    /**
     * Get kernel system info
     */
    DECLARE_NATIVE(SystemInfo);

    /**
     * Convert JavaScript value converted to string to
     * UTF-8 encoded ArrayBuffer
     */
    DECLARE_NATIVE(ToBuffer);

    /**
     * Reinterpret ArrayBuffer as UTF-8 string
     */
    DECLARE_NATIVE(BufferToString);

    /**
     * Create new handle pool
     */
    DECLARE_NATIVE(CreateHandlePool);

    /**
     * Get array of all initrd file names
     */
    DECLARE_NATIVE(InitrdList);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("kernelLog", KernelLog);
        obj.SetCallback("resources", Resources);
        obj.SetCallback("args", Args);
        obj.SetCallback("installInternals", InstallInternals);
        obj.SetCallback("callResult", CallResult);
        obj.SetCallback("initrdText", InitrdText);
        obj.SetCallback("debug", Debug);
        obj.SetCallback("reboot", Reboot);
        obj.SetCallback("stopVideoLog", StopVideoLog);
        obj.SetCallback("initrdList", InitrdList);
        obj.SetCallback("bufferAddress", BufferAddress);
        obj.SetCallback("createHandlePool", CreateHandlePool);
        obj.SetCallback("systemInfo", SystemInfo);
        obj.SetCallback("isolatesInfo", IsolatesInfo);
        obj.SetCallback("handleIndex", HandleIndex);
        obj.SetCallback("netChecksum", NetChecksum);
    }

    JsObjectWrapperBase* Clone() const {
        return nullptr; // Not clonable
    }
};

class IoPortX64Object : public JsObjectWrapper<IoPortX64Object,
        NativeTypeId::TYPEID_RESOURCE_IO_PORT> {
public:
    IoPortX64Object(uint16_t port_number)
        :	JsObjectWrapper(),
            port_number_(port_number) { }

    DECLARE_NATIVE(Write8);
    DECLARE_NATIVE(Write16);
    DECLARE_NATIVE(Write32);
    DECLARE_NATIVE(Read8);
    DECLARE_NATIVE(Read16);
    DECLARE_NATIVE(Read32);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("write8", Write8);
        obj.SetCallback("write16", Write16);
        obj.SetCallback("write32", Write32);
        obj.SetCallback("read8", Read8);
        obj.SetCallback("read16", Read16);
        obj.SetCallback("read32", Read32);
    }

    JsObjectWrapperBase* Clone() const {
        return new IoPortX64Object(port_number_);
    }
private:
    uint16_t port_number_;
};

class AcpiHandleObject : public JsObjectWrapper<AcpiHandleObject,
        NativeTypeId::TYPEID_ACPI_HANDLE> {
public:
    AcpiHandleObject(ACPI_HANDLE handle)
        :	JsObjectWrapper(),
            handle_(handle),
            devinfo_(nullptr) {
        RT_ASSERT(handle);
    }

    DECLARE_NATIVE(IsRootBridge);
    DECLARE_NATIVE(Address);
    DECLARE_NATIVE(Parent);
    DECLARE_NATIVE(IsDevice);
    DECLARE_NATIVE(HardwareId);
    DECLARE_NATIVE(GetIrqRoutingTable);
    DECLARE_NATIVE(GetRootBridgeBusNumber);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("isRootBridge", IsRootBridge);
        obj.SetCallback("getRootBridgeBusNumber", GetRootBridgeBusNumber);
        obj.SetCallback("address", Address);
        obj.SetCallback("parent", Parent);
        obj.SetCallback("isDevice", IsDevice);
        obj.SetCallback("hardwareId", HardwareId);
        obj.SetCallback("getIrqRoutingTable", GetIrqRoutingTable);
    }

    ~AcpiHandleObject() {
        if (devinfo_) delete devinfo_;
    }

    JsObjectWrapperBase* Clone() const {
        return nullptr; // Not clonable
    }
private:
    ACPI_DEVICE_INFO* GetInfo() {
        if (nullptr == devinfo_) {
            ACPI_DEVICE_INFO* devinfo = nullptr;
            ACPI_STATUS s = AcpiGetObjectInfo(handle_, &devinfo);
            if (ACPI_FAILURE(s)) {
                return nullptr;
            }
            devinfo_ = devinfo;
        }
        return devinfo_;
    }


    ACPI_HANDLE handle_;
    ACPI_DEVICE_INFO* devinfo_;
};

class AcpiManagerObject : public JsObjectWrapper<AcpiManagerObject,
        NativeTypeId::TYPEID_ACPI_MANAGER> {
public:
    AcpiManagerObject(AcpiManager* mgr)
        :	JsObjectWrapper(),
            mgr_(mgr) {
        RT_ASSERT(mgr_);
    }

    DECLARE_NATIVE(GetPciDevices);
    DECLARE_NATIVE(SystemReset);

    /**
     * Use ACPICA to switch system sleep state
     */
    DECLARE_NATIVE(EnterSleepState);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("getPciDevices", GetPciDevices);
        obj.SetCallback("systemReset", SystemReset);
        obj.SetCallback("enterSleepState", EnterSleepState);
    }

    JsObjectWrapperBase* Clone() const {
        return new AcpiManagerObject(mgr_);
    }
private:
    AcpiManager* mgr_;
};

class ResourceMemoryRangeObject : public JsObjectWrapper<ResourceMemoryRangeObject,
        NativeTypeId::TYPEID_RESOURCE_MEMORY_RANGE> {
public:
    ResourceMemoryRangeObject(Range<size_t> memory_range)
        :	JsObjectWrapper(),
            memory_range_(memory_range) { }

    DECLARE_NATIVE(Begin);
    DECLARE_NATIVE(End);
    DECLARE_NATIVE(Subrange);
    DECLARE_NATIVE(Block);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("begin", Begin);
        obj.SetCallback("end", End);
        obj.SetCallback("subrange", Subrange);
        obj.SetCallback("block", Block);
    }

    JsObjectWrapperBase* Clone() const {
        return new ResourceMemoryRangeObject(memory_range_);
    }
private:
    Range<size_t> memory_range_;
};

class ResourceIORangeObject : public JsObjectWrapper<ResourceIORangeObject,
        NativeTypeId::TYPEID_RESOURCE_IO_RANGE> {
public:
    ResourceIORangeObject(Range<uint16_t> io_range)
        :	JsObjectWrapper(),
            io_range_(io_range) { }

    DECLARE_NATIVE(Begin);
    DECLARE_NATIVE(End);
    DECLARE_NATIVE(Subrange);
    DECLARE_NATIVE(Port);
    DECLARE_NATIVE(OffsetPort);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("begin", Begin);
        obj.SetCallback("end", End);
        obj.SetCallback("subrange", Subrange);
        obj.SetCallback("port", Port);
        obj.SetCallback("offsetPort", OffsetPort);
    }

    JsObjectWrapperBase* Clone() const {
        return new ResourceIORangeObject(io_range_);
    }
private:
    Range<uint16_t> io_range_;
};

class ResourceIRQRangeObject : public JsObjectWrapper<ResourceIRQRangeObject,
        NativeTypeId::TYPEID_RESOURCE_IRQ_RANGE> {
public:
    ResourceIRQRangeObject(Range<uint8_t> irq_range)
        :	JsObjectWrapper(),
            irq_range_(irq_range) { }

    DECLARE_NATIVE(Irq);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("irq", Irq);
    }

    JsObjectWrapperBase* Clone() const {
        return new ResourceIRQRangeObject(irq_range_);
    }
private:
    Range<uint8_t> irq_range_;
};

class ResourceIRQObject : public JsObjectWrapper<ResourceIRQObject,
        NativeTypeId::TYPEID_RESOURCE_IRQ> {
public:
    ResourceIRQObject(uint8_t irq_number)
        :	JsObjectWrapper(), irq_number_(irq_number) { }

    DECLARE_NATIVE(On);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("on", On);
    }

    JsObjectWrapperBase* Clone() const {
        return new ResourceIRQObject(irq_number_);
    }
private:
    uint8_t irq_number_;
};

class ResourceMemoryBlockObject : public JsObjectWrapper<ResourceMemoryBlockObject,
        NativeTypeId::TYPEID_RESOURCE_MEMORY_BLOCK> {
public:
    ResourceMemoryBlockObject(MemoryBlock<uint32_t> memory_block)
        :	JsObjectWrapper(), memory_block_(memory_block) { }

    DECLARE_NATIVE(Buffer);
    DECLARE_NATIVE(Length);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("buffer", Buffer);
        obj.SetCallback("length", Length);
    }

    JsObjectWrapperBase* Clone() const {
        return new ResourceMemoryBlockObject(memory_block_);
    }
private:
    MemoryBlock<uint32_t> memory_block_;
};

class IsolatesManagerObject : public JsObjectWrapper<IsolatesManagerObject,
    NativeTypeId::TYPEID_ISOLATES_MANAGER> {
public:
    IsolatesManagerObject() : JsObjectWrapper() { }

    DECLARE_NATIVE(Create);
    DECLARE_NATIVE(List);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("create", Create);
        obj.SetCallback("list", List);
    }

    JsObjectWrapperBase* Clone() const {
        return new IsolatesManagerObject();
    }
private:
};

class AllocatorObject : public JsObjectWrapper<AllocatorObject,
    NativeTypeId::TYPEID_ALLOCATOR> {
public:
    AllocatorObject() : JsObjectWrapper() { }

    DECLARE_NATIVE(AllocDMA);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("allocDMA", AllocDMA);
    }

    JsObjectWrapperBase* Clone() const {
        return new AllocatorObject();
    }
private:
};

/**
 * Lightweight handle for kernel objects (socket, file, etc)
 */
class HandleObject : public NativeObjectWrapper {
public:
    HandleObject(uint32_t pool_id, uint32_t handle_id)
        :	NativeObjectWrapper(NativeTypeId::TYPEID_HANDLE),
            pool_id_(pool_id), handle_id_(handle_id) { }

    uint32_t pool_id() const { return pool_id_; }
    uint32_t handle_id() const { return handle_id_; }

    static HandleObject* FromInstance(v8::Local<v8::Value> val) {
        if (!val->IsObject()) return nullptr;
        NativeObjectWrapper* ptr = TemplateCache::GetWrapped(val);
        if (nullptr == ptr) return nullptr;
        if (NativeTypeId::TYPEID_HANDLE != ptr->type_id()) return nullptr;
        RT_ASSERT(ptr);
        auto handle_object = reinterpret_cast<HandleObject*>(ptr);
        RT_ASSERT(handle_object);
        return handle_object;
    }
private:
    uint32_t pool_id_;
    uint32_t handle_id_;
};

class HandlePoolObject : public JsObjectWrapper<HandlePoolObject,
    NativeTypeId::TYPEID_HANDLE_POOL> {
public:
    HandlePoolObject(HandlePool* pool) : JsObjectWrapper(), pool_(pool) {
        RT_ASSERT(pool_);
    }

    DECLARE_NATIVE(CreateHandle);
    DECLARE_NATIVE(Has);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("createHandle", CreateHandle);
        obj.SetCallback("has", Has);

    }

    JsObjectWrapperBase* Clone() const {
        return nullptr; // Not clonable
    }
private:
    HandlePool* pool_;
    uint32_t max_handle_id_;
};

class PipeObject : public JsObjectWrapper<PipeObject,
    NativeTypeId::TYPEID_PIPE> {
public:
    PipeObject(Pipe* pipe) : JsObjectWrapper(), pipe_(pipe) {
        RT_ASSERT(pipe_);
    }

    DECLARE_NATIVE(Push);
    DECLARE_NATIVE(Pull);
    DECLARE_NATIVE(Wait);
    DECLARE_NATIVE(Close);

    void ObjectInit(ExportBuilder obj) {
        obj.SetCallback("push", Push);
        obj.SetCallback("pull", Pull);
        obj.SetCallback("wait", Wait);
        obj.SetCallback("close", Close);
    }

    ~PipeObject() {
        pipe_->Unref();
    }

    JsObjectWrapperBase* Clone() const {
        pipe_->Ref();
        return new PipeObject(pipe_);
    }
private:
    Pipe* pipe_;
};

} // namespace rt
