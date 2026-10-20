#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of a byte xor/test result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

static const uint32_t kStc = 1u;

// FUNCTION: GP4 0x0042d4e6
// ecx = byte offset into the source record edi (0x10 or another entry of the same table), esi / ebx =
// the two bodies. Returns with CF set when the source passes the checks (a sign shortcut, or the two
// weighted comparisons fail) and CF clear when it fails the last ones; eax is saved and restored,
// ebp and edx hold the last products, the flags are those of the final cmp with CF forced by clc/stc.
void __cdecl FUN_0042d4e6(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t ecx = r->ecx;

    uint8_t sign = GP4_FIELD(uint8_t, source, 0x10);
    if (ecx != 0x10u) {
        sign = static_cast<uint8_t>(sign ^ 0x80u);
    }
    GP4_GLOBAL(uint8_t, 0x00604154) = sign;

    if ((GP4_FIELD(uint8_t, other, 0x71) & 0x80u) != 0u && (GP4_FIELD(uint8_t, source, 0x64) & 0x80u) != 0u) {
        const uint8_t shortcut = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00604154) ^ GP4_GLOBAL(uint8_t, 0x00868582));
        if ((shortcut & 0x80u) == 0u) {
            r->edx = (r->edx & 0xffffff00u) | shortcut;
            r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(shortcut) | kStc;
            return;
        }
    }

    // Base weight: the 7-bit table entry * 0x40, plus up to four steps of [0x603bb8] when the
    // signed byte +0xa9 agrees in sign with the direction byte.
    const uint16_t pending = GP4_GLOBAL(uint16_t, 0x00603bfa);
    GP4_GLOBAL(uint16_t, 0x00603bf8) = pending;
    uint32_t weight = (GP4_FIELD(uint8_t, source, ecx) & 0x7fu) << 6;
    uint32_t edx = (r->edx & 0xffff0000u) | pending;
    const uint8_t extra = GP4_FIELD(uint8_t, source, 0xa9);
    edx = (edx & 0xffffff00u) | extra;
    if (extra != 0u) {
        const uint8_t agree = static_cast<uint8_t>(extra ^ sign);
        edx = (edx & 0xffffff00u) | agree;
        if ((agree & 0x80u) == 0u) {
            uint8_t magnitude = extra;
            if ((magnitude & 0x80u) != 0u) {
                magnitude = static_cast<uint8_t>(0u - magnitude);
            }
            int32_t steps = magnitude;
            if (steps > 4) {
                steps = 4;
            }
            const int64_t scaled = static_cast<int64_t>(steps << 6) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603bb8));
            GP4_GLOBAL(uint16_t, 0x00603bf8) = GP4_GLOBAL(uint16_t, 0x00603bfa);
            edx = weight;
            weight = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14) + weight;
        }
    }
    uint32_t ebp = weight;

    // First comparison: (distance of the bodies) * [0x603bdc] >> 14 against the weight.
    uint16_t ax;
    if ((GP4_FIELD(uint8_t, self, 0x9c) & 2u) == 0u && (GP4_FIELD(uint8_t, other, 0x9c) & 2u) == 0u) {
        ax = static_cast<uint16_t>(GP4_FIELD(uint16_t, other, 0xe2) - GP4_FIELD(uint16_t, self, 0xe2));
    } else {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.eax = weight;
        call.edx = edx;
        call.ebp = ebp;
        gp4::call_regs(0x0040dc8a, call);
        ax = static_cast<uint16_t>(call.eax);
    }
    int64_t product = static_cast<int64_t>(static_cast<int32_t>(static_cast<int16_t>(ax))) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603bdc));
    uint32_t eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    if (static_cast<int32_t>(eax) >= static_cast<int32_t>(ebp)) {
        r->ebp = ebp;
        r->edx = edx;
        r->eflags = (r->eflags & ~0x8d5u) | (cmpFlags32(eax, ebp) | kStc);
        return;
    }

    // Second comparison: the closing speed product against the same weight.
    const int32_t selfSpeed = static_cast<int16_t>(GP4_FIELD(uint16_t, self, 0x9a));
    const uint32_t factor = ebp - static_cast<uint32_t>(selfSpeed >> 5);
    uint16_t ax2 = static_cast<uint16_t>(GP4_FIELD(uint16_t, self, 0x9a) - GP4_FIELD(uint16_t, other, 0x9a));
    ax2 = static_cast<uint16_t>(ax2 + GP4_GLOBAL(uint16_t, 0x00603bf8));
    product = static_cast<int64_t>(static_cast<int32_t>(static_cast<int16_t>(ax2))) * static_cast<int64_t>(static_cast<int32_t>(factor));
    ebp = static_cast<uint32_t>(product);
    const uint16_t ax3 = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00603e70) - GP4_GLOBAL(uint16_t, 0x00603b9e));
    product = static_cast<int64_t>(static_cast<int32_t>(static_cast<int16_t>(ax3))) * static_cast<int64_t>(selfSpeed);
    eax = static_cast<uint32_t>(product);
    edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    if (static_cast<int32_t>(eax) >= static_cast<int32_t>(ebp)) {
        r->ebp = ebp;
        r->edx = edx;
        r->eflags = (r->eflags & ~0x8d5u) | (cmpFlags32(eax, ebp) | kStc);
        return;
    }

    // Third comparison: the entry of the record `index` places further in the ring (index = the
    // 7-bit table entry, below [0x603be0]) against that index.
    if ((GP4_FIELD(uint8_t, source, 0x1b) & 0x20u) != 0u) {
        r->ebp = ebp;
        r->edx = edx;
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(0x20u);
        return;
    }
    const uint32_t index = GP4_FIELD(uint8_t, source, ecx) & 0x7fu;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00603be0);
    if (static_cast<int32_t>(index) >= static_cast<int32_t>(limit)) {
        r->ebp = ebp;
        r->edx = edx;
        r->eflags = (r->eflags & ~0x8d5u) | (cmpFlags32(index, limit) & ~kStc);
        return;
    }
    ebp = index;
    uint8_t* entry = source;
    const uint32_t offset = index * 0x160u;
    if (reinterpret_cast<uintptr_t>(source) < GP4_GLOBAL(uint32_t, 0x00891500)) {
        entry += offset;
        if (!(reinterpret_cast<uintptr_t>(entry) < GP4_GLOBAL(uint32_t, 0x00866ecc))) {
            entry -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    } else {
        entry += offset;
        if (!(reinterpret_cast<uintptr_t>(entry) < GP4_GLOBAL(uint32_t, 0x00866ef8))) {
            entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u));
        }
    }
    product = static_cast<int64_t>(static_cast<int32_t>(GP4_FIELD(uint8_t, entry, 0x10) & 0x7fu)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603be4));
    eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    uint32_t flags = cmpFlags32(eax, ebp);
    flags = static_cast<int32_t>(eax) >= static_cast<int32_t>(ebp) ? (flags | kStc) : (flags & ~kStc);
    r->ebp = ebp;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042d4e6, FUN_0042d4e6, "regs(ecx:i32[8..24], edx:u32, edi:ptr[0xb1]:bytes, esi:ptr[0x225]:bytes, ebx:ptr[0x225]:bytes) -> (edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0")
