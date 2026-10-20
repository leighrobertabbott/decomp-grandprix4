#include <gp4/gp4.h>
#include <gp4/flags.h>

// Steps a 4-entry rotating index (word 0x0088f5ce, wrapping at the word limit 0x0088f5d0) while
// any of the bits 2..3 of the record's byte +0x30 is also set in the mask byte 0x008705ed. When the
// byte tables 0x0088f5c9 / 0x0088f5ca differ at the new index (mod 4) the record's flags byte +0x3c
// gets 0x0c set. The index (0xffff when inactive, else the new one) is stored in the byte +0x23.
// Leaves ax = the new index, ebx = index & 3 and cl = the table byte (active case) and the flags
// of the last instruction executed.
// FUNCTION: GP4 0x0055a53e
void __cdecl FUN_0055a53e(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(r->edi);
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t ecx = r->ecx;

    const uint8_t active = static_cast<uint8_t>((rec[0x30] & 0x0cu) & GP4_GLOBAL(uint8_t, 0x008705ed));
    uint32_t arith = gp4::flags::logic8(active);
    uint16_t ax;
    if (active == 0u) {
        GP4_GLOBAL(uint16_t, 0x0088f5ce) = 0xffffu;
        ax = 0u;
    } else {
        ax = GP4_GLOBAL(uint16_t, 0x0088f5ce);
        arith = gp4::flags::add16(ax, 1u);
        ax = static_cast<uint16_t>(ax + 1u);
        const uint16_t limit = GP4_GLOBAL(uint16_t, 0x0088f5d0);
        arith = gp4::flags::sub16(ax, limit);
        if (static_cast<int16_t>(ax) >= static_cast<int16_t>(limit)) {
            ax = 0u;
        }
        GP4_GLOBAL(uint16_t, 0x0088f5ce) = ax;
        ebx = static_cast<uint32_t>(ax) & 3u;
        const uint8_t cl = GP4_ARRAY(uint8_t, 0x0088f5ca)[ebx];
        const uint8_t other = GP4_ARRAY(uint8_t, 0x0088f5c9)[ebx];
        arith = gp4::flags::sub8(cl, other);
        ecx = (ecx & 0xffffff00u) | cl;
        if (cl != other) {
            const uint8_t v = static_cast<uint8_t>(rec[0x3c] | 0x0cu);
            rec[0x3c] = v;
            arith = gp4::flags::logic8(v);
        }
    }
    rec[0x23] = static_cast<uint8_t>(ax);

    r->eax = (eax & 0xffff0000u) | ax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0055a53e, FUN_0055a53e, "regs(edi:ptr[0x44]:bytes, eax:u32, ebx:u32, ecx:u32) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
