#include <gp4/gp4.h>
#include <gp4/flags.h>

// Flags of `shl r32, n` for 2 <= n <= 31 in the pinned oracle model: CF = last bit shifted out,
// AF = 0, OF = bit31(value << (n-1)) ^ bit31(result).
static inline uint32_t Shl32Flags(uint32_t value, unsigned n) {
    const uint32_t result = value << n;
    uint32_t flags = gp4::flags::szp(result, 32);
    if ((value >> (32u - n)) & 1u) {
        flags |= gp4::flags::CF;
    }
    if (((value << (n - 1u)) ^ result) & 0x80000000u) {
        flags |= gp4::flags::OF;
    }
    return flags;
}

// Byte at (0x6f3a3c + row * 16 + column) of the 16-byte-per-row table at 0x6f3a3c.
static __declspec(noinline) uint32_t TableByte(uint32_t row, uint32_t column) {
    return GP4_ARRAY(uint8_t, 0x006f3a3c)[column + row * 16u];
}

// FUNCTION: GP4 0x00525dc2
// pushal wrapper. Unless bit 0x40 of the byte 0x66d7a4 is set, draws a random byte (0x4015dc);
// below 0x68 it sets 0x852d57 = 0xc0, 0x852d5c = 1 and loads 0x6f3a34 with the table byte at
// 0x6f3a3c[ecx * 16 + (second draw & 0xf)] (flags: `shl edx, 4` of ecx). Otherwise it sets bit 4
// of 0x6f3adc, 0x852d57 = 0x80 and 0x852d5c = 0x20 (flags: those of `or byte [0x852d5c], 0x20`).
// popal restores every GPR; it does not restore flags.
void __cdecl FUN_00525dc2(gp4::Regs* r) {
    const uint32_t ecx = r->ecx;
    uint32_t flags = 0u;
    bool fallback = true;

    if ((GP4_GLOBAL(uint8_t, 0x0066d7a4) & 0x40u) == 0u) {
        gp4::Regs first = *r;
        gp4::call_regs(0x004015dc, first);
        const uint32_t draw = first.eax & 0xffu;
        if (draw < 0x68u) {
            fallback = false;
            GP4_GLOBAL(uint8_t, 0x00852d57) = 0xc0u;
            GP4_GLOBAL(uint8_t, 0x00852d5c) = 1u;
            gp4::Regs second = *r;
            second.eax = draw;
            gp4::call_regs(0x004015dc, second);
            GP4_GLOBAL(uint32_t, 0x006f3a34) = TableByte(ecx, second.eax & 0xfu);
            flags = Shl32Flags(ecx, 4u);
        }
    }
    if (fallback) {
        GP4_GLOBAL(uint32_t, 0x006f3adc) = GP4_GLOBAL(uint32_t, 0x006f3adc) | 4u;
        GP4_GLOBAL(uint8_t, 0x00852d57) = 0x80u;
        GP4_GLOBAL(uint8_t, 0x00852d5c) = 0x20u;
        flags = gp4::flags::logic8(0x20u);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00525dc2, FUN_00525dc2, "regs(eax:u32, ecx:u32[0..9]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0062d1b4")
