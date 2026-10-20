#include <gp4/gp4.h>
#include <gp4/flags.h>

// Record behind [0x0083e69e]: a scale ratio (dword at +0 over dword at +4) and four words that
// bound a box (+8/+0xc and +0xa/+0xe).
struct ScaleBox {
    int32_t Numerator00;
    int32_t Denominator04;
    uint16_t Word08;
    uint16_t Word0a;
    uint16_t Word0c;
    uint16_t Word0e;
};

// `pushal ... popal`: queues the box in the sorted list (0x00549db9 with ebx = +8, ebp = +0xc,
// eax = +0xa, ecx = +0xe), then scales its width: with x0 = +8 + 0x14 and x1 = +0xc - 0x14,
// x = (x1 - x0 + 1) * Numerator / Denominator (0 for a zero denominator) + x0 - 1. If x >= x0 (signed)
// the value 0x1194 is passed to 0x00531af4 and the empty routine 0x00531b2e runs. popal restores
// every register; the flags are those of `cmp x, x0` when skipped, else those 0x00531af4 leaves.
// FUNCTION: GP4 0x0054fe11
void __cdecl FUN_0054fe11(gp4::Regs* r) {
    const ScaleBox* box = reinterpret_cast<const ScaleBox*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0083e69e)));
    const uint32_t left = box->Word08;
    const uint32_t right = box->Word0c;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = left;
    c.ebp = right;
    c.eax = box->Word0a;
    c.ecx = box->Word0e;
    gp4::call_regs(0x00549db9, c);

    gp4::Regs d = *r;
    d.st_in = 0;
    d.st_out = 0;
    d.edx = 0x5au;
    gp4::call_regs(0x00531b37, d);

    const uint32_t low = left + 0x10u + 4u;
    const uint32_t high = right - 0x10u - 4u;
    const int32_t width = static_cast<int32_t>(high - low + 1u);
    const int64_t product = static_cast<int64_t>(width) * box->Numerator00;

    int32_t scaled = 0;
    if (box->Denominator04 != 0) {
        int32_t remainder;
        scaled = gp4::idiv64(product, box->Denominator04, &remainder);
    }
    const uint32_t edge = static_cast<uint32_t>(scaled) + low - 1u;

    uint32_t flags = gp4::flags::sub32(edge, low);
    if (static_cast<int32_t>(edge) >= static_cast<int32_t>(low)) {
        gp4::Regs e = *r;
        e.st_in = 0;
        e.st_out = 0;
        e.eax = 0x1194u;
        gp4::call_regs(0x00531af4, e);
        flags = e.eflags;
        gp4::Regs f = *r;
        f.st_in = 0;
        f.st_out = 0;
        gp4::call_regs(0x00531b2e, f);
    }

    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054fe11, FUN_0054fe11, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+boxp_0054fe11")
