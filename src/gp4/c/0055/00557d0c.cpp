#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

// Walks the 0x160-byte ring records from [0x866ec0] while the following record ([0x866eb8] is
// the stop address) and, for each record and its successor, stores two fixed-point angles in the
// first record: +0xe4 = round(atan2(dx, dz) * [0x6f2be4]) and +0xe8 = round(atan2(dy,
// sqrt(dx*dx + dz*dz)) * [0x6f2be4]) where d* are the +0xf0 / +0xf4 / +0xf8 position deltas.
// All intermediates stay on the x87 stack (80-bit). 0x6f2bf4 is the scratch dword the deltas
// pass through (it ends holding the last dz). pushal/popal restore the GPRs; the flags left are
// those of the final `cmp ebx, [0x866eb8]` (equal).
// FUNCTION: GP4 0x00557d0c
void __cdecl FUN_00557d0c(gp4::Regs* r) {
    uint32_t cur = GP4_GLOBAL(uint32_t, 0x00866ec0);
    uint32_t next;
    uint32_t end;
    do {
        uint8_t* a = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(cur));
        uint8_t* b = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(cur + 0x160u));

        const int32_t dx = static_cast<int32_t>(static_cast<uint32_t>(GP4_FIELD(int32_t, b, 0xf0)) -
                                                static_cast<uint32_t>(GP4_FIELD(int32_t, a, 0xf0)));
        const int32_t dz = static_cast<int32_t>(static_cast<uint32_t>(GP4_FIELD(int32_t, b, 0xf8)) -
                                                static_cast<uint32_t>(GP4_FIELD(int32_t, a, 0xf8)));
        const int32_t dy = static_cast<int32_t>(static_cast<uint32_t>(GP4_FIELD(int32_t, b, 0xf4)) -
                                                static_cast<uint32_t>(GP4_FIELD(int32_t, a, 0xf4)));
        GP4_GLOBAL(int32_t, 0x006f2bf4) = dz;

        const gp4::x87::Ext scale = gp4::x87::ext(GP4_GLOBAL(double, 0x006f2be4));

        const gp4::x87::Ext yaw = gp4::x87::fpatan(gp4::x87::ext_i32(dx), gp4::x87::ext_i32(dz));
        GP4_FIELD(int32_t, a, 0xe4) = gp4::x87::fistp32(yaw * scale);

        const gp4::x87::Ext ex = gp4::x87::ext_i32(dx);
        const gp4::x87::Ext ez = gp4::x87::ext_i32(dz);
        const gp4::x87::Ext run = gp4::x87::fsqrt(ex * ex + ez * ez);
        const gp4::x87::Ext pitch = gp4::x87::fpatan(gp4::x87::ext_i32(dy), run);
        GP4_FIELD(int32_t, a, 0xe8) = gp4::x87::fistp32(pitch * scale);

        cur += 0x160u;
        next = cur + 0x160u;
        end = GP4_GLOBAL(uint32_t, 0x00866eb8);
    } while (next != end);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(next, end));
}
GP4_IMPL(0x00557d0c, FUN_00557d0c, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=walk_00557083+geo_005558c6")
