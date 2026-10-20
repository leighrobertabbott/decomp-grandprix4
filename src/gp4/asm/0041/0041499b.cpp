#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic op (OR): CF = OF = AF = 0.
static inline uint32_t flagsLogic8(uint8_t res) {
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | (res & 0x80u);
}

static inline uint32_t setLow16(uint32_t reg, uint16_t v) {
    return (reg & 0xffff0000u) | v;
}

// esi = car record, edi = pointer to a word (angle). ax = |[edi] - [esi+0xe]|. With bit 2 of +0x161
// set the work is done by 0x00414886; otherwise the sign of +0x90 (angle gap >= 0x4000) or of +0xb4
// (gap smaller; [0x00603c34] = 0xff, no change when +0xb4 is zero) picks bit 4 (negative) or 8 of
// the byte at +0x6f. The flags are those of that `or`, or of `test word [esi+0xb4], 0xffff` = 0.
// FUNCTION: GP4 0x0041499b
void __cdecl FUN_0041499b(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* other = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    uint16_t ax = static_cast<uint16_t>(*reinterpret_cast<const uint16_t*>(other) - GP4_FIELD(uint16_t, rec, 0x0e));
    if (static_cast<int16_t>(ax) < 0) ax = static_cast<uint16_t>(0u - ax);
    r->eax = setLow16(r->eax, ax);

    if (rec[0x161] & 2u) {
        gp4::call_regs(0x00414886, *r);
        return;
    }

    bool negative;
    if (static_cast<int16_t>(ax) >= 0x4000) {
        negative = GP4_FIELD(int16_t, rec, 0x90) < 0;
    } else {
        GP4_GLOBAL(uint8_t, 0x00603c34) = 0xff;
        const int16_t steer = GP4_FIELD(int16_t, rec, 0xb4);
        if (steer == 0) {
            // test word [esi+0xb4], 0xffff -> 0: PF = ZF = 1, the rest clear
            r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
            return;
        }
        negative = steer < 0;
    }
    rec[0x6f] |= negative ? 4u : 8u;
    r->eflags = (r->eflags & ~0x8d5u) | flagsLogic8(rec[0x6f]);
}
GP4_IMPL(0x0041499b, FUN_0041499b, "regs(edi:ptr[8]:bytes, esi:ptr[0x169]:layout=car_0041499b, eax:u32, edx:u32, ebp:u32) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
