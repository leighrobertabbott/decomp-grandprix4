#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF PF AF ZF SF OF) of the 32-bit "cmp left, right".
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) >> 20);
}

static inline uint32_t setLow16(uint32_t reg, uint16_t v) {
    return (reg & 0xffff0000u) | v;
}

// esi = car record, edi = segment record (read by the callee 0x00415f79). Returns in eax the
// signed word picked below (0, 0x4000, [0x0062fbe0], [esi+0xee] or the callee's clamped value),
// raised to at least [0x00624744]; the flags are those of that last signed comparison.
// FUNCTION: GP4 0x00415fe2
void __cdecl FUN_00415fe2(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t eax = setLow16(r->eax, GP4_FIELD(uint16_t, rec, 0xee));
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    GP4_GLOBAL(uint8_t, 0x0062fb48) = 0;
    GP4_GLOBAL(uint32_t, 0x00624744) = 0;

    // 0 = straight to the result, 1 = range check on +0x72 / +0x9a, 2 = the tail that may call 0x00415f79.
    int target;
    if (rec[0xe6] & 4u) {
        if (rec[0xe5] & 0x10u) {
            target = 0;
        } else {
            ecx = setLow16(ecx, GP4_FIELD(uint16_t, rec, 0x9a));
            target = (static_cast<int16_t>(ecx) < GP4_GLOBAL(int16_t, 0x006013e4)) ? 0 : 1;
        }
    } else if (!(rec[0x6f] & 1u)) {
        eax = setLow16(eax, 0);
        target = 0;
    } else if (rec[0xe5] & 0x10u) {
        if (rec[0x203] & 2u) {
            eax = GP4_GLOBAL(uint32_t, 0x0062fbe0);
        } else {
            eax = setLow16(eax, 0x4000);
        }
        target = 0;
    } else if (rec[0x7b] & 0x40u) {
        eax = setLow16(eax, (rec[0xe5] & 4u) ? 0 : 0x4000);
        target = 0;
    } else if (rec[0xe5] & 4u) {
        if (GP4_FIELD(int16_t, rec, 0x9a) < 0x1d5) {
            edx = GP4_GLOBAL(uint32_t, 0x00624740);
            GP4_GLOBAL(uint32_t, 0x00624744) = edx;
        }
        target = 2;
    } else if (rec[0x26b] & 0x80u) {
        target = 2;
    } else {
        ecx = setLow16(ecx, GP4_FIELD(uint16_t, rec, 0x9a));
        if (static_cast<int16_t>(ecx) >= GP4_GLOBAL(int16_t, 0x006013e4)) {
            target = 1;
        } else {
            eax = setLow16(eax, 0x4000);
            target = 0;
        }
    }

    if (target == 1) {
        if (static_cast<int8_t>(rec[0x72]) > 1) {
            target = 2;
        } else {
            ecx = setLow16(ecx, GP4_FIELD(uint16_t, rec, 0x9a));
            target = (static_cast<int16_t>(ecx) < GP4_GLOBAL(int16_t, 0x006013e6)) ? 0 : 2;
        }
    }

    if (target == 2) {
        GP4_GLOBAL(uint8_t, 0x0062fb48) = 0xff;
        if ((rec[0x78] & 1u) || (rec[0x9c] & 1u) || (rec[0xe5] & 4u)) {
            gp4::Regs call = *r;
            call.eax = eax;
            call.ecx = ecx;
            call.edx = edx;
            gp4::call_regs(0x00415f79, call);
            eax = call.eax;
            edx = call.edx;
        }
    }

    const int32_t picked = static_cast<int16_t>(eax);
    const int32_t floor = GP4_GLOBAL(int32_t, 0x00624744);
    r->eflags = (r->eflags & ~0x8d5u) | compareFlags32(static_cast<uint32_t>(picked), static_cast<uint32_t>(floor));
    r->eax = static_cast<uint32_t>(picked < floor ? floor : picked);
    r->ecx = ecx;
    r->edx = edx;
}
GP4_IMPL(0x00415fe2, FUN_00415fe2, "regs(esi:ptr[0x273]:bytes, edi:ptr[0x59]:bytes, eax:u32, ecx:u32, edx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); cases=600; globals=fuzz:i32+layout=lim_00415f79")
