#include <gp4/gp4.h>

enum : uint32_t {
    kArithFlags = 0x8d5u,   // OF SF ZF AF PF CF
    kPF = 0x4u, kAF = 0x10u, kZF = 0x40u, kSF = 0x80u, kOF = 0x800u
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? kPF : 0u;
}

// Flags left by a logic operation (OR) on a 32-bit or 8-bit result: CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result, uint32_t signBit) {
    return parityFlag8(result) | (result == 0u ? kZF : 0u) | ((result & signBit) != 0u ? kSF : 0u);
}

// The working car record that 0x00412af5 fills. Kept behind a call so its field offsets stay
// register-relative like the original's "mov ebx, 0x639204".
static __declspec(noinline) uint8_t* WorkRecord() {
    return GP4_ARRAY(uint8_t, 0x00639204);
}

// FUNCTION: GP4 0x00412938
// Copies the state of the working record (ebx = 0x00639204: bit 3 of +0x71 -> edx = -1/0, dword
// +0x1a0 -> eax, byte +0x294 -> ecx) into the car record at esi through 0x00429284, with eax forced
// to 0x4000 when byte 0x0063964a is set or the working record has +0x8d bit 1 without +0x9c bit 1.
// [0x00601ba6] is 0xff and [0x00601b34] the car's word +0x66 during the call. Stores eax to dword
// +0x1a0, cl to byte +0x294 and bit 3 of +0x71 from the sign of edx.
void __cdecl FUN_00412938(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* work = WorkRecord();

    uint32_t edx = 0u;
    if ((work[0x71] & 0x08u) != 0u) {
        edx = 0xffffffffu;
    }
    uint32_t eax = GP4_FIELD(uint32_t, work, 0x1a0);
    const uint32_t ecx = work[0x294];
    if (GP4_GLOBAL(uint8_t, 0x0063964a) != 0u ||
        ((work[0x8d] & 0x02u) != 0u && (work[0x9c] & 0x02u) == 0u)) {
        eax = 0x4000u;
    }

    GP4_GLOBAL(uint8_t, 0x00601ba6) = 0xff;
    const uint32_t savedFraction = GP4_GLOBAL(uint32_t, 0x00601b34);
    GP4_GLOBAL(int32_t, 0x00601b34) = GP4_FIELD(int16_t, car, 0x66);

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.ebx = 0x00639204u;
    call.eax = eax;
    call.ecx = ecx;
    call.edx = edx;
    gp4::call_regs(0x00429284, call);

    GP4_GLOBAL(uint32_t, 0x00601b34) = savedFraction;
    GP4_GLOBAL(uint8_t, 0x00601ba6) = 0;
    GP4_FIELD(uint32_t, car, 0x1a0) = call.eax;
    car[0x294] = static_cast<uint8_t>(call.ecx);
    car[0x71] = static_cast<uint8_t>(car[0x71] & 0xf7u);

    uint32_t flags = logicFlags(call.edx, 0x80000000u);
    if ((call.edx & 0x80000000u) != 0u) {
        car[0x71] = static_cast<uint8_t>(car[0x71] | 0x08u);
        flags = logicFlags(car[0x71], 0x80u);
    }

    r->eax = call.eax;
    r->ebx = 0x00639204u;
    r->ecx = call.ecx;
    r->edx = call.edx;
    r->eflags = (r->eflags & ~kArithFlags) | flags;
}
GP4_IMPL(0x00412938, FUN_00412938, "regs(esi:ptr[0x2a0]:bytes, edi:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+widths_00852c44+work_00639204")
