#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the 22-byte per-car flag table at 0x007c2a48 and, for every byte whose top two bits are both set,
// clears them (`and byte [ebx], 0x3f`). Exit state: ebx past the table, ecx = 0, al = (last byte & 0xc0).
// The flags are those of the last `inc ebx`, which keeps the CF of the last `cmp al, 0xc0` (or of the `and`
// on the clearing path).
// FUNCTION: GP4 0x0054ad66
void __cdecl FUN_0054ad66(gp4::Regs* r) {
    uint32_t cursor = 0x007c2a48u;
    uint32_t masked = 0u;
    uint32_t arith = 0u;
    for (uint32_t remaining = 0x16u; remaining != 0u; --remaining) {
        uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(cursor));
        masked = *entry & 0xc0u;
        arith = gp4::flags::sub8(masked, 0xc0u);
        if (masked == 0xc0u) {
            const uint8_t cleared = static_cast<uint8_t>(*entry & 0x3fu);
            *entry = cleared;
            arith = gp4::flags::logic8(cleared);
        }
        arith = gp4::flags::inc32(cursor, arith);
        cursor = cursor + 1u;
    }
    r->eax = (r->eax & 0xffffff00u) | masked;
    r->ebx = cursor;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054ad66, FUN_0054ad66, "regs(eax:u32) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043bcd4")
