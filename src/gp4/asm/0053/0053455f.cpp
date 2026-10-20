#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053455f
// Debounce of a key byte (dl) against the previous frame's byte kept in 0x7aaffd:
//   0x7aaffe == 0xff (key was released/changed): cleared to 0 when the new key is 0, nothing sent.
//   previous byte 0x7aaffd == 0: a new press is sent (0x7aaffc = key) with repeat counter 0x7aafff = 10.
//   same key as before: sent again (0x7aaffc = key) with counter 3 once the counter has reached 0.
//   a different key: 0x7aaffe = 0xff.
// 0x7aaffd then takes the current key byte at 0x828235. dl leaves holding the byte sent (0 when
// none), al holds [0x828235]; the other bits of eax/edx are kept. The flags are those of the last
// compare executed in the taken path.
void __cdecl FUN_0053455f(gp4::Regs* r) {
    const uint32_t key = r->edx & 0xffu;
    uint32_t sent = 0u;
    uint32_t flags;

    if (GP4_GLOBAL(uint8_t, 0x007aaffe) == 0xffu) {
        flags = gp4::flags::sub8(key, 0u);
        if (key == 0u) {
            GP4_GLOBAL(uint8_t, 0x007aaffe) = 0u;
        }
    } else if (GP4_GLOBAL(uint8_t, 0x007aaffd) == 0u) {
        flags = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x007aaffd), 0u);
        sent = key;
        GP4_GLOBAL(uint8_t, 0x007aafff) = 0x0au;
    } else {
        const uint32_t previous = GP4_GLOBAL(uint8_t, 0x007aaffd);
        flags = gp4::flags::sub8(key, previous);
        if (key != previous) {
            GP4_GLOBAL(uint8_t, 0x007aaffe) = 0xffu;
        } else {
            const uint32_t counter = GP4_GLOBAL(uint8_t, 0x007aafff);
            flags = gp4::flags::sub8(counter, 0u);
            if (counter == 0u) {
                sent = key;
                GP4_GLOBAL(uint8_t, 0x007aafff) = 3u;
            }
        }
    }

    GP4_GLOBAL(uint8_t, 0x007aaffc) = static_cast<uint8_t>(sent);
    const uint32_t current = GP4_GLOBAL(uint8_t, 0x00828235);
    GP4_GLOBAL(uint8_t, 0x007aaffd) = static_cast<uint8_t>(current);

    r->eax = (r->eax & 0xffffff00u) | current;
    r->edx = (r->edx & 0xffffff00u) | sent;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053455f, FUN_0053455f, "regs(eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
