#include <gp4/gp4.h>

// FUNCTION: GP4 0x00409c44
// Integer square root of the 32-bit value dx:ax (dx high), returned in ax (dx = 0). The value
// is first normalised by shifting left two bits at a time (count) until dx >= 0x4000, then
// three Newton steps estimate = estimate / 2 + (value / 2) / estimate (the 16-bit div is skipped,
// keeping the low word of value / 2, when its high word is not below the estimate), and the
// estimate is shifted right by count. A zero value returns ax = 0 with dx untouched. All other
// registers are restored (push/pop). The routine ends with "add esp, 0x20", so its flags are a
// function of the stack pointer, not of the data: with the five pushes of this prologue the
// oracle's stack pointer leaves CF = AF = ZF = SF = OF = 0 and PF = 1 (pinned oracle model).
void __cdecl FUN_00409c44(gp4::Regs* r) {
    uint16_t di = static_cast<uint16_t>(r->eax);
    uint16_t si = static_cast<uint16_t>(r->edx);
    uint32_t count = 0;

    // Normalise: shl di,1 / rcl si,1 twice is a 32-bit shift left by two.
    while (si < 0x4000u) {
        count++;
        const uint32_t shifted = ((static_cast<uint32_t>(si) << 16) | di) << 2;
        si = static_cast<uint16_t>(shifted >> 16);
        di = static_cast<uint16_t>(shifted);
        if (si == 0u && di == 0u) {
            r->eax = r->eax & 0xffff0000u;
            r->eflags = (r->eflags & ~0x8d5u) | 0x04u;
            return;
        }
    }

    // shr cx,1 / rcr bx,1: half = value >> 1 as the 32-bit pair cx:bx
    const uint32_t half = ((static_cast<uint32_t>(si) << 16) | di) >> 1;
    const uint16_t bx = static_cast<uint16_t>(half);
    const uint16_t cx = static_cast<uint16_t>(half >> 16);
    uint16_t estimate = static_cast<uint16_t>((si >> 1) + 0x8000u);

    for (int step = 0; step < 3; ++step) {
        uint16_t quotient = bx;
        if (cx < estimate) {
            quotient = static_cast<uint16_t>(((static_cast<uint32_t>(cx) << 16) | bx) / estimate);
        }
        estimate = static_cast<uint16_t>((estimate >> 1) + quotient);
    }

    estimate = static_cast<uint16_t>(static_cast<uint32_t>(estimate) >> (count & 31u));
    r->eax = (r->eax & 0xffff0000u) | estimate;
    r->edx = r->edx & 0xffff0000u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x04u;
}
GP4_IMPL(0x00409c44, FUN_00409c44, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of)")
