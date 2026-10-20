#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

// FUNCTION: GP4 0x00431e58
// For every group in the pointer table at 0x0062a970 (ends at 0xffffffff), each 16-byte record
// at group+8 (count at group+4) gets +6 = (word0 * [627894]) >> 14 and +8 = a scaled arctangent
// of tan(clamp(word1, +-0x3800) * k1) * [627894] * k2.
// pushal ... popal restores every register; the exit is the `cmp ebx, -1` that found the sentinel.
void __cdecl FUN_00431e58(gp4::Regs* r) {
    const int32_t scale = GP4_GLOBAL(int32_t, 0x00627894);

    for (uint32_t index = 0;; ++index) {
        uint8_t* group = reinterpret_cast<uint8_t*>(GP4_ARRAY(uint32_t, 0x0062a970)[index]);
        if (reinterpret_cast<uint32_t>(group) == 0xffffffffu) {
            break;
        }
        uint32_t count = GP4_FIELD(uint32_t, group, 4);
        if (count == 0u) {
            continue;
        }
        uint8_t* record = group + 8;
        do {
            const int32_t word0 = GP4_FIELD(int16_t, record, 0);
            const int64_t product = static_cast<int64_t>(word0) * static_cast<int64_t>(scale);
            GP4_FIELD(uint16_t, record, 6) = static_cast<uint16_t>(static_cast<uint64_t>(product) >> 14);

            int32_t angle = GP4_FIELD(int16_t, record, 2);
            if (angle < 0) {
                uint32_t magnitude = 0u - static_cast<uint32_t>(angle);
                if (magnitude >= 0x3800u) magnitude = 0x3800u;
                angle = static_cast<int32_t>(0u - magnitude);
            } else if (static_cast<uint32_t>(angle) >= 0x3800u) {
                angle = 0x3800;
            }
            GP4_GLOBAL(int32_t, 0x006f2bf4) = angle;

            const Ext x = gp4::x87::ext_i32(angle) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bc4));
            Ext sine;
            Ext cosine;
            gp4::x87::fsincos(x, &sine, &cosine);
            const Ext tangent = sine / cosine;
            const Ext slope = tangent * gp4::x87::ext_i32(scale) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2b9c));
            const Ext arc = gp4::x87::fpatan(slope, gp4::x87::ext(1.0));
            const int32_t result = gp4::x87::fistp32(arc * gp4::x87::ext(GP4_GLOBAL(double, 0x006f29d4)));
            GP4_GLOBAL(int32_t, 0x006f2bf4) = result;
            GP4_FIELD(uint16_t, record, 8) = static_cast<uint16_t>(result);

            record += 0x10;
            --count;
        } while (count != 0u);
    }

    // cmp ebx, -1 found the sentinel: result 0 -> ZF = PF = 1, CF = AF = SF = OF = 0
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00431e58, FUN_00431e58, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=state_00431e58")
