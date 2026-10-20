#include <gp4/gp4.h>

// FUNCTION: GP4 0x004102df
// Stores a Q14 fraction ((value + 400) << 14) / 800 into the word at esi+0x1e2. value is the byte
// at ebx+9, unless [0x00601f20] != 0xff and the mask byte [0x00601fab] hits the (low byte of the)
// dword selected from the table at 0x00601f28 by [0x00601fa8] & 3 (& 1 when [0x00601fa9] != 0);
// then it is the byte [0x0062d1d1] (mode 1 or 3) or [0x0062d1c1].
// Leaves eax = quotient, edx = remainder, ebp = 0x320; DIV keeps the flags of "xor edx, edx"
// (ZF and PF set, the rest clear).
void __cdecl FUN_004102df(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t value = GP4_FIELD(uint8_t, source, 9);
    if (GP4_GLOBAL(uint8_t, 0x00601f20) != 0xffu) {
        uint32_t index = GP4_GLOBAL(uint8_t, 0x00601fa8) & 3u;
        if (GP4_GLOBAL(uint8_t, 0x00601fa9) != 0u) {
            index &= 1u;
        }
        const uint32_t mask = GP4_ARRAY(uint32_t, 0x00601f28)[index];
        if ((GP4_GLOBAL(uint8_t, 0x00601fab) & static_cast<uint8_t>(mask)) != 0u) {
            if (GP4_GLOBAL(uint8_t, 0x00601fa8) == 1u || GP4_GLOBAL(uint8_t, 0x00601fa8) == 3u) {
                value = GP4_GLOBAL(uint8_t, 0x0062d1d1);
            } else {
                value = GP4_GLOBAL(uint8_t, 0x0062d1c1);
            }
        }
    }

    const uint32_t dividend = (value + 0x190u) << 14;
    const uint32_t quotient = dividend / 0x320u;
    const uint32_t remainder = dividend % 0x320u;

    GP4_FIELD(uint16_t, dest, 0x1e2) = static_cast<uint16_t>(quotient);

    r->eax = quotient;
    r->edx = remainder;
    r->ebp = 0x320u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004102df, FUN_004102df, "regs(ebx:ptr[0x40]:bytes, esi:ptr[0x1ea]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
