#include <gp4/gp4.h>

// FUNCTION: GP4 0x00429fcd
// Two cases for the car record in esi and the segment record in edi:
//  - segment byte +0x92 bit 0, car byte +0x71 bit 7 set and car byte +0x8d bit 1 clear: the dword
//    +0xbc is cleared and ecx receives the word 0x0062d22c (flags: result of "test +0x8d, 2" = 0);
//  - otherwise (pushal / popal, so every register comes back): when the car bit 0x135.0 is set it
//    is cleared and the byte +0x234 is reloaded from the segment pointer [esi+0x1b8] (byte +0x2a, or
//    +0x8d when its byte +0x51 has bit 7); 0x00603e6c is cleared, 0x00425cb3 runs with al = byte
//    +0x234 and its ax / dx become the words +0xbc / +0xbe. The flags are the callee's.
void __cdecl FUN_00429fcd(gp4::Regs* r) {
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* const segment = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    if ((segment[0x92] & 1u) != 0u && (car[0x71] & 0x80u) != 0u && (car[0x8d] & 2u) == 0u) {
        r->ecx = GP4_GLOBAL(uint16_t, 0x0062d22c);
        *reinterpret_cast<uint32_t*>(car + 0xbc) = 0;
        // test byte [esi+0x8d], 2 with a zero result: PF and ZF set, the rest clear
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    uint8_t lane = car[0x234];
    if ((car[0x135] & 1u) != 0u) {
        car[0x135] &= 0xfeu;
        const uint8_t* const block =
            reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(*reinterpret_cast<uint32_t*>(car + 0x1b8)));
        lane = block[0x2a];
        if ((block[0x51] & 0x80u) != 0u) {
            lane = block[0x8d];
        }
        car[0x234] = lane;
    }
    GP4_GLOBAL(uint8_t, 0x00603e6c) = 0;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = (r->eax & 0xffffff00u) | lane;
    gp4::call_regs(0x00425cb3, call);
    *reinterpret_cast<uint16_t*>(car + 0xbc) = static_cast<uint16_t>(call.eax);
    *reinterpret_cast<uint16_t*>(car + 0xbe) = static_cast<uint16_t>(call.edx);

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00429fcd, FUN_00429fcd, "regs(eax:u32, ecx:u32, edi:ptr[0x9a]:bytes, esi:ptr[0x420]:layout=car_0042ba87+rec_00409416+car_00429fcd+gears_0041034e) -> (ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0042595c+ring_008943a0+words_008943a0+words_00413090+engine_00413606+lanes_00429fcd")
