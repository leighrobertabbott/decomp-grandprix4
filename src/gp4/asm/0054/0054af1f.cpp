#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the zero-terminated string at esi into the 0x101-byte buffer at 0x007a97b0 (at most 0x100
// bytes), then writes a terminating zero at the end. eax's low byte = the last byte read, esi = past
// the last byte read, edi = past the last byte stored (the terminator slot), ecx = 0x100 - bytes
// stored. The flags are those of the last `or al, al`.
// FUNCTION: GP4 0x0054af1f
void __cdecl FUN_0054af1f(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007a97b0);

    uint32_t count = 0x100;
    uint8_t ch;
    do {
        ch = *src++;
        if (ch == 0) {
            break;
        }
        *dst++ = ch;
        --count;
    } while (count != 0);
    *dst = 0;

    r->eax = (r->eax & 0xffffff00u) | ch;
    r->ecx = count;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(ch));
}
GP4_IMPL(0x0054af1f, FUN_0054af1f, "regs(eax:u32, esi:ptr[0x108]:bytes) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of)")
