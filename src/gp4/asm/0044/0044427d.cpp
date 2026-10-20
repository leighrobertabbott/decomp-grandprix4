#include <gp4/gp4.h>

// Record in esi: the dword at +0x240 is returned in eax.
struct Unknown_0044427d_Esi {
    uint8_t Unknown00[0x240];
    uint32_t Value240;
};

// FUNCTION: GP4 0x0044427d
void __cdecl FUN_0044427d(gp4::Regs* r) {
    const Unknown_0044427d_Esi* record = reinterpret_cast<const Unknown_0044427d_Esi*>(static_cast<uintptr_t>(r->esi));
    r->eax = record->Value240;
}
GP4_IMPL(0x0044427d, FUN_0044427d, "regs(esi:ptr[0x248]:bytes) -> (eax)")
