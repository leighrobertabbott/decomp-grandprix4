#include <gp4/gp4.h>

// Clears flag 0x04 of byte +0xce and flag 0x20 of byte +0xff of the car record in esi, runs the empty
// stub 0x00405f76, then 0x004345e3. The flags are those 0x004345e3 leaves (its first instruction is a
// test, so the `and` flags never survive).
// FUNCTION: GP4 0x0043464e
void __cdecl FUN_0043464e(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_FIELD(uint8_t, record, 0xce) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0xce) & 0xfbu);
    GP4_FIELD(uint8_t, record, 0xff) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0xff) & 0xdfu);
    gp4::Regs call = *r;
    gp4::call_regs(0x00405f76, call);
    gp4::call_regs(0x004345e3, call);
    *r = call;
}
GP4_IMPL(0x0043464e, FUN_0043464e, "regs(esi:ptr[0x107]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
