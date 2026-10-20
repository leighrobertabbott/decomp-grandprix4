#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes the index byte cl at edi, then copies the NUL-terminated name at 0x007c2b3c + cl * 0x18
// (24-byte entries, "michael schumacher" first) after it, terminator included. eax = the entry
// address with al = 0, esi = past the terminator in the table, edi = past the terminator written;
// the flags are those of the final `or al, al` on the zero byte (ZF = PF = 1).
// FUNCTION: GP4 0x00540480
void __cdecl FUN_00540480(gp4::Regs* r) {
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t index = r->ecx & 0xffu;

    *dst++ = static_cast<uint8_t>(index);
    const uint8_t* src = GP4_ARRAY(const uint8_t, 0x007c2b3c) + index * 0x18u;
    const uint32_t entry = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    uint8_t ch;
    do {
        ch = *src++;
        *dst++ = ch;
    } while (ch != 0);

    r->eax = (entry & 0xffffff00u) | ch;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(ch));
}
GP4_IMPL(0x00540480, FUN_00540480, "regs(ecx:u32[0..255], edi:ptr[0x100]:bytes) -> (eax, esi, edi, cf, pf, af, zf, sf, of)")
