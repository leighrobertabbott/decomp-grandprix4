#include <gp4/gp4.h>

// Calls the matrix product 0x004020ef with eax = [obj+0x40], ecx = [obj+0x44] (both pointers to 0xa0-byte
// double blocks) and edx = the block at 0x00622e98, where obj is the pointer stored at 0x00609860.
// FUNCTION: GP4 0x004020d8
void __cdecl FUN_004020d8(gp4::Regs* r) {
    uint8_t* object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00609860)));
    gp4::Regs call = *r;
    call.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(object));
    call.eax = GP4_FIELD(uint32_t, object, 0x40);
    call.edx = 0x00622e98u;
    call.ecx = GP4_FIELD(uint32_t, object, 0x44);
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x004020ef, call);
    *r = call;
}
GP4_IMPL(0x004020d8, FUN_004020d8, "regs() -> (eax, ecx, edx, edi); globals=layout=view_00609860")
