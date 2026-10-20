#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043cec0
// `push edx ; mov al,[esi+0x7c] ; call 0x0043ce1d ; mov [esi+0xf6], ax ; pop edx ; ret`: looks up
// the value for the car index byte at +0x7c through 0x0043ce1d and stores it as a word at +0xf6.
// Only al is replaced before the call, so the upper bits of eax are an input. edx is restored;
// the flags are those left by the callee.
void __cdecl FUN_0043cec0(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t savedEdx = r->edx;
    r->eax = (r->eax & 0xffffff00u) | record[0x7c];
    gp4::call_regs(0x0043ce1d, *r);
    *reinterpret_cast<uint16_t*>(record + 0xf6) = static_cast<uint16_t>(r->eax);
    r->edx = savedEdx;
}
GP4_IMPL(0x0043cec0, FUN_0043cec0, "regs(eax:u32, esi:ptr[0xfe]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
