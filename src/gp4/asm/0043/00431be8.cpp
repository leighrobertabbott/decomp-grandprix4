#include <gp4/gp4.h>

// FUNCTION: GP4 0x00431be8
// `push eax ; sub ax,[esi+0x60] ; movsx eax,ax ; call 0x0043240e ; pop eax ; ret`: passes the
// signed 16-bit difference between ax and the word at +0x60 to 0x0043240e (which adds the scaled
// delta to +0x1d8 and stores the larger of that and +0x1dc at +0xf0). eax is restored; the flags
// come straight back from the callee's final compare (pushal/popal keeps them).
void __cdecl FUN_00431be8(gp4::Regs* r) {
    const uint32_t savedEax = r->eax;
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint16_t word60 = *reinterpret_cast<const uint16_t*>(record + 0x60);
    const uint16_t diff = static_cast<uint16_t>(static_cast<uint16_t>(r->eax) - word60);
    r->eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(diff)));
    gp4::call_regs(0x0043240e, *r);
    r->eax = savedEax;
}
GP4_IMPL(0x00431be8, FUN_00431be8, "regs(eax:u32, esi:ptr[0x1e8]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
