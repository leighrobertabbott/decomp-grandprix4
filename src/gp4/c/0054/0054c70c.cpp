#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes one "name = value" line at the cursor edi: the text at esi padded to 0x20 columns
// (0x0054c6f6), the separator "=    " at 0x00833b16, then the value text found in the table at ebx:
// 8-byte entries {text pointer, key} searched from ebx until an entry's key equals eax or an
// entry's text pointer is zero (the terminator). A non-null text is expanded after the separator,
// then the line end text at 0x00833b13 follows (each expansion is 0x0054d7c0). esi and eax come back
// unchanged (the original saves them on the stack), ebx is left on the entry that was found, and
// the flags are those of the last expansion.
// FUNCTION: GP4 0x0054c70c
void __cdecl FUN_0054c70c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0054c6f6, c);
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x00833b16)));
    gp4::call_regs(0x0054d7c0, c);

    c.ebx = r->ebx;
    c.eax = r->eax;
    uint32_t entry = c.ebx;
    for (;;) {
        if (GP4_FIELD(uint32_t, entry, 4) == c.eax) {
            break;
        }
        if (GP4_FIELD(uint32_t, entry, 0) == 0u) {
            break;
        }
        entry += 8u;
    }
    c.ebx = entry;

    c.esi = GP4_FIELD(uint32_t, entry, 0);
    if (c.esi != 0u) {
        gp4::call_regs(0x0054d7c0, c);
    }
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(const char, 0x00833b13)));
    gp4::call_regs(0x0054d7c0, c);

    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054c70c, FUN_0054c70c, "regs(eax:u32{=0,1,2,3,4,5,0xffffffff,0x80,0xffffff80}, ebx:u32{=0x833a2c,0x833a44,0x833a6c}, edx:u32, ebp:u32, esi:ptr[0x40]:u8{=0,0,0,0,0x41,0x61,0x20,0x30,0x39}, edi:ptr[0x200]:bytes) -> (ebx, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
