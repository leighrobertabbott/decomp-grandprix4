#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00409349
// Fills the output record at esi from the three Q8 values at 0x006278b0 / b4 / bc and the
// input record at edi (word +0x14 scale, words +0x60 / +0x62 limits):
//   +0x90 = clamp(0x6278b0 >> 8, -0x6000, 0x6000)          +0x66 = 0x6278b4
//   +0x146 = max(0x6278bc >> 8, 0x10)
//   +0x92 / +0x96 = the combined terms from 0x0040931e (clamped 0x6278b0 * word [edi+0x14] >> 15)
//   and (0x6278bc * 0x6278b4 >> 14); +0x11c / +0x11e = edi words +0x60 / +0x62 minus 0x100;
//   +0xb0 = +0x11c when +0x90 is negative, else +0x11e.
// Leaves eax = (second sum's high half, last stored word), edx = ebp >> 8, ebp = the first term
// and the flags of `test word [esi+0x90], 0xffff`.
void __cdecl FUN_00409349(gp4::Regs* r) {
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* in = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const int32_t pitch = GP4_GLOBAL(int32_t, 0x006278b0) >> 8;
    uint32_t clamped;
    if (pitch < 0) {
        clamped = 0u - static_cast<uint32_t>(pitch);
        if (clamped >= 0x6000u) {
            clamped = 0x6000u;
        }
        clamped = 0u - clamped;
    } else {
        clamped = static_cast<uint32_t>(pitch);
        if (clamped >= 0x6000u) {
            clamped = 0x6000u;
        }
    }
    GP4_FIELD(uint16_t, out, 0x90) = static_cast<uint16_t>(clamped);
    GP4_FIELD(uint16_t, out, 0x66) = static_cast<uint16_t>(GP4_GLOBAL(uint32_t, 0x006278b4));

    int32_t floorValue = GP4_GLOBAL(int32_t, 0x006278bc) >> 8;
    if (floorValue < 0x10) {
        floorValue = 0x10;
    }
    GP4_FIELD(uint16_t, out, 0x146) = static_cast<uint16_t>(floorValue);

    gp4::Regs callee = *r;
    callee.eax = GP4_GLOBAL(uint32_t, 0x006278b0);
    gp4::call_regs(0x0040931e, callee);

    const int64_t first = static_cast<int64_t>(static_cast<int32_t>(callee.eax))
        * static_cast<int64_t>(GP4_FIELD(int16_t, in, 0x14));
    const uint32_t ebp = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 15);

    const int64_t second = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006278bc))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006278b4));
    uint32_t eax = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 14);
    eax += ebp;
    eax = static_cast<uint32_t>(static_cast<int32_t>(eax) >> 8);
    GP4_FIELD(uint16_t, out, 0x92) = static_cast<uint16_t>(eax);

    const uint32_t edx = static_cast<uint32_t>(static_cast<int32_t>(ebp) >> 8);
    eax += edx;
    GP4_FIELD(uint16_t, out, 0x96) = static_cast<uint16_t>(eax);

    uint16_t ax = static_cast<uint16_t>(GP4_FIELD(uint16_t, in, 0x60) - 0x100u);
    GP4_FIELD(uint16_t, out, 0x11c) = ax;
    ax = static_cast<uint16_t>(GP4_FIELD(uint16_t, in, 0x62) - 0x100u);
    GP4_FIELD(uint16_t, out, 0x11e) = ax;

    const uint32_t word = GP4_FIELD(uint16_t, out, 0x90);
    if ((word & 0x8000u) != 0u) {
        ax = GP4_FIELD(uint16_t, out, 0x11c);
    }
    GP4_FIELD(uint16_t, out, 0xb0) = ax;

    r->eax = (eax & 0xffff0000u) | ax;
    r->edx = edx;
    r->ebp = ebp;
    // test word [esi+0x90], 0xffff: CF = OF = AF = 0
    const uint32_t flags = parityFlag(word) | (word == 0u ? 0x40u : 0u) | ((word & 0x8000u) != 0u ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00409349, FUN_00409349, "regs(esi:ptr[0x150]:bytes, edi:ptr[0x68]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
