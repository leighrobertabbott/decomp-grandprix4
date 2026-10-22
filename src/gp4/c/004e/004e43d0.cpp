#include <gp4/gp4.h>

typedef void* (__stdcall *GetProcessHeapFn)();
typedef void* (__stdcall *HeapAllocFn)(void* heap, uint32_t flags, uint32_t bytes);

#pragma pack(push, 1)
// Growable vector of dwords backed by a process-heap block.
struct Unknown_004e43d0 {
    uint32_t Vptr00;
    uint32_t* Items04;
    int32_t Count08;
    int32_t Field0c;
};
#pragma pack(pop)

// Constructor: stores the count at +8 and the vtable pointer 0x005f0f80 at +0, allocates count
// dwords from the process heap into +4 and zeroes them (only when the allocation succeeded), clears
// the dword at +0xc; returns this.
// FUNCTION: GP4 0x004e43d0
Unknown_004e43d0* __fastcall FUN_004e43d0(Unknown_004e43d0* self, void* /*edx*/, int32_t count) {
    self->Count08 = count;
    self->Vptr00 = 0x005f0f80;
    self->Items04 = static_cast<uint32_t*>(
        GP4_IAT(HeapAllocFn, 0x005f02a8)(GP4_IAT(GetProcessHeapFn, 0x005f02a4)(), 0u,
                                         static_cast<uint32_t>(count) << 2));
    if (self->Items04 != nullptr) {
        for (int32_t i = 0; i < self->Count08; i++) {
            self->Items04[i] = 0;
        }
    }
    self->Field0c = 0;
    return self;
}
GP4_IMPL(0x004e43d0, FUN_004e43d0, "thiscall(ptr[0x10]:bytes, i32[-2..20]) -> u32")
