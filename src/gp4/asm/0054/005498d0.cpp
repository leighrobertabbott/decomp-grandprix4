#include <gp4/gp4.h>
#include <gp4/flags.h>

// Event handler keyed by edx (sibling of 0x00549849, same record and cursor): 1 does nothing, 5 or 6
// clears the dword 0x82fd88. 2 reads the byte at 0x7c31dc + [esi+0x26] (zero: skip to the 0x81e4fb
// store), masks it with 0x3f and skips the rest when the dword [0x7c3208 + (code-1)*4] is <= -1000.
// Otherwise it maps the code through 0x00549976 (index 0..21 or 0xff) and looks the result up in the
// six bytes at 0x831cfd (repne scasb). Found at index i: the cursor dword 0x831d03 becomes i and the
// byte 0x831cfd + i is set to 0xff. Not found: the byte is stored at 0x831cfd + cursor and the cursor
// advances, wrapping 6 -> 0. Then 0x831d64 = 4. Every path of key 2 past the zero test ends with
// 0x81e4fb = 0xff. edx ends 0. Flags are those of the last flag writer of the path taken.
// FUNCTION: GP4 0x005498d0
void __cdecl FUN_005498d0(gp4::Regs* r) {
    const uint32_t key = r->edx;
    uint32_t flags;
    if (key == 1u) {
        flags = gp4::flags::sub32(key, 1u);
    } else if (key == 2u) {
        const uint32_t slot = 0x007c31dcu + GP4_FIELD(uint32_t, r->esi, 0x26);
        const uint32_t code = GP4_FIELD(uint8_t, slot, 0);
        flags = gp4::flags::logic8(code);
        r->eax = (slot & 0xffffff00u) | code;
        if (code != 0u) {
            const uint32_t masked = code & 0x3fu;
            const int32_t index = static_cast<int32_t>(masked) - 1;
            flags = gp4::flags::logic32(masked);
            flags = gp4::flags::dec32(masked, flags);
            const int32_t limit = GP4_ARRAY(int32_t, 0x007c3208)[index];
            flags = gp4::flags::sub32(static_cast<uint32_t>(limit), 0xfffffc18u);
            r->eax = static_cast<uint32_t>(index);
            if (limit > -1000) {
                flags = gp4::flags::inc32(static_cast<uint32_t>(index), flags);
                r->eax = masked;
                r->eflags = gp4::flags::merge(r->eflags, flags);
                gp4::call_regs(0x00549976, *r);
                const uint8_t wanted = static_cast<uint8_t>(r->eax);
                const uint8_t* slots = GP4_ARRAY(const uint8_t, 0x00831cfd);
                uint32_t count = 6u;
                uint32_t scanned = 0u;
                bool found = false;
                while (count != 0u) {
                    --count;
                    const uint8_t cell = slots[scanned];
                    ++scanned;
                    if (cell == wanted) {
                        found = true;
                        break;
                    }
                }
                r->edi = 0x00831cfdu + scanned;
                r->ecx = count;
                if (found) {
                    const uint32_t hit = 5u - count;
                    GP4_GLOBAL(uint32_t, 0x00831d03) = hit;
                    flags = gp4::flags::add32(hit, 0x00831cfdu);
                    r->eax = hit + 0x00831cfdu;
                    GP4_FIELD(uint8_t, r->eax, 0) = 0xffu;
                } else {
                    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x00831d03);
                    r->ebx = cursor + 0x00831cfdu;
                    GP4_FIELD(uint8_t, r->ebx, 0) = wanted;
                    uint32_t next = GP4_GLOBAL(uint32_t, 0x00831d03) + 1u;
                    flags = gp4::flags::sub32(next, 6u);
                    if (next == 6u) {
                        next = 0u;
                    }
                    r->eax = next;
                    GP4_GLOBAL(uint32_t, 0x00831d03) = next;
                }
                GP4_GLOBAL(uint8_t, 0x00831d64) = 4u;
            }
        }
        GP4_GLOBAL(uint8_t, 0x0081e4fb) = 0xffu;
    } else if (key == 5u) {
        flags = gp4::flags::sub32(key, 5u);
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
    } else {
        flags = gp4::flags::sub32(key, 6u);
        if (key == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005498d0, FUN_005498d0, "regs(edx:u32{=1,2,2,2,5,6,0,3,7}, esi:ptr[0x40]:layout=order_005498d0) -> (eax, ebx, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cur_00549849+keys_00831d67+order_005498d0")
