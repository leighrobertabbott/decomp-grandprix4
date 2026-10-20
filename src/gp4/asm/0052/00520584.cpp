#include <gp4/gp4.h>
#include <gp4/flags.h>

// For table slot eax: stores 1 into the dword at row lo[eax] .. hi[eax] (at least one row,
// signed compare) of the surface at [0x007a8e18] with row pitch [0x006975c4], then sets
// 0x00679b34[eax] = lo[eax] and 0x00679b24[eax] = -1. pushal/popal restore every GPR, so
// only the flags of the final loop `cmp` remain.
// FUNCTION: GP4 0x00520584
void __cdecl FUN_00520584(gp4::Regs* r) {
    const uint32_t slot = r->eax;
    const int32_t* lo = GP4_ARRAY(int32_t, 0x00679b9c);
    const int32_t* hi = GP4_ARRAY(int32_t, 0x00679bac);
    int32_t* cursor = GP4_ARRAY(int32_t, 0x00679b34);
    int32_t* marker = GP4_ARRAY(int32_t, 0x00679b24);

    int32_t row = lo[slot];
    const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x006975c4);
    uint32_t addr = (uint32_t)row * pitch + GP4_GLOBAL(uint32_t, 0x007a8e18);
    int32_t limit;
    do {
        *(uint32_t*)(uintptr_t)addr = 1u;
        addr += pitch;
        row++;
        limit = hi[slot];
    } while (row <= limit);

    cursor[slot] = lo[slot];
    marker[slot] = -1;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32((uint32_t)row, (uint32_t)limit));
}
GP4_IMPL(0x00520584, FUN_00520584, "regs(eax:u32[0..3]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rows_00520584")
