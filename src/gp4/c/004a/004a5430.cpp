#include <gp4/gp4.h>

typedef void* (__stdcall *GetProcessHeapFn)();
typedef int (__stdcall *HeapFreeFn)(void* heap, uint32_t flags, void* mem);

#pragma pack(push, 1)
// One heap-block slot: a used flag byte, then the block pointer (stride 8, flag at +0x1c + 8*i).
struct BlockSlot {
    uint8_t InUse;
    uint8_t Unknown01[3];
    void* Block;
};
struct BlockTable {
    uint8_t Unknown00[0x1c];
    BlockSlot Slots[22];     // +0x1c
    uint8_t Count;           // +0xcc live blocks
    uint8_t Unknown0cd[0x33];
};
#pragma pack(pop)

// Frees heap block `index` of the table (HeapFree on the process heap), clearing its used flag and
// pointer and decrementing the live-block counter; nothing happens when the slot holds no block.
// FUNCTION: GP4 0x004a5430
void __fastcall FUN_004a5430(BlockTable* self, void* /*edx*/, int index) {
    void* block = self->Slots[index].Block;
    if (block != nullptr) {
        self->Slots[index].InUse = 0;
        GP4_IAT(HeapFreeFn, 0x005f02a0)(GP4_IAT(GetProcessHeapFn, 0x005f02a4)(), 0u, block);
        self->Slots[index].Block = nullptr;
        self->Count = static_cast<uint8_t>(self->Count - 1);
    }
}
GP4_IMPL(0x004a5430, FUN_004a5430, "thiscall(ptr[0x100]:u32{=0,0,0x1000,0x2000,0x3000}, i32[0..21]) -> void")
