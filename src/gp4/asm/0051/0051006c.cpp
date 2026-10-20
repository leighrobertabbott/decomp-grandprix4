#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline void copyBytes(uint8_t* dst, const uint8_t* src, uint32_t n) {
    for (; n != 0u; --n) {
        *dst = *src;
        ++dst;
        ++src;
    }
}

// FUNCTION: GP4 0x0051006c
// Restores the state saved by 0x00510309 from slot eax (1..3) of the per-slot tables at
// 0x853ac2..0x85408c: 0x50 bytes to 0x73d1bc, 0xc bytes to 0x679d20, and (when byte 0x7c2e6c is
// set) 0x48 bytes to 0x89108d and 0xc4 bytes to 0x7ab02c. When the slot dword at 0x853efc is zero
// the scalars are left alone and CF is set (stc); otherwise the scalars are restored (0x679bd8
// only when byte 0x852d98 is clear, 0x679bdc only when dword 0x852d3c is zero; the +0x853fbc slot
// is copied onto 0x853fbc itself, as in the original) and CF is cleared (clc). pushal/popal
// restore every GPR; the other flags are those of `and eax, eax` or of the second
// `cmp byte [0x7c2e6c], 0`.
void __cdecl FUN_0051006c(gp4::Regs* r) {
    const uint32_t i = r->eax - 1u;

    copyBytes(GP4_ARRAY(uint8_t, 0x0073d1bc), GP4_ARRAY(uint8_t, 0x00853ac2) + i * 0x50u, 0x50u);
    copyBytes(GP4_ARRAY(uint8_t, 0x00679d20), GP4_ARRAY(uint8_t, 0x00853bb2) + i * 0x0cu, 0x0cu);
    if (GP4_GLOBAL(uint8_t, 0x007c2e6c) != 0u) {
        copyBytes(GP4_ARRAY(uint8_t, 0x0089108d), GP4_ARRAY(uint8_t, 0x00853bd6) + i * 0x48u, 0x48u);
        copyBytes(GP4_ARRAY(uint8_t, 0x007ab02c), GP4_ARRAY(uint8_t, 0x00853cae) + i * 0xc4u, 0xc4u);
    }

    const uint32_t saved = GP4_ARRAY(uint32_t, 0x00853efc)[i];
    if (saved == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, (gp4::flags::logic32(saved) & ~1u) | 1u);
        return;
    }

    GP4_GLOBAL(uint32_t, 0x00679d30) = saved;
    GP4_GLOBAL(uint32_t, 0x00679d2c) = GP4_ARRAY(uint32_t, 0x00853f14)[i];
    GP4_GLOBAL(uint32_t, 0x006f3aac) = GP4_ARRAY(uint32_t, 0x00853f20)[i];
    GP4_GLOBAL(uint32_t, 0x006f3ab0) = GP4_ARRAY(uint32_t, 0x00853f2c)[i];
    GP4_GLOBAL(uint32_t, 0x00875228) = GP4_ARRAY(uint32_t, 0x00853f38)[i];
    GP4_GLOBAL(uint32_t, 0x0087522c) = GP4_ARRAY(uint32_t, 0x00853f44)[i];
    GP4_GLOBAL(uint32_t, 0x00875274) = GP4_ARRAY(uint32_t, 0x00853f50)[i];
    GP4_GLOBAL(uint32_t, 0x007a8dd4) = GP4_ARRAY(uint32_t, 0x00853f5c)[i];
    GP4_GLOBAL(uint32_t, 0x007a8dd8) = GP4_ARRAY(uint32_t, 0x00853f68)[i];
    GP4_GLOBAL(uint32_t, 0x007a8ddc) = GP4_ARRAY(uint32_t, 0x00853f74)[i];
    GP4_GLOBAL(uint32_t, 0x007ad818) = GP4_ARRAY(uint32_t, 0x00853f80)[i];
    if (GP4_GLOBAL(uint8_t, 0x00852d98) == 0u) {
        GP4_GLOBAL(uint32_t, 0x00679bd8) = GP4_ARRAY(uint32_t, 0x00853f8c)[i];
    }
    if (GP4_GLOBAL(uint32_t, 0x00852d3c) == 0u) {
        GP4_GLOBAL(uint32_t, 0x00679bdc) = GP4_ARRAY(uint32_t, 0x00853f98)[i];
    }
    GP4_GLOBAL(uint32_t, 0x00679be0) = GP4_ARRAY(uint32_t, 0x00853fa4)[i];
    GP4_GLOBAL(uint32_t, 0x00679be4) = GP4_ARRAY(uint32_t, 0x00853fb0)[i];
    GP4_GLOBAL(uint32_t, 0x00853fbc) = GP4_ARRAY(uint32_t, 0x00853fbc)[i];
    GP4_GLOBAL(uint32_t, 0x00875278) = GP4_ARRAY(uint32_t, 0x00853fc8)[i];
    GP4_GLOBAL(uint32_t, 0x006f3b00) = *reinterpret_cast<const uint32_t*>(GP4_ARRAY(uint8_t, 0x00853fd4) + i * 2u);
    GP4_GLOBAL(uint32_t, 0x006f3b04) = *reinterpret_cast<const uint32_t*>(GP4_ARRAY(uint8_t, 0x00853fe0) + i * 2u);
    GP4_GLOBAL(uint32_t, 0x006f3b08) = *reinterpret_cast<const uint32_t*>(GP4_ARRAY(uint8_t, 0x00853fec) + i * 2u);
    GP4_GLOBAL(uint16_t, 0x00679b5c) = GP4_ARRAY(uint16_t, 0x00853ff8)[i];
    GP4_GLOBAL(uint16_t, 0x00679b5e) = GP4_ARRAY(uint16_t, 0x00853ffe)[i];
    GP4_GLOBAL(uint16_t, 0x00891510) = GP4_ARRAY(uint16_t, 0x00854004)[i];
    GP4_GLOBAL(uint16_t, 0x0066d7a6) = GP4_ARRAY(uint16_t, 0x0085400a)[i];
    GP4_GLOBAL(uint8_t, 0x00852df7) = GP4_ARRAY(uint8_t, 0x00854010)[i];
    const uint8_t mode = GP4_ARRAY(uint8_t, 0x00854013)[i];
    GP4_GLOBAL(uint8_t, 0x0066d7a4) = mode;
    GP4_GLOBAL(uint8_t, 0x00a6e21c) = mode;
    GP4_GLOBAL(uint8_t, 0x007ad81d) = GP4_ARRAY(uint8_t, 0x00854016)[i];
    GP4_GLOBAL(uint8_t, 0x009e8570) = GP4_ARRAY(uint8_t, 0x00854019)[i];

    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007c2e6c);
    if (enabled != 0u) {
        GP4_GLOBAL(uint8_t, 0x00679d18) = GP4_ARRAY(uint8_t, 0x0085401c)[i];
        GP4_GLOBAL(uint8_t, 0x00679d19) = GP4_ARRAY(uint8_t, 0x0085401f)[i];
        GP4_GLOBAL(uint8_t, 0x00679d1a) = GP4_ARRAY(uint8_t, 0x00854022)[i];
        GP4_GLOBAL(uint8_t, 0x00679d1b) = GP4_ARRAY(uint8_t, 0x00854025)[i];
        GP4_GLOBAL(uint8_t, 0x00679d1c) = GP4_ARRAY(uint8_t, 0x00854028)[i];
        GP4_GLOBAL(uint32_t, 0x00852d44) = GP4_ARRAY(uint32_t, 0x0085402c)[i];
        GP4_GLOBAL(uint32_t, 0x00852d48) = GP4_ARRAY(uint32_t, 0x00854038)[i];
        GP4_GLOBAL(uint32_t, 0x0066d84c) = GP4_ARRAY(uint32_t, 0x00854044)[i];
        GP4_GLOBAL(uint32_t, 0x0066d850) = GP4_ARRAY(uint32_t, 0x00854050)[i];
        GP4_GLOBAL(uint32_t, 0x00852d64) = GP4_ARRAY(uint32_t, 0x0085405c)[i];
        GP4_GLOBAL(uint32_t, 0x00891514) = GP4_ARRAY(uint32_t, 0x00854068)[i];
        GP4_GLOBAL(uint32_t, 0x00891518) = GP4_ARRAY(uint32_t, 0x00854074)[i];
        GP4_GLOBAL(uint32_t, 0x0089151c) = GP4_ARRAY(uint32_t, 0x00854080)[i];
        GP4_GLOBAL(uint16_t, 0x00852d68) = GP4_ARRAY(uint16_t, 0x0085408c)[i];
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(enabled, 0u) & ~1u);
}
GP4_IMPL(0x0051006c, FUN_0051006c, "regs(eax:u32[1..3]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
