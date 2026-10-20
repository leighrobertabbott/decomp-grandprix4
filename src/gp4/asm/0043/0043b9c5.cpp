#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0043b9c5
void __cdecl FUN_0043b9c5(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    // Two 16-bit words at +0x2e and +0x2c: when the high byte is zero it is
    // filled with (a global byte + 0x11). eax and edx are saved and restored.
    uint32_t flags = 0;

    uint16_t w2e = GP4_FIELD(uint16_t, rec, 0x2e);
    if ((w2e >> 8) == 0u) {
        const uint8_t fill = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0062f178) + 0x11u);
        w2e = static_cast<uint16_t>(w2e | (static_cast<uint16_t>(fill) << 8));
        GP4_FIELD(uint16_t, rec, 0x2e) = w2e;
    }

    uint16_t w2c = GP4_FIELD(uint16_t, rec, 0x2c);
    const uint32_t hi = static_cast<uint32_t>(w2c >> 8);
    if (hi != 0u) {
        // last flag writer: test dl, 0xff  (dl = high byte)
        flags = parityFlag(hi) | (hi == 0u ? 0x40u : 0u) | ((hi & 0x80u) ? 0x80u : 0u);
    } else {
        const uint8_t fill = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x0062f179) + 0x11u);
        w2c = static_cast<uint16_t>(w2c | (static_cast<uint16_t>(fill) << 8));
        GP4_FIELD(uint16_t, rec, 0x2c) = w2c;
        // last flag writer: or ax, dx  (16-bit): CF=OF=AF=0
        flags = parityFlag(w2c) | (w2c == 0u ? 0x40u : 0u) | ((w2c & 0x8000u) ? 0x80u : 0u);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043b9c5, FUN_0043b9c5, "regs(edi:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of)")
