#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores a driver-name string into slot [esi] of the 24-byte name table at 0x007c2b3c: the first
// byte at esi is the slot index, the NUL-terminated text that follows is copied into the slot
// (at most 0x17 characters) and the slot is NUL-terminated. Registers left behind:
// esi = one past the last byte read, edi = one past the terminator written, ecx = 0x17 minus the
// characters copied, eax = the slot address with its low byte cleared (the last `lodsb` /
// `mov al, 0` leaves al = 0). Flags: a NUL source byte = `or al, al` on zero; a full 0x17-character
// copy = the final `dec ecx` (CF kept from the `or`).
// FUNCTION: GP4 0x00540825
void __cdecl FUN_00540825(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t index = *src++;
    const uint32_t slot = index * 0x18u + 0x007c2b3cu;
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(slot));
    uint32_t remaining = 0x17u;
    uint32_t arith;
    for (;;) {
        const uint8_t ch = *src++;
        arith = gp4::flags::logic8(ch);                  // or al, al
        if (ch == 0u) {
            break;
        }
        *dst++ = ch;
        arith = gp4::flags::dec32(remaining, arith);     // dec ecx (CF kept)
        remaining -= 1u;
        if (remaining == 0u) {
            break;
        }
    }
    *dst++ = 0u;
    r->eax = slot & 0xffffff00u;
    r->ecx = remaining;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00540825, FUN_00540825, "regs(esi:ptr[0x40]:layout=name_00540825) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of)")
