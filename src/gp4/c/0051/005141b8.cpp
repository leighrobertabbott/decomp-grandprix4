#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the input-handler selected by the mode dword 0x0088ff30 (13-entry table of code pointers at
// 0x00817270) for the viewed car, but only when the player's car is selected (car pointer 0x00679d30
// equals 0x00679d2c) and the master byte 0x0066d7a4 is zero, and not when car byte +0x8d has bit 4
// set while mode != 7 and car byte +0x84 == 7. The handler runs with edx = mode, esi = car and the
// caller's other registers. pushal/popal/pop esi restore every GPR; the flags are those of the last
// flag writer: the handler's flags when it ran, else the test/cmp that made the routine skip it.
// FUNCTION: GP4 0x005141b8
void __cdecl FUN_005141b8(gp4::Regs* r) {
    uint32_t arith = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x0066d7a4), 0u);
    if (GP4_GLOBAL(uint8_t, 0x0066d7a4) == 0u) {
        const uint32_t car = GP4_GLOBAL(uint32_t, 0x00679d30);
        arith = gp4::flags::sub32(car, GP4_GLOBAL(uint32_t, 0x00679d2c));
        if (car == GP4_GLOBAL(uint32_t, 0x00679d2c)) {
            const uint8_t* record = (const uint8_t*)(uintptr_t)car;
            const uint32_t flags8d = record[0x8d];
            arith = gp4::flags::logic8(flags8d & 0x10u);
            bool run = true;
            if ((flags8d & 0x10u) != 0u) {
                arith = gp4::flags::sub32(GP4_GLOBAL(uint32_t, 0x0088ff30), 7u);
                if (GP4_GLOBAL(uint32_t, 0x0088ff30) != 7u) {
                    const uint32_t kind = record[0x84];
                    arith = gp4::flags::sub8(kind, 7u);
                    run = (kind != 7u);
                }
            }
            if (run) {
                const uint32_t mode = GP4_GLOBAL(uint32_t, 0x0088ff30);
                gp4::Regs c = *r;
                c.edx = mode;
                c.esi = car;
                c.eflags = gp4::flags::merge(r->eflags, arith);
                gp4::call_regs(GP4_ARRAY(uint32_t, 0x00817270)[mode], c);
                arith = c.eflags & gp4::flags::ARITH;
            }
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005141b8, FUN_005141b8, "regs(eax:u32) -> (cf, zf, sf, of); globals=layout=player_0040f8d9+disp_005141b8")
