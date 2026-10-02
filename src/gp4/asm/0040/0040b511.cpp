#include <gp4/gp4.h>

// FUNCTION: GP4 0x0040b511
void __cdecl FUN_0040b511(gp4::Regs* r) {
    // rep movsd: copies 0xa5e dwords from the table at 0x63b814 to [edi];
    // eax returns the advanced destination, ecx/edi/esi are restored.
    uint32_t* destination = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t* source = GP4_ARRAY(uint32_t, 0x0063b814);
    for (uint32_t i = 0; i < 0xa5eu; i++) {
        destination[i] = source[i];
    }
    r->eax = r->edi + 0xa5eu * 4u;
}
GP4_IMPL(0x0040b511, FUN_0040b511, "regs(edi:ptr[0x2980]:bytes) -> (eax)")
