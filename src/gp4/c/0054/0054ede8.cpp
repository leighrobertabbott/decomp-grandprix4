#include <gp4/gp4.h>
#include <gp4/flags.h>

// Hit test of the cursor (low words of 0x00838e40 / 0x00838e44) against a list of rectangles.
// Only active while 0x007ab17f == 0xff and 0x007aafd3 == 0. The list head is the dword at +0x1c of
// the structure at [0x0082fd34]; each node is { next, u16 x0 (+4), u16 y0 (+6), u16 x1 (+8), u16 y1 (+0xa),
// u8 flags (+0x10, bit 0 = disabled) }. Returns the first enabled node containing the point in eax
// (else 0); edx is set to 1 once the list is entered, otherwise left alone. ebx/ecx/esi are saved and
// restored. The flags are those of the last compare/test/or executed.
// FUNCTION: GP4 0x0054ede8
void __cdecl FUN_0054ede8(gp4::Regs* r) {
    uint32_t flags = r->eflags;
    uint32_t edx = r->edx;
    uint32_t result = 0;

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007ab17f);
    flags = gp4::flags::merge(flags, gp4::flags::sub8(mode, 0xffu));
    if (mode == 0xffu) {
        const uint8_t busy = GP4_GLOBAL(uint8_t, 0x007aafd3);
        flags = gp4::flags::merge(flags, gp4::flags::sub8(busy, 0u));
        if (busy == 0u) {
            const uint32_t root = GP4_GLOBAL(uint32_t, 0x0082fd34);
            uint32_t node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(root + 0x1cu));
            flags = gp4::flags::merge(flags, gp4::flags::logic32(node));
            if (node != 0u) {
                edx = 1u;
                const uint16_t x = static_cast<uint16_t>(GP4_GLOBAL(uint32_t, 0x00838e40));
                const uint16_t y = static_cast<uint16_t>(GP4_GLOBAL(uint32_t, 0x00838e44));
                while (node != 0u) {
                    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(node));
                    bool inside = false;
                    flags = gp4::flags::merge(flags, gp4::flags::logic8(rec[0x10] & 1u));
                    if ((rec[0x10] & 1u) == 0u) {
                        const uint16_t x0 = *reinterpret_cast<const uint16_t*>(rec + 4);
                        flags = gp4::flags::merge(flags, gp4::flags::sub16(x, x0));
                        if (x >= x0) {
                            const uint16_t x1 = *reinterpret_cast<const uint16_t*>(rec + 8);
                            flags = gp4::flags::merge(flags, gp4::flags::sub16(x, x1));
                            if (x <= x1) {
                                const uint16_t y0 = *reinterpret_cast<const uint16_t*>(rec + 6);
                                flags = gp4::flags::merge(flags, gp4::flags::sub16(y, y0));
                                if (y >= y0) {
                                    const uint16_t y1 = *reinterpret_cast<const uint16_t*>(rec + 0xa);
                                    flags = gp4::flags::merge(flags, gp4::flags::sub16(y, y1));
                                    inside = (y <= y1);
                                }
                            }
                        }
                    }
                    if (inside) {
                        result = node;
                        break;
                    }
                    node = *reinterpret_cast<const uint32_t*>(rec);
                    flags = gp4::flags::merge(flags, gp4::flags::logic32(node));
                }
            }
        }
    }

    r->eax = result;
    r->edx = edx;
    r->eflags = flags;
}
GP4_IMPL(0x0054ede8, FUN_0054ede8, "regs(edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=hot_0054ede8")
