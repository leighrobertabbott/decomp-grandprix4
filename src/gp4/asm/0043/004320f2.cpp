#include <gp4/gp4.h>

// esi: a car record (only the fields this routine reads or writes are named).
struct Unknown_004320f2_Car {
    uint8_t Unknown00[0x1c];
    int32_t Unknown1C;          // segment pointer
    uint8_t Unknown20[0x5c];
    uint8_t Flags7C;
    uint8_t Unknown7D;
    uint8_t Kind7E;
    uint8_t Unknown7F[0x8d - 0x7f];
    uint8_t Flags8D;
    uint8_t Unknown8E[2];
    int16_t Unknown90;
    uint8_t Unknown92[0x9c - 0x92];
    uint8_t Flags9C;
    uint8_t Unknown9D[0x1ce - 0x9d];
    uint8_t Flags1CE;
    uint8_t Unknown1CF[0x230 - 0x1cf];
    uint8_t Flags230;
    uint8_t Unknown231[0x25f - 0x231];
    uint8_t Flags25F;
    uint8_t Unknown260[0x26b - 0x260];
    uint8_t Flags26B;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a byte logic operation (and / or): CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// FUNCTION: GP4 0x004320f2
// Per-car steering/pit-lane state: decides from the record at esi (bit 1 of +0x8d, bit 4 of +0x9c,
// the global byte 0x7c2e75 and the distance between the car's segment +0x1c and 0x0043207c's
// segment) whether the car is on a lane. When it is not, the bits of +0x26b and +0x25f/+0x1ce are
// cleared and the kind byte +0x7e becomes 0; otherwise +0x7e (1..5) and bits 7/6/5 of +0x26b are
// set from the car flags, and the 0x004320b2 offset (stored in 0x627868) compared with the signed
// word +0x90 sets bit 3 / bit 4 of +0x26b. eax, ecx, edx and the flags are whatever the last
// instructions of the taken path leave (the flags are those of the last and / or on a record byte).
void __cdecl FUN_004320f2(gp4::Regs* r) {
    Unknown_004320f2_Car* car = reinterpret_cast<Unknown_004320f2_Car*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    bool reset = false;

    if ((car->Flags8D & 2u) == 0u || (car->Flags9C & 0x10u) != 0u || GP4_GLOBAL(uint8_t, 0x007c2e75) == 0xffu) {
        reset = true;
    } else if ((car->Flags1CE & 4u) == 0u && (car->Flags25F & 2u) == 0u && (car->Flags25F & 1u) == 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x0043207c, call);
        eax = call.eax - static_cast<uint32_t>(car->Unknown1C);
        edx = eax;
        if (static_cast<int32_t>(edx) < 0) {
            edx = 0u - edx;
        }
        if (static_cast<int32_t>(edx) >= 0xdc0) {
            reset = true;
        }
    }

    uint8_t last;
    if (reset) {
        car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0xf7u);
        car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0xefu);
        car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0xbfu);
        car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0xdfu);
        car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0x7fu);
        car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0xfdu);
        car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0xfeu);
        car->Flags25F = static_cast<uint8_t>(car->Flags25F & 0xfdu);
        car->Flags25F = static_cast<uint8_t>(car->Flags25F & 0xfeu);
        car->Flags1CE = static_cast<uint8_t>(car->Flags1CE & 0xfbu);
        car->Kind7E = 0;
        last = car->Flags1CE;
    } else {
        car->Flags26B = static_cast<uint8_t>(car->Flags26B | 0x80u);
        if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
            car->Kind7E = 1;
        } else if ((car->Flags230 & 0x80u) != 0u) {
            car->Flags26B = static_cast<uint8_t>(car->Flags26B | 0x20u);
            car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0xbfu);
            car->Kind7E = 5;
        } else {
            car->Flags26B = static_cast<uint8_t>(car->Flags26B | 0x40u);
            car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0xdfu);
            if ((car->Flags26B & 1u) == 0u) {
                car->Kind7E = 2;
                if ((car->Flags7C & 1u) == 0u) {
                    car->Kind7E = 3;
                }
            } else {
                car->Kind7E = 4;
            }
        }

        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x004320b2, call);
        const uint32_t offset = call.eax;
        GP4_GLOBAL(uint32_t, 0x00627868) = offset;
        ecx = offset;
        edx = static_cast<uint32_t>(static_cast<int32_t>(car->Unknown90));
        eax = offset ^ edx;
        bool lane = false;
        if (static_cast<int32_t>(eax) >= 0) {
            ecx = offset - edx;
            if (static_cast<int32_t>(edx) < 0) {
                ecx = 0u - ecx;
            }
            // imul dword [0x627894]; shrd eax, edx, 14 (edx keeps the high half of the product)
            const int64_t product = static_cast<int64_t>(static_cast<int32_t>(ecx)) * GP4_GLOBAL(int32_t, 0x00627894);
            edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
            ecx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
            if (static_cast<int32_t>(ecx) < 0) {
                car->Flags26B = static_cast<uint8_t>(car->Flags26B | 8u);
            } else {
                car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0xf7u);
            }
            eax = static_cast<uint32_t>(static_cast<int32_t>(car->Unknown90));
            if (static_cast<int32_t>(eax) < 0) {
                eax = 0u - eax;
            }
            if (static_cast<int32_t>(eax) >= GP4_GLOBAL(int32_t, 0x00627864)
                && static_cast<int32_t>(ecx) < GP4_GLOBAL(int32_t, 0x0062ac40)) {
                lane = true;
            }
        }
        if (lane) {
            car->Flags26B = static_cast<uint8_t>(car->Flags26B | 0x10u);
        } else {
            car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0xefu);
        }
        last = car->Flags26B;
    }

    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(last);
}
GP4_IMPL(0x004320f2, FUN_004320f2, "regs(esi:ptr[0x273]:layout=car_0043c677+car_00431ffb, edi:ptr[0x118]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00431ffb+state_004059ef+rec_00433c4c+state_004320f2")
