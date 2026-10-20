#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads eax from the dword at 0x00ae6ca4 and bl from the byte esi points at, steps esi past it
// (`inc esi`) and tail-calls 0x0053eeb7 (records the pair in the 15-entry ring at 0x8300db).
// Registers and flags are whatever the callee leaves.
// FUNCTION: GP4 0x005408d4
void __cdecl FUN_005408d4(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00ae6ca4);
    const uint8_t* cursor = reinterpret_cast<const uint8_t*>(r->esi);
    const uint32_t before = r->esi;
    r->eax = value;
    r->ebx = (r->ebx & 0xffffff00u) | *cursor;
    r->esi = before + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(before, r->eflags));
    gp4::call_regs(0x0053eeb7, *r);
}
GP4_IMPL(0x005408d4, FUN_005408d4, "regs(esi:ptr[0x41]:layout=msg_005408d4, ebx:u32) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=log_005408d4")
