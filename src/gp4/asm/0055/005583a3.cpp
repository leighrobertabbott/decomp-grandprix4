#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper over a record edi: for each of 10 slots k whose bit mask dword [0x00875d50 + 4k]
// shares a bit with eax, the word [0x00875d78 + 2k] is or-ed into the record word +0x78. Then
// eax is or-ed into five record dwords by 0x00559bc0 (eax and edi unchanged). popal restores every
// register; the flags are those left by that routine.
// FUNCTION: GP4 0x005583a3
void __cdecl FUN_005583a3(gp4::Regs* r) {
    const uint32_t bits = r->eax;
    const uint32_t record = r->edi;
    const uint32_t* const slotMask = GP4_ARRAY(const uint32_t, 0x00875d50);
    const uint16_t* const slotWord = GP4_ARRAY(const uint16_t, 0x00875d78);
    for (uint32_t k = 0u; k < 10u; ++k) {
        if ((slotMask[k] & bits) != 0u) {
            GP4_FIELD(uint16_t, record, 0x78) |= slotWord[k];
        }
    }
    gp4::call_regs(0x00559bc0, *r);
}
GP4_IMPL(0x005583a3, FUN_005583a3, "regs(eax:u32, edi:ptr[0x80]:bytes) -> (cf, pf, af, zf, sf, of)")
