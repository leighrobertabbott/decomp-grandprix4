#include <gp4/gp4.h>
#include <gp4/x87.h>

// Running state of the original's subtraction chain: the last `sub` decides
// eax (on the clc exit) and every arithmetic flag except CF at return.
struct SubState {
    uint32_t eax;
    uint32_t flags;   // CF/PF/AF/ZF/SF/OF of the last 32-bit sub
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// eax = a - b; returns SF of the 32-bit result.
static inline bool subSign(SubState& st, uint32_t a, uint32_t b) {
    const uint32_t res = a - b;
    st.eax = res;
    st.flags = (a < b ? 0x01u : 0u)
        | parityFlag(res)
        | (((a ^ b ^ res) & 0x10u) != 0u ? 0x10u : 0u)
        | (res == 0u ? 0x40u : 0u)
        | ((res & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((a ^ b) & (a ^ res) & 0x80000000u) != 0u ? 0x800u : 0u);
    return (res & 0x80000000u) != 0u;
}

// Circular (wrapping 32-bit angle) test: is v inside the arc [lo, hi)?
// Evaluated in the original's order: hi - lo, then v - lo, then v - hi.
static inline bool inArc(SubState& st, uint32_t v, uint32_t lo, uint32_t hi) {
    if (!subSign(st, hi, lo)) {
        return !subSign(st, v, lo) && subSign(st, v, hi);
    }
    return !subSign(st, v, lo) || subSign(st, v, hi);
}

// fistp dword on an 80-bit value. The hardware helper takes a double, so the
// extended value is rounded to double first; the dropped low bits only matter
// when the double lands exactly on a rounding boundary of the current mode.
static inline int32_t fistp32Ext(const gp4::x87::Ext& x) {
    const double d = gp4::x87::narrow(x);
    const int32_t n = gp4::x87::fistp32(d);
    const gp4::x87::Ext rest = x - gp4::x87::ext(d);   // exact
    bool restZero = true;
    for (int i = 0; i < 8; ++i) {
        if (rest.b[i] != 0) restZero = false;
    }
    if (restZero || !(d > -4294967296.0 && d < 4294967296.0)) return n;
    const bool restNeg = (rest.b[9] & 0x80u) != 0;
    const double twiceD = d * 2.0;
    const int64_t twice = gp4::x87::fistp64(twiceD);
    if ((double)twice != twiceD) return n;   // not on a half/whole integer
    const uint32_t rc = (gp4::x87::get_cw() >> 10) & 3u;
    int64_t k;
    if ((twice & 1) != 0) {
        if (rc != 0u) return n;               // only round-to-nearest splits at .5
        k = ((twice - 1) >> 1) + (restNeg ? 0 : 1);   // floor(d)
    } else {
        const int64_t m = twice >> 1;                // d itself
        if (rc == 1u && restNeg) k = m - 1;
        else if (rc == 2u && !restNeg) k = m + 1;
        else if (rc == 3u && m > 0 && restNeg) k = m - 1;
        else if (rc == 3u && m < 0 && !restNeg) k = m + 1;
        else return n;
    }
    if (k < -2147483647LL - 1 || k > 2147483647LL) return (int32_t)0x80000000u;
    return (int32_t)k;
}

// Angle (fixed-point, scaled by [0x6f2be4]) from the record's point to the
// global point, through the scratch slot 0x6f2bf4 like the original.
static inline int32_t angleTo(int16_t coarseA, int32_t fineA, int16_t coarseB, int32_t fineB) {
    const uint32_t dy = GP4_GLOBAL(uint32_t, 0x006278c0)
        - (((uint32_t)(int32_t)coarseA << 8) + (uint32_t)fineA);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = (int32_t)dy;
    const gp4::x87::Ext y = gp4::x87::ext((double)GP4_GLOBAL(int32_t, 0x006f2bf4));
    const uint32_t dx = GP4_GLOBAL(uint32_t, 0x006278c8)
        - (((uint32_t)(int32_t)coarseB << 8) + (uint32_t)fineB);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = (int32_t)dx;
    const gp4::x87::Ext x = gp4::x87::ext((double)GP4_GLOBAL(int32_t, 0x006f2bf4));
    const gp4::x87::Ext scaled = gp4::x87::fpatan(y, x)
        * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2be4));
    const int32_t angle = fistp32Ext(scaled);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = angle;
    return angle;
}

// FUNCTION: GP4 0x004067ea
void __cdecl FUN_004067ea(gp4::Regs* r) {
    uint8_t* rec = (uint8_t*)r->edi;
    const int32_t base0f0 = GP4_FIELD(int32_t, rec, 0xf0);
    const int32_t base0f8 = GP4_FIELD(int32_t, rec, 0xf8);

    const int32_t angleA = angleTo(GP4_FIELD(int16_t, rec, 0xd4), base0f0,
                                   GP4_FIELD(int16_t, rec, 0xdc), base0f8);
    GP4_GLOBAL(int32_t, 0x006278f8) = angleA;
    const int32_t angleB = angleTo(GP4_FIELD(int16_t, rec, 0xd6), base0f0,
                                   GP4_FIELD(int16_t, rec, 0xde), base0f8);
    GP4_GLOBAL(int32_t, 0x006278fc) = angleB;

    const uint32_t a = (uint32_t)angleA;
    const uint32_t b = (uint32_t)angleB;
    const uint32_t limitA = (uint32_t)(int32_t)GP4_FIELD(int16_t, rec, 0x128) << 16;
    const uint32_t limitB = (uint32_t)(int32_t)GP4_FIELD(int16_t, rec, 0x12a) << 16;
    const uint32_t heading = GP4_FIELD(uint32_t, rec, 0xec);
    const uint32_t arcA104 = GP4_FIELD(uint32_t, rec, 0x104);
    const uint32_t arcB110 = GP4_FIELD(uint32_t, rec, 0x110);

    SubState st;
    uint32_t ecx = heading + 0x40000000u;
    uint32_t edx = limitA;
    bool carry = true;
    uint32_t result;

    if (inArc(st, a, limitA, heading + 0x40000000u)
        && (ecx = heading - 0x40000000u, edx = limitB,
            inArc(st, b, heading - 0x40000000u, limitB))) {
        result = 0u;
    } else if (edx = limitA, inArc(st, a, arcA104, limitA)) {
        result = 0xffffffffu;
    } else if (edx = limitB, inArc(st, b, limitB, arcB110)) {
        result = 1u;
    } else {
        result = st.eax;
        carry = false;
    }

    r->eax = result;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | (st.flags & ~0x01u) | (carry ? 0x01u : 0u);
}
GP4_IMPL(0x004067ea, FUN_004067ea, "regs(edi:ptr[0x160]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")
