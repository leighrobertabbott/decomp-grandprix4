#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint32_t rotateLeft3(uint32_t value) { return (value << 3) | (value >> 29); }

// Verifies the checksum of a received packet (twin of 0x0054161c, which appends it): the length
// [0x007a97ac] shrinks by 4 and the remaining bytes at 0x007a97b0 are hashed with
// h = rol(h, 3) + dword per full dword, then rol(h, 3) + word if bit 1 of the length is set, then
// rol(h, 3) + byte if bit 0 is set. The hash is compared with the dword that follows the data
// (`cmp eax, [esi]`); the caller reads the flags (equal = ZF). Result registers: eax = ebx = hash,
// ecx = the new length, esi = the address after the data.
// FUNCTION: GP4 0x0054166e
void __cdecl FUN_0054166e(gp4::Regs* r) {
    const uint32_t length = GP4_GLOBAL(uint32_t, 0x007a97ac) - 4u;
    GP4_GLOBAL(uint32_t, 0x007a97ac) = length;
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
    const uint32_t stored = *reinterpret_cast<const uint32_t*>(cursor);

    r->eax = hash;
    r->ebx = hash;
    r->ecx = length;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(hash, stored));
}
GP4_IMPL(0x0054166e, FUN_0054166e, "regs() -> (eax, ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=packet_0054166e")
