#include <gp4/gp4.h>
#include <gp4/flags.h>

// Key/value lookup in a byte table at ebx: byte 0 is the entry count n, followed by n (key, value)
// byte pairs. Returns eax = the value of the first pair whose key equals al, or 0 when none does.
// The loop runs on ecx = n, so n = 0 would wrap to 4G iterations (callers never pass it). ebx, ecx
// and edx are pushed and restored. Flags: found = the equal `cmp al, [key]`; not found = the final
// `dec ecx` (CF kept from the `add ebx, 2` just before it).
// FUNCTION: GP4 0x0052ca1b
void __cdecl FUN_0052ca1b(gp4::Regs* r) {
    const uint8_t key = static_cast<uint8_t>(r->eax);
    const uint32_t table = r->ebx;
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(table));
    uint32_t count = bytes[0];
    uint32_t offset = 1u;
    uint32_t arith;
    uint32_t result;
    for (;;) {
        const uint8_t entry = bytes[offset];
        arith = gp4::flags::sub8(key, entry);              // cmp al, [ebx]
        if (key == entry) {
            result = bytes[offset + 1u];
            break;
        }
        arith = gp4::flags::add32(table + offset, 2u);     // add ebx, 2
        offset += 2u;
        arith = gp4::flags::dec32(count, arith);           // dec ecx (CF kept)
        count -= 1u;
        if (count == 0u) {
            result = 0u;
            break;
        }
    }
    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052ca1b, FUN_0052ca1b, "regs(eax:u32[1..8], ebx:ptr[0x40]:u8[1..8]) -> (eax, cf, pf, af, zf, sf, of)")
