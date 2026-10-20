#include <gp4/gp4.h>

// Advances the "current car" pointer [0x0066d81c] to the next car in the 22-record ring at 0x0066fef0
// (stride 0x41c) whose byte +0x7c has bit 7 set and bit 6 clear and whose byte +0x7a has bit 5 clear
// (the car itself is tried last). Stores that car's byte +0x7c at 0x0066d897. CF is set when there is
// no current car or no car qualifies; the other flags are those of the last `or`/`test`.
// FUNCTION: GP4 0x00442416
void __cdecl FUN_00442416(gp4::Regs* r) {
    const uint32_t current = GP4_GLOBAL(uint32_t, 0x0066d81c);
    r->eax = current;
    if (current == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x045u;
        return;
    }

    uint32_t car = current;
    bool found = false;
    for (;;) {
        car += 0x41cu;
        if (car == 0x00675958u) {
            car = 0x0066fef0u;
        }
        if (car == GP4_GLOBAL(uint32_t, 0x0066d81c)) {
            break;
        }
        const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(car));
        if (static_cast<int8_t>(record[0x7c]) < 0 && (record[0x7c] & 0x40u) == 0u && (record[0x7a] & 0x20u) == 0u) {
            found = true;
            break;
        }
    }

    r->esi = car;
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(car));
    if (!found && (record[0x7a] & 0x20u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x001u;
        return;
    }

    const uint8_t id = record[0x7c];
    r->eax = (current & 0xffffff00u) | id;
    GP4_GLOBAL(uint8_t, 0x0066d897) = id;
    GP4_GLOBAL(uint32_t, 0x0066d81c) = car;
    r->eflags = (r->eflags & ~0x8d5u) | 0x044u;
}
GP4_IMPL(0x00442416, FUN_00442416, "regs(esi:u32) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+ring_00405a12+links_00405a12+flags_00408f7b+cur_0066d81c")
