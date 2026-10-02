#include <gp4/gp4.h>

// FUNCTION: GP4 0x00431de8
void __cdecl FUN_00431de8(gp4::Regs* r) {
    const int32_t delta = GP4_GLOBAL(int32_t, 0x00627888);

    if (delta != 0) {
        for (uint32_t index = 0;; ++index) {
            uint8_t* group = reinterpret_cast<uint8_t*>(GP4_ARRAY(uint32_t, 0x0062a970)[index]);
            if (reinterpret_cast<uint32_t>(group) == 0xffffffffu) {
                break;
            }
            if ((GP4_ARRAY(uint8_t, 0x0062a990)[index] & 1u) == 0u || index != 1u) {
                continue;
            }
            uint32_t count = GP4_FIELD(uint32_t, group, 4);
            if (count == 0u) {
                continue;
            }
            uint8_t* record = group + 8;
            do {
                int32_t value = static_cast<int32_t>(static_cast<int16_t>(GP4_FIELD(uint16_t, record, 0))) - delta;
                if (value < 0
                    && ((GP4_ARRAY(uint8_t, 0x0062a990)[index] & 2u) == 0u
                        || static_cast<int16_t>(GP4_FIELD(uint16_t, record, 2)) >= 0)) {
                    GP4_FIELD(uint16_t, record, 2) = 0;
                    value = 0;
                }
                GP4_FIELD(uint16_t, record, 6) = static_cast<uint16_t>(value);
                record += 0x10;
                count -= 1u;
            } while (count != 0u);
        }
    }

    // popal restores every register; both exits (value == 0 test, cmp sentinel, sentinel)
    // leave ZF=1 and PF=1 with CF/AF/SF/OF clear.
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00431de8, FUN_00431de8, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
