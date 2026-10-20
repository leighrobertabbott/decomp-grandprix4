#include <gp4/gp4.h>
#include <gp4/flags.h>

// Snapshots four values of the car/record at esi into globals: 0x00865fc8 = result of 0x0040832b
// (max(0, byte[+0x198] - byte[+0x136])), 0x0086609c = byte [+0x7f], 0x008660f4 = word [+0x60] >> 6,
// 0x00865fc4 = 1 when bit 5 of byte [+0x8d] is set. eax ends as that last 0/1; the flags are those of
// "test byte [esi+0x8d],0x20". esi and the other registers are unchanged.
// FUNCTION: GP4 0x005159a2
void __cdecl FUN_005159a2(gp4::Regs* r) {
    const uint8_t* record = (const uint8_t*)r->esi;

    gp4::Regs call = *r;
    gp4::call_regs(0x0040832b, call);
    GP4_GLOBAL(uint32_t, 0x00865fc8) = call.eax;

    GP4_GLOBAL(uint32_t, 0x0086609c) = record[0x7f];
    GP4_GLOBAL(uint32_t, 0x008660f4) = (uint32_t)(*(const uint16_t*)(record + 0x60) >> 6);

    const uint8_t flagsByte = record[0x8d];
    const uint32_t bit = (flagsByte & 0x20u) != 0u ? 1u : 0u;
    GP4_GLOBAL(uint32_t, 0x00865fc4) = bit;

    r->eax = bit;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flagsByte & 0x20u));
}
GP4_IMPL(0x005159a2, FUN_005159a2, "regs(esi:ptr[0x1a0]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
