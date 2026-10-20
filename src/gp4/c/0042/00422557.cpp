#include <gp4/gp4.h>

// For every entry 0 .. [0x0062a05c]-1 (do-while: at least one pass) whose threshold dword in
// the table at 0x0060832c is not below DAT_0062a060 (signed), increments the matching counter
// byte of the table at 0x0060932c. Leaves ebx = entry count, edi = DAT_0062a060; the loop ends
// on "cmp ebx, [0x0062a05c]" with equality (ZF and PF set, the rest clear).
// FUNCTION: GP4 0x00422557
void __cdecl FUN_00422557(gp4::Regs* r) {
    const int32_t threshold = GP4_GLOBAL(int32_t, 0x0062a060);
    uint32_t index = 0u;
    do {
        if (!(threshold > GP4_ARRAY(int32_t, 0x0060832c)[index])) {
            GP4_ARRAY(uint8_t, 0x0060932c)[index]++;
        }
        index++;
    } while (index != GP4_GLOBAL(uint32_t, 0x0062a05c));
    r->edi = static_cast<uint32_t>(threshold);
    r->ebx = index;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00422557, FUN_00422557, "regs() -> (ebx, edi, cf, pf, af, zf, sf, of); globals=layout=state_004225c0")
