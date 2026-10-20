#include <gp4/gp4.h>

// Saves the word at +0xee of the record at esi, clears it while 0x0043d909 runs, then puts it
// back. eax is pushed after `mov ax, [esi+0xee]` and popped again, so on exit it holds the
// entry high half and the saved word; the flags are those the callee left.
// FUNCTION: GP4 0x0043da57
void __cdecl FUN_0043da57(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t saved = (r->eax & 0xffff0000u) | GP4_FIELD(uint16_t, record, 0xee);
    GP4_FIELD(uint16_t, record, 0xee) = 0;

    r->eax = saved;
    gp4::call_regs(0x0043d909, *r);

    r->eax = saved;
    uint8_t* after = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_FIELD(uint16_t, after, 0xee) = static_cast<uint16_t>(saved);
}
GP4_IMPL(0x0043da57, FUN_0043da57, "regs(eax:u32, esi:ptr[0xf6]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
