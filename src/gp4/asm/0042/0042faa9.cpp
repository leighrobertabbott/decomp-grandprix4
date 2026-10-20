#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit `or` on memory: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

struct Unknown_0042faa9_Record {
    uint8_t Unknown00[0x94];
    int16_t Unknown94;
    uint8_t Unknown96[0xca - 0x96];
    uint8_t UnknownCA;
    uint8_t UnknownCB;
    uint8_t UnknownCC[0xdc - 0xcc];
    int16_t UnknownDC;
    uint8_t UnknownDE[0x273 - 0xde];
    uint8_t Unknown273;
};

// FUNCTION: GP4 0x0042faa9
// esi = record. Stores the average of the words at 0x00603c8e / 0x00603c90 into +0xdc, runs
// 0x004037a9 with ecx = that average and eax = word +0x94, then sets flag bits in +0xcb, +0xca and
// +0x273. eax and edx are the outputs of 0x004037a9, ecx is the sign-extended average, and the
// flags are those of the final `or byte [esi+0x273], 0x40`.
void __cdecl FUN_0042faa9(gp4::Regs* r) {
    Unknown_0042faa9_Record* record = reinterpret_cast<Unknown_0042faa9_Record*>(static_cast<uintptr_t>(r->esi));

    const int32_t sum = static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00603c8e)) + static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x00603c90));
    const int16_t average = static_cast<int16_t>(sum >> 1);
    record->UnknownDC = average;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.ecx = static_cast<uint32_t>(static_cast<int32_t>(average));
    call.eax = static_cast<uint32_t>(static_cast<int32_t>(record->Unknown94));
    gp4::call_regs(0x004037a9, call);

    record->UnknownCB |= 0x80u;
    record->UnknownCB |= 0x40u;
    record->UnknownCA |= 0x10u;
    record->Unknown273 |= 0x80u;
    record->Unknown273 |= 0x40u;

    r->eax = call.eax;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(record->Unknown273);
}
GP4_IMPL(0x0042faa9, FUN_0042faa9, "regs(esi:ptr[0x420]:layout=rec_00409416) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800")
