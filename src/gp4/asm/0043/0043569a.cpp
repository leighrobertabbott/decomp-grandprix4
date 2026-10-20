#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline void callWith(const gp4::Regs* r, uint32_t addr) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(addr, call);
}

// Starts the manoeuvre (bit 6 plus `modeBit` of the byte +0x179) unless bit 6 is already set:
// 0x00434e1e with al = [0x00868582], 0x0042784b and 0x0042783c, then bits 2 and 0 of +0x135.
static inline void startManoeuvre(gp4::Regs* r, uint8_t* rec, uint8_t modeBit) {
    if ((GP4_FIELD(uint8_t, rec, 0x179) & 0x40u) != 0u) {
        return;
    }
    GP4_FIELD(uint8_t, rec, 0x179) = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x179) | 0x40u);
    GP4_FIELD(uint8_t, rec, 0x179) = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x179) | modeBit);
    r->eax = (r->eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00868582);
    callWith(r, 0x00434e1e);
    callWith(r, 0x0042784b);
    callWith(r, 0x0042783c);
    GP4_FIELD(uint8_t, rec, 0x135) = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x135) | 4u);
    GP4_FIELD(uint8_t, rec, 0x135) = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x135) | 1u);
}

// Starts a manoeuvre on the car esi when its bit 7 of +0x71 is set and bit 1 of +0x8d is clear:
// mode 0x10 when bit 0 of [edi+0x92] is set, mode 0x20 when only bit 1 is set and bit 7 of
// [esi+0x230] is set. Otherwise bits 6, 5, 4 of +0x179 are cleared. Finally the byte +0xe4 is
// cleared when edi is below [0x00891500]. al ends as [0x00868582] when a manoeuvre starts; the
// flags are those of `cmp edi, [0x00891500]`.
// FUNCTION: GP4 0x0043569a
void __cdecl FUN_0043569a(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    bool handled = false;
    if ((GP4_FIELD(uint8_t, rec, 0x71) & 0x80u) != 0u && (GP4_FIELD(uint8_t, rec, 0x8d) & 2u) == 0u) {
        if ((GP4_FIELD(uint8_t, other, 0x92) & 1u) != 0u) {
            startManoeuvre(r, rec, 0x10u);
            handled = true;
        } else if ((GP4_FIELD(uint8_t, other, 0x92) & 2u) != 0u && (GP4_FIELD(uint8_t, rec, 0x230) & 0x80u) != 0u) {
            startManoeuvre(r, rec, 0x20u);
            handled = true;
        }
    }
    if (!handled) {
        GP4_FIELD(uint8_t, rec, 0x179) = static_cast<uint8_t>(GP4_FIELD(uint8_t, rec, 0x179) & 0x8fu);
    }

    const uint32_t reference = GP4_GLOBAL(uint32_t, 0x00891500);
    if (r->edi < reference) {
        GP4_FIELD(uint8_t, rec, 0xe4) = 0;
    }
    r->eflags = (r->eflags & ~0x8d5u) | subFlags32(r->edi, reference);
}
GP4_IMPL(0x0043569a, FUN_0043569a, "regs(eax:u32, edi:ptr[0x1d0]:bytes, esi:ptr[0x420]:layout=rec_0043569a) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800")
