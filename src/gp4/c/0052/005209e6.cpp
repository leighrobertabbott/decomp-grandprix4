#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct Car_66fef0 {
    uint8_t Unknown00[0x1c];
    uint32_t Segment1c;      // +0x1c current track-segment pointer (0x8943a0 ring)
    uint8_t Unknown20[0x50];
    uint8_t Unknown70;       // +0x70
    uint8_t Unknown71[0x12];
    uint8_t Flags83;         // +0x83 (bit 6 latched here)
};
#pragma pack(pop)

// Register routine (no saves: leaves eax/edx/esi/edi changed). When the mode byte [0x852d98]
// is zero, advances save ring 2 through 0x520ad0 once the clock [0x66d814] reaches the due
// time [0x679b0c] (or the ring's upper mark [0x679b2c] is negative), rescheduling it
// [0x866e94] later. Then (mode zero or bit 2 set) for the car [0x66d820]: while its segment
// +0x1c lies within 0x2940 bytes (30 segments) behind [0x866ecc] and byte +0x70 is set, the
// first time (bit 0x40 of +0x83) it shifts the event triple 0x7a9128..30 into 0x7a9134..3c,
// records (+0x70 + 1, ring-1 position, [0x66d834]) and advances ring 1; otherwise clears bit 0x40.
// FUNCTION: GP4 0x005209e6
void __cdecl FUN_005209e6(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00852d98);
    if (mode != 0) {
        if ((mode & 4) == 0) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(mode & 4));
            return;
        }
    } else {
        r->edx = 2;
        const int32_t clock = GP4_GLOBAL(int32_t, 0x0066d814);
        r->eax = static_cast<uint32_t>(clock);
        if (GP4_ARRAY(int32_t, 0x00679b24)[2] < 0 || clock >= GP4_GLOBAL(int32_t, 0x00679b0c)) {
            const uint32_t due = static_cast<uint32_t>(clock) + GP4_GLOBAL(uint32_t, 0x00866e94);
            r->eax = due;
            GP4_GLOBAL(uint32_t, 0x00679b0c) = due;
            GP4_GLOBAL(uint32_t, 0x00679b44) = 2;
            gp4::call_regs(0x00520ad0, *r);
        }
    }

    Car_66fef0* car = reinterpret_cast<Car_66fef0*>(GP4_GLOBAL(uint32_t, 0x0066d820));
    r->esi = reinterpret_cast<uint32_t>(car);
    if (car == 0) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0));
        return;
    }

    const uint32_t seg = car->Segment1c;
    r->edi = seg;
    const uint32_t current = GP4_GLOBAL(uint32_t, 0x00866ecc);
    r->eax = current;
    if (seg <= current) {
        const uint32_t oldest = current - 0x2940u;
        r->eax = oldest;
        if (seg >= oldest && car->Unknown70 != 0) {
            const uint8_t flags = car->Flags83;
            if ((flags & 0x40) != 0) {
                r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flags & 0x40));
                return;
            }
            car->Flags83 = static_cast<uint8_t>(flags | 0x40);
            GP4_GLOBAL(uint32_t, 0x007a9134) = GP4_GLOBAL(uint32_t, 0x007a9128);
            GP4_GLOBAL(uint32_t, 0x007a9138) = GP4_GLOBAL(uint32_t, 0x007a912c);
            GP4_GLOBAL(uint32_t, 0x007a913c) = GP4_GLOBAL(uint32_t, 0x007a9130);
            GP4_GLOBAL(uint32_t, 0x00679b44) = 1;
            r->edx = 1;
            GP4_GLOBAL(uint32_t, 0x007a912c) = GP4_ARRAY(uint32_t, 0x00679b34)[1];
            const uint32_t number = static_cast<uint32_t>(car->Unknown70) + 1u;
            GP4_GLOBAL(uint32_t, 0x007a9128) = number;
            const uint32_t value = GP4_GLOBAL(uint32_t, 0x0066d834);
            r->eax = value;
            GP4_GLOBAL(uint32_t, 0x007a9130) = value;
            gp4::call_regs(0x00520ad0, *r);
            return;
        }
    }
    const uint8_t cleared = static_cast<uint8_t>(car->Flags83 & 0xbf);
    car->Flags83 = cleared;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
}
GP4_IMPL(0x005209e6, FUN_005209e6, "regs() -> (eax, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carbg_0066fef0+slot_00520b1b+copy_0063b814+links_0066fef0+ptrs_0066d818+ring_008943a0+records_008943a0+segw_00441e25+words_008943a0+ptr_0086850c+idx_00520be3+ring_00512d68+event_005207a9+cars_00444d60+win_005209e6")
