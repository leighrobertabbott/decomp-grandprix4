#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags left by a byte TEST/OR/AND and by a CMP against zero of a nonzero byte: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t value) {
    return parityFlag(value) | ((value & 0xffu) == 0u ? 0x40u : 0u) | (value & 0x80u);
}

// Flags of the byte `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return ((left & 0xffu) < (right & 0xffu) ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Car record fields this routine touches.
struct Unknown_0040eb6b_Car {
    uint8_t Unknown00[0x60];
    uint16_t Unknown60;
    uint8_t Unknown62[0x0e];
    uint8_t Unknown70;
    uint8_t Flags71;
    uint8_t Unknown72[0x0a];
    uint8_t CarNumber7C;
    uint8_t Unknown7D[0x0e];
    uint8_t Flags8B;
    uint8_t Unknown8C[0x59];
    uint8_t Unknown_E5;
    uint8_t UnknownE6[0x18];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x63];
    uint8_t Flags162;
    uint8_t Unknown163[0xcd];
    uint8_t Flags230;
};

// Index of the car's entry in the per-car byte table at 0x0066ec4a: (number & 0x3f) - 1, at most 0x15.
static inline int32_t tableIndex(const Unknown_0040eb6b_Car* car) {
    int32_t index = static_cast<int32_t>(car->CarNumber7C & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }
    return index;
}

static __declspec(noinline) uint32_t drawRandom(const gp4::Regs* entry, uint32_t eax) {
    gp4::Regs call = *entry;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = eax;
    gp4::call_regs(0x00401462, call);
    return call.eax;
}

static __declspec(noinline) void refreshGate(gp4::Regs& call) { gp4::call_regs(0x0040e97d, call); }
static __declspec(noinline) void markEqual(gp4::Regs& call) { gp4::call_regs(0x0040ec79, call); }
static __declspec(noinline) void markLower(gp4::Regs& call) { gp4::call_regs(0x0040ecc4, call); }
static __declspec(noinline) void markHigher(gp4::Regs& call) { gp4::call_regs(0x0040ec92, call); }

// FUNCTION: GP4 0x0040eb6b
// Decides from the mode byte [0x007c2ea9] (0x80: nothing, 0: quiet start, bit 0x40: the flagged
// branch, else a random draw) and the car's state whether to flag the car in esi (bit 5 of +0x162,
// bit 7 of +0x71, bit 7 of +0x230 cleared, bit 2 of +0xfe). Leaves eax = the level compared with the
// car's byte +0x70 and edx = the car's word +0x60 (or the gate value on the flagged branch); the
// flags are those of the last compare, test or OR.
void __cdecl FUN_0040eb6b(gp4::Regs* r) {
    Unknown_0040eb6b_Car* car = reinterpret_cast<Unknown_0040eb6b_Car*>(static_cast<uintptr_t>(r->esi));
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    if (mode == 0x80u) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;                    // cmp 0x80, equal
        return;
    }
    car->Flags162 = static_cast<uint8_t>(car->Flags162 & 0xbfu);

    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t flags;
    bool settle = true;                                                // false: go straight to the flagging

    if (mode == 0u) {
        const uint8_t quiet = GP4_GLOBAL(uint8_t, 0x007c2e75);
        if (quiet != 0u) {
            r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(quiet);   // cmp byte, 0 (nonzero)
            return;
        }
        const uint8_t number = car->CarNumber7C;
        if (static_cast<int8_t>(number) < 0) {
            r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(number);  // test byte, 0xff; js
            return;
        }
        eax = (eax & 0xffff0000u) | 0x0au;                            // mov ax, 0xa
    } else if ((mode & 0x40u) == 0u) {
        const int32_t index = tableIndex(car);
        eax = (static_cast<uint32_t>(index) & 0xffffff00u) | GP4_ARRAY(uint8_t, 0x0066ec4a)[index];
        edx = GP4_GLOBAL(uint32_t, 0x007c2e98) - 1u;
        eax = (drawRandom(r, eax) & 0xfu) + 3u;
    } else {
        car->Flags8B = static_cast<uint8_t>(car->Flags8B | 0x80u);
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        refreshGate(call);
        if (GP4_GLOBAL(uint32_t, 0x0066d8cc) == 0u) {
            settle = false;
        } else {
            const int32_t index = tableIndex(car);
            const uint8_t entry = GP4_ARRAY(uint8_t, 0x0066ec4a)[index];
            eax = (static_cast<uint32_t>(index) & 0xffffff00u) | entry;
            edx = GP4_GLOBAL(uint32_t, 0x007c2e98) - 1u;
            call.eax = eax;
            call.edx = edx;
            const int8_t entryByte = static_cast<int8_t>(entry);
            const int8_t limitByte = static_cast<int8_t>(edx);
            if (entryByte < limitByte) {
                const int32_t cap = GP4_GLOBAL(int32_t, 0x00679b50);
                eax = 3u;
                if (cap < 3) {
                    eax = static_cast<uint32_t>(cap);
                }
            } else {
                if (entryByte == limitByte) {
                    markEqual(call);
                } else {
                    markLower(call);
                    markHigher(call);
                }
                settle = false;
            }
        }
    }

    if (settle) {
        edx = car->Unknown60;
        if (static_cast<int16_t>(edx) >= 0x80) {
            const uint32_t gate = car->Unknown_E5 & 4u;
            if (gate == 0u) {
                r->eax = eax;
                r->edx = edx;
                r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(gate);   // test byte, 4: zero
                return;
            }
            flags = cmpFlags8(eax, car->Unknown70);
            if (static_cast<int8_t>(eax) > static_cast<int8_t>(car->Unknown70)) {
                r->eax = eax;
                r->edx = edx;
                r->eflags = (r->eflags & ~0x8d5u) | flags;
                return;
            }
        }
    }

    car->Flags162 = static_cast<uint8_t>(car->Flags162 | 0x20u);
    car->Flags71 = static_cast<uint8_t>(car->Flags71 | 0x80u);
    car->Flags230 = static_cast<uint8_t>(car->Flags230 & 0x7fu);
    car->FlagsFE = static_cast<uint8_t>(car->FlagsFE | 4u);

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(car->FlagsFE);
}
GP4_IMPL(0x0040eb6b, FUN_0040eb6b, "regs(eax:u32, edx:u32, esi:ptr[0x238]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040ee7b+order_00675958+state_0040ed29+state_0040f112+state_0040eb6b")
