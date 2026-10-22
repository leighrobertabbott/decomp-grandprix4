#include <gp4/gp4.h>

typedef void*(__stdcall* GetProcessHeap_t)();
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
struct Unknown_0045afe0 {
    uint32_t Vtable00;
    uint8_t Unknown04[0x14];
    void* Buffer18;
};
#pragma pack(pop)

// Destructor body: installs the class vtable 0x005f05bc and frees the process-heap block at +0x18
// when it is set (the pointer is left as is).
// FUNCTION: GP4 0x0045afe0
void __fastcall FUN_0045afe0(Unknown_0045afe0* self, void* /*edx*/) {
    self->Vtable00 = 0x005f05bc;
    void* buffer = self->Buffer18;
    if (buffer != nullptr) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, buffer);
    }
}
GP4_IMPL(0x0045afe0, FUN_0045afe0, "thiscall(ptr[0x20]:{0x18:nptr[0x20]:bytes}) -> void")
