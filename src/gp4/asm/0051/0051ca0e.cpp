#include <gp4/gp4.h>
#include <gp4/flags.h>

// Mirrors a 256 x 256 byte map row by row into another one: the pointer table at 0x00811928
// (4 entries) gives the source map (index edx) and the destination map (index eax). Each of the 256
// rows is read forward from the source and written backward into the destination row, so every row
// comes out reversed. Leaves esi = source end, edi = destination end, ecx = ebp = 0, al = the last
// byte read, and the flags of the final `dec ebp` (1 -> 0) with CF kept from the last
// `add edi, 0x100`.
// FUNCTION: GP4 0x0051ca0e
void __cdecl FUN_0051ca0e(gp4::Regs* r) {
    const uint32_t* const maps = GP4_ARRAY(const uint32_t, 0x00811928);
    const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(maps[r->edx]));
    uint32_t rowEnd = maps[r->eax];
    uint32_t carry = 0u;
    uint8_t last = static_cast<uint8_t>(r->eax);
    for (uint32_t row = 0x100u; row != 0u; --row) {
        carry = gp4::flags::add32(rowEnd, 0x100u);
        rowEnd += 0x100u;
        uint8_t* target = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(rowEnd));
        for (uint32_t column = 0x100u; column != 0u; --column) {
            last = *source++;
            *--target = last;
        }
    }
    r->eax = (r->eax & 0xffffff00u) | last;
    r->ecx = 0u;
    r->ebp = 0u;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(source));
    r->edi = rowEnd;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, carry));
}
GP4_IMPL(0x0051ca0e, FUN_0051ca0e, "regs(eax:u32[0..3], edx:u32[0..3]) -> (eax, ecx, ebp, esi, edi, cf, pf, af, zf, sf, of); globals=layout=buf_0051c9cb; wall=1800")
