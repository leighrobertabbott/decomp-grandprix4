#include <gp4/gp4.h>

struct Unknown_00a6988c {
    uint8_t Unknown00[0x118];
    uint32_t Unknown118;
    uint32_t Unknown11C;
    uint32_t Unknown120;
    uint32_t Unknown124;
};

// Fetches the linked record of the record at [0x00a69878] (0x0040825e returns the dword at +0x1c),
// stores it in 0x00a6988c and copies that record's dwords +0x118, +0x11c, +0x120, +0x124 to
// 0x00a6987c, 0x00a69880, 0x00a69884, 0x00a69888 (pairs loaded and stored in the original order).
// pushal/popal wrapper: no register or flag changes.
// FUNCTION: GP4 0x0051fa72
void __cdecl FUN_0051fa72(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.esi = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::call_regs(0x0040825e, call);

    const uint32_t address = call.eax;
    GP4_GLOBAL(uint32_t, 0x00a6988c) = address;
    const Unknown_00a6988c* rec = reinterpret_cast<const Unknown_00a6988c*>(static_cast<uintptr_t>(address));
    uint32_t first = rec->Unknown118;
    uint32_t second = rec->Unknown11C;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = first;
    GP4_GLOBAL(uint32_t, 0x00a69880) = second;
    first = rec->Unknown120;
    second = rec->Unknown124;
    GP4_GLOBAL(uint32_t, 0x00a69884) = first;
    GP4_GLOBAL(uint32_t, 0x00a69888) = second;
}
GP4_IMPL(0x0051fa72, FUN_0051fa72, "regs() -> (); globals=layout=ptr2_00a69878")
