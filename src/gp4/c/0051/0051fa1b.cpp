#include <gp4/gp4.h>

struct Unknown_00a69878 {
    uint8_t Unknown00[0x120];
    uint32_t Unknown120;
    uint32_t Unknown124;
};

// Copies the dwords +0x120 and +0x124 of the record at [0x00a69878] to the globals 0x00a6987c and 0x00a69880.
// pushal/popal wrapper around plain movs: no register or flag changes.
// FUNCTION: GP4 0x0051fa1b
void __cdecl FUN_0051fa1b(gp4::Regs* /*r*/) {
    const Unknown_00a69878* rec = reinterpret_cast<const Unknown_00a69878*>(GP4_GLOBAL(uint32_t, 0x00a69878));
    const uint32_t first = rec->Unknown120;
    const uint32_t second = rec->Unknown124;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = first;
    GP4_GLOBAL(uint32_t, 0x00a69880) = second;
}
GP4_IMPL(0x0051fa1b, FUN_0051fa1b, "regs() -> (); globals=layout=ptr_00a69878")
