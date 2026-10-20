#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint32_t rotateLeft3(uint32_t value) { return (value << 3) | (value >> 29); }

// Checksums the [0x007a97ac] bytes of the packet buffer 0x007a97b0 and appends the checksum there:
// h = rol(h, 3) + dword for every full dword, then rol(h, 3) + word if bit 1 of the length is set,
// then rol(h, 3) + byte if bit 0 is set. The 32-bit checksum is stored (unaligned) right after the
// data and the length grows by 4. Result registers: eax = ebx = checksum, ecx = length, esi = the
// address after the data; the flags are those of the closing `add dword [0x7a97ac], 4`.
// FUNCTION: GP4 0x0054161c
void __cdecl FUN_0054161c(gp4::Regs* r) {
    const uint32_t length = GP4_GLOBAL(uint32_t, 0x007a97ac);
    const uint8_t* cursor = GP4_ARRAY(uint8_t, 0x007a97b0);
    uint32_t hash = 0u;
    for (uint32_t words = length >> 2; words != 0u; --words) {
        hash = rotateLeft3(hash) + *reinterpret_cast<const uint32_t*>(cursor);
        cursor += 4;
    }
    if ((length & 2u) != 0u) {
        hash = rotateLeft3(hash) + *reinterpret_cast<const uint16_t*>(cursor);
        cursor += 2;
    }
    if ((length & 1u) != 0u) {
        hash = rotateLeft3(hash) + *cursor;
        cursor += 1;
    }
    *reinterpret_cast<uint32_t*>(const_cast<uint8_t*>(cursor)) = hash;
    GP4_GLOBAL(uint32_t, 0x007a97ac) = length + 4u;

    r->eax = hash;
    r->ebx = hash;
    r->ecx = length;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(length, 4u));
}
GP4_IMPL(0x0054161c, FUN_0054161c, "regs() -> (eax, ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0054b978")
