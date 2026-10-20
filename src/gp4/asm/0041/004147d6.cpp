#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic operation (`and`, `test`): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

static inline void setLow16(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffff0000u) | (value & 0xffffu);
}

// FUNCTION: GP4 0x004147d6
// Car-ahead braking flags in the body esi (byte +0x6f: bit 0 = 1, bit 1 = 2 are cleared first).
// Unless byte +0x161 bit 1 is set: [0x00934df4] = word +0xd8 - word +0x9a, limited by 0x4257c9.
// A negative result sets bit 1 and [0x00601d40] = min(4, ...): ((-word) << 16) / [0x00603854] * 4, at
// most 0x4000. Otherwise, unless byte +0xff bit 3 is set, 0x414562 decides: CF clear -> bit 0;
// CF set -> bit 1 when the target body (dword +0x128) has byte +0xff bit 3. Byte +0xff bit 3 -> bit 0.
// 0x41ac8b always runs last; byte +0x161 bit 1 then clears bit 0 again.
// The flags are those of that last "and" (or "test" of +0x161, ZF and PF, when the bit is clear).
void __cdecl FUN_004147d6(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs w = *r;
    w.st_in = 0;
    w.st_out = 0;
    w.st_xin_mask = 0;
    w.st_xout_mask = 0;

    GP4_FIELD(uint8_t, car, 0x6f) &= 0xfdu;
    GP4_FIELD(uint8_t, car, 0x6f) &= 0xfeu;
    if ((GP4_FIELD(uint8_t, car, 0x161) & 2u) == 0u) {
        setLow16(w.eax, static_cast<uint32_t>(GP4_FIELD(uint16_t, car, 0xd8)) - GP4_FIELD(uint16_t, car, 0x9a));
        GP4_GLOBAL(uint16_t, 0x00934df4) = static_cast<uint16_t>(w.eax);
        gp4::call_regs(0x004257c9, w);

        const int16_t closing = GP4_GLOBAL(int16_t, 0x00934df4);
        if (closing < 0) {
            const uint32_t magnitude = static_cast<uint32_t>(-static_cast<int32_t>(closing));
            int32_t remainder;
            const int32_t quotient = gp4::idiv64(static_cast<int64_t>(magnitude << 16), GP4_GLOBAL(int32_t, 0x00603854), &remainder);
            int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(quotient) << 2);
            if (scaled > 0x4000) {
                scaled = 0x4000;
            }
            GP4_GLOBAL(int32_t, 0x00601d40) = scaled;
            w.eax = static_cast<uint32_t>(scaled);
            w.edx = static_cast<uint32_t>(remainder);
            GP4_FIELD(uint8_t, car, 0x6f) |= 2u;
        } else if ((GP4_FIELD(uint8_t, car, 0xff) & 8u) != 0u) {
            GP4_FIELD(uint8_t, car, 0x6f) |= 1u;
        } else {
            gp4::call_regs(0x00414562, w);
            if ((w.eflags & 1u) == 0u) {
                GP4_FIELD(uint8_t, car, 0x6f) |= 1u;
            } else {
                w.ebx = GP4_FIELD(uint32_t, car, 0x128);
                if (w.ebx != 0u && (GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(w.ebx)), 0xff) & 8u) != 0u) {
                    GP4_FIELD(uint8_t, car, 0x6f) |= 2u;
                }
            }
        }
    }

    gp4::call_regs(0x0041ac8b, w);

    uint32_t flags = 0x44u;   // test byte [esi+0x161], 2 with the bit clear
    if ((GP4_FIELD(uint8_t, car, 0x161) & 2u) != 0u) {
        GP4_FIELD(uint8_t, car, 0x6f) &= 0xfeu;
        flags = logicFlags8(GP4_FIELD(uint8_t, car, 0x6f));
    }

    r->eax = w.eax;
    r->ebx = w.ebx;
    r->ecx = w.ecx;
    r->edx = w.edx;
    r->edi = w.edi;
    r->ebp = w.ebp;
    r->eflags = (w.eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004147d6, FUN_004147d6, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x240]:layout=rec_00414562+rec_00426091+car_00425473+car_004149f4+car_004147d6+car_004257c9) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00426091+cache_00426091+state_00425473+opt_00625a53+tune_00425473")
