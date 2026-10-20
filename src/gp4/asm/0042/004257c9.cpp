#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `cmp a, b`.
static inline uint32_t cmpFlags16(uint32_t a16, uint32_t b16) {
    const uint32_t a = a16 & 0xffffu;
    const uint32_t b = b16 & 0xffffu;
    const uint32_t result = (a - b) & 0xffffu;
    return (a < b ? 1u : 0u)
        | parityFlag(result)
        | (((a ^ b ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

static inline void setLow16(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffff0000u) | (value & 0xffffu);
}

// Kept as separate calls so each original callee address stays a literal in the object code.
static __declspec(noinline) void callFollowA(gp4::Regs& regs) {
    gp4::call_regs(0x00425473, regs);
}
static __declspec(noinline) void callFollowB(gp4::Regs& regs) {
    gp4::call_regs(0x004255d4, regs);
}

// FUNCTION: GP4 0x004257c9
// Limits the closing-speed word [0x00934df4] for the car in esi (callers 0x4147d6 / 0x426b30 store the
// candidate there first). Clears word +0x21e. The candidate cx comes from the car ahead (dword +0x110):
// without byte +0xcb bit 3 it is that car's word +0xda when +0xcb bit 0 is set and the word is below
// [0x006039a0], otherwise 0; with bit 3 it is word +0xd4 + the ahead car's +0xda, or the result of
// 0x425473 (when [0x00625a53] is nonzero) / 0x4255d4 when those return CF = 1 and ax < [0x006039a0].
// cx is then capped by [0x006039ac] (+0xca bit 1), [0x006039aa] (+0xcb bit 1) and [0x006039ae]
// (+0xca bit 7) and, if below the signed word [0x00934df4], raised to at least [0x00623e02] and stored
// (CF = 1); CF = 0 otherwise. Registers and flags are those of the last writers.
void __cdecl FUN_004257c9(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs w = *r;
    w.st_in = 0;
    w.st_out = 0;
    w.st_xin_mask = 0;
    w.st_xout_mask = 0;

    uint32_t flags;
    uint8_t* other;

    GP4_FIELD(uint16_t, body, 0x21e) = 0;
    if ((GP4_FIELD(uint8_t, body, 0xcb) & 8u) == 0u) {
        if ((GP4_FIELD(uint8_t, body, 0xcb) & 1u) == 0u) {
            goto zero_path;
        }
        w.ebx = GP4_FIELD(uint32_t, body, 0x110);
        if (w.ebx == 0u) {
            goto zero_path;
        }
        other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(w.ebx));
        setLow16(w.ecx, GP4_FIELD(uint16_t, other, 0xda));
        if (static_cast<int16_t>(w.ecx) < GP4_GLOBAL(int16_t, 0x006039a0)) {
            goto clamp;
        }
        goto zero_path;
    }

    w.ebx = GP4_FIELD(uint32_t, body, 0x110);
    if (w.ebx == 0u) {
        goto zero_path;
    }
    other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(w.ebx));
    setLow16(w.edx, GP4_FIELD(uint16_t, body, 0x102));
    if (static_cast<int16_t>(w.edx) < GP4_GLOBAL(int16_t, 0x006039a2)) {
        goto sum_path;
    }
    if (GP4_FIELD(uint16_t, body, 0xd4) == 0x4000u) {
        goto zero_path;
    }
    if ((GP4_FIELD(uint8_t, body, 0xce) & 0x80u) != 0u) {
        goto sum_path;
    }
    if ((GP4_FIELD(uint8_t, other, 0xcb) & 1u) != 0u) {
        setLow16(w.eax, 0x4000u);
        const uint32_t ahead = GP4_FIELD(uint32_t, other, 0x110);
        if (ahead != 0u) {
            setLow16(w.eax, *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(ahead) + 0xda));
        }
        if (static_cast<int16_t>(w.eax) < GP4_GLOBAL(int16_t, 0x006039a0)) {
            goto sum_path;
        }
    }
    if (GP4_GLOBAL(uint8_t, 0x00625a53) != 0u) {
        callFollowA(w);
    } else {
        callFollowB(w);
    }
    if ((w.eflags & 1u) == 0u) {
        goto sum_path;
    }
    if (static_cast<int16_t>(w.eax) >= GP4_GLOBAL(int16_t, 0x006039a0)) {
        goto zero_path;
    }
    setLow16(w.ecx, w.eax);
    goto clamp;

sum_path:
    setLow16(w.ecx, GP4_FIELD(uint16_t, body, 0xd4) + GP4_FIELD(uint16_t, other, 0xda));
    goto clamp;

zero_path:
    setLow16(w.ecx, 0u);
    if ((GP4_FIELD(uint8_t, body, 0xca) & 0x80u) == 0u && (GP4_FIELD(uint8_t, body, 0xca) & 2u) == 0u
        && (GP4_FIELD(uint8_t, body, 0xcb) & 2u) == 0u) {
        // test byte [esi+0xcb], 2 leaves ZF, PF; clc
        flags = 0x44u;
        goto done;
    }

clamp:
    {
        int16_t value = static_cast<int16_t>(w.ecx);
        if ((GP4_FIELD(uint8_t, body, 0xca) & 2u) != 0u && value > GP4_GLOBAL(int16_t, 0x006039ac)) {
            value = GP4_GLOBAL(int16_t, 0x006039ac);
        }
        if ((GP4_FIELD(uint8_t, body, 0xcb) & 2u) != 0u && value > GP4_GLOBAL(int16_t, 0x006039aa)) {
            value = GP4_GLOBAL(int16_t, 0x006039aa);
        }
        if ((GP4_FIELD(uint8_t, body, 0xca) & 0x80u) != 0u && value > GP4_GLOBAL(int16_t, 0x006039ae)) {
            value = GP4_GLOBAL(int16_t, 0x006039ae);
        }
        setLow16(w.ecx, static_cast<uint16_t>(value));

        const int16_t current = GP4_GLOBAL(int16_t, 0x00934df4);
        if (value >= current) {
            flags = cmpFlags16(static_cast<uint16_t>(value), static_cast<uint16_t>(current)) & ~1u;   // clc
            goto done;
        }
        const int16_t floorValue = GP4_GLOBAL(int16_t, 0x00623e02);
        flags = cmpFlags16(static_cast<uint16_t>(value), static_cast<uint16_t>(floorValue)) | 1u;     // stc
        if (value < floorValue) {
            value = floorValue;
        }
        setLow16(w.ecx, static_cast<uint16_t>(value));
        GP4_GLOBAL(int16_t, 0x00934df4) = value;
    }

done:
    r->eax = w.eax;
    r->ebx = w.ebx;
    r->ecx = w.ecx;
    r->edx = w.edx;
    r->edi = w.edi;
    r->ebp = w.ebp;
    r->eflags = (w.eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004257c9, FUN_004257c9, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x228]:layout=rec_00426091+car_00425473+car_004149f4+car_004257c9) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00426091+cache_00426091+state_00425473+opt_00625a53+tune_00425473")
