#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0055906c
// Linear search of the table at 0x00961b9c (entries of 10 bytes: dword, dword, word) for the key
// stored at 0x00962f9c / 0x00962fa0 / 0x00962fa4. The count is the dword 0x009559f4 (the first entry
// is compared before the count is looked at). eax = index of the match (or the count when nothing
// matches), edx = the key's second dword. esi/edi/ecx/ebp are saved and restored. CF reports the
// result (clc = found, stc = not found); the other flags are those of the last compare (the equal
// `cmp bp, [edi+8]` of the match, or `cmp eax, count` which ended the scan).
void __cdecl FUN_0055906c(gp4::Regs* r) {
    const uint32_t keyA = GP4_GLOBAL(uint32_t, 0x00962f9c);
    const uint32_t keyB = GP4_GLOBAL(uint32_t, 0x00962fa0);
    const uint16_t keyC = GP4_GLOBAL(uint16_t, 0x00962fa4);

    const uint8_t* entry = GP4_ARRAY(const uint8_t, 0x00961b9c);
    uint32_t index = 0u;
    uint32_t arith;
    uint32_t carry;
    for (;;) {
        if (keyA == *reinterpret_cast<const uint32_t*>(entry) &&
            keyB == *reinterpret_cast<const uint32_t*>(entry + 4) &&
            keyC == *reinterpret_cast<const uint16_t*>(entry + 8)) {
            arith = gp4::flags::sub16(keyC, *reinterpret_cast<const uint16_t*>(entry + 8));
            carry = 0u;
            break;
        }
        entry += 10;
        index += 1u;
        const uint32_t count = GP4_GLOBAL(uint32_t, 0x009559f4);
        arith = gp4::flags::sub32(index, count);
        if (index == count) {
            carry = 1u;
            break;
        }
    }

    r->eax = index;
    r->edx = keyB;
    r->eflags = gp4::flags::merge(r->eflags, (arith & ~gp4::flags::CF) | carry);
}
GP4_IMPL(0x0055906c, FUN_0055906c, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=keys_0055906c")
