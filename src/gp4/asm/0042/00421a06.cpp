#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `test value, value` (or a test against 0xffffffff).
static inline uint32_t testFlags32(uint32_t value) {
    return parityFlag8(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

static inline void setFlags(gp4::Regs& st, uint32_t flags) {
    st.eflags = (st.eflags & ~0x8d5u) | flags;
}

// FUNCTION: GP4 0x00421a06
// Runs one pass of the render/projection pipeline for the object at esi (pushal/popal: every register is
// preserved, the flags are those of the last flag writer). eax / edx are the two stage callbacks (stored at
// 0x607fb0 / 0x607fb4), ebp the body pointer (stored at 0x6097b0, +0x38 at 0x6097ac) and ecx the numerator
// of the step count [0x607fa4] = (ecx + [0x608164] / 2) / max([0x608164], 1). The pass is skipped when the
// mode byte [0x60827d] is zero, [0x60827c] == 2 and the object's dword +0x74 is -1. Every callee is run
// with the register file (and the flags of the last compare) the original leaves it.
void __cdecl FUN_00421a06(gp4::Regs* r) {
    gp4::Regs st = *r;
    st.st_in = 0;
    st.st_out = 0;

    GP4_GLOBAL(uint32_t, 0x00609744) = r->esi;
    GP4_GLOBAL(uint8_t, 0x00608280) = 0;
    GP4_GLOBAL(uint32_t, 0x006080bc) = 0x4000u;
    GP4_GLOBAL(uint32_t, 0x006080c4) = 0u;
    gp4::call_regs(0x00421eed, st);
    gp4::call_regs(0x00421ef8, st);

    if (GP4_GLOBAL(uint8_t, 0x0060827d) == 0u && GP4_GLOBAL(uint8_t, 0x0060827c) == 2u
        && GP4_FIELD(uint32_t, st.esi, 0x74) == 0xffffffffu) {
        // 0x421b32: popal; the flags are those of `cmp dword [esi+0x74], -1` with equal operands
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    GP4_GLOBAL(uint32_t, 0x00607fb0) = st.eax;
    GP4_GLOBAL(uint32_t, 0x00607fb4) = st.edx;
    GP4_GLOBAL(uint32_t, 0x006097b0) = st.ebp;
    GP4_GLOBAL(uint32_t, 0x006097ac) = st.ebp + 0x38u;

    const uint32_t extent = GP4_GLOBAL(uint32_t, 0x00608164);
    const int64_t numerator = static_cast<int64_t>(static_cast<int32_t>(st.ecx + (extent >> 1)));
    const int32_t divisor = extent != 0u ? static_cast<int32_t>(extent) : 1;
    int32_t remainder;
    const int32_t steps = gp4::idiv64(numerator, divisor, &remainder);
    st.eax = static_cast<uint32_t>(steps);
    st.edx = static_cast<uint32_t>(remainder);
    st.ecx = static_cast<uint32_t>(divisor);
    GP4_GLOBAL(int32_t, 0x00607fa4) = steps;

    setFlags(st, testFlags32(extent));
    gp4::call_regs(0x0042192d, st);
    gp4::call_regs(GP4_GLOBAL(uint32_t, 0x00607fb0), st);
    gp4::call_regs(0x0040200c, st);
    gp4::call_regs(0x004250f1, st);
    GP4_GLOBAL(uint32_t, 0x00607fac) = 0u;
    gp4::call_regs(0x0042194a, st);
    if (GP4_GLOBAL(uint8_t, 0x006259d8) == 0xffu) {
        GP4_GLOBAL(uint32_t, 0x00607fdc) = 0u;
        GP4_GLOBAL(uint32_t, 0x00607fe0) = 0u;
        GP4_GLOBAL(uint32_t, 0x00607fe4) = 0u;
    }
    st.eax = GP4_GLOBAL(uint32_t, 0x00608164);
    GP4_GLOBAL(uint32_t, 0x00607fa0) = st.eax;
    setFlags(st, testFlags32(GP4_GLOBAL(uint32_t, 0x00607fa4)));
    if (GP4_GLOBAL(uint32_t, 0x00607fa4) != 0u) {
        gp4::call_regs(0x0040200c, st);
        gp4::call_regs(0x00422bc9, st);
        GP4_GLOBAL(uint8_t, 0x0060827c) = 3;
        gp4::call_regs(0x004240ff, st);
    }
    gp4::call_regs(0x00421b34, st);
    uint32_t flags = cmpFlags8(GP4_GLOBAL(uint8_t, 0x00608280), 0u);
    if (GP4_GLOBAL(uint8_t, 0x00608280) == 0u) {
        flags = cmpFlags8(GP4_GLOBAL(uint8_t, 0x0060827c), 1u);
        if (GP4_GLOBAL(uint8_t, 0x0060827c) == 1u) {
            GP4_GLOBAL(uint8_t, 0x0060827c) = 2;
        }
    }
    setFlags(st, flags);
    gp4::call_regs(0x0040200c, st);
    gp4::call_regs(GP4_GLOBAL(uint32_t, 0x00607fb4), st);
    r->eflags = (r->eflags & ~0x8d5u) | (st.eflags & 0x8d5u);
}
GP4_IMPL(0x00421a06, FUN_00421a06, "regs(eax:u32[0x42429d..0x42429d], ecx:u32[0..0x100], edx:u32[0x42430b..0x42430b], ebp:ptr[0x5d8]:layout=blk_00421a06, esi:ptr[0x7c]:layout=rec_00421a06) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=view_00609860+state_0042430b+state_00422ac2+state_00424011+state_00422397+recs_00621aa0+state_004250f1+state_00422bc9+state_00421fc3+state_0042320a+divide_00609834+state_004240ff+gain_004240ff+state_00421a06")
