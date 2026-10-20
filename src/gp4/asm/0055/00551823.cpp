#include <gp4/gp4.h>

// pushal; loads eax/edx/ebp/edi from [esi+0x28] and the globals 0x8757dc/0x8757d8/0x87587c; popal;
// ret. Every register is restored and the flags are never written, so nothing caller-visible changes.
// FUNCTION: GP4 0x00551823
void __cdecl FUN_00551823(gp4::Regs* r) {
    const gp4::Regs saved = *r;
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    r->eax = static_cast<uint32_t>(static_cast<int32_t>(*reinterpret_cast<const int16_t*>(record + 0x28)));
    r->edx = GP4_GLOBAL(uint32_t, 0x008757dc);
    r->ebp = GP4_GLOBAL(uint32_t, 0x008757d8);
    r->edi = GP4_GLOBAL(uint32_t, 0x0087587c);
    *r = saved;
}
GP4_IMPL(0x00551823, FUN_00551823, "regs(esi:ptr[0x40]:bytes) -> (); globals=fuzz:i32")
