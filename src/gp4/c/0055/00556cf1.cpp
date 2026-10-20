#include <gp4/gp4.h>
#include <gp4/flags.h>

// Ring-record step: runs the high-water-mark marker 0x00557f0e for the record edi (its eax/edx/flags results are
// kept), then sets bit 2 of the record byte +0x64 when the pending flag byte [0x00868584] is nonzero, and clears
// both pending bytes [0x00868584] / [0x00868585] when the second one is 0xff. The flags are those of the final
// `cmp byte [0x00868585], 0xff`.
// FUNCTION: GP4 0x00556cf1
void __cdecl FUN_00556cf1(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00557f0e, c);

    if (GP4_GLOBAL(uint8_t, 0x00868584) != 0u) {
        GP4_FIELD(uint8_t, c.edi, 0x64) = static_cast<uint8_t>(GP4_FIELD(uint8_t, c.edi, 0x64) | 4u);
    }
    const uint8_t second = GP4_GLOBAL(uint8_t, 0x00868585);
    if (second == 0xffu) {
        GP4_GLOBAL(uint8_t, 0x00868584) = 0u;
        GP4_GLOBAL(uint8_t, 0x00868585) = 0u;
    }

    r->eax = c.eax;
    r->edx = c.edx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(second, 0xffu));
}
GP4_IMPL(0x00556cf1, FUN_00556cf1, "regs(edx:u32, edi:ptr[0x70]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=hwm_00557f0e+masks_008705f4")
