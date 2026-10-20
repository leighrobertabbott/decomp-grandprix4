#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// The low dword of `imul b ; shrd eax, edx, 8` on the 64-bit product a * b.
static inline uint32_t shiftedProduct(int64_t product) {
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 8);
}

// Flags of `shrd eax, edx, 8` on the product (edx:eax): CF = last bit shifted out, AF = 0, OF from
// the pair shifted by 7 (the pinned oracle model).
static inline uint32_t shrdFlags8(int64_t product) {
    const uint64_t pair = static_cast<uint64_t>(product);
    const uint32_t low = static_cast<uint32_t>(pair);
    const uint32_t result = static_cast<uint32_t>(pair >> 8);
    const uint32_t previous = static_cast<uint32_t>(pair >> 7);
    return ((low >> 7) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((previous ^ result) >> 31) & 1u) << 11);
}

// The midpoint of an old and a new dword, `((new - old) >> 1) + old`, times the gain.
static inline int64_t midpointProduct(uint32_t previous, uint32_t current, int32_t gain) {
    const int32_t half = static_cast<int32_t>(current - previous) >> 1;
    const int32_t midpoint = static_cast<int32_t>(static_cast<uint32_t>(half) + previous);
    return static_cast<int64_t>(midpoint) * static_cast<int64_t>(gain);
}

// FUNCTION: GP4 0x0042027f
// Integrates one step with the time gain [0x006228ac]: the dword accumulator 0x00638050 advances by
// 0x00637f78 * gain >> 8 and the 64-bit counter 0x00638044:0x00638d58 by the midpoint of its old and
// new value times gain << 8; the vector 0x00623eb8 / ebc / ec0 advances by the velocities
// 0x006228d0 / d4 / d8 times gain >> 8 and is clamped to the length [0x006080d8] by 0x00402cd0. The
// midpoints of old and clamped vector times gain >> 8 go to 0x00622b40 / b44 / b48, and 0x00402ad1
// applies them; its register state and flags are the result (ebp keeps the old 0x00623eb8).
void __cdecl FUN_0042027f(gp4::Regs* r) {
    const int32_t gain = GP4_GLOBAL(int32_t, 0x006228ac);

    const uint32_t oldAccumulator = GP4_GLOBAL(uint32_t, 0x00638050);
    GP4_GLOBAL(uint32_t, 0x00638050) = oldAccumulator
        + shiftedProduct(static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00637f78)) * gain);

    const int64_t counterStep = midpointProduct(oldAccumulator, GP4_GLOBAL(uint32_t, 0x00638050), gain);
    const uint64_t counter = (static_cast<uint64_t>(GP4_GLOBAL(uint32_t, 0x00638044)) << 32)
        | GP4_GLOBAL(uint32_t, 0x00638d58);
    const uint64_t advanced = counter + (static_cast<uint64_t>(counterStep) << 8);
    GP4_GLOBAL(uint32_t, 0x00638d58) = static_cast<uint32_t>(advanced);
    GP4_GLOBAL(uint32_t, 0x00638044) = static_cast<uint32_t>(advanced >> 32);

    const uint32_t oldX = GP4_GLOBAL(uint32_t, 0x00623eb8);
    const uint32_t oldY = GP4_GLOBAL(uint32_t, 0x00623ebc);
    const uint32_t oldZ = GP4_GLOBAL(uint32_t, 0x00623ec0);
    GP4_GLOBAL(uint32_t, 0x00623eb8) = oldX
        + shiftedProduct(static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006228d0)) * gain);
    GP4_GLOBAL(uint32_t, 0x00623ebc) = oldY
        + shiftedProduct(static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006228d4)) * gain);
    GP4_GLOBAL(uint32_t, 0x00623ec0) = oldZ
        + shiftedProduct(static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006228d8)) * gain);

    gp4::Regs clamp = *r;
    clamp.st_in = 0;
    clamp.st_out = 0;
    clamp.eax = GP4_GLOBAL(uint32_t, 0x00623eb8);
    clamp.ebp = GP4_GLOBAL(uint32_t, 0x00623ebc);
    clamp.edx = GP4_GLOBAL(uint32_t, 0x00623ec0);
    clamp.ecx = GP4_GLOBAL(uint32_t, 0x006080d8);
    gp4::call_regs(0x00402cd0, clamp);
    GP4_GLOBAL(uint32_t, 0x00623eb8) = clamp.eax;
    GP4_GLOBAL(uint32_t, 0x00623ebc) = clamp.ebp;
    GP4_GLOBAL(uint32_t, 0x00623ec0) = clamp.edx;

    GP4_GLOBAL(uint32_t, 0x00622b48) =
        shiftedProduct(midpointProduct(oldZ, GP4_GLOBAL(uint32_t, 0x00623ec0), gain));
    GP4_GLOBAL(uint32_t, 0x00622b44) =
        shiftedProduct(midpointProduct(oldY, GP4_GLOBAL(uint32_t, 0x00623ebc), gain));
    const int64_t lastProduct = midpointProduct(oldX, GP4_GLOBAL(uint32_t, 0x00623eb8), gain);
    GP4_GLOBAL(uint32_t, 0x00622b40) = shiftedProduct(lastProduct);

    gp4::Regs apply = *r;
    apply.st_in = 0;
    apply.st_out = 0;
    apply.eax = shiftedProduct(lastProduct);
    apply.edx = static_cast<uint32_t>(static_cast<uint64_t>(lastProduct) >> 32);
    apply.ebp = oldX;
    apply.eflags = (r->eflags & ~0x8d5u) | shrdFlags8(lastProduct);
    gp4::call_regs(0x00402ad1, apply);

    r->eax = apply.eax;
    r->ebx = apply.ebx;
    r->ecx = apply.ecx;
    r->edx = apply.edx;
    r->ebp = apply.ebp;
    r->esi = apply.esi;
    r->edi = apply.edi;
    r->eflags = apply.eflags;
}
GP4_IMPL(0x0042027f, FUN_0042027f, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0042027f")
