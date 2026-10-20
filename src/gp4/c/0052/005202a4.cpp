#include <gp4/gp4.h>
#include <gp4/flags.h>

// A car record (array at 0x66fef0, stride 0x41c); only the fields used here are named.
struct Unknown_005202a4_Car {
    uint8_t Unknown00[0x70];
    uint8_t Laps70;
    uint8_t Unknown71[0x7c - 0x71];
    uint8_t Flags7C;
    uint8_t Unknown7D[0x84 - 0x7d];
    uint8_t Unknown84;
    uint8_t Unknown85[0x8d - 0x85];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x41c - 0x8e];
};

// Slot of the per-car-number dword table at 0x7c3208 for a flag byte ((b & 0x3f) - 1, -1..62).
static inline uint16_t* tableSlot(uint8_t flags) {
    const int32_t index = static_cast<int32_t>(flags & 0x3fu) - 1;
    return GP4_ARRAY(uint16_t, 0x007c3208) + index * 2;
}

// Sort key of a table slot: an entry whose high word is 0x8000 sorts last (0x7fffffff).
static inline uint32_t slotKey(uint8_t flags) {
    const uint16_t* slot = tableSlot(flags);
    uint16_t lo = slot[0];
    uint16_t hi = slot[1];
    if (hi == 0x8000u) {
        lo = 0xffffu;
        hi = 0x7fffu;
    }
    return (static_cast<uint32_t>(hi) << 16) | lo;
}

// FUNCTION: GP4 0x005202a4
// pushal / popal. Calls 0x43b689 (which ranks the cars and copies their flag bytes +0x7c into
// the order bytes 0x7c31dc..), then, with the threshold t = ((byte [0x66d8c8] * 0x3999) >> 14) + 1
// (about 90% of that count): for each car whose (+0x7c & 0x3f) is non-zero, the table dword
// 0x7c3208[(+0x7c & 0x3f) - 1] becomes 0x80000000 when t > signed byte +0x70, else its high word
// grows by 16 * ([0x66d8c8] + 1 - byte +0x70) when that is positive. The 22 order bytes are then
// bubble-sorted by their table dword (signed, high word 0x8000 = last; the left key is kept in
// [0x934de0]). A final pass stores -(byte +0x84 + 1000) for cars below t with bit 4 of +0x8d,
// the negative count difference for the others, and 0x80000000 for any zero dword (through the
// last slot used, as the original's edi). The flags that escape are those of the last `dec ecx`.
void __cdecl FUN_005202a4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0043b689, c);

    const uint32_t product = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x0066d8c8)) * 0x3999u;
    const uint16_t scaled = static_cast<uint16_t>(static_cast<uint16_t>((product << 2) >> 16) + 1u);
    const int8_t threshold = static_cast<int8_t>(static_cast<uint8_t>(scaled));

    uint16_t* slot = reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(r->edi));

    Unknown_005202a4_Car* car = GP4_ARRAY(Unknown_005202a4_Car, 0x0066fef0);
    for (int k = 0; k < 0x16; ++k, ++car) {
        if ((car->Flags7C & 0x3fu) == 0u) {
            continue;
        }
        slot = tableSlot(car->Flags7C);
        if (threshold > static_cast<int8_t>(car->Laps70)) {
            slot[0] = 0;
            slot[1] = 0x8000u;
        } else {
            const uint16_t limit = static_cast<uint16_t>(GP4_GLOBAL(uint8_t, 0x0066d8c8) + 1u);
            const int16_t diff = static_cast<int16_t>(static_cast<uint16_t>(car->Laps70) - limit);
            if (diff < 0) {
                const uint16_t behind = static_cast<uint16_t>(-diff);
                slot[1] = static_cast<uint16_t>(slot[1] + static_cast<uint16_t>(behind << 4));
            }
        }
    }

    uint8_t* order = GP4_ARRAY(uint8_t, 0x007c31dc);
    bool swapped;
    do {
        swapped = false;
        for (int i = 0; i < 0x15; ++i) {
            const uint32_t left = slotKey(order[i]);
            GP4_GLOBAL(uint32_t, 0x00934de0) = left;
            const uint32_t right = slotKey(order[i + 1]);
            if (static_cast<int32_t>(left) > static_cast<int32_t>(right)) {
                const uint8_t kept = order[i + 1];
                order[i + 1] = order[i];
                order[i] = kept;
                swapped = true;
            }
        }
    } while (swapped);

    car = GP4_ARRAY(Unknown_005202a4_Car, 0x0066fef0);
    for (int k = 0; k < 0x16; ++k, ++car) {
        if ((car->Flags7C & 0x3fu) != 0u) {
            slot = tableSlot(car->Flags7C);
            if (threshold > static_cast<int8_t>(car->Laps70)) {
                if ((car->Flags8D & 0x10u) != 0u) {
                    *reinterpret_cast<int32_t*>(slot) = -(static_cast<int32_t>(car->Unknown84) + 1000);
                }
            } else {
                const uint16_t limit = static_cast<uint16_t>(GP4_GLOBAL(uint8_t, 0x0066d8c8) + 1u);
                const int16_t diff = static_cast<int16_t>(static_cast<uint16_t>(car->Laps70) - limit);
                if (diff < 0) {
                    slot[0] = static_cast<uint16_t>(diff);
                    slot[1] = 0xffffu;
                }
            }
        }
        if (*reinterpret_cast<uint32_t*>(slot) == 0u) {
            *reinterpret_cast<uint32_t*>(slot) = 0x80000000u;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, 0u));
}
GP4_IMPL(0x005202a4, FUN_005202a4, "regs(eax:u32, edx:u32) -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+state_0040e84e+carno_0066ff6c+cars_0066fef0+lap_0043b482+state_0044334d+state_0040dfe9")
