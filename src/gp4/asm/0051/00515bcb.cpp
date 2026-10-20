#include <gp4/gp4.h>
#include <gp4/flags.h>

// eax = (signed byte at esi+0x72) + 1, stored to the global at 0x0086612c; the flags are those of
// the `add eax, 1`.
// FUNCTION: GP4 0x00515bcb
void __cdecl FUN_00515bcb(gp4::Regs* r) {
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t value = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int8_t>(rec[0x72])));
    const uint32_t sum = value + 1u;
    GP4_GLOBAL(uint32_t, 0x0086612c) = sum;
    r->eax = sum;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(value, 1u));
}
GP4_IMPL(0x00515bcb, FUN_00515bcb, "regs(esi:ptr[0x7a]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
