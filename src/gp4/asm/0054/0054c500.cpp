#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054c500
// Token copier. esi = source text, edi = destination, ecx = maximum number of bytes to copy.
// A leading '"' or '\'' is skipped and becomes the terminator (the token then runs to the matching
// quote and may contain spaces); otherwise the token stops at the first byte <= 0x20. The token is
// stored at edi and a zero byte is appended. eax and ecx are preserved (pushed and popped); esi and
// edi come back advanced. Quoted tokens leave esi one past the closing quote; unquoted ones leave
// esi on the terminating byte. Flags are those of the last compare (the unquoted path ends with a
// `dec esi`, which keeps the compare's CF).
void __cdecl FUN_0054c500(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t count = r->ecx;
    uint8_t quote = 0;

    uint8_t ch = *src;
    if (ch == 0x22u || ch == 0x27u) {
        quote = ch;
        ++src;
    }
    ch = *src++;

    uint32_t flags;
    if (quote != 0u) {
        for (;;) {
            flags = gp4::flags::sub8(ch, quote);
            if (ch == quote) {
                break;
            }
            flags = gp4::flags::sub32(count, 0u);
            if (count == 0u) {
                break;
            }
            *dst++ = ch;
            ch = *src++;
            --count;
        }
    } else {
        for (;;) {
            flags = gp4::flags::sub8(ch, 0x20u);
            if (ch <= 0x20u) {
                break;
            }
            flags = gp4::flags::sub32(count, 0u);
            if (count == 0u) {
                break;
            }
            *dst++ = ch;
            ch = *src++;
            --count;
        }
        flags = gp4::flags::dec32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src)), flags);
        --src;
    }
    *dst = 0;

    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054c500, FUN_0054c500, "regs(esi:ptr[0x40]:u8[0x20..0x30], edi:ptr[0x40]:bytes, ecx:u32[0..0x1f]) -> (esi, edi, cf, pf, af, zf, sf, of)")
