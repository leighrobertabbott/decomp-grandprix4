#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sorts the driver list at 0x00831a69 by the keys of the 22-dword table copy at 0x00822300.
// The 22 dwords at 0x007c3184 are copied to 0x00822300, 0x00548bfd compacts the flag bytes into the list,
// then bubble passes over the first [0x008236a8] bytes (pairs 0..n-2) swap neighbours until a pass swaps
// nothing. A byte's key is the table dword at (byte & 0x3f) - 1, or 0xffffffff when (byte & 0x3f) is 0;
// a pair is swapped when the left key is above the right key (unsigned). The smallest key at the head
// of the list ends in [0x00831b5c].
// Exit registers: eax = that key, ebx = list + n - 1, ecx = 0, edx = the last right key tested,
// esi = ((first & 0x3f) - 1) * 4, edi = the last right index * 4 (0 for a zero index), ebp = 0; the flags
// are those of the final `shl esi, 2`.
// FUNCTION: GP4 0x00548b68
void __cdecl FUN_00548b68(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    const uint32_t* source = GP4_ARRAY(const uint32_t, 0x007c3184);
    uint32_t* keys = GP4_ARRAY(uint32_t, 0x00822300);
    for (uint32_t i = 0u; i != 0x16u; ++i) {
        keys[i] = source[i];
    }
    c.eax = keys[0x15];
    c.ebx = 0x58u;
    c.ecx = 0u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(0x54u, 4u));
    gp4::call_regs(0x00548bfd, c);

    uint32_t rightKey = 0u;
    uint32_t rightIndex = 0u;
    uint32_t pairAddress = 0u;
    uint32_t swapped;
    do {
        pairAddress = 0x00831a69u;
        uint32_t left = GP4_GLOBAL(uint32_t, 0x008236a8) - 1u;
        swapped = 0u;
        do {
            uint8_t* pair = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(pairAddress));
            uint32_t leftKey = 0xffffffffu;
            const uint32_t leftIndex = pair[0] & 0x3fu;
            if (leftIndex != 0u) {
                leftKey = keys[leftIndex - 1u];
            }
            rightKey = 0xffffffffu;
            const uint32_t rightByte = pair[1] & 0x3fu;
            rightIndex = 0u;
            if (rightByte != 0u) {
                rightIndex = (rightByte - 1u) << 2;
                rightKey = keys[rightByte - 1u];
            }
            if (leftKey > rightKey) {
                const uint8_t first = pair[0];
                pair[0] = pair[1];
                pair[1] = first;
                swapped = 0xffffffffu;
            }
            ++pairAddress;
            --left;
        } while (left != 0u);
    } while (swapped == 0xffffffffu);

    const uint32_t head = (GP4_GLOBAL(uint8_t, 0x00831a69) & 0x3fu) - 1u;
    const uint32_t scaled = head << 2;
    const uint32_t smallest = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(scaled + 0x00822300u));
    GP4_GLOBAL(uint32_t, 0x00831b5c) = smallest;

    // shl esi, 2: CF = last bit out, AF = 0, OF = bit31(value << 1 ^ result) in the pinned oracle model.
    uint32_t flags = gp4::flags::szp(scaled, 32);
    if ((head >> 30) & 1u) {
        flags |= gp4::flags::CF;
    }
    if (((head << 1) ^ scaled) & 0x80000000u) {
        flags |= gp4::flags::OF;
    }

    r->eax = smallest;
    r->ebx = pairAddress;
    r->ecx = 0u;
    r->edx = rightKey;
    r->esi = scaled;
    r->edi = rightIndex;
    r->ebp = swapped;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00548b68, FUN_00548b68, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=neg22_00520023+sort_00548b68")
