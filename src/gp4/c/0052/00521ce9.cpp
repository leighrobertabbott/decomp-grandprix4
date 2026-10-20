#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00521ce9
// pushal / popal wrapper. Calls 0x00521d29 (which picks the pair 0x0066d8d4 / 0x0066d8d8 from the
// mode byte 0x007c2ea9), then, with the index k = dword [0x0066d8d8], copies the two per-index
// dword tables into 0x0066d8dc (from 0x00893ed8[k]) and into both 0x0066d8e0 and 0x008660a4
// (from 0x00893f00[k]) and clears the bytes 0x0066d8e4..0x0066d8e6. Every GPR comes back
// unchanged; the flags that escape are those the callee left (nothing in between writes them).
void __cdecl FUN_00521ce9(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00521d29, c);

    const uint32_t k = GP4_GLOBAL(uint32_t, 0x0066d8d8);
    GP4_GLOBAL(uint32_t, 0x0066d8dc) = GP4_ARRAY(uint32_t, 0x00893ed8)[k];
    const uint32_t second = GP4_ARRAY(uint32_t, 0x00893f00)[k];
    GP4_GLOBAL(uint32_t, 0x0066d8e0) = second;
    GP4_GLOBAL(uint32_t, 0x008660a4) = second;
    GP4_GLOBAL(uint8_t, 0x0066d8e5) = 0;
    GP4_GLOBAL(uint8_t, 0x0066d8e4) = 0;
    GP4_GLOBAL(uint8_t, 0x0066d8e6) = 0;

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00521ce9, FUN_00521ce9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
