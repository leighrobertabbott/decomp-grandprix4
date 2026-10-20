#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint8_t* namePtr(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// Copies the NUL-terminated name at esi into the name field of the record eax (name = eax + 4) and
// makes it unique among the 22 records of the 0x1c byte stride table at 0x00b1b8dc: while some
// other record's 16 name bytes equal the new name's 16 bytes, the character at the original NUL
// position becomes the next one of the suffix string "23456789abcdefghijklmnopqrstuvwxyz" (index
// byte [0x00830868], NUL written after it) and the table is scanned again. [0x00830860] holds the
// name pointer, [0x00830864] the NUL position. pushal/popal keep every register; the only flags
// left are those of the final `dec ecx` of the last scan step (ecx = 1 -> 0, CF kept from the
// preceding `add esi, 0x1c`).
// FUNCTION: GP4 0x00541a94
void __cdecl FUN_00541a94(gp4::Regs* r) {
    uint8_t* const name = namePtr(r->eax + 4u);
    GP4_GLOBAL(uint32_t, 0x00830860) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(name));

    const uint8_t* source = namePtr(r->esi);
    uint8_t* out = name;
    uint8_t value;
    do {
        value = *source++;
        *out++ = value;
    } while (value != 0u);
    out -= 1;
    GP4_GLOBAL(uint32_t, 0x00830864) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out));
    GP4_GLOBAL(uint8_t, 0x00830868) = 0u;

    for (;;) {
        bool duplicate = false;
        uint32_t record = 0x00b1b8dcu;
        for (uint32_t left = 0x16u; left != 0u; --left, record += 0x1cu) {
            if (record == GP4_GLOBAL(uint32_t, 0x00830860)) {
                continue;
            }
            const uint8_t* a = namePtr(record);
            const uint8_t* b = namePtr(GP4_GLOBAL(uint32_t, 0x00830860));
            uint32_t count = 0x10u;
            bool equal = true;
            while (count != 0u) {
                count -= 1u;
                equal = (*a++ == *b++);
                if (!equal) {
                    break;
                }
            }
            if (equal) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            break;
        }
        uint8_t* position = namePtr(GP4_GLOBAL(uint32_t, 0x00830864));
        position[0] = GP4_ARRAY(const uint8_t, 0x00830869)[GP4_GLOBAL(uint8_t, 0x00830868)];
        position[1] = 0u;
        GP4_GLOBAL(uint8_t, 0x00830868) = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x00830868) + 1u);
    }

    const uint32_t lastRecord = 0x00b1b8dcu + 0x15u * 0x1cu;
    const uint32_t stepFlags = gp4::flags::add32(lastRecord, 0x1cu);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, stepFlags));
}
GP4_IMPL(0x00541a94, FUN_00541a94, "regs(eax:rec[0xb1b8d8,0x1c,0..21], esi:ptr[0x20]:u8{0,0,0,0x41,0x42}) -> (cf, pf, af, zf, sf, of); globals=layout=names_00b1b8dc")
