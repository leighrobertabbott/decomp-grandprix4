#include <gp4/gp4.h>
#include <gp4/flags.h>

// Expands the 0x40-byte text script number [0x007ab4a4] of the table at 0x007ab1c4 (esi = 0x007ab1c4 + 0x40 * index,
// eax keeps that address) into the buffer 0x007ab4cc (edi) with the script interpreter 0x0054d7c0. esi and edi come
// back advanced; the flags are those the interpreter leaves.
// FUNCTION: GP4 0x0053f15c
void __cdecl FUN_0053f15c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x007ab4a4)) * 0x40u + 0x007ab1c4u;
    c.esi = c.eax;
    c.edi = 0x007ab4ccu;
    gp4::call_regs(0x0054d7c0, c);
    *r = c;
}
GP4_IMPL(0x0053f15c, FUN_0053f15c, "regs(ebx:u32, ecx:u32, edx:u32, ebp:u32) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=items_0054d7c0")
