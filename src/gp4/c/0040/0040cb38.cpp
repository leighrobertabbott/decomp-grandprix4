#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Sub-record at +0x18c0 of the structure pointed to by DAT_00866f08 (only the words this
// routine reads).
struct Unknown_00866f08_18c0 {
    uint8_t Unknown00[0x60];
    uint16_t Unknown60;
    uint16_t Unknown62;
    uint8_t Unknown64[0x74 - 0x64];
    uint16_t Unknown74;
    uint16_t Unknown76;
};

// FUNCTION: GP4 0x0040cb38
// pushal/popal wrapper (every GPR is preserved). Derives, from the sum of two words of the
// record (which pair is chosen by the sign of byte DAT_00868582), the value
// DAT_00627870 = word sum + 0x50 - 0xbd - DAT_00627890 (or the override DAT_0062d2b4 when
// non-zero), then DAT_00627894 = (DAT_00627870 << 14) / 0x5a4, the word DAT_00603b1a =
// DAT_00603b1c * DAT_00627894 >> 14, DAT_00627874 = max(DAT_00627870, DAT_0062788c)
// (override DAT_0062d2b8 when non-zero) and DAT_00627888 = max(0x5a4 - DAT_00627874, 0).
// The flags are those of the final "cmp eax, 0" on 0x5a4 - DAT_00627874.
void __cdecl FUN_0040cb38(gp4::Regs* r) {
    const Unknown_00866f08_18c0* rec = reinterpret_cast<const Unknown_00866f08_18c0*>(
        static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00866f08) + 0x18c0u));

    uint32_t eax;
    if (GP4_GLOBAL(int8_t, 0x00868582) < 0) {
        eax = (uint16_t)(rec->Unknown74 + rec->Unknown60);
    } else {
        eax = (uint16_t)(rec->Unknown76 + rec->Unknown62);
    }
    eax = eax + 0x50u - 0xbdu - GP4_GLOBAL(uint32_t, 0x00627890);
    if (GP4_GLOBAL(uint32_t, 0x0062d2b4) != 0u) {
        eax = GP4_GLOBAL(uint32_t, 0x0062d2b4);
    }
    GP4_GLOBAL(uint32_t, 0x00627870) = eax;

    int32_t remainder;
    const int32_t quotient = gp4::idiv64((int64_t)(int32_t)eax << 14, 0x5a4, &remainder);
    GP4_GLOBAL(int32_t, 0x00627894) = quotient;

    const int64_t product = (int64_t)GP4_GLOBAL(int32_t, 0x00603b1c) * (int64_t)GP4_GLOBAL(int32_t, 0x00627894);
    GP4_GLOBAL(uint16_t, 0x00603b1a) = (uint16_t)(uint32_t)((uint64_t)product >> 14);

    eax = GP4_GLOBAL(uint32_t, 0x00627870);
    if ((int32_t)eax < GP4_GLOBAL(int32_t, 0x0062788c)) {
        eax = GP4_GLOBAL(uint32_t, 0x0062788c);
    }
    if (GP4_GLOBAL(uint32_t, 0x0062d2b8) != 0u) {
        eax = GP4_GLOBAL(uint32_t, 0x0062d2b8);
    }
    GP4_GLOBAL(uint32_t, 0x00627874) = eax;

    eax = 0x5a4u - GP4_GLOBAL(uint32_t, 0x00627874);
    // cmp eax, 0: CF = OF = AF = 0, ZF/SF/PF from the value
    const uint32_t flags = parityFlag(eax)
        | (eax == 0u ? 0x40u : 0u)
        | ((eax & 0x80000000u) != 0u ? 0x80u : 0u);
    if ((int32_t)eax < 0) {
        eax = 0u;
    }
    GP4_GLOBAL(uint32_t, 0x00627888) = eax;

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040cb38, FUN_0040cb38, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_0040cb38")
