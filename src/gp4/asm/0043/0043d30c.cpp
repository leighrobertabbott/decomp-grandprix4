#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `cmp left, right`.
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// Arithmetic flags of `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

struct Unknown_0043d30c_Record {
    uint8_t Unknown00[0x12];
    uint8_t Flags12;
    uint8_t Unknown13[0xca - 0x13];
    uint16_t UnknownCA;
};

// One step back through the ring of 0x160-byte records: p - 0x160, plus the ring length when it
// falls below the ring start. `flags` receives the flags of the last compare/add executed.
static inline uint32_t stepBack(uint32_t position, uint32_t* flags) {
    position -= 0x160u;
    const uint32_t start = GP4_GLOBAL(uint32_t, 0x00866ec0);
    *flags = cmpFlags(position, start);
    if (position < start) {
        const uint32_t length = GP4_GLOBAL(uint32_t, 0x00866ebc);
        *flags = addFlags(position, length);
        position += length;
    }
    return position;
}

// Walks back through the ring of 0x160-byte records from edi, OR-ing dl into the byte at +0x12 of
// every visited record: [0x6269a0] records first, then up to [0x6269a4] more while the word at
// +0xca of the next record is at least 0xc0 above the previous one (signed), then [0x6269a8] more.
// ebx ends on the record after the last visited and ecx on 0 (loop); eax and ebp are saved and
// restored; the flags are those of the last wrap compare/add.
// FUNCTION: GP4 0x0043d30c
void __cdecl FUN_0043d30c(gp4::Regs* r) {
    const uint8_t mask = static_cast<uint8_t>(r->edx);
    uint32_t position = r->edi;
    uint32_t flags = 0;
    uint32_t previous = 0;

    uint32_t count = GP4_GLOBAL(uint32_t, 0x006269a0);
    do {
        Unknown_0043d30c_Record* record = reinterpret_cast<Unknown_0043d30c_Record*>(static_cast<uintptr_t>(position));
        record->Flags12 = static_cast<uint8_t>(record->Flags12 | mask);
        previous = record->UnknownCA;
        position = stepBack(position, &flags);
        --count;
    } while (count != 0u);

    count = GP4_GLOBAL(uint32_t, 0x006269a4);
    do {
        Unknown_0043d30c_Record* record = reinterpret_cast<Unknown_0043d30c_Record*>(static_cast<uintptr_t>(position));
        const int32_t difference = static_cast<int32_t>(static_cast<uint32_t>(record->UnknownCA) - previous);
        if (difference < 0xc0) {
            break;
        }
        record->Flags12 = static_cast<uint8_t>(record->Flags12 | mask);
        previous = record->UnknownCA;
        position = stepBack(position, &flags);
        --count;
    } while (count != 0u);

    count = GP4_GLOBAL(uint32_t, 0x006269a8);
    do {
        Unknown_0043d30c_Record* record = reinterpret_cast<Unknown_0043d30c_Record*>(static_cast<uintptr_t>(position));
        record->Flags12 = static_cast<uint8_t>(record->Flags12 | mask);
        position = stepBack(position, &flags);
        --count;
    } while (count != 0u);

    r->ebx = position;
    r->ecx = 0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043d30c, FUN_0043d30c, "regs(edi:rec[0x8943a0,0x160,0..25], edx:u32) -> (ebx, ecx, cf, pf, af, zf, sf, of); globals=layout=ring_0043d30c")
