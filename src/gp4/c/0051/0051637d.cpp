#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine: loads the 16-bit value at +0x80 of the record [0x00679d30] (ah = +0x81,
// al = +0x80, upper half of eax cleared) and stores it in 0x00a6987c. popal does not restore flags,
// so they are those of `xor eax, eax`.
// FUNCTION: GP4 0x0051637d
void __cdecl FUN_0051637d(gp4::Regs* r) {
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00679d30)));
    GP4_GLOBAL(uint32_t, 0x00a6987c) = static_cast<uint32_t>(rec[0x81]) << 8 | rec[0x80];
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x0051637d, FUN_0051637d, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ptr_00679d30")
