#include <gp4/gp4.h>
#include <gp4/flags.h>

// Character output: with the direct-buffer mode byte [0x0089152b] == 1 the character al is stored
// at the cursor edi (stosb, edi advances; flags of `cmp byte [0x0089152b], 1`). Otherwise, inside a
// pushal/popal pair, the glyph writer 0x0051d319 runs, except when [0x0088fe12] and [0x0093251c]
// are both 0xff, which selects the alternate 0x0051d47a. popal restores every register but not the
// flags, which are those the chosen routine leaves.
// FUNCTION: GP4 0x0051d2ec
void __cdecl FUN_0051d2ec(gp4::Regs* r) {
    const uint8_t bufferMode = GP4_GLOBAL(uint8_t, 0x0089152b);
    if (bufferMode == 1u) {
        uint32_t cursor = r->edi;
        GP4_FIELD(uint8_t, cursor, 0) = static_cast<uint8_t>(r->eax);
        r->edi = cursor + 1u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(bufferMode, 1u));
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    const uint8_t first = GP4_GLOBAL(uint8_t, 0x0088fe12);
    uint32_t arith = gp4::flags::sub8(first, 0xffu);
    uint32_t target = 0x0051d319u;
    if (first == 0xffu) {
        const uint8_t second = GP4_GLOBAL(uint8_t, 0x0093251c);
        arith = gp4::flags::sub8(second, 0xffu);
        if (second != 0xffu) {
            target = 0x0051d47au;
        }
    }
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(target, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051d2ec, FUN_0051d2ec, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, edi:ptr[0x40]:bytes) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")
