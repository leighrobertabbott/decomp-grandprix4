#include <gp4/gp4.h>
#include <gp4/flags.h>

// Name table header at 0x838b8d: +0x04 is the entry count, +0x0c the array of string pointers.
struct Unknown_0054e4c2_Table {
    uint8_t Unknown00[4];
    uint32_t Count04;
    uint8_t Unknown08[4];
    uint32_t Strings0C[1];
};

// `repe cmpsb` over 0x30 bytes (esi = a, edi = b): true when the last compared pair has a > b
// (the `ja` that follows reads CF/ZF of that compare).
static bool stringAbove(const uint8_t* a, const uint8_t* b) {
    for (uint32_t n = 0x30u; n != 0u; --n) {
        if (*a != *b) {
            return *a > *b;
        }
        ++a;
        ++b;
    }
    return false;
}

// FUNCTION: GP4 0x0054e4c2
// Shell sort of the pointer array of the string table in ebx (count at +4, pointers from +0xc) by
// the first 0x30 bytes of each string, using the gap sequence of seven dwords at 0x838c2e. An
// insertion pass per gap: element i + gap is compared with element i and shifted down in gap steps
// while it is not above the one before. pushal/popal restore every GPR; the flags are those of
// `cmp [ebx+4], 2` when the count is below 2, else of the final `cmp esi, 0x838c4a` (equal).
// The only caller passes ebx = 0x838b8d (count 0x838b91, pointers 0x838b99).
void __cdecl FUN_0054e4c2(gp4::Regs* r) {
    Unknown_0054e4c2_Table* const table = reinterpret_cast<Unknown_0054e4c2_Table*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t count = table->Count04;
    if (count < 2u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(count, 2u));
        return;
    }

    const uint32_t gapEnd = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00838c4a)));
    uint32_t gapCursor = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x00838c2e)));
    do {
        const uint32_t gap = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(gapCursor));
        gapCursor = gapCursor + 4u;

        const uint32_t spanFlags = gp4::flags::sub32(count, gap);
        const bool skip = (spanFlags & gp4::flags::ZF) != 0u ||
                          ((spanFlags & gp4::flags::SF) != 0u) != ((spanFlags & gp4::flags::OF) != 0u);
        if (!skip) {
            uint32_t remaining = count - gap;
            const uint32_t first = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&table->Strings0C[0]));
            const uint32_t step = gap << 2;
            uint32_t slot = first;
            do {
                const uint32_t saved = slot;
                const uint32_t moving = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(slot + step));
                uint32_t other = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(slot));
                for (;;) {
                    if (stringAbove(reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(moving)),
                                    reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(other)))) {
                        break;
                    }
                    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(slot + step)) = other;
                    slot = slot - step;
                    const uint32_t back = gp4::flags::sub32(slot, first);
                    if (((back & gp4::flags::SF) != 0u) != ((back & gp4::flags::OF) != 0u)) {
                        break;
                    }
                    other = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(slot));
                }
                *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(slot + step)) = moving;
                slot = saved + 4u;
                --remaining;
            } while (remaining != 0u);
        }
    } while (gapCursor < gapEnd);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(gapCursor, gapEnd));
}
GP4_IMPL(0x0054e4c2, FUN_0054e4c2, "regs(ebx:u32[0x838b8d..0x838b8d]) -> (cf, pf, af, zf, sf, of); globals=layout=strs_00838b91")
