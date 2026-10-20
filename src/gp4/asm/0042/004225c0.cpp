#include <gp4/gp4.h>

// FUNCTION: GP4 0x004225c0
// Clears every active (low 23 bits set, none of the 0x14280 bits) entry of the dword table
// at [0x0062a048] whose threshold byte in the table at 0x0060932c is <= the low byte of
// [0x00608328]; the entry count is [0x0062a05c] (do-while: at least one pass).
void __cdecl FUN_004225c0(gp4::Regs* r) {
    uint32_t* entries = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062a048)));
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00608328);
    uint32_t remaining = GP4_GLOBAL(uint32_t, 0x0062a05c);

    uint32_t index = 0;
    do {
        const uint32_t entry = entries[index];
        if ((entry & 0x7fffffu) != 0u
            && (entry & 0x14280u) == 0u
            && static_cast<uint8_t>(limit) >= GP4_ARRAY(uint8_t, 0x0060932c)[index]) {
            entries[index] = 0u;
            --remaining;
        }
        ++index;
    } while (index != GP4_GLOBAL(uint32_t, 0x0062a05c));

    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entries));
    r->edi = 0x7fffffu;
    r->ecx = limit;
    r->edx = remaining;
    r->ebx = index;
    // the loop ends on cmp ebx, [0x0062a05c] with equality: ZF = PF = 1, the rest clear
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004225c0, FUN_004225c0, "regs() -> (ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=state_004225c0")
