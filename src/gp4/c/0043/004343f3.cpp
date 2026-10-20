#include <gp4/gp4.h>

// Per-car event timers: 9 dwords per car (22 cars) at 0x0063b9e4. Slot k is 0 when the event k is
// idle, else the clock value at which it fires.
struct Unknown_0063b9e4 {
    uint32_t Timer[9];
};
static_assert(sizeof(Unknown_0063b9e4) == 0x24, "timer record stride");

// FUNCTION: GP4 0x004343f3
// pushal/popal wrapper (every GPR is restored; the flags are those of the final `cmp ebx, 9`).
// 1. For each of the 16 rows of the bit table 0x00433fe0 (0x0062ff6c) that is nonzero, and each car
//    0..21 whose bit is set in it (0x00434557 returns CF), runs the event handler 0x0043410d with the
//    row as the event.
// 2. For each category 0..8 and car 0..21 whose timer [0x0063b9e4 + car * 0x24 + category * 4] is
//    nonzero and due (clock - timer >= 0; for category 4 also the car's byte +2 at [0x0062ff68]
//    + car * 4 is not 0xff), clears the timer, sets the car's bit in the category mask (0x00434583)
//    and, for category 4, arms the timers 1, 2 and 5 at clock + 0xfa / 0x1f4 / 0x3e8 and clears the
//    car's bytes +0 and +1.
void __cdecl FUN_004343f3(gp4::Regs* r) {
    const uint32_t* row = gp4::cdecl_<0x00433fe0, const uint32_t*>();
    uint32_t edx = r->edx;

    for (uint32_t category = 0u; category != 0x10u; ++category, ++row) {
        if (*row == 0u) {
            continue;
        }
        uint32_t car = 0u;
        do {
            gp4::Regs test = *r;
            test.st_in = 0;
            test.st_out = 0;
            test.eax = category;
            test.ebx = category;
            test.edx = edx;
            test.esi = car;
            test.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(row));
            gp4::call_regs(0x00434557, test);
            if ((test.eflags & 1u) != 0u) {
                test.eax = category;
                gp4::call_regs(0x0043410d, test);
                edx = test.edx;
            }
            ++car;
        } while (car != 0x16u);
    }

    Unknown_0063b9e4* timers = GP4_ARRAY(Unknown_0063b9e4, 0x0063b9e4);
    for (uint32_t category = 0u; category != 9u; ++category) {
        for (uint32_t car = 0u; car != 0x16u; ++car) {
            uint32_t* slot = &timers[car].Timer[category];
            if (*slot == 0u) {
                continue;
            }
            if (static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d814) - *slot) < 0) {
                continue;
            }
            if (category == 4u
                && reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(car * 4u + GP4_GLOBAL(uint32_t, 0x0062ff68)))[2] == 0xffu) {
                continue;
            }
            *slot = 0u;

            gp4::Regs bit = *r;
            bit.st_in = 0;
            bit.st_out = 0;
            bit.eax = category;
            bit.edx = car;
            bit.esi = car;
            bit.ebx = category;
            gp4::call_regs(0x00434583, bit);

            if (category == 4u) {
                timers[car].Timer[1] = GP4_GLOBAL(uint32_t, 0x0066d814) + 0xfau;
                timers[car].Timer[2] = GP4_GLOBAL(uint32_t, 0x0066d814) + 0x1f4u;
                timers[car].Timer[5] = GP4_GLOBAL(uint32_t, 0x0066d814) + 0x3e8u;
                uint8_t* flags = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(car * 4u + GP4_GLOBAL(uint32_t, 0x0062ff68)));
                flags[0] = 0u;
                flags[1] = 0u;
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004343f3, FUN_004343f3, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00433ff8+state_00434557+clock_0066d814+recs_004340b1+state_004343f3")
