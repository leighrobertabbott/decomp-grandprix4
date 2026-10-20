#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies a zero-terminated string from esi to edi, at most 0x17 bytes (the terminator is not
// copied). esi and ecx are preserved; edi is advanced past the copied bytes; al = the last byte
// read; the flags are those of the last `test al, 0xff`.
// FUNCTION: GP4 0x00516d94
void __cdecl FUN_00516d94(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    uint8_t ch = static_cast<uint8_t>(r->eax);
    uint32_t count = 0x17;
    do {
        ch = *src++;
        if (ch == 0) {
            break;
        }
        *dst++ = ch;
        --count;
    } while (count != 0);

    r->eax = (r->eax & 0xffffff00u) | ch;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(ch));
}
GP4_IMPL(0x00516d94, FUN_00516d94, "regs(eax:u32, esi:ptr[0x40]:bytes, edi:ptr[0x40]:bytes) -> (eax, edi, cf, pf, af, zf, sf, of)")
