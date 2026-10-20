#include <gp4/gp4.h>

enum : uint32_t {
    kArithFlags = 0x8d5u,   // OF SF ZF AF PF CF
    kPF = 0x4u, kZF = 0x40u, kSF = 0x80u
};

// Flags left by a logic operation (TEST / AND / OR) on an 8-bit result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? kPF : 0u) | ((result & 0xffu) == 0u ? kZF : 0u) |
           ((result & 0x80u) != 0u ? kSF : 0u);
}

// FUNCTION: GP4 0x00412af5
// Copies the car record in esi into the working record 0x00639204 (0x00412a3d), advances its
// position by the [0x00601354] >> 8 step (0x004129e0), re-seats its segment walk (0x004129fa) and
// refreshes its state (0x0040373b). Then, from the source record's flags, either sets the
// working record's value dword +0x1a0 to 0x4000 (flag 0x10 of byte +0xcb: also source word
// +0x10a and bytes +0x294 / +0x295), or, unless bit 1 of +0x9c is set and when bit 2 of +0xe5 is
// set, copies word +0x10a (sign-extended) to +0x1a0, byte +0x295 to +0x294 and bit 3 of +0x71.
// esi/edi are restored by the pops; ebx is the working record.
void __cdecl FUN_00412af5(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.ebx = 0x00639204u;
    gp4::call_regs(0x00412a3d, call);

    call.esi = 0x00639204u;
    call.eax = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00601354) >> 8);
    gp4::call_regs(0x004129e0, call);
    gp4::call_regs(0x004129fa, call);
    gp4::call_regs(0x0040373b, call);

    uint8_t* work = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(call.esi));
    uint32_t eax = call.eax;

    const uint32_t refresh = source[0xcb] & 0x10u;
    uint32_t flags = logicFlags8(refresh);
    if (refresh != 0u) {
        eax = 0x4000u;
        GP4_FIELD(uint32_t, work, 0x1a0) = 0x4000u;
        GP4_FIELD(uint16_t, source, 0x10a) = 0x4000;
        source[0x295] = 1;
        source[0x294] = 1;
    } else {
        const uint32_t held = source[0x9c] & 0x02u;
        flags = logicFlags8(held);
        if (held == 0u) {
            const uint32_t ready = source[0xe5] & 0x04u;
            flags = logicFlags8(ready);
            if (ready != 0u) {
                const int32_t value = GP4_FIELD(int16_t, source, 0x10a);
                GP4_FIELD(int32_t, work, 0x1a0) = value;
                eax = (static_cast<uint32_t>(value) & 0xffffff00u) | source[0x295];
                work[0x294] = source[0x295];

                uint32_t bits = work[0x71] & 0xf7u;
                work[0x71] = static_cast<uint8_t>(bits);
                flags = logicFlags8(bits);
                const uint32_t marker = source[0x71] & 0x08u;
                flags = logicFlags8(marker);
                if (marker != 0u) {
                    bits = work[0x71] | 0x08u;
                    work[0x71] = static_cast<uint8_t>(bits);
                    flags = logicFlags8(bits);
                }
            }
        }
    }

    r->eax = eax;
    r->ebx = call.esi;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->eflags = (r->eflags & ~kArithFlags) | flags;
}
GP4_IMPL(0x00412af5, FUN_00412af5, "regs(esi:ptr[0x420]:layout=car_004129fa, edi:rec[0x8943a0,0x160,0..11]) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_00412af5+state_00403800+div_00403bb6+widths_00852c44+ranges_004322a6+ring_00405a12+links_00405a12+sign_00639652")
