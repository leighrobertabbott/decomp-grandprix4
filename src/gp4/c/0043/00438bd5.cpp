#include <gp4/gp4.h>

// FUNCTION: GP4 0x00438bd5
// Walks the car-order list at 0x0066fe3c (16-bit offsets of the 0x41c-byte car records at
// 0x0066fef0, the guard word before it at 0x0066fe3a, terminated by 0xffff). For every car whose
// dwords +0x40 and +0x44 differ it runs 0x004379a0, then 0x00444419 (wrapped in pushal/popal) and,
// when bit 1 of byte +0x7b and bit 0x1000 of dword +0x40 are set, 0x00418097 (also pushal/popal).
// ebx ends on the terminating word, esi on the last car visited, ax on 0xffff; the flags are those
// of the final "cmp ax, -1" (equal).
void __cdecl FUN_00438bd5(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.ebx = 0x0066fe3au;
    for (;;) {
        c.ebx += 2u;
        const uint16_t entry = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(c.ebx));
        c.eax = (c.eax & 0xffff0000u) | entry;
        if (entry == 0xffffu) {
            break;
        }
        c.esi = static_cast<uint32_t>(entry) + 0x0066fef0u;
        uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));

        c.eax = GP4_FIELD(uint32_t, car, 0x40);
        if (c.eax == GP4_FIELD(uint32_t, car, 0x44)) {
            continue;
        }
        gp4::call_regs(0x004379a0, c);

        gp4::Regs wrapped = c;
        gp4::call_regs(0x00444419, wrapped);

        if ((GP4_FIELD(uint8_t, car, 0x7b) & 2u) == 0u) {
            continue;
        }
        if ((GP4_FIELD(uint32_t, car, 0x40) & 0x1000u) == 0u) {
            continue;
        }
        wrapped = c;
        gp4::call_regs(0x00418097, wrapped);
    }

    c.eflags = (r->eflags & ~0x8d5u) | 0x44u;
    *r = c;
}
GP4_IMPL(0x00438bd5, FUN_00438bd5, "regs(eax:u32, ecx:u32, edx:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carsbg_00438bd5+list_0066fe3c+curves_0062ecb0+state_004087aa+div_0060138c+trk_007ad894+circ_007adb50+codes_0043ba12+state_0040fbf3+state_004059ef+state_0040e84e+state_00444517")
