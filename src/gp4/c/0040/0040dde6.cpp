#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of "cmp left, right" (CF, PF, AF, ZF, SF, OF).
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040dde6
// pushal ... popal routine. edi = a car/ring record. Starting 50 records (0x44c0 bytes) behind the
// ring position at edi+0x44 (wrapped by DAT_00866ec4), walks 100 records of 0x160 bytes and finds the
// one whose position pair (+0x118/+0x11c or +0x120/+0x124, depending on the sign of
// byte 0x00868582, the opposite pair of the one read from edi) is nearest to edi's pair.
// DAT_0062d2cc = nearest record, DAT_0062d2d0 = its rounded distance (also returned in eax: the pointer);
// the flags are those of "cmp [0x0062d2d0], 0x100".
void __cdecl FUN_0040dde6(gp4::Regs* r) {
    using namespace gp4::x87;
    uint8_t* self = reinterpret_cast<uint8_t*>(r->edi);
    const bool negative = static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00868582)) < 0;

    int32_t refX;
    int32_t refZ;
    if (negative) {
        refX = GP4_FIELD(int32_t, self, 0x120);
        refZ = GP4_FIELD(int32_t, self, 0x124);
    } else {
        refX = GP4_FIELD(int32_t, self, 0x118);
        refZ = GP4_FIELD(int32_t, self, 0x11c);
    }

    uint32_t cursor = GP4_FIELD(uint32_t, self, 0x44);
    if (cursor >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        cursor -= 0x44c0u;
        if (cursor < GP4_GLOBAL(uint32_t, 0x00891500)) {
            cursor = GP4_GLOBAL(uint32_t, 0x00891500);
        }
    } else {
        cursor -= 0x44c0u;
        if (cursor < GP4_GLOBAL(uint32_t, 0x00891504)) {
            cursor += GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    }

    int32_t best = 0x7fffffff;
    for (int32_t count = 100; count != 0; --count) {
        uint8_t* record = reinterpret_cast<uint8_t*>(cursor);
        int32_t posX;
        int32_t posZ;
        if (negative) {
            posX = GP4_FIELD(int32_t, record, 0x118);
            posZ = GP4_FIELD(int32_t, record, 0x11c);
        } else {
            posX = GP4_FIELD(int32_t, record, 0x120);
            posZ = GP4_FIELD(int32_t, record, 0x124);
        }
        const int32_t dx = static_cast<int32_t>(static_cast<uint32_t>(posX) - static_cast<uint32_t>(refX));
        const int32_t dz = static_cast<int32_t>(static_cast<uint32_t>(posZ) - static_cast<uint32_t>(refZ));
        const Ext fx = ext_i32(dx);
        const Ext fz = ext_i32(dz);
        GP4_GLOBAL(int32_t, 0x006f2bf4) = fistp32(fsqrt(fx * fx + fz * fz));
        const int32_t distance = GP4_GLOBAL(int32_t, 0x006f2bf4);
        if (distance < best) {
            best = distance;
            GP4_GLOBAL(uint32_t, 0x0062d2cc) = cursor;
            GP4_GLOBAL(int32_t, 0x0062d2d0) = distance;
        }
        cursor += 0x160u;
        if (cursor >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            if (cursor < GP4_GLOBAL(uint32_t, 0x00891500)) {
                cursor -= GP4_GLOBAL(uint32_t, 0x00866ec4);
            } else if (cursor >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                cursor = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
            }
        }
    }

    r->eax = GP4_GLOBAL(uint32_t, 0x0062d2cc);
    r->eflags = (r->eflags & ~0x8d5u) | compareFlags32(GP4_GLOBAL(uint32_t, 0x0062d2d0), 0x100u);
}
GP4_IMPL(0x0040dde6, FUN_0040dde6, "regs(edi:rec[0x8943a0,0x160,0..3]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=ring_0040dde6")
