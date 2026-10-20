#include <gp4/gp4.h>

// Calls the matrix product 0x004020ef with eax = the fixed block at 0x00622cf8, edx = the fixed block at
// 0x006231d8 and ecx = [obj+0x40] (pointer to a 0xa0-byte double block), where obj is the pointer stored
// at 0x00609860.
// FUNCTION: GP4 0x00402038
void __cdecl FUN_00402038(gp4::Regs* r) {
    uint8_t* object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00609860)));
    gp4::Regs call = *r;
    call.eax = 0x00622cf8u;
    call.edx = 0x006231d8u;
    call.ecx = GP4_FIELD(uint32_t, object, 0x40);
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x004020ef, call);
    *r = call;
}
GP4_IMPL(0x00402038, FUN_00402038, "regs() -> (eax, ecx, edx); globals=layout=view_00609860")
