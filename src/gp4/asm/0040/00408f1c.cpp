#include <gp4/gp4.h>

struct UnknownBlock_00408f1c {
    uint8_t Unknown00[0x7c];
    uint8_t Key7C;
};

// Looks the byte at esi + 0x7c up in the 22-entry key table at 0x007c2a48 and returns the
// matching byte of the parallel table at 0x007c2a1c in al (entry 0 when nothing matches).
// ecx is left as the remaining loop count; edx is preserved. The flags are those of the matching
// "cmp" when found, else CF of the last "cmp" with the other flags of the last "inc edx" (22).
// FUNCTION: GP4 0x00408f1c
void __cdecl FUN_00408f1c(gp4::Regs* r) {
    const UnknownBlock_00408f1c* block = (const UnknownBlock_00408f1c*)r->esi;
    const uint8_t key = block->Key7C;
    const uint8_t* const keys = GP4_ARRAY(uint8_t, 0x007c2a48);
    const uint8_t* const values = GP4_ARRAY(uint8_t, 0x007c2a1c);

    uint32_t index = 0u;
    uint32_t remaining = 0x16u;
    uint32_t flags = 0u;
    bool found = false;
    for (;;) {
        const uint8_t entry = keys[index];
        if (key == entry) {
            // cmp al, entry (equal)
            flags = 0x44u;
            found = true;
            break;
        }
        // cmp al, entry carry, then inc edx keeps it
        flags = key < entry ? 1u : 0u;
        ++index;
        if (--remaining == 0u) {
            break;
        }
    }
    if (!found) {
        // inc edx took edx from 21 to 22: PF clear (0x16 has three bits), AF clear, ZF/SF/OF clear
        index = 0u;
    }

    r->eax = (r->eax & 0xffffff00u) | values[index];
    r->ecx = remaining;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408f1c, FUN_00408f1c, "regs(eax:u32, esi:ptr[0x84]:bytes) -> (eax, ecx, cf, pf, af, zf, sf, of)")
