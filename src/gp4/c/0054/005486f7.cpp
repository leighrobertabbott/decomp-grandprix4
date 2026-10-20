#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x005486f7
// Sums per-car score words in pairs. The 22 per-car entries are visited in order (esi = 0..21):
// an entry whose state byte 0x007c2a74[esi] is 1 and whose car number (0x007c2a48[esi] & 0x3f) - 1
// is below 0x16 adds the word 0x007a93fa[car] to the pair's running sum (dx). Each of the 11
// pairs stores its sum in the word 0x007a9426[pair].
// Register interface: ebx keeps the last car-number based value computed (the caller's ebx when no
// entry qualified), esi ends at 0x16, edi one word past the sums, ecx 0, edx the last pair's sum.
// The flags that escape are those of the final `add edi, 2`.
void __cdecl FUN_005486f7(gp4::Regs* r) {
    uint32_t ebx = r->ebx;
    uint32_t esi = 0u;
    uint32_t edx = 0u;
    uint32_t edi = 0x007a9426u;
    uint32_t flags = 0u;

    for (uint32_t pair = 0; pair < 0x0bu; ++pair) {
        edx = 0u;
        for (uint32_t k = 0; k < 2u; ++k) {
            if (GP4_ARRAY(uint8_t, 0x007c2a74)[esi] == 1u) {
                ebx = static_cast<uint32_t>(GP4_ARRAY(uint8_t, 0x007c2a48)[esi]) & 0x3fu;
                ebx -= 1u;
                if (ebx < 0x16u) {
                    ebx = ebx * 2u + 0x007a93fau;
                    const uint16_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(ebx));
                    edx = (edx & 0xffff0000u) | ((edx + word) & 0xffffu);
                }
            }
            esi += 1u;
        }
        *reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(edi)) = static_cast<uint16_t>(edx);
        flags = gp4::flags::add32(edi, 2u);
        edi += 2u;
    }

    r->ebx = ebx;
    r->ecx = 0u;
    r->edx = edx;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005486f7, FUN_005486f7, "regs(ebx:u32) -> (ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=state_0040dfe9+tabs_0043ced2")
