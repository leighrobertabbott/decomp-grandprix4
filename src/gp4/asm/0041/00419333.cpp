#include <gp4/gp4.h>

static const uint32_t kPF = 1u << 2;
static const uint32_t kAF = 1u << 4;
static const uint32_t kOF = 1u << 11;
static const uint32_t kArithFlags = 0x8d5u;  // CF|PF|AF|ZF|SF|OF

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? kPF : 0u;
}

struct Clamped {
    uint32_t value;
    uint32_t flags;  // arithmetic flags left by the last flag writer (neg, or cmp when value >= 0)
};

// Limits the magnitude of the signed value to `limit` (unsigned compare):
//   value >= 0: cmp value, limit; jb keep; mov value, limit
//   value <  0: neg; cmp m, limit; jb keep; mov m, limit; neg
static inline Clamped clampMagnitude(uint32_t value, uint32_t limit) {
    Clamped c;
    if (static_cast<int32_t>(value) < 0) {
        uint32_t m = 0u - value;
        if (m >= limit) {
            m = limit;
        }
        const uint32_t result = 0u - m;
        c.value = result;
        c.flags = (m != 0u ? gp4::CF : 0u) | parityFlag(result) | (((m ^ result) & 0x10u) != 0u ? kAF : 0u)
            | (result == 0u ? gp4::ZF : 0u) | ((result >> 31) != 0u ? gp4::SF : 0u)
            | (m == 0x80000000u ? kOF : 0u);
    } else {
        const uint32_t diff = value - limit;
        c.value = value < limit ? value : limit;
        c.flags = (value < limit ? gp4::CF : 0u) | parityFlag(diff)
            | (((value ^ limit ^ diff) & 0x10u) != 0u ? kAF : 0u) | (diff == 0u ? gp4::ZF : 0u)
            | ((diff >> 31) != 0u ? gp4::SF : 0u)
            | ((((value ^ limit) & (value ^ diff)) >> 31) != 0u ? kOF : 0u);
    }
    return c;
}

// FUNCTION: GP4 0x00419333
void __cdecl FUN_00419333(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t keep = r->eflags & ~kArithFlags;

    const uint8_t stateBits = GP4_FIELD(uint8_t, rec, 0x26b);
    if ((stateBits & 0x80u) != 0u && (stateBits & 2u) != 0u) {
        // test byte, 2 (result 2 leaves ZF=SF=PF=0), clc
        r->eflags = keep;
        return;
    }

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006013ac) << 16;
    const uint32_t value = clampMagnitude(GP4_FIELD(uint32_t, rec, 0x3c), limit).value;
    GP4_FIELD(uint32_t, rec, 0x3c) = value;
    r->ebp = limit;
    r->eax = value;

    const uint32_t reference = GP4_GLOBAL(uint32_t, 0x00638d20);
    if ((GP4_FIELD(uint8_t, rec, 0xe6) & 8u) == 0u && reference == 0u) {
        // test dword [0x638d20], -1 with a zero dword: ZF and PF set, clc
        r->eflags = keep | gp4::ZF | kPF;
        return;
    }

    const uint32_t mixed = reference ^ value;
    r->edx = mixed;
    if (static_cast<int32_t>(mixed) >= 0) {
        // xor edx, eax with the signs equal: SF=0, CF=OF=AF=0, then clc
        r->eflags = keep | (mixed == 0u ? gp4::ZF : 0u) | parityFlag(mixed);
        return;
    }

    const Clamped c = clampMagnitude(value, GP4_GLOBAL(uint32_t, 0x00603580));
    GP4_FIELD(uint32_t, rec, 0x3c) = c.value;
    r->eax = c.value;
    // stc after the last neg/cmp: CF forced, the other flags stay.
    r->eflags = keep | c.flags | gp4::CF;
}
GP4_IMPL(0x00419333, FUN_00419333, "regs(eax:u32, edx:u32, ebp:u32, esi:ptr[0x273]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
