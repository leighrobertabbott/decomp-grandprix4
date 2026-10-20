#include <gp4/gp4.h>
#include <gp4/flags.h>

// Forward byte copy (`rep movsb`, direction flag clear) of `count` bytes from the address in esi to the one in edi.
static inline void CopyBytes(uint32_t src, uint32_t dst, uint32_t count) {
    const uint8_t* from = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(src));
    uint8_t* to = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(dst));
    for (uint32_t i = 0; i != count; ++i) {
        to[i] = from[i];
    }
}

// Copies the driver strings chosen by the 22 slot bytes at 0x007c2a48 out of the loaded text block
// ([0x008334bf]) when the byte [0x007aafdb] is set. A slot with bit 7 set, bit 6 clear and a nonzero low six
// bits selects car k = (b & 0x3f) - 1: its 0x18-byte name goes from block + k*0x18 to the table 0x007c2b3c + k*0x18
// and, for an even slot position i (team j = i / 2), two 13-byte strings go from block + 0x210 + j*13 and block
// + 0x29f + j*13 to 0x007c2d4c + j*13 and 0x007c2ddb + j*13. The registers are left as the loops leave them:
// eax = the last slot's value (low byte), ebx = slots + 22, ecx = 0, esi / edi = the end of the last copy; the
// flags are those of the final `inc ebx` (carry kept from the previous writer: 1 only after an odd position's
// `shr eax, 1`). Disabled: only the flags of `cmp byte [0x007aafdb], 0` change.
// FUNCTION: GP4 0x0054b5f0
void __cdecl FUN_0054b5f0(gp4::Regs* r) {
    const uint8_t enabled = GP4_GLOBAL(uint8_t, 0x007aafdb);
    if (enabled == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(enabled, 0u));
        return;
    }

    const uint8_t* slots = GP4_ARRAY(uint8_t, 0x007c2a48);
    uint32_t eax = r->eax;
    uint32_t esi = r->esi;
    uint32_t edi = r->edi;
    uint32_t carry = 0u;

    for (uint32_t i = 0; i < 0x16u; ++i) {
        const uint32_t slot = slots[i];
        eax = (eax & 0xffffff00u) | slot;
        carry = 0u;
        if ((slot & 0x80u) == 0u || (slot & 0x40u) != 0u) {
            continue;
        }
        eax = slot & 0x3fu;
        eax -= 1u;
        if (static_cast<int32_t>(eax) < 0) {
            continue;
        }

        eax *= 0x18u;
        esi = eax + GP4_GLOBAL(uint32_t, 0x008334bf);
        edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007c2b3c) + eax));
        CopyBytes(esi, edi, 0x18u);
        esi += 0x18u;
        edi += 0x18u;

        eax = i;
        carry = eax & 1u;
        eax >>= 1;
        if (carry != 0u) {
            continue;
        }

        const uint32_t team = eax;
        eax = team * 13u;
        esi = eax + 0x210u + GP4_GLOBAL(uint32_t, 0x008334bf);
        edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007c2d4c) + eax));
        CopyBytes(esi, edi, 13u);
        esi += 13u;
        edi += 13u;

        eax = team * 13u;
        esi = eax + 0x29fu + GP4_GLOBAL(uint32_t, 0x008334bf);
        edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007c2ddb) + eax));
        carry = gp4::flags::add32(eax, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x007c2ddb)))) & gp4::flags::CF;
        CopyBytes(esi, edi, 13u);
        esi += 13u;
        edi += 13u;
    }

    r->eax = eax;
    r->esi = esi;
    r->edi = edi;
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slots)) + 0x16u;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slots)) + 0x15u, carry));
}
GP4_IMPL(0x0054b5f0, FUN_0054b5f0, "regs(eax:u32, ebx:u32, ecx:u32, esi:u32, edi:u32) -> (eax, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pairs_005487cb+ptr_0054b5f0")
