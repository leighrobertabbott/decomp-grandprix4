#include <gp4/gp4.h>
#include <gp4/flags.h>

// Checksum accumulator over ecx bytes at esi (ecx >= 1): for each byte, bx = rol16(bx, 3) + byte,
// dx += byte, bp |= byte (all 16-bit; upper halves kept). Leaves esi past the data, ecx = 0, eax's
// low word = the last byte. The flags are those of the final `dec ecx` (CF kept from `or bp, ax`).
// FUNCTION: GP4 0x00536437
void __cdecl FUN_00536437(gp4::Regs* r) {
    const uint8_t* data = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t count = r->ecx;

    uint16_t rolled = static_cast<uint16_t>(r->ebx);
    uint16_t sum = static_cast<uint16_t>(r->edx);
    uint16_t bits = static_cast<uint16_t>(r->ebp);
    uint16_t value = 0;
    uint32_t orFlags = 0;
    uint32_t prev = 0;
    do {
        rolled = static_cast<uint16_t>((rolled << 3) | (rolled >> 13));
        value = *data++;
        sum = static_cast<uint16_t>(sum + value);
        rolled = static_cast<uint16_t>(rolled + value);
        bits = static_cast<uint16_t>(bits | value);
        orFlags = gp4::flags::logic16(bits);
        prev = count;
        --count;
    } while (count != 0);

    r->eax = (r->eax & 0xffff0000u) | value;
    r->ebx = (r->ebx & 0xffff0000u) | rolled;
    r->edx = (r->edx & 0xffff0000u) | sum;
    r->ebp = (r->ebp & 0xffff0000u) | bits;
    r->ecx = count;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(prev, orFlags));
}
GP4_IMPL(0x00536437, FUN_00536437, "regs(eax:u32, ebx:u32, ecx:u32[1..0x40], edx:u32, ebp:u32, esi:ptr[0x48]:bytes) -> (eax, ebx, ecx, edx, ebp, esi, cf, pf, af, zf, sf, of)")
