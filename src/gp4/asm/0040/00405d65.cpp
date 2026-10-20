#include <gp4/gp4.h>

// FUNCTION: GP4 0x00405d65
// `push ebx ; push ecx ; mov ecx,[esi] ; mov ebx,[esi+4] ; call 0x00405d05 ; pop ecx ; pop ebx ; ret`:
// walks the 0x160-byte records at 0x008943a0 from edi to the one containing the point
// ([esi], [esi+4]); ebx and ecx are restored, eax/edx/edi and the flags come from the walk.
void __cdecl FUN_00405d65(gp4::Regs* r) {
    const uint32_t savedEbx = r->ebx;
    const uint32_t savedEcx = r->ecx;
    const uint8_t* point = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->ecx = *reinterpret_cast<const uint32_t*>(point);
    r->ebx = *reinterpret_cast<const uint32_t*>(point + 4);
    gp4::call_regs(0x00405d05, *r);
    r->ecx = savedEcx;
    r->ebx = savedEbx;
}
GP4_IMPL(0x00405d65, FUN_00405d65, "regs(esi:ptr[0x40]:bytes, edi:rec[0x8943a0,0x160,0..25]) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
