#include <gp4/gp4.h>

typedef void* (__stdcall* GetProcessHeap_t)(void);
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
struct Unknown_00471250 {
    uint8_t Unknown00[8];
    void* File08;     // CRT FILE closed through 0x005dc52c
    void* Buffer0c;   // process-heap block
    void* Buffer10;   // process-heap block
};
#pragma pack(pop)

// Releases the object's resources: closes the CRT FILE at +8 (0x005dc52c) and clears the pointer, then
// frees the heap blocks at +0xc and +0x10 (HeapFree(GetProcessHeap(), 0, ...)) when they are set.
// FUNCTION: GP4 0x00471250
void __fastcall FUN_00471250(Unknown_00471250* self, void* /*edx*/) {
    if (self->File08 != nullptr) {
        gp4::cdecl_<0x005dc52c, int>(self->File08);
    }
    void* buffer0c = self->Buffer0c;
    HeapFree_t heapFree = GP4_IAT(HeapFree_t, 0x005f02a0);
    GetProcessHeap_t getProcessHeap = GP4_IAT(GetProcessHeap_t, 0x005f02a4);
    self->File08 = nullptr;
    if (buffer0c != nullptr) {
        heapFree(getProcessHeap(), 0, buffer0c);
    }
    void* buffer10 = self->Buffer10;
    if (buffer10 != nullptr) {
        heapFree(getProcessHeap(), 0, buffer10);
    }
}
GP4_IMPL(0x00471250, FUN_00471250, "thiscall(ptr[0x20]:{8:nptr[0x20]:layout=iostrg_0046b580, 0xc:nptr[0x20]:bytes, 0x10:nptr[0x20]:bytes}) -> void")
