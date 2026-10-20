#include <gp4/gp4.h>

// Saves the caller's register block (ebx, eax, ebp, ecx, edi and dh) into a
// register-save area at 0x008280dd.., clears three flag bytes and copies the
// byte at 0x00828106 to 0x00828101 (leaving it in al).
// FUNCTION: GP4 0x0053398b
void __cdecl FUN_0053398b(gp4::Regs* r) {
    const uint32_t eax = r->eax;
    const uint32_t ebx = r->ebx;
    const uint32_t ecx = r->ecx;
    const uint32_t edx = r->edx;
    const uint32_t ebp = r->ebp;
    const uint32_t edi = r->edi;

    GP4_FIELD(uint32_t, (uint8_t*)0, 0x008280dd) = ebx;
    GP4_FIELD(uint32_t, (uint8_t*)0, 0x008280e1) = eax;
    GP4_FIELD(uint32_t, (uint8_t*)0, 0x008280e5) = ebp;
    GP4_FIELD(uint32_t, (uint8_t*)0, 0x008280e9) = ecx;
    GP4_FIELD(uint32_t, (uint8_t*)0, 0x008280fd) = edi;
    GP4_FIELD(uint32_t, (uint8_t*)0, 0x008280f1) = (edx >> 8) & 0xff;
    GP4_FIELD(uint8_t, (uint8_t*)0, 0x00828103) = 0;
    GP4_FIELD(uint8_t, (uint8_t*)0, 0x00828104) = 0;
    GP4_FIELD(uint8_t, (uint8_t*)0, 0x00828105) = 0;
    const uint8_t b = GP4_FIELD(uint8_t, (uint8_t*)0, 0x00828106);
    GP4_FIELD(uint8_t, (uint8_t*)0, 0x00828101) = b;

    r->eax = (eax & 0xffffff00u) | b;
}
GP4_IMPL(0x0053398b, FUN_0053398b, "regs(eax:u32, ebp:u32, ebx:u32, ecx:u32, edi:u32, edx:u32) -> (eax); globals=fuzz:i32")
