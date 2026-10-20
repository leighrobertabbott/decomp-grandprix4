#include <gp4/gp4.h>

// Car record in esi: dword +0 / +4 are the 2D position handed to the ring walkers in
// ecx / ebx (shifted >> 11 there), words +0x90 and +0xb0 select the walker.
// If |word +0x90| - 0x200 is below word +0xb0 the walker 0x00405c9a runs, otherwise
// byte 0x00603c78 is set to 0xff for a negative +0x90 (else 0) and 0x00405a12 runs.
// edi (a 0x160-byte record of the ring at 0x008943a0), eax, edx and the flags come
// back from the walker; ebx and ecx are saved and restored.
// FUNCTION: GP4 0x00405b8c
void __cdecl FUN_00405b8c(gp4::Regs* r) {
    uint8_t* const self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const int16_t selector = GP4_FIELD(int16_t, self, 0x90);
    int32_t magnitude = selector;
    if (magnitude < 0) magnitude = -magnitude;
    const int16_t bound = static_cast<int16_t>(magnitude - 0x200);

    gp4::Regs walker = *r;
    walker.st_in = 0;
    walker.st_out = 0;
    walker.ecx = GP4_FIELD(uint32_t, self, 0);
    walker.ebx = GP4_FIELD(uint32_t, self, 4);

    if (bound < GP4_FIELD(int16_t, self, 0xb0)) {
        gp4::call_regs(0x00405c9a, walker);
    } else {
        GP4_GLOBAL(uint8_t, 0x00603c78) = selector < 0 ? 0xffu : 0u;
        gp4::call_regs(0x00405a12, walker);
    }

    r->eax = walker.eax;
    r->edx = walker.edx;
    r->edi = walker.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (walker.eflags & 0x8d5u);
}
GP4_IMPL(0x00405b8c, FUN_00405b8c, "regs(esi:ptr[0xb8]:layout=pos_00405b8c, edi:rec[0x8943a0,0x160,0..11]) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12")
