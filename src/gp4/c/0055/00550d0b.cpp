#include <gp4/gp4.h>
#include <gp4/flags.h>

// Unless the mode byte [0x0066d7a4] is 0x80: clears the sign/step dword [0x0073d1bc], computes the
// angular distances of the ring record [0x00679b78] with 0x00526f21 (which leaves the "behind"
// marker 0 or 0x8000 in the word [0x00875e92]) and sets [0x0073d1bc] to 3 when the marker is below
// 0x4000 or above 0xc000, else to -3. Mode 0x80 only compares and leaves. eax/ecx/edx/edi are those
// the callee leaves (edi = [0x00679b78], ax = the marker word); the flags are those of the last
// `cmp` (`cmp byte [0x0066d7a4], 0x80`, `cmp ax, 0x4000` or `cmp ax, 0xc000`).
// FUNCTION: GP4 0x00550d0b
void __cdecl FUN_00550d0b(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    if (mode == 0x80u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0x80u));
        return;
    }

    GP4_GLOBAL(uint32_t, 0x0073d1bc) = 0u;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = GP4_GLOBAL(uint32_t, 0x00679b78);
    gp4::call_regs(0x00526f21, c);

    const uint16_t marker = GP4_GLOBAL(uint16_t, 0x00875e92);
    uint32_t arith = gp4::flags::sub16(marker, 0x4000u);
    if (marker < 0x4000u) {
        GP4_GLOBAL(uint32_t, 0x0073d1bc) = 3u;
    } else {
        arith = gp4::flags::sub16(marker, 0xc000u);
        if (marker > 0xc000u) {
            GP4_GLOBAL(uint32_t, 0x0073d1bc) = 3u;
        } else {
            GP4_GLOBAL(uint32_t, 0x0073d1bc) = 0xfffffffdu;
        }
    }

    r->eax = (c.eax & 0xffff0000u) | marker;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00550d0b, FUN_00550d0b, "regs(eax:u32, ecx:u32, edx:u32, edi:u32) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679b78+ring_00555b28+ring2_00526f21+ref_00679b90+mode_00550d0b")
