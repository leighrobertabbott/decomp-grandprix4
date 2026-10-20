#include <gp4/gp4.h>
#include <gp4/x87.h>

// Keeps the compiler from folding state-relative offsets into absolute globals.
static __declspec(noinline) uint8_t* StateBase() { return GP4_ARRAY(uint8_t, 0x006380d8); }

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "test reg, reg" (logic: CF = OF = AF = 0)
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

static inline int32_t absValue(int32_t v) {
    return v < 0 ? static_cast<int32_t>(0u - static_cast<uint32_t>(v)) : v;
}

// FUNCTION: GP4 0x00419220
// Weighted average of two rounded products (|dword * qword| pairs from the
// 0x006380d8 state) weighted by the dwords at esi+0x2f4 / esi+0x2f8; the divisor
// is replaced by 1 when the weights sum to zero. Result stored at 0x0060351c.
void __cdecl FUN_00419220(gp4::Regs* r) {
    uint8_t* const self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const state = StateBase();

    GP4_GLOBAL(int32_t, 0x006f2bf4) = gp4::x87::fistp32(
        gp4::x87::ext_i32(GP4_FIELD(int32_t, state, 0x86c)) * gp4::x87::ext(GP4_FIELD(double, state, 0x880)));
    const int32_t first = absValue(GP4_GLOBAL(int32_t, 0x006f2bf4));

    GP4_GLOBAL(int32_t, 0x006f2bf4) = gp4::x87::fistp32(
        gp4::x87::ext_i32(GP4_FIELD(int32_t, state, 0xb6c)) * gp4::x87::ext(GP4_FIELD(double, state, 0xb80)));
    const int32_t second = absValue(GP4_GLOBAL(int32_t, 0x006f2bf4));

    const int32_t weightA = GP4_FIELD(int32_t, self, 0x2f4);
    const int32_t weightB = GP4_FIELD(int32_t, self, 0x2f8);
    const uint64_t sum = static_cast<uint64_t>(static_cast<int64_t>(weightA) * first)
        + static_cast<uint64_t>(static_cast<int64_t>(weightB) * second);

    uint32_t divisor = static_cast<uint32_t>(weightA) + static_cast<uint32_t>(weightB);
    const uint32_t flags = logicFlags(divisor);
    if (divisor == 0u) divisor = 1u;

    int32_t remainder = 0;
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(sum), static_cast<int32_t>(divisor), &remainder);
    GP4_GLOBAL(int32_t, 0x0060351c) = quotient;

    r->eax = static_cast<uint32_t>(quotient);
    r->edx = static_cast<uint32_t>(remainder);
    r->ebp = divisor;
    // IDIV leaves the flags of the preceding TEST EBP,EBP (pinned oracle model).
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00419220, FUN_00419220, "regs(esi:ptr[0x2fc]:layout=rec_00419220) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00419220")
