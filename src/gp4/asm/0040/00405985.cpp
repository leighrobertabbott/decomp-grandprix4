#include <gp4/gp4.h>

static inline uint32_t comparisonFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// FUNCTION: GP4 0x00405985
// 16-bit fixed-point position split: ax = ((ax - base) * scale) with the top bits folded back
// onto base; the fraction bits of the product are added to cx and every 0x40 in cx carries one
// into ax. Only ax, cx and dx change (upper halves are preserved); ebx is saved and restored.
void __cdecl FUN_00405985(gp4::Regs* r) {
    const uint16_t base = GP4_GLOBAL(uint16_t, 0x0086856a);
    const uint16_t scale = GP4_GLOBAL(uint16_t, 0x00868568);

    const uint16_t shifted = static_cast<uint16_t>(static_cast<uint16_t>(r->eax) - base);
    const uint32_t product = static_cast<uint32_t>(shifted) * static_cast<uint32_t>(scale);  // mul word: dx:ax
    const uint16_t productLow = static_cast<uint16_t>(product);
    const uint16_t productHigh = static_cast<uint16_t>(product >> 16);

    // bx = (ax & 0x3fff) >> 8
    const uint16_t fraction = static_cast<uint16_t>((productLow & 0x3fffu) >> 8);
    // shl ax,1 / rcl dx,1 twice: dx = (dx << 2) | top two bits of ax
    const uint16_t folded = static_cast<uint16_t>((productHigh << 2) | ((productLow >> 14) & 3u));

    uint16_t whole = static_cast<uint16_t>(folded + base);
    uint16_t sub = static_cast<uint16_t>(static_cast<uint16_t>(r->ecx) + fraction);

    // cmp cx,0x40 / jl; sub cx,0x40 / add ax,1 until cx < 0x40 (signed)
    while (static_cast<int16_t>(sub) >= 0x40) {
        sub = static_cast<uint16_t>(sub - 0x40);
        whole = static_cast<uint16_t>(whole + 1u);
    }

    r->eax = (r->eax & 0xffff0000u) | whole;
    r->ecx = (r->ecx & 0xffff0000u) | sub;
    r->edx = (r->edx & 0xffff0000u) | folded;
    r->eflags = (r->eflags & ~0x8d5u) | comparisonFlags16(sub, 0x40u);
}
GP4_IMPL(0x00405985, FUN_00405985, "regs(eax:u32, ecx:u32, edx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
