#include <gp4/gp4.h>

// Entry of the 0xa8-byte table at 0x00625abc (22 entries); only the fields used here.
struct Unknown_00625abc { uint32_t Flags00; uint8_t Unknown04[0x94]; uint32_t Unknown98; uint32_t Unknown9C; uint8_t UnknownA0[8]; };

// FUNCTION: GP4 0x00418d27
void __cdecl FUN_00418d27(gp4::Regs* r) {
    uint8_t* record = GP4_ARRAY(uint8_t, 0x0066fef0);
    uint32_t eax = 0;
    Unknown_00625abc* entry = nullptr;
    for (int i = 0; i < 0x16; i++) {
        // 0x00418901 maps the record (esi) to its table entry
        r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
        gp4::call_regs(0x00418901, *r);
        eax = r->eax;
        entry = reinterpret_cast<Unknown_00625abc*>(static_cast<uintptr_t>(eax));
        const uint32_t deadline = GP4_FIELD(uint32_t, record, 0x254);
        if ((entry->Flags00 & 0x80u) == 0 && deadline != 0
            && ((GP4_GLOBAL(uint32_t, 0x0066d814) - deadline) & 0x80000000u) != 0) {
            entry->Unknown98 = 0;
            entry->Unknown9C = 0;
            eax = 0xffffffffu;
            entry->Flags00 &= 0xffffff7fu;
            if (GP4_GLOBAL(uint8_t, 0x00638d1c) == 0) {
                entry->Flags00 |= 0x80u;
            }
        }
        record += 0x41c;
    }
    // Registers left by the loop: ecx exhausted, esi one record past the
    // last, edi the last entry; flags those of the final "add esi, 0x41c"
    // (0x0067553c + 0x41c = 0x00675958: only AF set).
    r->eax = eax;
    r->ecx = 0;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
    r->eflags = (r->eflags & ~0x8d5u) | 0x10u;
}
GP4_IMPL(0x00418d27, FUN_00418d27, "regs() -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=state_00418d27")
