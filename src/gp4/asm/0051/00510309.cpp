#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline void copyBytes(uint8_t* dst, const uint8_t* src, uint32_t n) {
    for (; n != 0u; --n) {
        *dst = *src;
        ++dst;
        ++src;
    }
}

// FUNCTION: GP4 0x00510309
// Copies the current state into slot eax (1..3) of the per-slot tables at 0x853ac2..0x85408c:
// 0x50 bytes from 0x73d1bc, 0xc bytes from 0x679d20, and (when byte 0x7c2e6c is set) 0x48 bytes
// from 0x89108d and 0xc4 bytes from 0x7ab02c, then a list of dword/word/byte scalars. Some dword
// tables are indexed with a stride of 2 (as in the original). pushal/popal restore every GPR; the
// flags are those of the second `cmp byte [0x7c2e6c], 0`.
void __cdecl FUN_00510309(gp4::Regs* r) {
    const uint32_t i = r->eax - 1u;
    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007c2e6c);

    copyBytes(GP4_ARRAY(uint8_t, 0x00853ac2) + i * 0x50u, GP4_ARRAY(uint8_t, 0x0073d1bc), 0x50u);
    copyBytes(GP4_ARRAY(uint8_t, 0x00853bb2) + i * 0x0cu, GP4_ARRAY(uint8_t, 0x00679d20), 0x0cu);
    if (enabled != 0u) {
        copyBytes(GP4_ARRAY(uint8_t, 0x00853bd6) + i * 0x48u, GP4_ARRAY(uint8_t, 0x0089108d), 0x48u);
        copyBytes(GP4_ARRAY(uint8_t, 0x00853cae) + i * 0xc4u, GP4_ARRAY(uint8_t, 0x007ab02c), 0xc4u);
    }

    GP4_ARRAY(uint32_t, 0x00853efc)[i] = GP4_GLOBAL(uint32_t, 0x00679d30);
    GP4_ARRAY(uint32_t, 0x00853f14)[i] = GP4_GLOBAL(uint32_t, 0x00679d2c);
    GP4_ARRAY(uint32_t, 0x00853f20)[i] = GP4_GLOBAL(uint32_t, 0x006f3aac);
    GP4_ARRAY(uint32_t, 0x00853f2c)[i] = GP4_GLOBAL(uint32_t, 0x006f3ab0);
    GP4_ARRAY(uint32_t, 0x00853f38)[i] = GP4_GLOBAL(uint32_t, 0x00875228);
    GP4_ARRAY(uint32_t, 0x00853f44)[i] = GP4_GLOBAL(uint32_t, 0x0087522c);
    GP4_ARRAY(uint32_t, 0x00853f50)[i] = GP4_GLOBAL(uint32_t, 0x00875274);
    GP4_ARRAY(uint32_t, 0x00853f5c)[i] = GP4_GLOBAL(uint32_t, 0x007a8dd4);
    GP4_ARRAY(uint32_t, 0x00853f68)[i] = GP4_GLOBAL(uint32_t, 0x007a8dd8);
    GP4_ARRAY(uint32_t, 0x00853f74)[i] = GP4_GLOBAL(uint32_t, 0x007a8ddc);
    GP4_ARRAY(uint32_t, 0x00853f80)[i] = GP4_GLOBAL(uint32_t, 0x007ad818);
    GP4_ARRAY(uint32_t, 0x00853f8c)[i] = GP4_GLOBAL(uint32_t, 0x00679bd8);
    GP4_ARRAY(uint32_t, 0x00853f98)[i] = GP4_GLOBAL(uint32_t, 0x00679bdc);
    GP4_ARRAY(uint32_t, 0x00853fa4)[i] = GP4_GLOBAL(uint32_t, 0x00679be0);
    GP4_ARRAY(uint32_t, 0x00853fb0)[i] = GP4_GLOBAL(uint32_t, 0x00679be4);
    GP4_ARRAY(uint32_t, 0x00853fbc)[i] = GP4_GLOBAL(uint32_t, 0x00679be8);
    GP4_ARRAY(uint32_t, 0x00853fc8)[i] = GP4_GLOBAL(uint32_t, 0x00875278);
    *reinterpret_cast<uint32_t*>(GP4_ARRAY(uint8_t, 0x00853fd4) + i * 2u) = GP4_GLOBAL(uint32_t, 0x006f3b00);
    *reinterpret_cast<uint32_t*>(GP4_ARRAY(uint8_t, 0x00853fe0) + i * 2u) = GP4_GLOBAL(uint32_t, 0x006f3b04);
    *reinterpret_cast<uint32_t*>(GP4_ARRAY(uint8_t, 0x00853fec) + i * 2u) = GP4_GLOBAL(uint32_t, 0x006f3b08);
    GP4_ARRAY(uint16_t, 0x00853ff8)[i] = GP4_GLOBAL(uint16_t, 0x00679b5c);
    GP4_ARRAY(uint16_t, 0x00853ffe)[i] = GP4_GLOBAL(uint16_t, 0x00679b5e);
    GP4_ARRAY(uint16_t, 0x00854004)[i] = GP4_GLOBAL(uint16_t, 0x00891510);
    GP4_ARRAY(uint16_t, 0x0085400a)[i] = GP4_GLOBAL(uint16_t, 0x0066d7a6);
    GP4_ARRAY(uint8_t, 0x00854010)[i] = GP4_GLOBAL(uint8_t, 0x00852df7);
    GP4_ARRAY(uint8_t, 0x00854013)[i] = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    GP4_ARRAY(uint8_t, 0x00854016)[i] = GP4_GLOBAL(uint8_t, 0x007ad81d);
    GP4_ARRAY(uint8_t, 0x00854019)[i] = GP4_GLOBAL(uint8_t, 0x009e8570);

    const uint8_t enabledAgain = GP4_GLOBAL(uint8_t, 0x007c2e6c);
    if (enabledAgain != 0u) {
        GP4_ARRAY(uint8_t, 0x0085401c)[i] = GP4_GLOBAL(uint8_t, 0x00679d18);
        GP4_ARRAY(uint8_t, 0x0085401f)[i] = GP4_GLOBAL(uint8_t, 0x00679d19);
        GP4_ARRAY(uint8_t, 0x00854022)[i] = GP4_GLOBAL(uint8_t, 0x00679d1a);
        GP4_ARRAY(uint8_t, 0x00854025)[i] = GP4_GLOBAL(uint8_t, 0x00679d1b);
        GP4_ARRAY(uint8_t, 0x00854028)[i] = GP4_GLOBAL(uint8_t, 0x00679d1c);
        GP4_ARRAY(uint32_t, 0x0085402c)[i] = GP4_GLOBAL(uint32_t, 0x00852d44);
        GP4_ARRAY(uint32_t, 0x00854038)[i] = GP4_GLOBAL(uint32_t, 0x00852d48);
        GP4_ARRAY(uint32_t, 0x00854044)[i] = GP4_GLOBAL(uint32_t, 0x0066d84c);
        GP4_ARRAY(uint32_t, 0x00854050)[i] = GP4_GLOBAL(uint32_t, 0x0066d850);
        GP4_ARRAY(uint32_t, 0x0085405c)[i] = GP4_GLOBAL(uint32_t, 0x00852d64);
        GP4_ARRAY(uint32_t, 0x00854068)[i] = GP4_GLOBAL(uint32_t, 0x00891514);
        GP4_ARRAY(uint32_t, 0x00854074)[i] = GP4_GLOBAL(uint32_t, 0x00891518);
        GP4_ARRAY(uint32_t, 0x00854080)[i] = GP4_GLOBAL(uint32_t, 0x0089151c);
        GP4_ARRAY(uint16_t, 0x0085408c)[i] = GP4_GLOBAL(uint16_t, 0x00852d68);
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(enabledAgain, 0u));
}
GP4_IMPL(0x00510309, FUN_00510309, "regs(eax:u32[1..3]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
