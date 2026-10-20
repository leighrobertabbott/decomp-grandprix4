#include <gp4/gp4.h>

// Re-seats the walker: depending on the sign of the byte at 0x00639652 it advances edi to the next
// linked record (0x004059d2) when bit 0x20 of byte +0x1b disagrees, walks edi to the record that
// contains the point at esi (0x00405d65 / 0x00405cf8), stores the resulting record pointer at
// esi+0x1c and refreshes the dependent state (0x004352e0, 0x004322a6, 0x00409239, 0x00409416).
// Registers and flags come straight back from the last call.
// FUNCTION: GP4 0x004129fa
void __cdecl FUN_004129fa(gp4::Regs* r) {
    if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00639652)) < 0) {
        uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
        if ((GP4_FIELD(uint8_t, source, 0x1b) & 0x20u) == 0u) {
            gp4::call_regs(0x004059d2, *r);
        }
        gp4::call_regs(0x00405d65, *r);
    } else {
        uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
        if ((GP4_FIELD(uint8_t, source, 0x1b) & 0x20u) != 0u) {
            gp4::call_regs(0x004059d2, *r);
        }
        gp4::call_regs(0x00405cf8, *r);
    }
    uint8_t* target = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_FIELD(uint32_t, target, 0x1c) = r->edi;
    gp4::call_regs(0x004352e0, *r);
    gp4::call_regs(0x004322a6, *r);
    gp4::call_regs(0x00409239, *r);
    gp4::call_regs(0x00409416, *r);
}
GP4_IMPL(0x004129fa, FUN_004129fa, "regs(esi:ptr[0x420]:layout=car_004129fa, edi:rec[0x8943a0,0x160,0..11]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ranges_004322a6+ring_00405a12+links_00405a12+sign_00639652")
