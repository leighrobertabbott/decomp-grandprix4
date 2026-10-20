#include <gp4/gp4.h>
#include <gp4/flags.h>

// Applies the two bit-masks and the byte pokes of the block 0x0083f0aa. When bit 7 of the byte
// 0x0081d1fb is clear the 128-bit mask edi:ebx:edx:eax = 0x1c:0xfff0:0x07ff803f:0xfc01ffe0 (else 0)
// goes to FUN_0053e5b1, then edx:eax = 0:0xffffff00 (0:0xff when the bit is set) to FUN_0053d1f4,
// then ebx = the triplet list 0x0081d34f (0x0081d342 when the bit is set) to FUN_00529b54, always
// with esi = 0x83f0aa. The callees restore their registers; the flags are those of the last one
// and the caller sees eax, edx, ebx, esi and edi as left here.
// FUNCTION: GP4 0x00529ad4
void __cdecl FUN_00529ad4(gp4::Regs* r) {
    r->edi = 0u;
    r->ebx = 0u;
    r->edx = 0u;
    r->eax = 0u;
    if ((GP4_GLOBAL(uint8_t, 0x0081d1fb) & 0x80u) == 0u) {
        r->eax = 0xfc01ffe0u;
        r->edx = 0x07ff803fu;
        r->ebx = 0xfff0u;
        r->edi = 0x1cu;
    }
    r->esi = 0x0083f0aau;
    gp4::call_regs(0x0053e5b1, *r);

    r->eax = 0xffu;
    r->edx = 0u;
    if ((GP4_GLOBAL(uint8_t, 0x0081d1fb) & 0x80u) == 0u) {
        r->eax = 0xffffff00u;
        r->edx = 0u;
    }
    r->esi = 0x0083f0aau;
    gp4::call_regs(0x0053d1f4, *r);

    r->ebx = 0x0081d342u;
    if ((GP4_GLOBAL(uint8_t, 0x0081d1fb) & 0x80u) == 0u) {
        r->ebx = 0x0081d34fu;
    }
    r->esi = 0x0083f0aau;
    gp4::call_regs(0x00529b54, *r);
}
GP4_IMPL(0x00529ad4, FUN_00529ad4, "regs() -> (eax, edx, ebx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=vals_00529148+menu_0083f0aa")
