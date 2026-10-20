#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes a driver-name string through the character writer 0x0051f3bd (al = character). The first word is
// skipped when the string has a space within its first 14 bytes (the writer then starts after the space;
// without a space, or a NUL first, it starts at the beginning). A name that starts with "Schu" is written as
// its first character of the string, a '.', and at most 10 further characters; any other name as at most 12
// characters, both ending at the NUL. esi, ecx and edx are saved and restored; eax, edi and the other registers
// come from the last callee. The flags are those of the final `test al, 0xff` (NUL) or of the last callee.
// FUNCTION: GP4 0x00516d38
void __cdecl FUN_00516d38(gp4::Regs* r) {
    const uint8_t* const start = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;

    const uint8_t* cursor = start;
    uint32_t count = 14u;
    for (;;) {
        const uint32_t ch = *cursor;
        cursor += 1;
        if (ch == 0u) {
            cursor = start;
            break;
        }
        if (ch == 0x20u) {
            break;
        }
        count -= 1u;
        if (count == 0u) {
            cursor = start;
            break;
        }
    }

    if (cursor[0] == 0x53u && cursor[1] == 0x63u && cursor[2] == 0x68u && cursor[3] == 0x75u) {
        c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor));
        c.ecx = 14u;
        c.edx = r->esi;
        c.eax = (c.eax & 0xffffff00u) | start[0];
        gp4::call_regs(0x0051f3bd, c);
        c.eax = (c.eax & 0xffffff00u) | 0x2eu;
        gp4::call_regs(0x0051f3bd, c);
        count = 10u;
    } else {
        count = 12u;
    }

    uint32_t flags = c.eflags;
    for (;;) {
        const uint32_t ch = *cursor;
        cursor += 1;
        c.eax = (c.eax & 0xffffff00u) | ch;
        flags = gp4::flags::logic8(ch);
        if (ch == 0u) {
            break;
        }
        c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor));
        c.ecx = count;
        c.edx = r->esi;
        c.eflags = gp4::flags::merge(c.eflags, flags);
        gp4::call_regs(0x0051f3bd, c);
        flags = c.eflags;
        count -= 1u;
        if (count == 0u) {
            break;
        }
    }
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00516d38, FUN_00516d38, "regs(eax:u32, esi:ptr[0x40]:u32{=0x75686353,0x75686353,0x75686353,0x75686353,0x41686353,0x41416353,0x41414153,0x41414141,0x20202020,0}, edi:ptr[0x100]:bytes) -> (eax, ebx, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329")
