#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `left - right` (sub / cmp / neg when left is 0).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Calls one of the interpolation routines 0x00403b19 / 0x00403b3f (edi = block, edx out, flags out).
static inline uint32_t interpolate(uintptr_t routine, const gp4::Regs* r, uint32_t* flags) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(routine, call);
    *flags = call.eflags;
    return call.edx;
}

// FUNCTION: GP4 0x00403938
// eax = value, edx = mode selector (0..7), edi = block pointer. Mode 2 / 3 average the value with
// the block's interpolation (0x00403b19 / 0x00403b3f) and keep the larger / smaller of the average
// and the value -/+ the word at 0x00603baa (mode 3 then falls through the 4..7 tests with edx as
// the callee left it); mode 4 / 5 return -/+ the interpolation; mode 6 returns the value clamped
// against it; mode 7 returns interpolation + 0x150 - the position from 0x00403800. edx and ebp
// are preserved; the flags are those of the last flag writer.
void __cdecl FUN_00403938(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t mode = r->edx;
    uint32_t flags = subFlags32(mode, 0);
    uint32_t calleeFlags;

    do {
        if (mode == 0) {
            break;
        }
        flags = subFlags32(mode, 1);
        if (mode == 1) {
            break;
        }
        flags = subFlags32(mode, 2);
        if (mode == 2) {
            const uint32_t ebp = eax - GP4_GLOBAL(uint16_t, 0x00603baa);
            const uint32_t d = interpolate(0x00403b19, r, &calleeFlags);
            uint32_t half = static_cast<uint32_t>(static_cast<int32_t>(eax - d) >> 1);
            flags = subFlags32(half, ebp);
            if (static_cast<int32_t>(half) < static_cast<int32_t>(ebp)) {
                half = ebp;
            }
            eax = half;
            break;
        }
        flags = subFlags32(mode, 3);
        if (mode == 3) {
            const uint32_t ebp = eax + GP4_GLOBAL(uint16_t, 0x00603baa);
            const uint32_t d = interpolate(0x00403b3f, r, &calleeFlags);
            uint32_t half = static_cast<uint32_t>(static_cast<int32_t>(eax + d) >> 1);
            if (static_cast<int32_t>(half) > static_cast<int32_t>(ebp)) {
                half = ebp;
            }
            eax = half;
            mode = d;
        }
        flags = subFlags32(mode, 4);
        if (mode == 4) {
            const uint32_t d = interpolate(0x00403b19, r, &calleeFlags);
            eax = 0u - d;
            flags = subFlags32(0u, d);
            break;
        }
        flags = subFlags32(mode, 5);
        if (mode == 5) {
            eax = interpolate(0x00403b3f, r, &calleeFlags);
            flags = calleeFlags;
            break;
        }
        flags = subFlags32(mode, 7);
        if (mode == 7) {
            gp4::Regs call = *r;
            call.st_in = 0;
            call.st_out = 0;
            gp4::call_regs(0x00403800, call);
            const uint32_t position = call.eax;
            const uint32_t sign = GP4_GLOBAL(uint8_t, 0x00868582);
            if ((sign & 0x80u) != 0u) {
                const uint32_t d = interpolate(0x00403b19, r, &calleeFlags);
                const uint32_t t = d + 0x150u;
                const uint32_t v = t - position;
                eax = 0u - v;
                flags = subFlags32(0u, v);
            } else {
                const uint32_t d = interpolate(0x00403b3f, r, &calleeFlags);
                const uint32_t t = d + 0x150u;
                eax = t - position;
                flags = subFlags32(t, position);
            }
            break;
        }
        flags = subFlags32(mode, 6);
        if (mode == 6) {
            const uint32_t negated = 0u - eax;
            flags = subFlags32(0u, eax);
            if ((negated & 0x80000000u) == 0u) {
                const uint32_t d = interpolate(0x00403b3f, r, &calleeFlags);
                flags = subFlags32(negated, d);
                eax = (static_cast<int32_t>(negated) > static_cast<int32_t>(d)) ? d : negated;
            } else {
                const uint32_t d = interpolate(0x00403b19, r, &calleeFlags);
                flags = subFlags32(eax, d);
                const uint32_t m = (static_cast<int32_t>(eax) > static_cast<int32_t>(d)) ? d : eax;
                eax = 0u - m;
                flags = subFlags32(0u, m);
            }
        }
    } while (0);

    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00403938, FUN_00403938, "regs(eax:i32, edx:u32[0..9], edi:ptr[0x1ca]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800")
