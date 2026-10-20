#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by AND/OR/TEST on a byte: CF = OF = AF = 0, PF/ZF/SF from the byte result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit SUB (a - b = res).
static inline uint32_t subFlags32(uint32_t a, uint32_t b, uint32_t res) {
    return (a < b ? 1u : 0u) | parityFlag8(res) | (((a ^ b ^ res) >> 4) & 1u ? 0x10u : 0u)
        | (res == 0u ? 0x40u : 0u) | ((res & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ res)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Per-car record (only the fields this routine touches; offsets in the names).
struct Unknown_0043190d_Car {
    uint8_t Unknown00[0x62];
    int16_t Unknown62;
    uint8_t Unknown64[0x8d - 0x64];
    uint8_t Flags8D;
    uint8_t Unknown8E;
    uint8_t Flags8F;
    uint8_t Unknown90[0x9a - 0x90];
    int16_t Unknown9A;
    uint8_t Unknown9C[0x1b4 - 0x9c];
    uint16_t Unknown1B4;
    uint8_t Unknown1B6[0x1ce - 0x1b6];
    uint8_t Flags1CE;
    uint8_t Unknown1CF[0x231 - 0x1cf];
    uint8_t Flags231;
    uint8_t Unknown232[0x36c - 0x232];
    int32_t Unknown36C;
    int32_t Unknown370;
};

// FUNCTION: GP4 0x0043190d
// Updates bits 4 and 3 of byte +0x231 and the countdown word +0x1b4. The gate is: byte +0xe5 bit 0,
// byte +0x8d bit 1 clear, byte +0x1ce bit 0 clear, word +0x9a < 0x1d5, byte +0x8f has bit 7 or 6 and,
// when word +0x62 >= 2000, max(|+0x36c|, |+0x370|) >= DAT_00603404. When the gate fails both bits
// are cleared. Otherwise, unless bit 3 is set: the first time sets bit 4 and loads +0x1b4 with
// 0xa00; afterwards +0x1b4 counts down by (DAT_00603854 >> 8) and sets bit 3 once it would go negative.
// Registers and flags are those of the last instructions executed on each path.
void __cdecl FUN_0043190d(gp4::Regs* r) {
    Unknown_0043190d_Car* car = reinterpret_cast<Unknown_0043190d_Car*>(static_cast<uintptr_t>(r->esi));

    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t flags = 0;
    bool cleared = false;

    if ((car->Flags8D & 2u) != 0u || (car->Flags1CE & 1u) != 0u || (car->Unknown00[0xe5] & 1u) == 0u) {
        cleared = true;
    } else {
        eax = (eax & 0xffff0000u) | static_cast<uint16_t>(car->Unknown9A);
        if (car->Unknown9A >= 0x1d5) {
            cleared = true;
        } else if ((car->Flags8F & 0xc0u) == 0u) {
            cleared = true;
        } else {
            eax = (eax & 0xffff0000u) | static_cast<uint16_t>(car->Unknown62);
            if (car->Unknown62 >= 0x7d0) {
                eax = static_cast<uint32_t>(car->Unknown36C);
                if (static_cast<int32_t>(eax) < 0) eax = 0u - eax;
                edx = static_cast<uint32_t>(car->Unknown370);
                if (static_cast<int32_t>(edx) < 0) edx = 0u - edx;
                if (static_cast<int32_t>(eax) < static_cast<int32_t>(edx)) eax = edx;
                if (static_cast<int32_t>(eax) < GP4_GLOBAL(int32_t, 0x00603404)) cleared = true;
            }
        }
    }

    if (cleared) {
        car->Flags231 = static_cast<uint8_t>(car->Flags231 & 0xefu);
        car->Flags231 = static_cast<uint8_t>(car->Flags231 & 0xf7u);
        flags = logicFlags8(car->Flags231);
    } else if ((car->Flags231 & 8u) != 0u) {
        flags = logicFlags8(car->Flags231 & 8u);
    } else if ((car->Flags231 & 0x10u) == 0u) {
        car->Flags231 = static_cast<uint8_t>(car->Flags231 | 0x10u);
        flags = logicFlags8(car->Flags231);
        car->Unknown1B4 = 0x0a00;
    } else {
        eax = car->Unknown1B4;
        edx = GP4_GLOBAL(uint32_t, 0x00603854) >> 8;
        const uint32_t before = eax;
        eax = before - edx;
        flags = subFlags32(before, edx, eax);
        if (static_cast<int32_t>(eax) < 0) {
            car->Flags231 = static_cast<uint8_t>(car->Flags231 | 8u);
            flags = logicFlags8(car->Flags231);
        } else {
            car->Unknown1B4 = static_cast<uint16_t>(eax);
        }
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043190d, FUN_0043190d, "regs(eax:u32, edx:u32, esi:ptr[0x378]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
