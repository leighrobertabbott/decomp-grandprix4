#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

// Derives a block of screen/viewport constants (0x8751a4..0x875244, 0x875290) from the
// ten-dword mode descriptor at 0x874f0c that the caller 0x525f6b copies in. Preserves every
// GPR (push/pop eax, ebp, edx); the flags left are those of the final `shl eax, 0xf`
// (the closing idiv leaves the flags untouched in the oracle).
// FUNCTION: GP4 0x005260cd
void __cdecl FUN_005260cd(gp4::Regs* r) {
    const uint32_t w = GP4_GLOBAL(uint32_t, 0x00874f0c);
    const uint32_t h = GP4_GLOBAL(uint32_t, 0x00874f10);
    const uint32_t a14 = GP4_GLOBAL(uint32_t, 0x00874f14);
    const uint32_t a18 = GP4_GLOBAL(uint32_t, 0x00874f18);
    const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x00874f20);

    GP4_GLOBAL(float, 0x00875290) =
        gp4::x87::narrow_f32(gp4::x87::ext_i32(0x280) / gp4::x87::ext_i32((int32_t)w));
    GP4_GLOBAL(uint32_t, 0x006f2bf8) = w;

    GP4_GLOBAL(uint32_t, 0x008751a4) = w;
    GP4_GLOBAL(uint32_t, 0x008751a8) = h;
    GP4_GLOBAL(uint32_t, 0x008751ac) = w >> 1;
    GP4_GLOBAL(uint32_t, 0x008751b0) = h >> 1;
    GP4_GLOBAL(uint32_t, 0x008751b4) = w + 1;
    GP4_GLOBAL(uint32_t, 0x008751b8) = h + 1;
    GP4_GLOBAL(uint32_t, 0x008751bc) = w - 1;
    GP4_GLOBAL(uint32_t, 0x008751c0) = h - 1;
    GP4_GLOBAL(uint32_t, 0x008751c4) = pitch;
    GP4_GLOBAL(uint32_t, 0x008751c8) = pitch - w;
    GP4_GLOBAL(uint32_t, 0x008751cc) = w * h;
    GP4_GLOBAL(uint32_t, 0x008751d0) = w << 8;
    GP4_GLOBAL(uint32_t, 0x008751d4) = h << 8;
    GP4_GLOBAL(uint32_t, 0x008751d8) = (h - 1) * pitch + w;
    GP4_GLOBAL(uint32_t, 0x008751dc) = h;
    GP4_GLOBAL(uint32_t, 0x008751e0) = 0;
    GP4_GLOBAL(uint32_t, 0x008751e4) = w;
    GP4_GLOBAL(uint32_t, 0x008751e8) = 0;
    GP4_GLOBAL(uint32_t, 0x008751ec) = a14;

    if (GP4_GLOBAL(int32_t, 0x00874f24) == -1) {
        GP4_GLOBAL(uint32_t, 0x008751f0) = 0x100;
        GP4_GLOBAL(uint32_t, 0x008751f4) = 0x100;
        GP4_GLOBAL(uint32_t, 0x008751f8) = 0x4000;
        GP4_GLOBAL(uint32_t, 0x008751fc) = 0x4000;
        GP4_GLOBAL(uint32_t, 0x00875200) = 0x4000;
        GP4_GLOBAL(uint32_t, 0x00875204) = 0x4000;
    } else {
        int32_t rem;
        GP4_GLOBAL(uint32_t, 0x008751f0) = w >> 1;
        GP4_GLOBAL(uint32_t, 0x008751f4) = h >> 1;
        const uint32_t sx = 0x800000u / w;
        GP4_GLOBAL(uint32_t, 0x008751f8) = sx;
        GP4_GLOBAL(int32_t, 0x00875200) =
            gp4::idiv64((int64_t)(uint32_t)(sx << 8), (int32_t)a14, &rem);
        const uint32_t sy = (a18 << 15) / h;
        GP4_GLOBAL(uint32_t, 0x008751fc) = sy;
        GP4_GLOBAL(int32_t, 0x00875204) =
            gp4::idiv64((int64_t)(uint32_t)(sy << 8), (int32_t)a14, &rem);
    }

    GP4_GLOBAL(uint32_t, 0x00875208) = 0;
    GP4_GLOBAL(uint32_t, 0x0087520c) = a18 << 6;
    GP4_GLOBAL(int32_t, 0x00875210) = 0x400000 / (int32_t)a18;
    GP4_GLOBAL(int32_t, 0x00875214) = 0x374000 / (int32_t)a18;
    GP4_GLOBAL(uint32_t, 0x00875258) = GP4_GLOBAL(uint32_t, 0x00874f28);
    GP4_GLOBAL(uint32_t, 0x0087525c) = GP4_GLOBAL(uint32_t, 0x00874f2c);

    GP4_GLOBAL(uint32_t, 0x00875218) = GP4_GLOBAL(uint32_t, 0x008751f0);
    GP4_GLOBAL(uint32_t, 0x0087521c) = GP4_GLOBAL(uint32_t, 0x008751f4);
    GP4_GLOBAL(uint32_t, 0x00875220) = GP4_GLOBAL(uint32_t, 0x008751a4);
    GP4_GLOBAL(uint32_t, 0x00875224) = GP4_GLOBAL(uint32_t, 0x008751a8);
    GP4_GLOBAL(uint32_t, 0x00875228) = GP4_GLOBAL(uint32_t, 0x008751ac);
    GP4_GLOBAL(uint32_t, 0x0087522c) = GP4_GLOBAL(uint32_t, 0x008751b0);
    GP4_GLOBAL(uint32_t, 0x00875230) = GP4_GLOBAL(uint32_t, 0x008751dc);
    GP4_GLOBAL(uint32_t, 0x00875234) = GP4_GLOBAL(uint32_t, 0x008751e0);
    GP4_GLOBAL(uint32_t, 0x00875238) = GP4_GLOBAL(uint32_t, 0x008751e4);
    GP4_GLOBAL(uint32_t, 0x0087523c) = GP4_GLOBAL(uint32_t, 0x008751e8);

    const uint32_t a30 = GP4_GLOBAL(uint32_t, 0x00874f30);
    const uint32_t v = (h >> 1) - a30;
    const uint32_t shifted = v << 15;
    GP4_GLOBAL(int32_t, 0x00875240) = (int32_t)shifted / (int32_t)h;
    GP4_GLOBAL(uint32_t, 0x00875244) = a30;

    // flags of `shl eax, 0xf`: CF = last bit out, AF = 0, OF = bit31(v<<14 ^ v<<15)
    uint32_t arith = gp4::flags::szp(shifted, 32);
    if ((v >> 17) & 1u) arith |= gp4::flags::CF;
    if (((v << 14) ^ shifted) >> 31) arith |= gp4::flags::OF;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005260cd, FUN_005260cd, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_00874f0c")
