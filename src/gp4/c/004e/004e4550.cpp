#include <gp4/gp4.h>

typedef void*(__stdcall* GetProcessHeap_t)();
typedef int(__stdcall* HeapFree_t)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
struct Unknown_004e4550 {
    uint8_t Unknown00[4];
    void** Items04;
    int32_t Capacity08;
    int32_t Count0c;
};
#pragma pack(pop)

// Appends a block pointer to a fixed-capacity pointer array (items at +4, capacity +8, count +0xc).
// Returns 1 for a null block, 0 when stored, 2 when the array is full (the block is freed on the
// process heap instead).
// FUNCTION: GP4 0x004e4550
int __fastcall FUN_004e4550(Unknown_004e4550* self, void* /*edx*/, void* block) {
    if (block == 0) {
        return 1;
    }
    if (self->Count0c < self->Capacity08) {
        self->Items04[self->Count0c] = block;
        self->Count0c = self->Count0c + 1;
        return 0;
    }
    GP4_IAT(HeapFree_t, 0x005f02a0)(GP4_IAT(GetProcessHeap_t, 0x005f02a4)(), 0, block);
    return 2;
}
GP4_IMPL(0x004e4550, FUN_004e4550, "thiscall(ptr[0x20]:{4:ptr[0x100]:bytes, 8:i32[0..40], 0xc:i32[0..40]}, nptr[16]:bytes) -> i32")
