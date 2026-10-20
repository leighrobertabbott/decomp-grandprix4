#include <gp4/gp4.h>

// Q14 cosine table word for a 16-bit angle already folded into 0..0x7fff: `(angle >> 2) & 0xfffe`
// is the byte offset into the word table at 0x007c5c5c.
static inline int32_t cosineAt(uint32_t angle) {
    const uint32_t offset = ((angle & 0xffffu) >> 2) & 0xfffeu;
    return static_cast<int32_t>(*reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(0x007c5c5cu + offset)));
}

// edx:eax = a * b (signed), shrd eax, edx, 14: low dword of the product >> 14.
static inline uint32_t mulQ14(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x00418d9a
// pushal/popal wrapper (every GPR restored; the flags are those 0x00418ef6 leaves). Fills the 0xa8-byte
// telemetry slot (0x00625abc + byte[esi+0x202] * 0xa8, found by 0x00418901) from the car in esi:
//   +0x10/+0x14/+0x18 = car dwords +0/+8/+4          +0x08 = max(0, dword +0x98) >> 8
//   +0x0c = word +0x18                               +0x1c = (+0x08 * cos(0x4000 - |word +0x18|)) >> 14
//   +0x20 = 0                                        +0x24 = (+0x08 * cos(|word +0x18|)) >> 14
//   +0x88 = byte +0x237      +0x28/+0x2c/+0x30/+0x34 = the four bytes of dword +0x240
//   +0x38.. = [0x0060309c + 4k], +0x48/+0x58/+0x68/+0x78 = 0 (k = 0..3)
//   +0x90 = word +0xee       +0x94 = word +0x62
// then 0x004188a5 refreshes the slot flags, the slot's first dword goes to car +0x26c and
// 0x00418ef6 merges the car flag bits into the slot's status dword +4.
void __cdecl FUN_00418d9a(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00418901, c);
    uint8_t* slot = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.eax));

    GP4_FIELD(uint32_t, slot, 0x10) = GP4_FIELD(uint32_t, car, 0x0);
    GP4_FIELD(uint32_t, slot, 0x14) = GP4_FIELD(uint32_t, car, 0x8);
    GP4_FIELD(uint32_t, slot, 0x18) = GP4_FIELD(uint32_t, car, 0x4);

    int32_t speed = GP4_FIELD(int32_t, car, 0x98);
    if (speed < 0) {
        speed = 0;
    }
    GP4_FIELD(uint32_t, slot, 0x8) = static_cast<uint32_t>(speed) >> 8;
    const uint32_t heading = GP4_FIELD(uint16_t, car, 0x18);
    GP4_FIELD(int32_t, slot, 0xc) = static_cast<int32_t>(static_cast<int16_t>(heading));

    uint32_t folded = (0x4000u - heading) & 0xffffu;
    if ((folded & 0x8000u) != 0u) {
        folded = (0u - folded) & 0xffffu;
    }
    GP4_FIELD(uint32_t, slot, 0x1c) = mulQ14(GP4_FIELD(int32_t, slot, 0x8), cosineAt(folded));
    GP4_FIELD(uint32_t, slot, 0x20) = 0u;

    uint32_t magnitude = heading;
    if ((magnitude & 0x8000u) != 0u) {
        magnitude = (0u - magnitude) & 0xffffu;
    }
    GP4_FIELD(uint32_t, slot, 0x24) = mulQ14(GP4_FIELD(int32_t, slot, 0x8), cosineAt(magnitude));

    GP4_FIELD(uint32_t, slot, 0x88) = GP4_FIELD(uint8_t, car, 0x237);
    uint32_t bytes = GP4_FIELD(uint32_t, car, 0x240);
    GP4_FIELD(uint32_t, slot, 0x28) = bytes & 0xffu;
    GP4_FIELD(uint32_t, slot, 0x2c) = (bytes >> 8) & 0xffu;
    GP4_FIELD(uint32_t, slot, 0x30) = (bytes >> 16) & 0xffu;
    GP4_FIELD(uint32_t, slot, 0x34) = (bytes >> 24) & 0xffu;

    for (uint32_t k = 0u; k != 4u; ++k) {
        GP4_FIELD(uint32_t, slot, 0x78 + k * 4u) = 0u;
        GP4_FIELD(uint32_t, slot, 0x68 + k * 4u) = 0u;
        GP4_FIELD(uint32_t, slot, 0x48 + k * 4u) = 0u;
        GP4_FIELD(uint32_t, slot, 0x58 + k * 4u) = 0u;
        GP4_FIELD(uint32_t, slot, 0x38 + k * 4u) = GP4_ARRAY(uint32_t, 0x0060309c)[k];
    }
    GP4_FIELD(uint32_t, slot, 0x90) = GP4_FIELD(uint16_t, car, 0xee);
    GP4_FIELD(uint32_t, slot, 0x94) = GP4_FIELD(uint16_t, car, 0x62);

    c.edi = c.eax;
    gp4::call_regs(0x004188a5, c);
    GP4_FIELD(uint32_t, car, 0x26c) = GP4_FIELD(uint32_t, slot, 0x0);
    gp4::call_regs(0x00418ef6, c);

    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00418d9a, FUN_00418d9a, "regs(esi:ptr[0x304]:layout=slot_00418942) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00418c68+state_00418d9a")
