#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `add left, right` (CF PF AF ZF SF OF).
static inline uint32_t addFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left + right);
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// 16-bit "imul word ; shl ax,1 ; rcl dx,1 ; shl ax,1 ; rcl dx,1": dx = low word of (a * b) >> 14.
static inline uint16_t mulShr14Word(int16_t a, int16_t b) {
    const int32_t product = static_cast<int32_t>(a) * static_cast<int32_t>(b);
    return static_cast<uint16_t>(static_cast<uint32_t>(product) >> 14);
}

// FUNCTION: GP4 0x004275d1
// esi = driver record (pointer to a paired block at +0x1b8, word +0x66, flag bytes +0x9c/+0xfe/
// +0x122/+0x163, dword +0x1a0), edi = pair of word arrays. Interpolates two target words (the
// ones at P+0x18 / P+0x178 against edi+0 / edi+0x160, and edi+0xa / edi+0x16a) by word [esi+0x66]
// in Q14, adds the slope term of 0x00427579, picks or mirrors the first target depending on the
// flag bytes and returns in ax the second value moved toward it by word [esi+0x1a0] (Q14).
// ebx = P; ecx = (eax high half, 0x00427579's ax); edx = ([esi+0x1a0] high half, last Q14 step);
// the flags are those of the final 16-bit add.
void __cdecl FUN_004275d1(gp4::Regs* r) {
    uint8_t* driver = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* pair = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, driver, 0x1b8)));
    const int16_t blend = GP4_FIELD(int16_t, driver, 0x66);
    const uint32_t eaxHigh = r->eax & 0xffff0000u;

    // first interpolation: cx + ((target - cx) * blend >> 14)
    const uint16_t base = static_cast<uint16_t>(GP4_FIELD(uint16_t, block, 0x18) + GP4_FIELD(uint16_t, pair, 0));
    const uint16_t target = static_cast<uint16_t>(GP4_FIELD(uint16_t, block, 0x178) + GP4_FIELD(uint16_t, pair, 0x160));
    const uint16_t first = static_cast<uint16_t>(mulShr14Word(static_cast<int16_t>(target - base), blend) + base);

    // second interpolation
    const uint16_t base2 = GP4_FIELD(uint16_t, pair, 0xa);
    const uint16_t target2 = GP4_FIELD(uint16_t, pair, 0x16a);
    const uint16_t second = static_cast<uint16_t>(mulShr14Word(static_cast<int16_t>(target2 - base2), blend) + base2);

    gp4::Regs slope = *r;
    slope.st_in = 0;
    slope.st_out = 0;
    slope.eax = eaxHigh | second;
    gp4::call_regs(0x00427579, slope);
    const uint16_t moved = static_cast<uint16_t>(slope.eax);

    uint16_t value = first;
    if ((GP4_FIELD(uint8_t, driver, 0xfe) & 0x40u) != 0u) {
        value = static_cast<uint16_t>((static_cast<int16_t>(static_cast<uint16_t>(value - moved)) >> 1) + moved);
    }
    if ((GP4_FIELD(uint8_t, driver, 0x9c) & 0x10u) != 0u) {
        value = moved;
    } else if ((GP4_FIELD(uint8_t, driver, 0x163) & 0x80u) != 0u) {
        if ((GP4_FIELD(uint8_t, driver, 0x122) & 0x80u) != 0u) {
            value = static_cast<uint16_t>(0u - static_cast<uint16_t>(value - moved) + moved);
        } else if ((GP4_FIELD(uint8_t, driver, 0x163) & 0x40u) != 0u) {
            value = moved;
        }
    }

    // xchg cx, ax ; sub ax, cx ; imul dx ; two shift/rotate pairs ; mov ax, dx ; add ax, cx
    const uint32_t rate = GP4_FIELD(uint32_t, driver, 0x1a0);
    const uint16_t step = mulShr14Word(static_cast<int16_t>(value - moved), static_cast<int16_t>(rate));
    const uint16_t result = static_cast<uint16_t>(step + moved);

    r->eax = (slope.eax & 0xffff0000u) | result;
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(block));
    r->ecx = eaxHigh | moved;
    r->edx = (rate & 0xffff0000u) | step;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags16(step, moved);
}
GP4_IMPL(0x004275d1, FUN_004275d1, "regs(eax:u32, edi:ptr[0x1c4]:bytes, esi:ptr[0x41e]:layout=rec_004275d1) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of)")
