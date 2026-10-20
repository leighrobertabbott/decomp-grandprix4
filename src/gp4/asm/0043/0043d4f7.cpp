#include <gp4/gp4.h>

// Parity flag (bit 2) of the low byte of a result: set when the number of 1 bits is even.
static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `sub left, right` / `cmp left, right`.
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Flags of a 32-bit `sub left, right` / `cmp left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Flags of an 8-bit `test byte, imm` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t result) {
    return parityFlag(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// `mov ax, [m]; test ax, ax; jns; neg ax`
static inline uint16_t absWord(uint16_t v) {
    return (static_cast<int16_t>(v) < 0) ? static_cast<uint16_t>(0u - v) : v;
}

// FUNCTION: GP4 0x0043d4f7
// Walks backwards through the 0x160-byte records from the record in esi down to the record in
// edi (exclusive). Succeeds (CF = 0) when |word [ebx]| >= [0x62239c] and |+0x14| of the start
// record >= [0x603aca], then a run of records with bit 7 of byte +0x51 set (counted in eax,
// each with |+0x14| <= [0x603ace]) ends at a record whose +0x16 (negated or not, by bit 6 of
// the byte +0x51 of the record above) plus 0x200 is not below its +0x60 / +0x62.
// eax = (negated) run length, edx = first record of the run; CF = 1 on any failure.
void __cdecl FUN_0043d4f7(gp4::Regs* r) {
    uint8_t* refPtr = reinterpret_cast<uint8_t*>(r->ebx);
    const uint32_t limit = r->edi;
    uint32_t esi = r->esi;
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t flags = 0;
    uint32_t carry = 1;

    const int16_t globalA = GP4_GLOBAL(int16_t, 0x0062239c);
    const int16_t globalB = GP4_GLOBAL(int16_t, 0x00603aca);
    const int16_t globalC = GP4_GLOBAL(int16_t, 0x00603acc);
    const int16_t globalD = GP4_GLOBAL(int16_t, 0x00603ace);

    uint16_t ax;
    uint16_t bp;
    uint16_t cx;
    uint32_t count;
    uint32_t absCount;
    uint8_t* rec;

    flags = subFlags32(esi, limit);
    if (esi <= limit) {
        goto done;
    }

    ax = absWord(GP4_FIELD(uint16_t, refPtr, 0));
    eax = (eax & 0xffff0000u) | ax;
    flags = subFlags16(ax, static_cast<uint16_t>(globalA));
    if (static_cast<int16_t>(ax) < globalA) {
        goto done;
    }

    rec = reinterpret_cast<uint8_t*>(esi);
    ax = absWord(GP4_FIELD(uint16_t, rec, 0x14));
    eax = (eax & 0xffff0000u) | ax;
    flags = subFlags16(ax, static_cast<uint16_t>(globalB));
    if (static_cast<int16_t>(ax) < globalB) {
        goto done;
    }

    // Skip records whose |+0x14| is at least [0x603acc].
    for (;;) {
        esi -= 0x160;
        flags = subFlags32(esi, limit);
        if (esi == limit) {
            goto done;
        }
        rec = reinterpret_cast<uint8_t*>(esi);
        ax = absWord(GP4_FIELD(uint16_t, rec, 0x14));
        eax = (eax & 0xffff0000u) | ax;
        flags = subFlags16(ax, static_cast<uint16_t>(globalC));
        if (static_cast<int16_t>(ax) < globalC) {
            break;
        }
    }

    flags = testFlags8(GP4_FIELD(uint8_t, rec, 0x51) & 0x80u);
    if ((GP4_FIELD(uint8_t, rec, 0x51) & 0x80u) == 0u) {
        goto done;
    }

    // Count the run of records with bit 7 set and |+0x14| <= [0x603ace].
    count = 0;
    edx = esi;
    for (;;) {
        count++;
        eax = count;
        esi -= 0x160;
        flags = subFlags32(esi, limit);
        if (esi == limit) {
            goto done;
        }
        rec = reinterpret_cast<uint8_t*>(esi);
        bp = absWord(GP4_FIELD(uint16_t, rec, 0x14));
        flags = subFlags16(bp, static_cast<uint16_t>(globalD));
        if (static_cast<int16_t>(bp) > globalD) {
            goto done;
        }
        flags = testFlags8(GP4_FIELD(uint8_t, rec, 0x51) & 0x80u);
        if ((GP4_FIELD(uint8_t, rec, 0x51) & 0x80u) == 0u) {
            break;
        }
    }

    cx = GP4_FIELD(uint16_t, rec, 0x16);
    if ((GP4_FIELD(uint8_t, rec, 0x1b1) & 0x40u) != 0u) {
        eax = 0u - eax;
        cx = static_cast<uint16_t>(cx + 0x200u);
        flags = subFlags16(cx, GP4_FIELD(uint16_t, rec, 0x62));
        if (static_cast<int16_t>(cx) < GP4_FIELD(int16_t, rec, 0x62)) {
            goto done;
        }
    } else {
        cx = static_cast<uint16_t>(0u - cx);
        cx = static_cast<uint16_t>(cx + 0x200u);
        flags = subFlags16(cx, GP4_FIELD(uint16_t, rec, 0x60));
        if (static_cast<int16_t>(cx) < GP4_FIELD(int16_t, rec, 0x60)) {
            goto done;
        }
    }

    flags = subFlags32(eax, 0);
    if (eax != 0u) {
        absCount = (static_cast<int32_t>(eax) < 0) ? (0u - eax) : eax;
        flags = subFlags32(absCount, 0x32u);
    }
    carry = 0;

done:
    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | carry;
}
GP4_IMPL(0x0043d4f7, FUN_0043d4f7, "regs(eax:u32, edx:u32, ebx:ptr[8]:bytes, esi:rec[0x8943a0,0x160,0..29], edi:rec[0x8943a0,0x160,0..25]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_0043ec1a+ring_008943a0+surf_0041f02b")
