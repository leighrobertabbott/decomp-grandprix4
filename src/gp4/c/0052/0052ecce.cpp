#include <gp4/gp4.h>

// `call 0x0052ceef ; stosb ; mov al, dl ; stosb ; mov al, 0 ; stosb ; stosb ; ret`: appends four bytes
// at edi: the low byte of the first count (eax), the low byte of the second count (edx), 0, 0, and
// advances edi by 4 (direction flag clear). eax comes back with its low byte cleared; edx and the
// flags are what 0x0052ceef left.
// FUNCTION: GP4 0x0052ecce
void __cdecl FUN_0052ecce(gp4::Regs* r) {
    gp4::call_regs(0x0052ceef, *r);
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    out[0] = static_cast<uint8_t>(r->eax);
    out[1] = static_cast<uint8_t>(r->edx);
    out[2] = 0u;
    out[3] = 0u;
    r->edi = r->edi + 4u;
    r->eax = r->eax & 0xffffff00u;
}
GP4_IMPL(0x0052ecce, FUN_0052ecce, "regs(edi:ptr[0x40]:bytes) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=layout=drv_005117d3")
