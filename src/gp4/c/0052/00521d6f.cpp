#include <gp4/gp4.h>

// Follows the link table 0x00893f78 from the index [0x0066d8d8] to the new index, stores it to
// [0x0066d8d8] and [0x008660a0], loads that entry's values from 0x00893f28 into [0x0066d8dc] and from
// 0x00893f50 into [0x0066d8e0] / [0x008660a4], then clears byte [0x0066d8e6] and sets byte
// [0x00893cfa] to 0xff. pushal/popal wrapper without any flag writer: registers and flags unchanged.
// FUNCTION: GP4 0x00521d6f
void __cdecl FUN_00521d6f(gp4::Regs* r) {
    (void)r;
    const uint32_t entry = GP4_ARRAY(uint32_t, 0x00893f78)[GP4_GLOBAL(uint32_t, 0x0066d8d8)];
    GP4_GLOBAL(uint32_t, 0x0066d8d8) = entry;
    GP4_GLOBAL(uint32_t, 0x008660a0) = entry;
    GP4_GLOBAL(uint32_t, 0x0066d8dc) = GP4_ARRAY(uint32_t, 0x00893f28)[entry];
    const uint32_t value = GP4_ARRAY(uint32_t, 0x00893f50)[entry];
    GP4_GLOBAL(uint32_t, 0x0066d8e0) = value;
    GP4_GLOBAL(uint32_t, 0x008660a4) = value;
    GP4_GLOBAL(uint8_t, 0x0066d8e6) = 0u;
    GP4_GLOBAL(uint8_t, 0x00893cfa) = 0xffu;
}
GP4_IMPL(0x00521d6f, FUN_00521d6f, "regs() -> (); globals=layout=links_00521d6f")
