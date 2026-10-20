#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal wrapper (every GPR restored; the flags are those of the last flag writer).
// The word at 0x9e1b20 + 0x38c*2 (= 0x9e2238) is 0xffff (none) or a byte offset into the 0x20-byte entry
// table at 0x6975ec. Unless entry word +0x1a has bit 2 set, the buffer at entry +0xc (+0x10 = bytes per
// row, as a word) is treated as four consecutive rows; every byte of rows 1..3 that is <= 0x27 is clamped
// (signed) to the limit byte at 0x51c919 + row. Row 0 is skipped.
static __declspec(noinline) uint32_t Word9e1b20(uint32_t index) {
    return GP4_ARRAY(uint16_t, 0x009e1b20)[index];
}

// FUNCTION: GP4 0x0051c91d
void __cdecl FUN_0051c91d(gp4::Regs* r) {
    const uint32_t index = Word9e1b20(0x38cu);
    uint32_t f = gp4::flags::sub16(index, 0xffffu);
    if (index != 0xffffu) {
        uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x006975ecu + index));
        const uint32_t entryFlags = GP4_FIELD(uint16_t, entry, 0x1a);
        f = gp4::flags::logic16(entryFlags & 4u);
        if ((entryFlags & 4u) == 0u) {
            uint8_t* row = GP4_FIELD(uint8_t*, entry, 0xc);
            const uint32_t width = GP4_FIELD(uint16_t, entry, 0x10);
            for (uint32_t pass = 0; pass != 4u; ++pass) {
                if (pass != 0u) {
                    uint8_t* p = row;
                    uint32_t n = width;
                    do {
                        const uint8_t value = *p;
                        if (value <= 0x27u) {
                            const int8_t limit = GP4_ARRAY(const int8_t, 0x0051c919)[pass];
                            *p = (static_cast<int8_t>(value) <= limit) ? value : static_cast<uint8_t>(limit);
                        }
                        ++p;
                    } while (--n != 0u);
                }
                row += width;
            }
            f = gp4::flags::sub32(4u, 4u);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0051c91d, FUN_0051c91d, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ent_0051c91d")
