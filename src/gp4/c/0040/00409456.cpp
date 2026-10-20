#include <gp4/gp4.h>

// FUNCTION: GP4 0x00409456
// `call 0x00402db6 ; mov [esi+8], edx ; mov [esi+0x12], ax ; mov [esi+0x16], bp ; ret`: stores the
// three values 0x00402db6 leaves in edx, ax and bp into the esi record. The flags are those of
// the callee's last flag writer (the stores do not touch them).
void __cdecl FUN_00409456(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::call_regs(0x00402db6, *r);
    *reinterpret_cast<uint32_t*>(record + 8) = r->edx;
    *reinterpret_cast<uint16_t*>(record + 0x12) = static_cast<uint16_t>(r->eax);
    *reinterpret_cast<uint16_t*>(record + 0x16) = static_cast<uint16_t>(r->ebp);
}
GP4_IMPL(0x00409456, FUN_00409456, "regs(edi:ptr[0x18e]:bytes, esi:ptr[0x286]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
