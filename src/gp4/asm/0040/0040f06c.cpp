#include <gp4/gp4.h>

// esi: a car block (only the fields read here are named).
struct Unknown_0040f06c_Car {
    uint8_t Unknown00[0x2c];
    uint32_t Unknown2C;
    uint32_t Unknown30;
    uint32_t Unknown34;
    uint8_t Unknown38[0x7c - 0x38];
    uint8_t CarNumber7C;        // low 6 bits: car number (1-based)
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a logic operation (test / and): CF = OF = AF = 0. `signBit` is 0x80 for a byte, 0x80000000 for a dword.
static inline uint32_t logicFlags8(uint32_t res) {
    res &= 0xffu;
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | (res & 0x80u);
}

static inline uint32_t logicFlags32(uint32_t res) {
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | ((res >> 31) << 7);
}

// Flags of the byte `cmp a, b`.
static inline uint32_t cmpFlags8(uint32_t a, uint32_t b) {
    const uint32_t res = (a - b) & 0xffu;
    return (a < b ? 0x01u : 0u) | parityFlag(res) | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u) | (res & 0x80u)
        | ((((a ^ b) & (a ^ res)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `add a, b`.
static inline uint32_t addFlags32(uint32_t a, uint32_t b) {
    const uint32_t res = a + b;
    return (res < a ? 0x001u : 0u) | parityFlag(res) | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u) | ((res >> 31) << 7)
        | ((((a ^ res) & (b ^ res)) >> 31) << 11);
}

// FUNCTION: GP4 0x0040f06c
// Records the car's three dwords (+0x34, +0x2c, +0x30) into the per-car tables at 0x675970 / 0x6759c8 /
// 0x675a20 (index = (byte +0x7c & 0x3f) - 1, clamped at 0x15), re-sorts the order list through
// 0x0051ef71 and sets byte 0x852d70 to 0x80. When the mode byte 0x7c2ea9 has bit 7 set and the car's
// byte +0x7c equals the first order-list byte 0x675958, it also stores [0x66d814] + 5000 in 0x852d44
// and clears bit 8 of 0x852d64. ecx = the table index; al = byte +0x7c on the mode-bit path; the flags are
// those of the last test / cmp / and executed.
void __cdecl FUN_0040f06c(gp4::Regs* r) {
    const Unknown_0040f06c_Car* car = reinterpret_cast<const Unknown_0040f06c_Car*>(static_cast<uintptr_t>(r->esi));

    int32_t index = static_cast<int32_t>(car->CarNumber7C & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }
    GP4_ARRAY(uint32_t, 0x00675970)[index] = car->Unknown34;
    GP4_ARRAY(uint32_t, 0x006759c8)[index] = car->Unknown2C;
    GP4_ARRAY(uint32_t, 0x00675a20)[index] = car->Unknown30;

    gp4::cdecl_<0x0051ef71, void>();

    GP4_GLOBAL(uint8_t, 0x00852d70) = 0x80;
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags = logicFlags8(mode);
    uint32_t eax = r->eax;
    if ((mode & 0x80u) != 0u) {
        const uint32_t number = car->CarNumber7C;
        eax = (eax & 0xffffff00u) | number;
        const uint32_t first = GP4_GLOBAL(uint8_t, 0x00675958);
        flags = cmpFlags8(number, first);
        if (number == first) {
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            GP4_GLOBAL(uint32_t, 0x00852d44) = clock + 0x1388u;
            const uint32_t cleared = GP4_GLOBAL(uint32_t, 0x00852d64) & 0xfffffeffu;
            GP4_GLOBAL(uint32_t, 0x00852d64) = cleared;
            flags = logicFlags32(cleared);
        }
    }
    r->eax = eax;
    r->ecx = static_cast<uint32_t>(index);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040f06c, FUN_0040f06c, "regs(esi:ptr[0x2c4]:layout=rec_0040ee7b+rec_0040f06c, eax:u32) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040ee7b+order_00675958+state_0040f06c")
