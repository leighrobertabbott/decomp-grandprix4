#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic result (TEST/AND/OR): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `cmp a, b`.
static inline uint32_t subFlags32(uint32_t a, uint32_t b) {
    const uint32_t res = a - b;
    uint32_t f = parityFlag(res) | (res == 0u ? 0x40u : 0u) | ((res & 0x80000000u) != 0u ? 0x80u : 0u);
    if (a < b) f |= 0x01u;
    if (((a ^ b ^ res) & 0x10u) != 0u) f |= 0x10u;
    if (((a ^ b) & (a ^ res) & 0x80000000u) != 0u) f |= 0x800u;
    return f;
}

static inline void setFlags(gp4::Regs* r, uint32_t flags) {
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}

// Runs an original asm routine on the current register block and takes every register and flag back.
static inline void callAsm(gp4::Regs* r, uintptr_t addr) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(addr, call);
    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->esi = call.esi;
    r->edi = call.edi;
    r->ebp = call.ebp;
    r->eflags = call.eflags;
}

static inline uint8_t* carOf(gp4::Regs* r) { return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi)); }
static inline const uint8_t* segOf(gp4::Regs* r) {
    return reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
}

// Car record in esi, segment record in edi. Updates the car flag bit 0xce.2 (with 0xff.5 / 0x135.2), then,
// when bit 0x135.2 is pending or the segment byte +0x51 bit 0 starts it, stores a word to +0x140 (one of
// [0x603e24], [0x603e28] or the clamped dword +0x1a0), runs 0x00429fcd and stores the steering-like word
// +0x13e = +-(max(+0x1a0, -0x4000) >> 1) -/+ 0x2000; bit 0x135.2 is cleared on the way out.
// FUNCTION: GP4 0x00429dca
void __cdecl FUN_00429dca(gp4::Regs* r) {
    uint8_t* car = carOf(r);

    // 0x429dca .. 0x429e5c
    int action = 0;  // 1: set 0xce.2, 2: clear it and raise 0x135.2
    if ((car[0x9c] & 0x10u) != 0u || (car[0x179] & 0x40u) != 0u) {
        action = 1;
    } else if ((car[0x8d] & 2u) != 0u) {
        if ((car[0xce] & 4u) != 0u) action = 2;
    } else if ((car[0x163] & 0x80u) != 0u) {
        action = 1;
    } else if ((car[0xce] & 4u) != 0u) {
        if ((segOf(r)[0x51] & 0x20u) != 0u) {
            if ((segOf(r)[0x51] & 0x80u) == 0u) car[0xff] |= 0x20u;
        } else {
            r->eax = *reinterpret_cast<uint32_t*>(car + 0x1c);
            callAsm(r, 0x00426320);
            car = carOf(r);
            const uint32_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(r->eax) + 0xa6u);
            r->edx = word;
            if (static_cast<int32_t>(word) >= GP4_GLOBAL(int32_t, 0x0062ac50) || (car[0xff] & 0x20u) != 0u) {
                action = 2;
            }
        }
    }
    if (action == 1) {
        car[0xce] |= 4u;
        car[0xff] &= 0xdfu;
    } else if (action == 2) {
        car[0xce] &= 0xfbu;
        car[0xff] &= 0xdfu;
        car[0x135] |= 4u;
    }

    if ((car[0x135] & 0x40u) != 0u) {
        r->eax = *reinterpret_cast<uint32_t*>(car + 0x1a0);
        if (static_cast<int32_t>(r->eax) >= GP4_GLOBAL(int32_t, 0x0062236c)) car[0x135] &= 0xbfu;
    }

    if ((car[0xce] & 4u) == 0u) {
        if ((car[0x135] & 4u) == 0u) {
            if ((segOf(r)[0x51] & 1u) == 0u) {
                setFlags(r, 0x44u);
                return;
            }
            if ((car[0x135] & 8u) != 0u) {
                setFlags(r, 0x00u);
                return;
            }
            int source = 1;  // 0: clamped +0x1a0, 1: [0x603e24], 2: [0x603e28]
            if (*reinterpret_cast<const uint16_t*>(car + 0x1bc) == 0xffffu) {
                const uint16_t lane = *reinterpret_cast<const uint16_t*>(car + 0xea);
                if ((segOf(r)[0x51] & 8u) != 0u) {
                    if ((car[0x71] & 8u) == 0u) {
                        setFlags(r, 0x44u);
                        return;
                    }
                    source = lane == 0xfff8u ? 1 : (lane == 0xfff0u ? 2 : 0);
                } else {
                    if ((car[0x71] & 8u) != 0u) {
                        setFlags(r, 0x00u);
                        return;
                    }
                    source = lane == 8u ? 1 : (lane == 0x10u ? 2 : 0);
                }
            }

            int32_t value;
            if (source == 1) {
                value = GP4_GLOBAL(int32_t, 0x00603e24);
            } else if (source == 2) {
                value = GP4_GLOBAL(int32_t, 0x00603e28);
            } else {
                if ((car[0x135] & 0x40u) != 0u) {
                    value = *reinterpret_cast<const int32_t*>(car + 0x1a0);
                    if (value > GP4_GLOBAL(int32_t, 0x00622370)) value = GP4_GLOBAL(int32_t, 0x00622370);
                } else {
                    callAsm(r, 0x00429d74);
                    car = carOf(r);
                    if ((r->eflags & 1u) != 0u) return;
                    value = *reinterpret_cast<const int32_t*>(car + 0x1a0);
                    const int32_t limit = GP4_GLOBAL(int32_t, 0x00622368);
                    if (value >= limit) {
                        r->eax = static_cast<uint32_t>(value);
                        setFlags(r, subFlags32(static_cast<uint32_t>(value), static_cast<uint32_t>(limit)));
                        return;
                    }
                }
                if (value < GP4_GLOBAL(int32_t, 0x00603e20)) value = GP4_GLOBAL(int32_t, 0x00603e20);
            }
            r->eax = static_cast<uint32_t>(value);
            *reinterpret_cast<uint16_t*>(car + 0x140) = static_cast<uint16_t>(value);
            car[0x135] |= 0x20u;
            car[0x135] |= 0x10u;
            if ((car[0x71] & 8u) == 0u) car[0x135] &= 0xefu;
        }

        // 0x429f7a
        callAsm(r, 0x00429fcd);
        car = carOf(r);
        int32_t steer = *reinterpret_cast<const int32_t*>(car + 0x1a0);
        if (steer < -0x4000) steer = -0x4000;
        steer >>= 1;
        if ((car[0x71] & 8u) != 0u) {
            steer -= 0x2000;
        } else {
            steer = 0x2000 - steer;
        }
        r->eax = static_cast<uint32_t>(steer);
        *reinterpret_cast<uint16_t*>(car + 0x13e) = static_cast<uint16_t>(steer);
        if ((car[0x135] & 0x20u) != 0u) {
            car[0x135] |= 8u;
            car[0x135] &= 0xdfu;
        }
    }

    // 0x429fc5
    car[0x135] &= 0xfbu;
    setFlags(r, logicFlags8(car[0x135]));
}
GP4_IMPL(0x00429dca, FUN_00429dca, "regs(eax:u32, ecx:u32, edx:u32, edi:ptr[0x9a]:layout=seg51_00429dca, esi:ptr[0x420]:layout=car_0042ba87+other_0042f16d+lane_0042df3a+rec_00409416+car_00429fcd+gears_0041034e+car_00426320+car_0043c677+car_00429dca) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0042595c+ring_008943a0+ring_0040c0ae+words_008943a0+words_00413090+engine_00413606+lanes_00429fcd+look_00426320")
