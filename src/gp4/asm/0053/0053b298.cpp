#include <gp4/gp4.h>
#include <gp4/flags.h>

// value * count / divisor, or the divisor itself when the 64-bit quotient would not fit
// (`mul ecx; cmp [divisor], edx; jbe skip; div [divisor]`).
static inline uint32_t Scaled(uint32_t value, uint32_t count, uint32_t divisor) {
    const uint64_t product = static_cast<uint64_t>(value) * static_cast<uint64_t>(count);
    if (divisor > static_cast<uint32_t>(product >> 32)) {
        uint32_t remainder = 0;
        return gp4::div64(product, divisor, &remainder);
    }
    return divisor;
}

// Menu slider bar draw. pushal/popal wrapper: every register comes back unchanged; only the flags of
// the last flag writer and the memory effects of the callees remain. The item's words +4 (ebx), +6
// (eax), +8 (ebp) and +0xa (ecx) are inserted into the sorted list by 0x00549db9 after the empty
// hook 0x0054a1d7 (eax - 2, ecx + 2) and the empty routine 0x00531b38 (edx = 0x60) run. Then the
// value block behind the pointer at +0xe ({numerator A, divisor, numerator B}) scales the item's
// height: with a nonzero divisor, ebx = the top word +6, ecx = height + 1 and
// eax = A * ecx / divisor + ebx, ecx = B * ecx / divisor + eax clamped to the word +0xa, ebx = the
// word +4. The empty routine 0x00531b38 then runs with ebx + 1 and edx = 0x4d (bit 5 of +0x1f) or
// 0x41. The flags are those of `test byte [esi + 0x1f], 0x20`.
// FUNCTION: GP4 0x0053b298
void __cdecl FUN_0053b298(gp4::Regs* r) {
    const uint32_t record = r->esi;
    const uint32_t word4 = GP4_FIELD(uint16_t, record, 4);
    const uint32_t word6 = GP4_FIELD(uint16_t, record, 6);
    const uint32_t word8 = GP4_FIELD(uint16_t, record, 8);
    const uint32_t wordA = GP4_FIELD(uint16_t, record, 0xa);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = word4;
    c.eax = word6 - 2u;
    c.ebp = word8;
    c.ecx = wordA + 2u;
    gp4::cdecl_<0x0054a1d7, void>();
    gp4::call_regs(0x00549db9, c);
    c.edx = 0x60u;
    gp4::call_regs(0x00531b38, c);

    uint32_t eax = word6;
    uint32_t ebx = word4;
    uint32_t ecx = wordA;
    const uint32_t block = GP4_FIELD(uint32_t, record, 0xe);
    const uint32_t divisor = GP4_FIELD(uint32_t, block, 4);
    if (divisor != 0u) {
        ebx = eax;
        ecx = ecx - ebx + 1u;
        eax = Scaled(GP4_FIELD(uint32_t, block, 0), ecx, divisor) + ebx;
        ecx = Scaled(GP4_FIELD(uint32_t, block, 8), ecx, GP4_FIELD(uint32_t, block, 4));
        ecx += eax;
        if (static_cast<uint16_t>(ecx) > wordA) {
            ecx = wordA;
        }
        ebx = word4;
    }
    ebx += 1u;

    const uint32_t masked = GP4_FIELD(uint8_t, record, 0x1f) & 0x20u;
    const uint32_t arith = gp4::flags::logic8(masked);
    gp4::Regs d = *r;
    d.st_in = 0;
    d.st_out = 0;
    d.eax = eax;
    d.ebx = ebx;
    d.ecx = ecx;
    d.edx = masked != 0u ? 0x4du : 0x41u;
    d.ebp = word8;
    d.edi = block;
    d.eflags = gp4::flags::merge(r->eflags, arith);
    gp4::call_regs(0x00531b38, d);

    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053b298, FUN_0053b298, "regs(esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+list_00549db9+cell_0053b298")
