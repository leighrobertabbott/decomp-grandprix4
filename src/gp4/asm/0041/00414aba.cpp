#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (test / and / or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a 16-bit logic result (test / xor): CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint32_t result) {
    result &= 0xffffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u);
}

// Flags of `left - right` on an n-bit operand (sub / cmp / neg = 0 - x).
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t bits) {
    const uint32_t mask = (bits == 32u) ? 0xffffffffu : ((1u << bits) - 1u);
    const uint32_t sign = 1u << (bits - 1u);
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & sign) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & sign) != 0u ? 0x800u : 0u);
}

// Flags of `left + right` on an n-bit operand.
static inline uint32_t addFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left + right) & 0xffffu;
    return (result < left ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((~(left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Car record (esi): flags and words this routine reads / writes.
struct Unknown_00414aba_Car {
    uint8_t Unknown00[0x0e];
    int16_t Unknown0E;
    uint8_t Unknown10[0x6f - 0x10];
    uint8_t Flags6F;
    uint8_t Unknown70;
    uint8_t Flags71;
    uint8_t Unknown72[0x7a - 0x72];
    uint8_t Flags7A;
    uint8_t Flags7B;
    uint8_t Unknown7C[0x90 - 0x7c];
    int16_t Unknown90;
    uint8_t Unknown92[0x9a - 0x92];
    int16_t Unknown9A;
    uint8_t Unknown9C[0x235 - 0x9c];
    uint8_t Unknown235;
};

// 0x00434090 then 0x00434044 with edx = 0x18, eax = 0xc after `mov ax, <side>`; eax and edx keep
// those values afterwards.
static void notifySide(gp4::Regs* r, uint32_t eax, uint16_t side) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = (eax & 0xffff0000u) | side;
    call.edx = 0x18;
    gp4::call_regs(0x00434090, call);
    call.eax = 0x0c;
    gp4::call_regs(0x00434044, call);
}

// FUNCTION: GP4 0x00414aba
// Sets the result bits in byte +0x6f (1 always; 2 = too fast or too close to the line, 4 / 8 =
// which way to steer) from the lateral difference dx = word +0xe - word [edi] and the heading
// ax = word +0x90, toggling bit 0x10 of +0x71 (with the 0x00434090 / 0x00434044 notifications, which
// leave eax = 0xc and edx = 0x18) when the difference leaves or re-enters the +/-0x4000 band.
// [0x934df0] ends as 0x8000 / 0 (heading half-turn flag). eax, ecx and edx carry the last words
// computed; the flags are those of the last flag writer before ret.
void __cdecl FUN_00414aba(gp4::Regs* r) {
    Unknown_00414aba_Car* car = reinterpret_cast<Unknown_00414aba_Car*>(static_cast<uintptr_t>(r->esi));
    const int16_t* other = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(r->edi));

    uint32_t axHi = r->eax & 0xffff0000u;
    uint32_t cxHi = r->ecx & 0xffff0000u;
    uint32_t dxHi = r->edx & 0xffff0000u;
    uint16_t cx = static_cast<uint16_t>(r->ecx);
    uint32_t flags;

    car->Flags6F = 0;
    car->Flags6F = static_cast<uint8_t>(car->Flags6F | 1u);
    uint16_t dx = static_cast<uint16_t>(static_cast<uint16_t>(car->Unknown0E) - static_cast<uint16_t>(*other));
    uint16_t ax = static_cast<uint16_t>(car->Unknown90);

    // 0 = skip the toggle test, 1 = |dx| against 0x4000, 2 = (dx ^ ax) sign
    int mode;
    bool done = false;
    if ((car->Flags7B & 0x80u) != 0u) {
        flags = subFlags(car->Unknown235, 2u, 8u);
        mode = car->Unknown235 < 2u ? 0 : 1;
    } else if (car->Unknown9A >= 0x80) {
        car->Flags6F = static_cast<uint8_t>(car->Flags6F | 2u);
        flags = logicFlags8(car->Flags6F);
        mode = 0;
        done = true;
    } else {
        flags = subFlags(car->Unknown235, 2u, 8u);
        mode = car->Unknown235 < 2u ? 2 : 1;
    }

    if (!done) {
        if (mode != 0) {
            bool setBit;
            if (mode == 1) {
                cx = dx;
                if (static_cast<int16_t>(cx) < 0) {
                    cx = static_cast<uint16_t>(0u - cx);
                }
                flags = subFlags(cx, 0x4000u, 16u);
                setBit = !(static_cast<int16_t>(cx) < 0x4000);
            } else {
                cx = static_cast<uint16_t>(dx ^ ax);
                flags = logicFlags16(cx);
                setBit = static_cast<int16_t>(cx) < 0;
            }

            flags = logicFlags8(car->Flags71 & 0x10u);
            if (setBit) {
                if ((car->Flags71 & 0x10u) == 0u) {
                    notifySide(r, axHi | ax, 0xffffu);
                    axHi = 0;
                    ax = 0x0c;
                    dxHi = 0;
                    dx = 0x18;
                    car->Flags71 = static_cast<uint8_t>(car->Flags71 | 0x10u);
                    flags = logicFlags8(car->Flags71);
                }
            } else {
                if ((car->Flags71 & 0x10u) != 0u) {
                    notifySide(r, axHi | ax, 0u);
                    axHi = 0;
                    ax = 0x0c;
                    dxHi = 0;
                    dx = 0x18;
                    car->Flags71 = static_cast<uint8_t>(car->Flags71 & 0xefu);
                    flags = logicFlags8(car->Flags71);
                }
            }
        }

        flags = logicFlags8(car->Flags71 & 0x10u);
        if ((car->Flags71 & 0x10u) != 0u) {
            flags = addFlags16(dx, 0x8000u);
            dx = static_cast<uint16_t>(dx + 0x8000u);
        }

        uint16_t half = dx;  // [0x934df0] after the sign fold
        flags = logicFlags16(half);
        if (static_cast<int16_t>(half) < 0) {
            flags = subFlags(0u, half, 16u);
            half = static_cast<uint16_t>(0u - half);
        }
        flags = subFlags(half, 0x4000u, 16u);
        half = half < 0x4000u ? 0u : 0x8000u;
        GP4_GLOBAL(uint16_t, 0x00934df0) = half;

        cx = dx;
        ax = static_cast<uint16_t>(ax ^ half);
        flags = logicFlags16(ax);
        if (static_cast<int16_t>(ax) < 0) {
            flags = subFlags(0u, cx, 16u);
            cx = static_cast<uint16_t>(0u - cx);
        }
        flags = logicFlags16(half);
        if (static_cast<int16_t>(half) < 0) {
            flags = addFlags16(cx, 0x8000u);
            cx = static_cast<uint16_t>(cx + 0x8000u);
        }

        // 1 = arrive at the test of ax (+0x7a bit 4 clear), 2 = after the xor of dx
        int outcome = 0;  // 0 = return, 4 = bit 4 result, 8 = bit 8 result
        flags = logicFlags8(car->Flags7A & 0x10u);
        bool axNegative = false;
        bool decide = true;
        if ((car->Flags7A & 0x10u) == 0u) {
            flags = subFlags(cx, 0x1555u, 16u);
            if (static_cast<int16_t>(cx) >= 0x1555) {
                flags = subFlags(cx, 0x1955u, 16u);
                if (static_cast<int16_t>(cx) < 0x1955) {
                    decide = false;
                } else {
                    flags = subFlags(0u, ax, 16u);
                    ax = static_cast<uint16_t>(0u - ax);
                }
            }
            if (decide) {
                flags = logicFlags16(ax);
                axNegative = static_cast<int16_t>(ax) < 0;
            }
        } else {
            flags = subFlags(cx, 0x200u, 16u);
            if (static_cast<int16_t>(cx) < 0x200) {
                car->Flags6F = 0;
                car->Flags6F = static_cast<uint8_t>(car->Flags6F | 2u);
                flags = logicFlags8(car->Flags6F);
                decide = false;
            } else {
                dx = static_cast<uint16_t>(dx ^ half);
                flags = logicFlags16(dx);
                axNegative = !(static_cast<int16_t>(dx) < 0);
            }
        }

        if (decide) {
            // ax negative (or the xor positive): bit 4 of +0x71 picks 8; otherwise it picks 4
            flags = logicFlags8(car->Flags71 & 0x10u);
            const bool bit4 = (car->Flags71 & 0x10u) != 0u;
            outcome = (axNegative == bit4) ? 8 : 4;
            car->Flags6F = static_cast<uint8_t>(car->Flags6F | outcome);
            flags = logicFlags8(car->Flags6F);
        }
    }

    r->eax = axHi | ax;
    r->ecx = cxHi | cx;
    r->edx = dxHi | dx;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}
GP4_IMPL(0x00414aba, FUN_00414aba, "regs(eax:u32, ecx:u32, edx:u32, edi:ptr[0x10]:bytes, esi:ptr[0x23d]:layout=rec_00414562) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
