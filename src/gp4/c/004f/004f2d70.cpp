#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Entry_004f2d70 {
    uint8_t Unknown00[0x50];
    uint32_t Value50;
    uint32_t Unknown54;
    uint32_t Value58;
    uint8_t Unknown5c[0x88 - 0x5c];
};
struct Renderer_004f2d70 {
    uint8_t Unknown00[0x44];
    Entry_004f2d70* Entries44;
};
struct Slot_004f2d70 {
    uint32_t Value00;
    uint32_t Unknown04;
    uint32_t Value08;
};
#pragma pack(pop)

// Copies the dwords +0x50 and +0x58 of entry `index` (0x88-byte entries behind [renderer + 0x44],
// renderer pointer at 0x00a539c0) to the dwords +0 and +8 of slot `index` (12-byte slots at the
// pointer 0x00aa0820).
// FUNCTION: GP4 0x004f2d70
void __stdcall FUN_004f2d70(int index) {
    GP4_GLOBAL(Slot_004f2d70*, 0x00aa0820)[index].Value00 =
        GP4_GLOBAL(Renderer_004f2d70*, 0x00a539c0)->Entries44[index].Value50;
    GP4_GLOBAL(Slot_004f2d70*, 0x00aa0820)[index].Value08 =
        GP4_GLOBAL(Renderer_004f2d70*, 0x00a539c0)->Entries44[index].Value58;
}
GP4_IMPL(0x004f2d70, FUN_004f2d70, "stdcall(i32[0..7]) -> void; gfields={0x00a539c0:ptr[0x70]:{0x44:ptr[0x448]:bytes}, 0x00aa0820:ptr[0x68]:bytes}")
