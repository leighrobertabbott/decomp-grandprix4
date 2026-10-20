#include <gp4/gp4.h>
#include <gp4/flags.h>

// Character translation through 0x0054d747 (al = table[al]).
static uint8_t Translate(uint8_t value) {
    gp4::Regs c = {};
    c.eax = value;
    gp4::call_regs(0x0054d747, c);
    return static_cast<uint8_t>(c.eax);
}

// True when the 0x30-byte string `key` sorts above `element`: the first differing byte of key is the
// larger one (unsigned). When [0x826cd9] is 0xff both strings are compared through the translation
// table (case folding), otherwise byte for byte (`repe cmpsb`). Equal strings are not above.
static bool KeyAbove(const uint8_t* key, const uint8_t* element) {
    if (GP4_GLOBAL(uint8_t, 0x00826cd9) != 0xffu) {
        for (uint32_t i = 0u; i != 0x30u; ++i) {
            if (key[i] != element[i]) {
                return key[i] > element[i];
            }
        }
        return false;
    }
    for (uint32_t i = 0u; i != 0x30u; ++i) {
        const uint8_t high = Translate(element[i]);
        const uint8_t low = Translate(key[i]);
        if (low != high) {
            return low > high;
        }
    }
    return false;
}

// Shell sort of the table of string pointers at ebx + 0xc (count dword at ebx + 4, strings of 0x30
// bytes) with the gap dwords 0x826e43..0x826e5b (seven, unaligned): for each gap h the entry h places
// further is inserted into the sorted run before it, shifting larger-or-equal entries up. Nothing
// is sorted below two entries. pushal/popal keep every register; the flags are those of the final
// `cmp esi, 0x826e5f` (equal) or, for a short table, of `cmp dword [ebx + 4], 2`.
// FUNCTION: GP4 0x0052f506
void __cdecl FUN_0052f506(gp4::Regs* r) {
    uint8_t* const table = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t count = GP4_FIELD(uint32_t, table, 4);
    uint32_t flags = gp4::flags::sub32(count, 2u);
    if (count >= 2u) {
        uint32_t gapAddress = 0x00826e43u;
        do {
            const uint32_t gap = GP4_GLOBAL(uint32_t, gapAddress);
            gapAddress += 4u;
            flags = gp4::flags::sub32(count, gap);
            const int32_t remaining = static_cast<int32_t>(count - gap);
            if (static_cast<int32_t>(count) > static_cast<int32_t>(gap)) {
                const uint32_t stride = gap << 2;
                uint8_t* const base = table + 0xc;
                uint8_t* position = base;
                uint32_t left = static_cast<uint32_t>(remaining);
                do {
                    const uint8_t* key = GP4_FIELD(const uint8_t*, position, stride);
                    uint8_t* slot = position;
                    do {
                        const uint8_t* element = *reinterpret_cast<const uint8_t* const*>(slot);
                        if (KeyAbove(key, element)) {
                            break;
                        }
                        GP4_FIELD(const uint8_t*, slot, stride) = element;
                        slot -= stride;
                    } while (static_cast<int32_t>(reinterpret_cast<uintptr_t>(slot)) >= static_cast<int32_t>(reinterpret_cast<uintptr_t>(base)));
                    GP4_FIELD(const uint8_t*, slot, stride) = key;
                    position += 4;
                    --left;
                } while (left != 0u);
            }
            flags = gp4::flags::sub32(gapAddress, 0x00826e5fu);
        } while (gapAddress < 0x00826e5fu);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052f506, FUN_0052f506, "regs(ebx:ptr[0x10c]:layout=list_0052f506) -> (cf, pf, af, zf, sf, of); globals=layout=xlat_00833d18+mode_0052f506")
