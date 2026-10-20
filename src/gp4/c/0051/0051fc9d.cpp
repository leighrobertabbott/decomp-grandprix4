#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scales the six signed words at 0x00852dd4 by the 16.16 factor [0x006f2e90]: each word becomes the
// low half of (word * factor) >> 16 (imul to edx:eax, then `shrd eax, edx, 16`) and is stored at the
// matching word of 0x00852de0. Leaves ebx = 0x00852de0, ecx = 0x00852dec, eax = the last shrd result,
// edx = the high half of the last product; the flags are those of the final `cmp ebx, 0x852de0`
// (equal).
// FUNCTION: GP4 0x0051fc9d
void __cdecl FUN_0051fc9d(gp4::Regs* r) {
    const int16_t* const src = GP4_ARRAY(const int16_t, 0x00852dd4);
    uint16_t* const dst = GP4_ARRAY(uint16_t, 0x00852de0);
    uint32_t eax = 0u;
    uint32_t edx = 0u;
    for (uint32_t i = 0u; i < 6u; ++i) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(src[i])) *
                                static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006f2e90));
        const uint32_t low = static_cast<uint32_t>(product);
        edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        eax = (low >> 16) | (edx << 16);
        dst[i] = static_cast<uint16_t>(eax);
    }
    r->eax = eax;
    r->ebx = 0x00852de0u;
    r->ecx = 0x00852decu;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x00852de0u, 0x00852de0u));
}
GP4_IMPL(0x0051fc9d, FUN_0051fc9d, "regs() -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
