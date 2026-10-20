#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) left by `sub word, word`.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Walks the word list at 0x66fe3c forward (the mirror of 0x4083ac) from byte offset body[0x7d]:
// each entry is a byte offset into the 0x41c-byte record array at 0x66fef0 and a negative word
// ends the list. Returns the first record (other than esi itself) whose flag byte +0x7a has bit 7
// clear with CF clear and the flags of the 16-bit difference of the word at +0x68; otherwise esi
// with CF set. ebx, edi and edx are saved and restored by the original.
// FUNCTION: GP4 0x004083f2
void __cdecl FUN_004083f2(gp4::Regs* r) {
    const uint32_t self = r->esi;
    const uint8_t* body = reinterpret_cast<const uint8_t*>(self);
    uint32_t cursor = 0x0066fe3cu + body[0x7d];
    uint32_t flags;

    for (;;) {
        cursor += 2;
        const uint16_t entry = *reinterpret_cast<const uint16_t*>(cursor);
        if ((entry & 0x8000u) != 0u) {
            // or ax, ax -> CF = OF = AF = 0, then stc
            flags = 0x80u | parityFlag(entry) | 1u;
            r->eax = self;
            break;
        }
        const uint32_t other = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(entry))) + 0x0066fef0u;
        if (other == self) {
            // cmp ebx, esi equal: ZF and PF set, then stc
            flags = 0x40u | 0x4u | 1u;
            r->eax = self;
            break;
        }
        const uint8_t* record = reinterpret_cast<const uint8_t*>(other);
        if ((record[0x7a] & 0x80u) != 0u) continue;

        const uint16_t mine = *reinterpret_cast<const uint16_t*>(body + 0x68);
        const uint16_t theirs = *reinterpret_cast<const uint16_t*>(record + 0x68);
        // mov dx, [ebx+0x68] ; sub dx, [esi+0x68] ; movsx edx, dx ; ... pop edx ; clc
        flags = subFlags16(theirs, mine) & ~1u;
        r->eax = other;
        break;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004083f2, FUN_004083f2, "regs(esi:u32[6750988..6750988]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=list_004083f2")
