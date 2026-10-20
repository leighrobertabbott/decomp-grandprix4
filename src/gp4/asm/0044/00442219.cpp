#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `left + right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((~(left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Walks a stream of (mask dword, category byte, packed data bytes) records at edi and feeds each one to
// 0x004421d3 (eax = category, ebp = bit mask, edi = data). Header dword +0 bit 31 set: a single record whose
// mask is the dword at +8 (bit 31 cleared) and whose category byte is at +4. Clear: dword +4 is the byte
// length, the records start at +8 (4-byte aligned) and repeat while edi < start + length - 4. Either way the
// routine ends with `add edi, 4`, so the flags are those of that add.
// FUNCTION: GP4 0x00442219
void __cdecl FUN_00442219(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    uint32_t cursor = r->edi;
    const uint32_t* header = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(cursor));

    if ((header[0] & 0x80000000u) != 0u) {
        s.ebp = header[2] & 0x7fffffffu;
        cursor += 4u;
        s.eax = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(cursor));
        cursor += 1u;
        s.edi = cursor;
        gp4::call_regs(0x004421d3, s);
        cursor = (s.edi + 3u) & 0xfffffffcu;
    } else {
        const uint32_t end = header[1] + cursor - 4u;
        s.esi = end;
        cursor += 8u;
        do {
            s.ebp = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(cursor));
            cursor += 4u;
            s.eax = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(cursor));
            cursor += 1u;
            s.edi = cursor;
            gp4::call_regs(0x004421d3, s);
            cursor = (s.edi + 3u) & 0xfffffffcu;
        } while (cursor < end);
    }

    const uint32_t flags = addFlags32(cursor, 4u);
    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->ebp = s.ebp;
    r->edi = cursor + 4u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00442219, FUN_00442219, "regs(edx:u32, edi:ptr[0x100]:i32[-64..64]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00433ff8")
