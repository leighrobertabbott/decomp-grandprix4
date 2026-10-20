#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t flags = parityFlag(result);
    if (result < left) flags |= 0x001u;
    if (((left ^ right ^ result) & 0x10u) != 0u) flags |= 0x010u;
    if (result == 0u) flags |= 0x040u;
    if ((result & 0x80000000u) != 0u) flags |= 0x080u;
    if ((((left ^ result) & (right ^ result)) & 0x80000000u) != 0u) flags |= 0x800u;
    return flags;
}

// FUNCTION: GP4 0x0040342a
// Samples the record at [0x006278d0] twice (the record itself and the one 0x160 bytes further):
// with edx == 3 through 0x0040ce7b, otherwise through 0x0040ceb0 (both take the record in edi and
// return eax). The samples go to 0x006278a4 / 0x006278a8, and the result is
// a + (((b - a) * [0x006278b4]) >> 14) with the 64-bit product shifted by SHRD. Returns eax = result
// and edx = [0x006278d0]; edi is preserved. The flags are those of the final add.
void __cdecl FUN_0040342a(gp4::Regs* r) {
    const uint32_t mode = r->edx;
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x006278d0);

    gp4::Regs call = *r;
    if (mode == 3u) {
        call.edi = record;
        gp4::call_regs(0x0040ce7b, call);
        GP4_GLOBAL(uint32_t, 0x006278a4) = call.eax;
        call.edi = record + 0x160u;
        gp4::call_regs(0x0040ce7b, call);
        GP4_GLOBAL(uint32_t, 0x006278a8) = call.eax;
    } else {
        call.edi = record;
        gp4::call_regs(0x0040ceb0, call);
        GP4_GLOBAL(uint32_t, 0x006278a4) = call.eax;
        call.edi = record + 0x160u;
        gp4::call_regs(0x0040ceb0, call);
        GP4_GLOBAL(uint32_t, 0x006278a8) = call.eax;
    }

    const uint32_t first = GP4_GLOBAL(uint32_t, 0x006278a4);
    const int32_t difference = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006278a8) - first);
    const int64_t product = static_cast<int64_t>(difference) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006278b4));
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t shifted = (low >> 14) | (high << 18);

    r->eax = shifted + first;
    r->edx = record;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(shifted, first);
}
GP4_IMPL(0x0040342a, FUN_0040342a, "regs(edx:u32[0..4]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0040342a")
