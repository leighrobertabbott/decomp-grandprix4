#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the six-slot list text builder 0x00534de4 (record in ebx), then counts the call in the
// (unaligned) dword 0x00828293. Registers are those the builder leaves; the flags are those of
// the final `add dword [0x828293], 1`.
// FUNCTION: GP4 0x00534d53
void __cdecl FUN_00534d53(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00534de4, c);
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00828293);
    GP4_GLOBAL(uint32_t, 0x00828293) = before + 1u;
    *r = c;
    r->eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(before, 1u));
}
GP4_IMPL(0x00534d53, FUN_00534d53, "regs(ebx:u32{=0x82870b,0x828b7b}) -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tab_00835846+script_00534de4+mask_00828164+head_00852837+rows_007ab02c")
