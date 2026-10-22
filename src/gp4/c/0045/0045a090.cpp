#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045a090 {
    void* Vtable00;
    void* Buffer04;
    uint32_t Size08;
};
#pragma pack(pop)

// Destructor-style routine: sets the vtable pointer to 0x005f0588, frees the heap block at +4
// (clearing +4 and +8), and frees the object itself when bit 0 of the flag argument is set.
// FUNCTION: GP4 0x0045a090
Unknown_0045a090* __fastcall FUN_0045a090(Unknown_0045a090* self, void* /*edx*/, uint8_t flags) {
    typedef void* (__stdcall *GetProcessHeap_t)(void);
    typedef int (__stdcall *HeapFree_t)(void*, uint32_t, void*);
    void* mem = self->Buffer04;
    self->Vtable00 = (void*)0x005f0588;
    if (mem != nullptr) {
        HeapFree_t heapFree = GP4_IAT(HeapFree_t, 0x005f02a0);
        GetProcessHeap_t getHeap = GP4_IAT(GetProcessHeap_t, 0x005f02a4);
        heapFree(getHeap(), 0, mem);
        self->Buffer04 = nullptr;
        self->Size08 = 0;
    }
    if (flags & 1) {
        GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, self);
    }
    return self;
}
GP4_IMPL(0x0045a090, FUN_0045a090, "thiscall(ptr[0x10]:{4:nptr[0x40]:bytes, 8:u32}, i32) -> i32")
