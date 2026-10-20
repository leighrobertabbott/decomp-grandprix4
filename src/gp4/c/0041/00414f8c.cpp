#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `test reg, 0xffffffff`: CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value >> 24) & 0x80u);
}

// Flags of a 32-bit `neg` of value.
static inline uint32_t negFlags32(uint32_t value) {
    const uint32_t result = 0u - value;
    return (value != 0u ? 1u : 0u)
        | parityFlag(result)
        | ((value ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

static inline void setFlags(gp4::Regs& cpu, uint32_t flags) {
    cpu.eflags = (cpu.eflags & ~0x8d5u) | flags;
}

// imul by 0x4082 (edx = high dword), shrd eax,edx,14, then clamp to -0x4000..0x4000.
static inline void scaleAxis(gp4::Regs& cpu, uint32_t raw) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(raw)) * 0x4082;
    cpu.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    uint32_t value = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    if (static_cast<int32_t>(value) < 0) {
        uint32_t magnitude = 0u - value;
        if (magnitude >= 0x4000u) magnitude = 0x4000u;
        value = 0u - magnitude;
    } else if (value >= 0x4000u) {
        value = 0x4000u;
    }
    cpu.eax = value;
}

// Response curve of percentage byte p (knee k = p*0x4000/100, gain m = (k<<14)/(0x8000-k)):
// below the knee m*|v|^2/k^2, above it m + (|v|-k)*(0x4000-m)/(0x4000-k) (divisor 1 when k = 0x4000);
// the sign of v is restored. Leaves ecx = the last divisor, ebp = m, edx = v. p = 0 keeps eax.
static inline void responseCurve(gp4::Regs& cpu, uint32_t percent) {
    if (percent == 0u) {
        setFlags(cpu, 0x44u);  // cmp byte, 0 / je
        return;
    }
    const uint32_t value = cpu.eax;
    const uint32_t magnitude = static_cast<int32_t>(value) < 0 ? 0u - value : value;
    const int32_t knee = static_cast<int32_t>(percent << 14) / 100;
    const int32_t gain = static_cast<int32_t>(static_cast<uint32_t>(knee) << 14) / (0x8000 - knee);
    uint32_t result;
    if (static_cast<int32_t>(magnitude) < knee) {
        const uint32_t square = magnitude * magnitude;
        uint32_t rem;
        const uint32_t first = gp4::div64(static_cast<uint64_t>(square) * static_cast<uint32_t>(gain),
            static_cast<uint32_t>(knee), &rem);
        result = gp4::div64(first, static_cast<uint32_t>(knee), &rem);
        cpu.ecx = static_cast<uint32_t>(knee);
    } else {
        uint32_t span = 0x4000u - static_cast<uint32_t>(knee);
        if (span == 0u) span = 1u;
        int32_t rem;
        result = static_cast<uint32_t>(gp4::idiv64(
            static_cast<int64_t>(static_cast<int32_t>(magnitude - static_cast<uint32_t>(knee)))
                * static_cast<int64_t>(0x4000 - gain),
            static_cast<int32_t>(span), &rem)) + static_cast<uint32_t>(gain);
        cpu.ecx = span;
    }
    cpu.ebp = static_cast<uint32_t>(gain);
    cpu.edx = value;
    uint32_t flags = logicFlags32(value);
    if (static_cast<int32_t>(value) < 0) {
        flags = negFlags32(result);
        result = 0u - result;
    }
    cpu.eax = result;
    setFlags(cpu, flags);
}

// sub 0x100, floor at 0, * 0x4105 >> 14 (shrd, edx = high dword), cap at 0x4000.
static inline void pedalCurve(gp4::Regs& cpu) {
    int32_t value = static_cast<int32_t>(cpu.eax - 0x100u);
    if (value < 0) value = 0;
    const int64_t product = static_cast<int64_t>(value) * 0x4105;
    cpu.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    uint32_t scaled = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    if (static_cast<int32_t>(scaled) > 0x4000) scaled = 0x4000u;
    cpu.eax = scaled;
}

static inline uint32_t signedByte(uint8_t value) {
    return static_cast<uint32_t>(static_cast<int32_t>(static_cast<int8_t>(value)));
}

// FUNCTION: GP4 0x00414f8c
// Control inputs of the record at esi (mode bits in byte +0xe6, digital bits in +0x6f): the
// analogue word +0x6a (<< 2) and bytes +0x6e/+0x6c/+0x6d (<< 7) are scaled by 0x4082 >> 14 to
// +-0x4000 and either stepped by 0x00414f09 or shaped by the percentage bytes +0x17a/+0x17b/
// +0x1d2/+0x1d3; the results go to [0x638d20], word +0xee (minus 0x100, * 0x4105 >> 14), [0x638d30]
// (after the 0x00414f29 floor, same curve) and [0x638d34] (0x00414f79 when analogue mode is off),
// which is copied to [0x6258bc]. With [0x622424] set, +0x7a bit 0x20 or +0xe5 bit 4, 0x004153c8
// fills them instead.
void __cdecl FUN_00414f8c(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs cpu = *r;
    cpu.st_in = 0;
    cpu.st_out = 0;
    cpu.st_xin_mask = 0;
    cpu.st_xout_mask = 0;

    if (GP4_GLOBAL(uint8_t, 0x00622424) != 0 || (GP4_FIELD(uint8_t, rec, 0x7a) & 0x20) != 0
        || (GP4_FIELD(uint8_t, rec, 0xe5) & 4) != 0) {
        gp4::call_regs(0x004153c8, cpu);
    } else {
        if ((GP4_FIELD(uint8_t, rec, 0xe6) & 0x80) == 0) {
            if ((GP4_FIELD(uint8_t, rec, 0x6f) & 4) != 0) {
                cpu.eax = 0xffffc000u;
            } else if ((GP4_FIELD(uint8_t, rec, 0x6f) & 8) != 0) {
                cpu.eax = 0x4000u;
            } else {
                cpu.eax = 0u;
            }
        } else {
            scaleAxis(cpu, static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0x6a))) << 2);
            if ((GP4_FIELD(uint8_t, rec, 0xe6) & 8) == 0) {
                gp4::call_regs(0x00414f09, cpu);
            } else {
                responseCurve(cpu, GP4_FIELD(uint8_t, rec, 0x17a));
            }
        }
        GP4_GLOBAL(uint32_t, 0x00638d20) = cpu.eax;

        if ((GP4_FIELD(uint8_t, rec, 0xe6) & 0x40) == 0) {
            cpu.eax = (GP4_FIELD(uint8_t, rec, 0x6f) & 1) != 0 ? 0x4000u : 0u;
        } else {
            scaleAxis(cpu, signedByte(GP4_FIELD(uint8_t, rec, 0x6e)) << 7);
            if ((GP4_FIELD(uint8_t, rec, 0xe6) & 4) == 0) {
                gp4::call_regs(0x00414f09, cpu);
            } else {
                responseCurve(cpu, GP4_FIELD(uint8_t, rec, 0x17b));
            }
        }
        pedalCurve(cpu);
        GP4_FIELD(uint16_t, rec, 0xee) = static_cast<uint16_t>(cpu.eax);

        if ((GP4_FIELD(uint8_t, rec, 0xe6) & 0x20) == 0) {
            cpu.eax = (GP4_FIELD(uint8_t, rec, 0x6f) & 2) != 0 ? 0x4000u : 0u;
        } else {
            scaleAxis(cpu, signedByte(GP4_FIELD(uint8_t, rec, 0x6c)) << 7);
            if ((GP4_FIELD(uint8_t, rec, 0xe6) & 2) == 0) {
                gp4::call_regs(0x00414f09, cpu);
            } else {
                responseCurve(cpu, GP4_FIELD(uint8_t, rec, 0x1d2));
            }
        }
        gp4::call_regs(0x00414f29, cpu);
        pedalCurve(cpu);
        GP4_GLOBAL(uint32_t, 0x00638d30) = cpu.eax;

        if ((GP4_FIELD(uint8_t, rec, 0xe6) & 0x10) == 0) {
            gp4::call_regs(0x00414f79, cpu);
        } else {
            scaleAxis(cpu, signedByte(GP4_FIELD(uint8_t, rec, 0x6d)) << 7);
            if ((GP4_FIELD(uint8_t, rec, 0xe6) & 1) == 0) {
                gp4::call_regs(0x00414f09, cpu);
            } else {
                responseCurve(cpu, GP4_FIELD(uint8_t, rec, 0x1d3));
            }
        }
        GP4_GLOBAL(uint32_t, 0x00638d34) = cpu.eax;
    }

    cpu.eax = GP4_GLOBAL(uint32_t, 0x00638d34);
    GP4_GLOBAL(uint32_t, 0x006258bc) = cpu.eax;

    r->eax = cpu.eax;
    r->ebx = cpu.ebx;
    r->ecx = cpu.ecx;
    r->edx = cpu.edx;
    r->esi = cpu.esi;
    r->edi = cpu.edi;
    r->ebp = cpu.ebp;
    r->eflags = cpu.eflags;
}
GP4_IMPL(0x00414f8c, FUN_00414f8c, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x280]:layout=car_00414f8c) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041b950")
