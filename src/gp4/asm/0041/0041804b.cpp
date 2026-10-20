#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of a 32-bit `cmp a, b` (a - b): CF PF AF ZF SF OF.
static inline uint32_t cmpFlags32(uint32_t a, uint32_t b) {
    const uint32_t res = a - b;
    uint32_t f = parityFlag(res);
    if (a < b) f |= 0x01u;
    if (((a ^ b ^ res) & 0x10u) != 0u) f |= 0x10u;
    if (res == 0u) f |= 0x40u;
    if ((res & 0x80000000u) != 0u) f |= 0x80u;
    if ((((a ^ b) & (a ^ res)) & 0x80000000u) != 0u) f |= 0x800u;
    return f;
}

// Picks the track-segment pointer stored to [0x602008]: the record pointer edi after the walk of 0x00406123
// when the record's byte +0x1ce bit 0 is set, the walk succeeded (CF clear) and edi lies in (pit start +
// [0x62d24a]*0x160, pit end - [0x62d24c]*0x160]; otherwise the record's own link dword +0x1c. edi is
// preserved for the caller.
// FUNCTION: GP4 0x0041804b
void __cdecl FUN_0041804b(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t entryEdi = r->edi;

    gp4::Regs call = *r;
    uint32_t flags;
    bool useWalked = false;
    if ((GP4_FIELD(uint8_t, record, 0x1ce) & 1u) == 0u) {
        // test byte, 1 with a clear bit: ZF and PF set, CF=OF=AF=SF=0
        flags = 0x44u;
    } else {
        gp4::call_regs(0x00406123, call);
        flags = call.eflags & 0x8d5u;
        if ((call.eflags & 1u) == 0u) {
            const uint32_t walked = call.edi;
            const uint32_t upper = GP4_GLOBAL(uint32_t, 0x00866ef8)
                - static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x0062d24c)) * 0x160u;
            call.eax = upper;
            flags = cmpFlags32(walked, upper);
            if (walked <= upper) {
                const uint32_t lower = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x0062d24a)) * 0x160u
                    + GP4_GLOBAL(uint32_t, 0x00891500);
                call.eax = lower;
                flags = cmpFlags32(walked, lower);
                useWalked = walked > lower;
            }
        }
    }
    GP4_GLOBAL(uint32_t, 0x00602008) = useWalked ? call.edi : GP4_FIELD(uint32_t, record, 0x1c);

    *r = call;
    r->edi = entryEdi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041804b, FUN_0041804b, "regs(esi:ptr[0x1d8]:i32[-67108864..67106815], edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12+links_00405a12")
