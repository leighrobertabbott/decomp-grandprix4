#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clamps two x values (ebx, ebp) into [w4, w8] and two y values (eax, ecx) into [w6, wA] of the
// rectangle at esi (unsigned words, signed comparisons). edx is preserved; the flags left behind
// are those of the final `cmp ecx, edx` (ecx already clamped to the lower bound).
// FUNCTION: GP4 0x00539d4a
void __cdecl FUN_00539d4a(gp4::Regs* r) {
    const uint8_t* rect = (const uint8_t*)r->esi;
    int32_t eax = (int32_t)r->eax;
    int32_t ebx = (int32_t)r->ebx;
    int32_t ecx = (int32_t)r->ecx;
    int32_t ebp = (int32_t)r->ebp;

    int32_t xmin = (int32_t)*(const uint16_t*)(rect + 4);
    int32_t xmax = (int32_t)*(const uint16_t*)(rect + 8);
    int32_t ymin = (int32_t)*(const uint16_t*)(rect + 6);
    int32_t ymax = (int32_t)*(const uint16_t*)(rect + 0xa);

    if (ebx < xmin) ebx = xmin;
    if (ebp < xmin) ebp = xmin;
    if (ebx > xmax) ebx = xmax;
    if (ebp > xmax) ebp = xmax;

    if (eax < ymin) eax = ymin;
    if (ecx < ymin) ecx = ymin;
    if (eax > ymax) eax = ymax;
    const uint32_t flags = gp4::flags::sub32((uint32_t)ecx, (uint32_t)ymax);
    if (ecx > ymax) ecx = ymax;

    r->eax = (uint32_t)eax;
    r->ebx = (uint32_t)ebx;
    r->ecx = (uint32_t)ecx;
    r->ebp = (uint32_t)ebp;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00539d4a, FUN_00539d4a, "regs(eax:i32[-300..0x3000], ebx:i32[-300..0x3000], ecx:i32[-300..0x3000], ebp:i32[-300..0x3000], esi:ptr[0x40]:u16[0..0x1800]) -> (eax, ebx, ecx, ebp, cf, pf, af, zf, sf, of)")
