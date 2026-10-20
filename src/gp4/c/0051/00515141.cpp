#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper. Takes the selected car record [0x00679d30], asks 0x0040824b for |dword +0x98| >> 8
// (eax), arithmetic-shifts it right by 8 more, scales it by 0x464 when the word 0x007ad8ac is negative
// or by 0x2ba otherwise, keeps the high 16 bits of the product, converts that to packed BCD through
// 0x0051fd0c and stores the result in 0x00a6987c. The byte 0x0088fda6 ends as 0x7c ('add byte [..],1'
// is overwritten by the following `mov byte [..], 0x7c`). popal restores every register; the flags are
// those the BCD routine leaves (`or dl, al`).
// FUNCTION: GP4 0x00515141
void __cdecl FUN_00515141(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0088fda6) = 0x7cu;

    gp4::Regs magnitude = *r;
    magnitude.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x0040824b, magnitude);

    const int32_t shifted = static_cast<int32_t>(magnitude.eax) >> 8;
    const uint32_t factor = (static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x007ad8ac)) < 0) ? 0x464u : 0x2bau;
    const uint32_t scaled = (static_cast<uint32_t>(shifted) * factor) >> 16;

    gp4::Regs bcd = *r;
    bcd.eax = scaled;
    bcd.eflags = magnitude.eflags;
    gp4::call_regs(0x0051fd0c, bcd);

    GP4_GLOBAL(uint32_t, 0x00a6987c) = bcd.eax;
    r->eflags = gp4::flags::merge(r->eflags, bcd.eflags);
}
GP4_IMPL(0x00515141, FUN_00515141, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00418f8f+carst_0040f6d0")
