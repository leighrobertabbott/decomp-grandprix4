#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte and/or/test result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a 32-bit add.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static const uint32_t kZeroTest = 0x44u;   // test with a zero result: ZF, PF

// movzx byte * [0x601d58] (signed 64-bit product), shrd by 14.
static inline uint32_t scaled(uint32_t byteValue) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(byteValue)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601d58));
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// The decision chain; returns the flags of the last flag writer before popal.
static uint32_t run(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    if ((GP4_FIELD(uint8_t, rec, 0xe5) & 4u) != 0u) {
        return logicFlags8(4u);
    }
    uint8_t* segment = GP4_FIELD(uint8_t*, rec, 0x1c);

    bool checkLimits = true;
    if (GP4_GLOBAL(int16_t, 0x007ad89e) >= 2 || GP4_GLOBAL(uint8_t, 0x007ad8af) != 0u) {
        if ((GP4_FIELD(uint8_t, rec, 0x231) & 1u) != 0u) {
            checkLimits = false;
        } else if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u && GP4_GLOBAL(uint8_t, 0x0066d88c) == 5u) {
            gp4::Regs call = *r;
            call.st_in = 0;
            call.st_out = 0;
            call.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(segment));
            gp4::call_regs(0x00417df0, call);
            return call.eflags & 0x8d5u;
        }
    }

    if (checkLimits) {
        if ((GP4_FIELD(uint8_t, rec, 0x1ce) & 1u) == 0u) {
            const int16_t lateral = GP4_FIELD(int16_t, rec, 0x90);
            const int16_t edge = GP4_FIELD(int16_t, rec, 0xb0);
            const int16_t speed = GP4_FIELD(int16_t, rec, 0x9a);
            bool hit = false;
            uint32_t limit = 0;
            if ((GP4_FIELD(uint8_t, segment, 0x51) & 4u) != 0u
                && !(static_cast<int16_t>(static_cast<uint16_t>(0u - static_cast<uint16_t>(lateral)) - 0x300u) < edge)) {
                limit = scaled(GP4_FIELD(uint8_t, segment, 0x68));
                hit = static_cast<int16_t>(limit) < speed;
            }
            if (!hit && (GP4_FIELD(uint8_t, segment, 0x51) & 2u) != 0u
                && !(static_cast<int16_t>(static_cast<uint16_t>(lateral) - 0x300u) < edge)) {
                limit = scaled(GP4_FIELD(uint8_t, segment, 0x69));
                hit = static_cast<int16_t>(limit) < speed;
            }
            if (hit) {
                GP4_FIELD(uint8_t, rec, 0x83) = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x83) | 4u);
                int32_t excess = static_cast<int16_t>(static_cast<uint16_t>(limit) - static_cast<uint16_t>(GP4_FIELD(uint16_t, rec, 0x9a)));
                if (excess < 0) {
                    excess = -excess;
                }
                const int64_t product = static_cast<int64_t>(excess) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062d290));
                int32_t delay = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + GP4_GLOBAL(uint32_t, 0x0062d274));
                if (delay < 10000) {
                    delay = 10000;
                }
                if (delay > 30000) {
                    delay = 30000;
                }
                const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
                GP4_FIELD(uint32_t, rec, 0x54) = now + static_cast<uint32_t>(delay);
                return addFlags32(now, static_cast<uint32_t>(delay));
            }
        }
        if ((GP4_FIELD(uint8_t, rec, 0x83) & 4u) == 0u) {
            return kZeroTest;
        }
    }

    const uint32_t expiry = GP4_FIELD(uint32_t, rec, 0x54);
    if (expiry != 0u) {
        const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
        const uint32_t flags = addFlags32(0u - expiry, now);
        if (static_cast<int32_t>(now - expiry) < 0) {
            return flags;
        }
    }
    GP4_FIELD(uint8_t, rec, 0x83) = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x83) & 0xfbu);
    const uint32_t result = GP4_FIELD(uint8_t, rec, 0x231) & 0xfeu;
    GP4_FIELD(uint8_t, rec, 0x231) = static_cast<uint8_t>(result);
    return logicFlags8(result);
}

// Track-limit timer of the car record in esi (segment pointer [esi+0x1c]): when the segment's side
// bits (+0x51 bits 2/1) allow it and the lateral word +0x90 is past the edge word +0xb0 by 0x300,
// a speed word +0x9a above the side's scaled limit (byte +0x68/+0x69 * [0x601d58] >> 14) sets
// +0x83 bit 2 and the expiry +0x54 = [0x66d814] + clamp(|limit - speed| * [0x62d290] >> 14 +
// [0x62d274], 10000, 30000). Otherwise it clears +0x83 bit 2 and +0x231 bit 0 once the expiry has
// passed. With +0xe5 bit 2 it does nothing; in modes with 0x7c2ea9 == 0x80 and 0x66d88c == 5 it
// defers to 0x417df0. pushal/popal keep the registers; the flags are those of the last flag writer.
// FUNCTION: GP4 0x00417e2b
void __cdecl FUN_00417e2b(gp4::Regs* r) {
    const uint32_t flags = run(r);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00417e2b, FUN_00417e2b, "regs(esi:ptr[0x2ac]:layout=car_0041e500) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041e500")
