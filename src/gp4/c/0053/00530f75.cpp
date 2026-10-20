#include <gp4/gp4.h>
#include <gp4/flags.h>

// Entry k (1..7) of a record at ebx is "present" when the dword at +0x14 + 4k is nonzero; its value
// is the dword at +0x28 + 4k. Starting at the first present entry (none: result entry 0), the
// walk moves forward to the next present entry while the current value is at most 0xa3d, stopping
// when the next one is missing (index 7), above 0xccc, or (entry 6) above 0xa3d. Then it moves
// backward to the previous present entry (entry 0 included) while the current value is above
// 0x1555 and one exists. eax = the present-flag dword of the final entry. ecx/edx are saved and
// restored; the flags are those of the last compare (cmp ecx, 7 on the empty case, the backward
// search's `cmp edx, 0` when it ran out, else `cmp eax, 0x1555`).
// FUNCTION: GP4 0x00530f75
void __cdecl FUN_00530f75(gp4::Regs* r) {
    const uint8_t* const rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint32_t index = 1u;
    while (*reinterpret_cast<const uint32_t*>(rec + 0x14 + index * 4u) == 0u && index < 7u) {
        ++index;
    }

    uint32_t arith;
    if (index >= 7u) {
        index = 0u;
        arith = gp4::flags::sub32(7u, 7u);
    } else {
        uint32_t value = *reinterpret_cast<const uint32_t*>(rec + 0x28 + index * 4u);
        while (value <= 0xa3du) {
            uint32_t next = index;
            do {
                ++next;
            } while (*reinterpret_cast<const uint32_t*>(rec + 0x14 + next * 4u) == 0u && next != 7u);
            if (next == 7u) {
                break;
            }
            value = *reinterpret_cast<const uint32_t*>(rec + 0x28 + next * 4u);
            if (value > 0xcccu) {
                break;
            }
            if (next == 6u && value > 0xa3du) {
                break;
            }
            index = next;
        }

        value = *reinterpret_cast<const uint32_t*>(rec + 0x28 + index * 4u);
        for (;;) {
            arith = gp4::flags::sub32(value, 0x1555u);
            if (value <= 0x1555u) {
                break;
            }
            int32_t previous = static_cast<int32_t>(index);
            do {
                --previous;
            } while (*reinterpret_cast<const uint32_t*>(rec + 0x14 + static_cast<uint32_t>(previous) * 4u) == 0u && previous >= 0);
            arith = gp4::flags::sub32(static_cast<uint32_t>(previous), 0u);
            if (previous < 0) {
                break;
            }
            index = static_cast<uint32_t>(previous);
            value = *reinterpret_cast<const uint32_t*>(rec + 0x28 + index * 4u);
        }
    }

    r->eax = *reinterpret_cast<const uint32_t*>(rec + 0x14 + index * 4u);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00530f75, FUN_00530f75, "regs(ebx:ptr[0x50]:u32{=0,0,0,1,0x800,0xa3d,0xa3e,0xccc,0xccd,0x1000,0x1555,0x1556,0x3000}) -> (eax, cf, pf, af, zf, sf, of)")
