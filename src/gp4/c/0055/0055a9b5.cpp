#include <gp4/gp4.h>
#include <gp4/flags.h>

struct Unknown_008943a0 {
    uint8_t Unknown00[0x92];
    uint8_t Flags92;
};

// Sets bit 3 of byte +0x92 in the track-segment record at the lap start [0x00891504]. Leaves ebx = that
// record pointer and the flags of the `or byte [ebx+0x92], 8`.
// FUNCTION: GP4 0x0055a9b5
void __cdecl FUN_0055a9b5(gp4::Regs* r) {
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x00891504);
    Unknown_008943a0* rec = reinterpret_cast<Unknown_008943a0*>(static_cast<uintptr_t>(record));
    rec->Flags92 = static_cast<uint8_t>(rec->Flags92 | 8u);
    r->ebx = record;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(rec->Flags92));
}
GP4_IMPL(0x0055a9b5, FUN_0055a9b5, "regs() -> (ebx, cf, pf, af, zf, sf, of); globals=layout=lap_0055a9b5")
