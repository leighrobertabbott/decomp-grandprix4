#include <gp4/gp4.h>
#include <gp4/flags.h>

// Translates the low six bits (1-based) of the byte at +0x7c of the esi record through the byte
// table at 0x7c2a5e; when the result differs from the global at 0x893e08 it is stored there and
// at 0x866084. eax is the translated value; the flags are those of
// `cmp eax, [0x893e08]` (the stores do not touch them).
// FUNCTION: GP4 0x00514e1c
void __cdecl FUN_00514e1c(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t slot = static_cast<int32_t>(GP4_FIELD(uint8_t, record, 0x7c) & 0x3fu) - 1;
    const uint32_t value = GP4_ARRAY(uint8_t, 0x007c2a5e)[slot];
    const uint32_t previous = GP4_GLOBAL(uint32_t, 0x00893e08);
    const uint32_t arith = gp4::flags::sub32(value, previous);
    if (value != previous) {
        GP4_GLOBAL(uint32_t, 0x00893e08) = value;
        GP4_GLOBAL(uint32_t, 0x00866084) = value;
    }
    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00514e1c, FUN_00514e1c, "regs(esi:ptr[0x84]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
