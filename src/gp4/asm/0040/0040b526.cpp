#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040b526
void __cdecl FUN_0040b526(gp4::Regs* r) {
    // rep movsd: copies 0xa5e dwords from [esi] into the table at 0x63b814;
    // eax returns the advanced source, ecx/edi/esi are restored.
    const uint32_t* source = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t* destination = GP4_ARRAY(uint32_t, 0x0063b814);
    for (uint32_t i = 0; i < 0xa5eu; i++) {
        destination[i] = source[i];
    }
    r->eax = r->esi + 0xa5eu * 4u;
}
GP4_IMPL(0x0040b526, FUN_0040b526, "regs(esi:ptr[0x2980]:bytes) -> (eax)")
