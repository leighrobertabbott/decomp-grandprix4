#include <gp4/gp4.h>

// The 22 car records at 0x0066fef0 (only the fields this routine reads or updates).
struct Unknown_0066fef0 {
    uint8_t Unknown00[0x1c];
    uint32_t Unknown1C;          // current track-segment record (0x160-byte ring at 0x008943a0)
    uint8_t Unknown20[0x7a - 0x20];
    uint8_t Unknown7A;
    uint8_t Unknown7B[0x8d - 0x7b];
    uint8_t Unknown8D;
    uint8_t Unknown8E[0x21c - 0x8e];
    uint8_t Unknown21C;
    uint8_t Unknown21D[0x41c - 0x21d];
};
static_assert(sizeof(Unknown_0066fef0) == 0x41c, "car stride");

// FUNCTION: GP4 0x0043af84
// pushal/popal wrapper. 0x0043aed0 rebuilds the list of nearby segment records ([0x00607ba0], count
// [0x00607b9c]). Then for each of the 22 cars that is active (+0x8d bit 1 clear, +0x7a bits 7 and 6
// clear): the car is "near" when one of the listed segments lies 0x6e0..0x897f (wrapping by
// [0x00866ec4]) ahead of the car's segment (+0x1c, below the pit start [0x00891500]). A near car
// whose +0x21c bit 4 is clear gets that bit set, after either scheduling the sound/event 0x00418fd0
// (when 0x004110dd accepts it and [0x007c2e75] is zero) or, when 0x004110dd refuses, checking the
// tag with 0x0041902e and clearing it with 0x0041904e. A car that is not near has the bit cleared.
// Every GPR is restored; the flags are those of the last `dec ecx` (ZF|PF, CF clear from the add).
void __cdecl FUN_0043af84(gp4::Regs* r) {
    gp4::Regs list = *r;
    list.st_in = 0;
    list.st_out = 0;
    gp4::call_regs(0x0043aed0, list);

    Unknown_0066fef0* car = GP4_ARRAY(Unknown_0066fef0, 0x0066fef0);
    for (uint32_t slot = 0x16u; slot != 0u; --slot, ++car) {
        if ((car->Unknown8D & 2u) != 0u || (car->Unknown7A & 0x80u) != 0u || (car->Unknown7A & 0x40u) != 0u) {
            continue;
        }

        uint32_t index = 0u;
        uint32_t distance = 0u;
        bool near = false;
        if (GP4_GLOBAL(uint32_t, 0x00607b9c) != 0u && car->Unknown1C < GP4_GLOBAL(uint32_t, 0x00891500)) {
            do {
                distance = GP4_ARRAY(uint32_t, 0x00607ba0)[index] - car->Unknown1C;
                if (static_cast<int32_t>(distance) < 0) {
                    distance += GP4_GLOBAL(uint32_t, 0x00866ec4);
                }
                if (static_cast<int32_t>(distance) < 0x8980 && static_cast<int32_t>(distance) >= 0x6e0) {
                    near = true;
                    break;
                }
                ++index;
            } while (index != GP4_GLOBAL(uint32_t, 0x00607b9c));
        }

        if (!near) {
            car->Unknown21C &= 0xefu;
            continue;
        }
        if ((car->Unknown21C & 0x10u) != 0u) {
            continue;
        }

        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
        c.ebx = car->Unknown1C;
        c.ecx = index;
        c.eax = distance;
        gp4::call_regs(0x004110dd, c);
        if ((c.eflags & 1u) == 0u) {
            if (GP4_GLOBAL(uint8_t, 0x007c2e75) == 0u) {
                gp4::Regs event = *r;
                event.st_in = 0;
                event.st_out = 0;
                event.esi = c.esi;
                event.ecx = 4u;
                event.eax = 0x7d0u;
                event.edx = 0xbb8u;
                gp4::call_regs(0x00418fd0, event);
            }
            car->Unknown21C |= 0x10u;
        } else {
            car->Unknown21C |= 0x10u;
            gp4::Regs tag = *r;
            tag.st_in = 0;
            tag.st_out = 0;
            tag.esi = c.esi;
            tag.ecx = 4u;
            gp4::call_regs(0x0041902e, tag);
            if ((tag.eflags & 1u) != 0u) {
                tag.ecx = 4u;
                gp4::call_regs(0x0041904e, tag);
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043af84, FUN_0043af84, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+carx_0042fe2f+state_00437888+flags_00408f7b+ring_008943a0+words_008943a0+bytes_0063bcfc+list_0043aed0+state_0066d8c0+clock_0066d814+state_0043af84")
