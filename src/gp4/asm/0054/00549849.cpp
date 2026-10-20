#include <gp4/gp4.h>
#include <gp4/flags.h>

// Event handler keyed by edx (no pushal: eax/ebx/ecx/edi are left as the code leaves them, edx ends 0).
// 1: nothing. 5 or 6: clears the dword 0x82fd88. 2: scans the six bytes 0x831cfd for the low byte of the
// dword at [esi+0x26] (repne scasb). Found at index i: the cursor dword 0x831d03 becomes i and the byte
// 0x831cfd + i is set to 0xff. Not found: the byte is stored at 0x831cfd + cursor and the cursor advances,
// wrapping 6 -> 0. Then 0x831d64 = 4, 0x81e4fb = 0xff, 0x81e4fe = 0. Flags: the last compare of the
// dispatch (`cmp edx, 1/5/6`), or on the 2 path the closing `add eax, 0x831cfd` (found) / `cmp eax, 6` (stored).
// FUNCTION: GP4 0x00549849
void __cdecl FUN_00549849(gp4::Regs* r) {
    const uint32_t key = r->edx;
    uint32_t flags;
    if (key == 1u) {
        flags = gp4::flags::sub32(key, 1u);
    } else if (key == 2u) {
        const uint8_t wanted = static_cast<uint8_t>(GP4_FIELD(uint32_t, r->esi, 0x26));
        uint8_t* slots = &GP4_GLOBAL(uint8_t, 0x00831cfd);
        uint32_t count = 6u;
        uint32_t scanned = 0u;
        bool found = false;
        flags = 0u;
        while (count != 0u) {
            --count;
            flags = gp4::flags::sub8(wanted, slots[scanned]);
            ++scanned;
            if (wanted == slots[scanned - 1u]) {
                found = true;
                break;
            }
        }
        r->edi = 0x00831cfdu + scanned;
        r->ecx = count;
        if (found) {
            const uint32_t index = 5u - count;
            GP4_GLOBAL(uint32_t, 0x00831d03) = index;
            flags = gp4::flags::add32(index, 0x00831cfdu);
            r->eax = index + 0x00831cfdu;
            slots[index] = 0xffu;
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
        GP4_GLOBAL(uint8_t, 0x0081e4fb) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x0081e4fe) = 0u;
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
GP4_IMPL(0x00549849, FUN_00549849, "regs(edx:u32{=1,2,5,6,0,3,7}, esi:ptr[0x40]:u8{1,2,3,4,0xff}) -> (eax, ebx, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cur_00549849")
