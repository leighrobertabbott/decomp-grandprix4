#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

struct Unknown_00417fc5_Car {
    uint8_t Unknown00[0x240];
    uint32_t Unknown240;       // one byte per wheel record, consumed low byte first
};

struct Unknown_00417fc5_Wheel {   // 0x300-byte records at 0x006380d8
    uint8_t Unknown00[0x2e];
    uint8_t Flags2E;
    uint8_t Unknown2F[0x290 - 0x2f];
    double Unknown290;
    uint8_t Unknown298[0x300 - 0x298];
};

// pushal ... popal wrapper. For each of the four 0x300-byte wheel records at 0x006380d8 whose byte
// +0x2e bit 5 is clear and whose byte of the car's dword +0x240 (low byte first) is below 0x20:
// clears the word [0x00627b7c], rounds |double +0x290 * [0x006f2b14]| (fistp, stored to [0x006f2bf4])
// and, when it reaches the unsigned threshold [0x00627b80], scales the excess by [0x00627b84] >> 28
// (capped at [0x00627b88]) and runs 0x00417f6b with ebx = that record. All registers are restored by
// popal; the flags are those of the final `cmp ebx, 0x638cd8` (equal: ZF and PF set).
// FUNCTION: GP4 0x00417fc5
void __cdecl FUN_00417fc5(gp4::Regs* r) {
    const Unknown_00417fc5_Car* car = reinterpret_cast<const Unknown_00417fc5_Car*>(static_cast<uintptr_t>(r->esi));
    uint32_t mask = car->Unknown240;
    Unknown_00417fc5_Wheel* wheel = GP4_ARRAY(Unknown_00417fc5_Wheel, 0x006380d8);
    uint32_t index = 0;

    do {
        if ((wheel->Flags2E & 0x20u) == 0u && static_cast<uint8_t>(mask) < 0x20u) {
            GP4_GLOBAL(uint16_t, 0x00627b7c) = 0;

            const Ext product = gp4::x87::ext(wheel->Unknown290) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b14));
            const int32_t rounded = gp4::x87::fistp32(product);
            GP4_GLOBAL(int32_t, 0x006f2bf4) = rounded;

            uint32_t magnitude = static_cast<uint32_t>(rounded);
            if (rounded < 0) {
                magnitude = 0u - magnitude;
            }

            const uint32_t threshold = GP4_GLOBAL(uint32_t, 0x00627b80);
            if (magnitude >= threshold) {
                const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(magnitude - threshold))
                    * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00627b84));
                int32_t level = static_cast<int32_t>(scaled >> 28);
                if (level > GP4_GLOBAL(int32_t, 0x00627b88)) {
                    level = GP4_GLOBAL(int32_t, 0x00627b88);
                }

                gp4::Regs call = *r;
                call.st_in = 0;
                call.st_out = 0;
                call.eax = static_cast<uint32_t>(level);
                call.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(wheel));
                call.ecx = index;
                call.edx = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 32);
                call.ebp = mask;
                gp4::call_regs(0x00417f6b, call);
            }
        }
        mask >>= 8;
        ++index;
        ++wheel;
    } while (reinterpret_cast<uint8_t*>(wheel) < GP4_ARRAY(uint8_t, 0x00638cd8));

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00417fc5, FUN_00417fc5, "regs(esi:ptr[0x248]:layout=rec_00417fc5, edi:rec[0x8943a0,0x160,0..11]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004178e0+ring_00405a12+state_00417fc5")
