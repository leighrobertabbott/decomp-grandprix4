#include <gp4/gp4.h>

// Car record fields read or written by 0x004161fb (offsets in the names).
struct Unknown_004161fb {
    uint8_t Unknown00[0x62];
    uint16_t Unknown62;
    uint8_t Unknown64[0x6f - 0x64];
    uint8_t Unknown6F;
    uint8_t Unknown70[0x72 - 0x70];
    int8_t Unknown72;
    uint8_t Unknown73[0x78 - 0x73];
    uint8_t Unknown78;
    uint8_t Unknown79[0x84 - 0x79];
    uint8_t Unknown84;
    uint8_t Unknown85[0x8d - 0x85];
    uint8_t Unknown8D;
    uint8_t Unknown8E[0x9a - 0x8e];
    int16_t Unknown9A;
    uint8_t Unknown9C;
    uint8_t Unknown9D[0xcf - 0x9d];
    uint8_t UnknownCF;
    uint8_t UnknownD0[0xe4 - 0xd0];
    int8_t UnknownE4;
    uint8_t UnknownE5;
    uint8_t UnknownE6[0xff - 0xe6];
    uint8_t UnknownFF;
    uint8_t Unknown100[0x1cd - 0x100];
    uint8_t Unknown1CD;
    uint8_t Unknown1CE[0x21d - 0x1ce];
    uint8_t Unknown21D;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of TEST / AND / OR on a value of the given width (mask 0xff, 0xffff, 0xffffffff).
static inline uint32_t logicFlags(uint32_t result, uint32_t mask) {
    const uint32_t sign = (mask >> 1) + 1u;
    result &= mask;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u);
}

// Flags of "cmp left, right" / "sub left, right" at the given width.
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t mask) {
    const uint32_t sign = (mask >> 1) + 1u;
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & sign) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & sign) != 0u ? 0x800u : 0u);
}

// Flags of "add left, right" at the given width.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t mask) {
    const uint32_t sign = (mask >> 1) + 1u;
    left &= mask;
    right &= mask;
    const uint32_t result = (left + right) & mask;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & sign) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & sign) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004161fb
// Pit-stop / stint step of the car record in esi: depending on the flag bytes +0xe5 (bit 2: in the
// pits, bit 4, bit 2 = "done"), +0xe4 (state 3..7 jumps straight to the "done" mark), the globals
// 0x00638dc4 / 0x00638ce0 / 0x00638dbc / 0x00638dc8 and the thresholds 0x006013e4 / 0x006013ea it
// raises or lowers the step counter byte +0x72 (bounded by the table limit +0x1cd and the value of
// 0x0041adbd) and finally sets bit 2 of +0xe5. Only al, dx (edx after the 0x41adbd call) and the
// flags of the last flag-writing instruction leave the routine.
void __cdecl FUN_004161fb(gp4::Regs* r) {
    Unknown_004161fb* const rec = reinterpret_cast<Unknown_004161fb*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t fl;
    uint32_t limit;
    uint16_t word;

    fl = logicFlags(rec->UnknownE5 & 4u, 0xffu);
    if ((rec->UnknownE5 & 4u) != 0u) {
        fl = logicFlags(rec->UnknownE5 & 0x10u, 0xffu);
        if ((rec->UnknownE5 & 0x10u) != 0u) goto done;
    }
    fl = logicFlags(rec->Unknown8D & 0x10u, 0xffu);
    if ((rec->Unknown8D & 0x10u) != 0u) {
        fl = subFlags(rec->Unknown84, 7u, 0xffu);
        if (rec->Unknown84 == 7u) goto done;
    }
    fl = logicFlags(rec->UnknownE5 & 2u, 0xffu);
    if ((rec->UnknownE5 & 2u) != 0u) goto done;
    fl = subFlags(static_cast<uint32_t>(rec->UnknownE4), 3u, 0xffu);
    if (rec->UnknownE4 >= 3) {
        fl = subFlags(static_cast<uint32_t>(rec->UnknownE4), 7u, 0xffu);
        if (rec->UnknownE4 <= 7) goto mark;
    }
    fl = logicFlags(GP4_GLOBAL(uint8_t, 0x00638dc4), 0xffu);
    if ((GP4_GLOBAL(uint8_t, 0x00638dc4) & 0x80u) != 0u) goto step_back;
    if (GP4_GLOBAL(uint8_t, 0x00638dc4) != 0u) goto advance;
    fl = logicFlags(rec->UnknownE5 & 4u, 0xffu);
    if ((rec->UnknownE5 & 4u) != 0u) goto check_6f_bit0;
    fl = logicFlags(rec->UnknownCF & 0x20u, 0xffu);
    if ((rec->UnknownCF & 0x20u) == 0u) goto check_6f_bit0;
    fl = logicFlags(rec->Unknown6F & 0x80u, 0xffu);
    if ((rec->Unknown6F & 0x80u) != 0u) goto lookup;
    goto advance;

check_6f_bit0:
    fl = logicFlags(rec->Unknown6F & 1u, 0xffu);
    if ((rec->Unknown6F & 1u) == 0u) goto lookup;

advance:
    eax = (eax & 0xffffff00u) | static_cast<uint8_t>(rec->Unknown72);
    fl = subFlags(eax, 1u, 0xffu);
    if (static_cast<int8_t>(eax) >= 1) {
        rec->UnknownFF |= 4u;
        fl = logicFlags(rec->UnknownFF, 0xffu);
        goto advance_tail;
    }
    word = static_cast<uint16_t>(rec->Unknown9A);
    edx = (edx & 0xffff0000u) | word;
    fl = subFlags(static_cast<uint32_t>(rec->Unknown72), 0u, 0xffu);
    if (rec->Unknown72 == 0) {
        fl = logicFlags(GP4_GLOBAL(uint32_t, 0x00638ce0), 0xffffffffu);
        if ((GP4_GLOBAL(uint32_t, 0x00638ce0) & 0x80000000u) == 0u) {
            fl = subFlags(word, GP4_GLOBAL(uint16_t, 0x006013ea), 0xffffu);
            if (static_cast<int16_t>(word) >= GP4_GLOBAL(int16_t, 0x006013ea)) goto done;
            goto advance_tail;
        }
    }
    fl = subFlags(word, GP4_GLOBAL(uint16_t, 0x006013e4), 0xffffu);
    if (static_cast<int16_t>(word) >= GP4_GLOBAL(int16_t, 0x006013e4)) goto done;

advance_tail:
    fl = subFlags(eax, 1u, 0xffu);
    if (static_cast<int8_t>(eax) >= 1) {
        fl = subFlags(GP4_GLOBAL(uint32_t, 0x00638dc8), 0x2328u, 0xffffffffu);
        if (GP4_GLOBAL(int32_t, 0x00638dc8) < 0x2328) {
            rec->UnknownFF &= 0xfbu;
            fl = logicFlags(rec->UnknownFF, 0xffu);
            goto done;
        }
        fl = subFlags(eax, rec->Unknown1CD, 0xffu);
        if (static_cast<uint8_t>(eax) == rec->Unknown1CD) goto done;
    }
    limit = (eax & 0xffu) + 1u;
    fl = addFlags(eax, 1u, 0xffu);
    eax = (eax & 0xffffff00u) | (limit & 0xffu);
    goto store_step;

lookup:
    eax = (eax & 0xffffff00u) | static_cast<uint8_t>(rec->Unknown72);
    fl = subFlags(eax, 2u, 0xffu);
    if (static_cast<int8_t>(eax) >= 2) {
        rec->UnknownFF |= 4u;
        fl = logicFlags(rec->UnknownFF, 0xffu);
    }
    {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.eax = eax;
        call.edx = edx;
        gp4::call_regs(0x0041adbd, call);
        edx = call.eax;
    }
    fl = subFlags(rec->Unknown62, edx, 0xffffu);
    if (rec->Unknown62 > static_cast<uint16_t>(edx)) goto done;
    fl = subFlags(static_cast<uint32_t>(rec->Unknown72), 1u, 0xffu);
    if (rec->Unknown72 <= 1) {
        word = static_cast<uint16_t>(rec->Unknown9A);
        edx = (edx & 0xffff0000u) | word;
        fl = subFlags(word, GP4_GLOBAL(uint16_t, 0x006013e4), 0xffffu);
        if (static_cast<int16_t>(word) >= GP4_GLOBAL(int16_t, 0x006013e4)) goto done;
    }
    fl = logicFlags(rec->UnknownE5 & 4u, 0xffu);
    if ((rec->UnknownE5 & 4u) != 0u) goto step_back;
    fl = logicFlags(rec->Unknown78 & 1u, 0xffu);
    if ((rec->Unknown78 & 1u) == 0u) goto step_back;
    fl = logicFlags(GP4_GLOBAL(uint16_t, 0x00638dbc), 0xffffu);
    if ((GP4_GLOBAL(uint16_t, 0x00638dbc) & 0x8000u) != 0u) goto advance;

step_back:
    eax = (eax & 0xffffff00u) | static_cast<uint8_t>(rec->Unknown72);
    fl = subFlags(eax, 2u, 0xffu);
    if (static_cast<int8_t>(eax) < 2) {
        fl = logicFlags(rec->UnknownE5 & 4u, 0xffu);
        if ((rec->UnknownE5 & 4u) != 0u) goto done;
    }
    fl = logicFlags(eax, 0xffu);
    if ((eax & 0xffu) == 0u) {
        fl = logicFlags(rec->UnknownE5 & 4u, 0xffu);
        if ((rec->UnknownE5 & 4u) == 0u) {
            fl = logicFlags(rec->Unknown21D & 0x80u, 0xffu);
            if ((rec->Unknown21D & 0x80u) == 0u) goto advance;
        }
    }
    fl = logicFlags(eax, 0xffu);
    if ((eax & 0x80u) == 0u) {
        limit = ((eax & 0xffu) - 1u) & 0xffu;
        fl = subFlags(eax, 1u, 0xffu);
        eax = (eax & 0xffffff00u) | limit;
        rec->Unknown9C |= 4u;
        fl = logicFlags(rec->Unknown9C, 0xffu);
    }

store_step:
    rec->Unknown72 = static_cast<int8_t>(eax);
mark:
    rec->UnknownE5 |= 2u;
    fl = logicFlags(rec->UnknownE5, 0xffu);

done:
    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | fl;
}
GP4_IMPL(0x004161fb, FUN_004161fb, "regs(eax:u32, edx:u32, esi:ptr[0x225]:layout=car_004161fb+gears_0041034e) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=thr_007c28bc+state_004161fb+setup_004112e3")
