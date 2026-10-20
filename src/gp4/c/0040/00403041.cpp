#include <gp4/gp4.h>

// FUNCTION: GP4 0x00403041
// `call 0x00402fcd ; mov [esi+0x150], eax ; ret`: stores the result of 0x00402fcd (edi/esi
// pair, high half of the product left in edx) into the esi record at +0x150. The flags are those
// the callee's final shift left (the store does not touch them).
void __cdecl FUN_00403041(gp4::Regs* r) {
    gp4::call_regs(0x00402fcd, *r);
    GP4_FIELD(uint32_t, reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi)), 0x150) = r->eax;
}
GP4_IMPL(0x00403041, FUN_00403041, "regs(edi:ptr[0x16a]:bytes, esi:ptr[0x158]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of)")
