#include <gp4/gp4.h>

typedef void* (__stdcall *GetProcessHeapFn)();
typedef int (__stdcall *HeapFreeFn)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
// Vector of heap-block pointers: items at +4, count at +8, a dword at +0xc.
struct Unknown_004e44b0 {
    uint8_t Unknown00[4];
    void** Items04;
    int32_t Count08;
    int32_t Field0c;
};
#pragma pack(pop)

// Frees every non-null block pointer of the vector (HeapFree on the process heap), nulling each
// slot, then clears the dword at +0xc. Always returns 0.
// FUNCTION: GP4 0x004e44b0
int __fastcall FUN_004e44b0(Unknown_004e44b0* self, void* /*edx*/) {
    int32_t i = 0;
    if (self->Count08 > 0) {
        HeapFreeFn heapFree = GP4_IAT(HeapFreeFn, 0x005f02a0);
        GetProcessHeapFn getProcessHeap = GP4_IAT(GetProcessHeapFn, 0x005f02a4);
        do {
            void* block = self->Items04[i];
            if (block != nullptr) {
                heapFree(getProcessHeap(), 0u, block);
                self->Items04[i] = nullptr;
            }
            i++;
        } while (i < self->Count08);
    }
    self->Field0c = 0;
    return 0;
}
GP4_IMPL(0x004e44b0, FUN_004e44b0, "thiscall(ptr[0x10]:{4:ptr[0x40]:{0*16/4:nptr[0x10]:bytes}, 8:i32[-1..8]}) -> i32")
