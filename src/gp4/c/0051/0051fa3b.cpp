#include <gp4/gp4.h>

struct Unknown_00a69878 {
    uint8_t Unknown00[0x118];
    uint32_t Unknown118;
    uint32_t Unknown11C;
    uint32_t Unknown120;
    uint32_t Unknown124;
};

// Copies the four dwords +0x118, +0x11c, +0x120, +0x124 of the record at [0x00a69878] to the globals
// 0x00a6987c, 0x00a69880, 0x00a69884, 0x00a69888 (pairs loaded and stored in the original order).
// pushal/popal wrapper: no register or flag changes.
// FUNCTION: GP4 0x0051fa3b
void __cdecl FUN_0051fa3b(gp4::Regs* /*r*/) {
    const Unknown_00a69878* rec = reinterpret_cast<const Unknown_00a69878*>(GP4_GLOBAL(uint32_t, 0x00a69878));
    uint32_t first = rec->Unknown118;
    uint32_t second = rec->Unknown11C;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = first;
    GP4_GLOBAL(uint32_t, 0x00a69880) = second;
    first = rec->Unknown120;
    second = rec->Unknown124;
    GP4_GLOBAL(uint32_t, 0x00a69884) = first;
    GP4_GLOBAL(uint32_t, 0x00a69888) = second;
}
GP4_IMPL(0x0051fa3b, FUN_0051fa3b, "regs() -> (); globals=layout=ptr_00a69878")
