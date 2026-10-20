#include <gp4/gp4.h>
#include <gp4/flags.h>

// Verifies the two trailing checksum words of a block. esi points at ecx - 4 data bytes followed by
// two words: the first must equal the 16-bit byte sum, the second the rotating sum
// (bx = rol(bx, 3) + byte). A mismatch stores 6 in the byte at 0x00824566 and returns CF = 1;
// otherwise CF = 0. Leaves esi past the two words (esi + ecx), ecx = first stored word,
// eax = (eax & 0xffff0000) | second stored word, dx / bx = the two sums (upper halves kept),
// and the flags of the last 16-bit cmp with CF replaced by the result.
// The direction flag is clear everywhere in the game.

// FUNCTION: GP4 0x0053648e
void __cdecl FUN_0053648e(gp4::Regs* r) {
    const uint8_t* data = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t count = r->ecx - 4u;

    uint16_t sum = 0;
    uint16_t rolled = 0;
    do {
        rolled = static_cast<uint16_t>((rolled << 3) | (rolled >> 13));
        const uint16_t value = *data++;
        sum = static_cast<uint16_t>(sum + value);
        rolled = static_cast<uint16_t>(rolled + value);
        --count;
    } while (count != 0);

    const uint16_t storedSum = static_cast<uint16_t>(data[0] | (data[1] << 8));
    const uint16_t storedRolled = static_cast<uint16_t>(data[2] | (data[3] << 8));
    data += 4;

    uint32_t flags = gp4::flags::sub16(storedSum, sum);
    uint32_t carry;
    if (storedSum != sum) {
        carry = 1;
    } else {
        flags = gp4::flags::sub16(storedRolled, rolled);
        if (storedRolled != rolled) {
            carry = 1;
        } else {
            carry = 0;
        }
    }
    if (carry != 0) {
        GP4_GLOBAL(uint8_t, 0x00824566) = 6;
    }

    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data));
    r->ecx = storedSum;
    r->eax = (r->eax & 0xffff0000u) | storedRolled;
    r->edx = (r->edx & 0xffff0000u) | sum;
    r->ebx = (r->ebx & 0xffff0000u) | rolled;
    r->eflags = gp4::flags::merge(r->eflags, (flags & ~1u) | carry);
}
GP4_IMPL(0x0053648e, FUN_0053648e, "regs(eax:u32, ebx:u32, ecx:u32[5..8], edx:u32, esi:ptr[0x40]:u8[0..1]) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of)")
