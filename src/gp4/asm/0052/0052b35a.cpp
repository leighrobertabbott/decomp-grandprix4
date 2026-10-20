#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies a token from the string at esi to the buffer at edi: at most 0x20 characters, stopping at the
// first character <= 0x20, ':' or '='. The stopper is consumed (esi is one past it, al holds it) but not
// copied. When 0x20 characters were copied without a stopper "..." is appended. Either way a zero byte
// ends the output at edi. ecx counts the characters left. The flags are those of the last compare, or of
// the final `dec ecx` (zero result, CF kept from `cmp al, 0x3d`) when the copy ran to its limit.
// FUNCTION: GP4 0x0052b35a
void __cdecl FUN_0052b35a(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t count = 0x20u;
    uint32_t ch;
    uint32_t arith;

    for (;;) {
        ch = *src++;
        if (ch <= 0x20u) {
            arith = gp4::flags::sub8(ch, 0x20u);
            break;
        }
        if (ch == 0x3au) {
            arith = gp4::flags::sub8(ch, 0x3au);
            break;
        }
        const uint32_t last = gp4::flags::sub8(ch, 0x3du);
        if (ch == 0x3du) {
            arith = last;
            break;
        }
        *dst++ = static_cast<uint8_t>(ch);
        arith = gp4::flags::dec32(count, last);
        --count;
        if (count == 0u) {
            ch = 0x2eu;
            *dst++ = 0x2eu;
            *dst++ = 0x2eu;
            *dst++ = 0x2eu;
            break;
        }
    }
    *dst = 0u;

    r->eax = (r->eax & 0xffffff00u) | ch;
    r->ecx = count;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052b35a, FUN_0052b35a, "regs(eax:u32, esi:ptr[0x40]:u8{=0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x20,0x3a,0x3d}, edi:ptr[0x40]:bytes) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of)")
