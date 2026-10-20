#include <gp4/gp4.h>

// The input car record (esi): only the fields this routine reads before esi is replaced.
struct Unknown_00412d5d_Car {
    uint8_t Unknown00[0x71];
    uint8_t Flags71;
    uint8_t Unknown72[0x8d - 0x72];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x9a - 0x8e];
    int16_t Unknown9A;
    uint8_t Unknown9C[0x162 - 0x9c];
    uint8_t Flags162;
};

// mov ebx, 0x639204 / mov esi, 0x638de8 (the working car and the car being built).
static __declspec(noinline) void selectRecords(gp4::Regs& c) {
    c.ebx = 0x00639204u;
    c.esi = 0x00638de8u;
}

static inline uint8_t* bytesAt(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x00412d5d
// One physics step of the working car: esi = the input car. 0x00412bd1 and 0x00412b88 first derive the
// steering value from the car's speed word (+0x9a) times DAT_006038cc >> 14 (looked up in the table at
// 0x00601c48 when bits +0x8d & 2 and +0x162 & 8 select a grid slot, else clamped to
// [DAT_006038c8, DAT_006038c4]; negated when +0x71 & 4), stored in DAT_00601acc. Then the records
// 0x639204 (ebx) and 0x638de8 (esi) go through 0x00412a3d, 0x004129e0, 0x004129fa, 0x00412938,
// 0x0041284a, 0x004035c3, 0x00412c29, 0x00404796, 0x00401315 and 0x00412eeb; the angle difference to the
// working car, corrected by (DAT_00638030 + DAT_00638dd0) >> 16, is clamped to +-0x2000 into
// DAT_00601c60. Returns DAT_006034f0 in eax; esi and edi are saved and restored.
void __cdecl FUN_00412d5d(gp4::Regs* r) {
    const Unknown_00412d5d_Car* car = reinterpret_cast<const Unknown_00412d5d_Car*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00412bd1, c);

    const int64_t product = static_cast<int64_t>(car->Unknown9A) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006038cc));
    c.eax = static_cast<uint32_t>(product >> 14);
    c.edx = static_cast<uint32_t>(product >> 32);

    bool tableHit = false;
    if ((car->Flags8D & 2u) != 0u && (car->Flags162 & 8u) != 0u) {
        int32_t value = static_cast<int32_t>(c.eax) >> 2;
        if (value > 0x2000) {
            value = 0x2000;
        }
        if (value < GP4_GLOBAL(int32_t, 0x00601c10)) {
            value = GP4_GLOBAL(int32_t, 0x00601c10);
        }
        c.eax = static_cast<uint32_t>(value);
        gp4::call_regs(0x00412b88, c);
        if (c.edx != 0xffffffffu) {
            c.eax = GP4_ARRAY(uint32_t, 0x00601c48)[c.edx];
            tableHit = true;
        }
    }
    if (!tableHit) {
        int32_t value = static_cast<int32_t>(c.eax);
        if (value > GP4_GLOBAL(int32_t, 0x006038c4)) {
            value = GP4_GLOBAL(int32_t, 0x006038c4);
        }
        if (value < GP4_GLOBAL(int32_t, 0x006038c8)) {
            value = GP4_GLOBAL(int32_t, 0x006038c8);
        }
        c.eax = static_cast<uint32_t>(value);
    }
    if ((car->Flags71 & 4u) != 0u) {
        c.eax = 0u - c.eax;
    }
    GP4_GLOBAL(uint32_t, 0x00601acc) = c.eax;

    // mov ebx, 0x639204 / mov esi, 0x638de8 / xchg ebx, esi / call / xchg ebx, esi
    selectRecords(c);
    const uint32_t swapTemp = c.ebx;
    c.ebx = c.esi;
    c.esi = swapTemp;
    gp4::call_regs(0x00412a3d, c);
    const uint32_t swapBack = c.ebx;
    c.ebx = c.esi;
    c.esi = swapBack;

    c.edi = GP4_FIELD(uint32_t, bytesAt(c.ebx), 0x1c);
    c.eax = GP4_GLOBAL(uint32_t, 0x00601acc);
    gp4::call_regs(0x004129e0, c);
    gp4::call_regs(0x004129fa, c);
    gp4::call_regs(0x00412938, c);

    if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x00639652)) < 0 && c.edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        GP4_FIELD(uint8_t, bytesAt(c.esi), 0x163) &= 0xbfu;
    }
    if (GP4_GLOBAL(uint8_t, 0x00639650) == 0u) {
        gp4::call_regs(0x0041284a, c);
    }

    GP4_GLOBAL(uint8_t, 0x00601ba6) = 0xff;
    c.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, bytesAt(c.esi), 0x66)));
    gp4::call_regs(0x004035c3, c);
    GP4_FIELD(uint16_t, bytesAt(c.esi), 0x90) = static_cast<uint16_t>(c.eax);
    GP4_GLOBAL(uint8_t, 0x00601ba6) = 0;

    c.edi = GP4_FIELD(uint32_t, bytesAt(c.esi), 0x1c);
    c.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, bytesAt(c.esi), 0x90)));
    c.edx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, bytesAt(c.esi), 0x66)));
    c.ecx = GP4_FIELD(uint32_t, bytesAt(c.esi), 0x1a0);
    gp4::call_regs(0x00412c29, c);
    gp4::call_regs(0x00404796, c);
    GP4_FIELD(uint32_t, bytesAt(c.esi), 0) = c.eax;
    GP4_FIELD(uint32_t, bytesAt(c.esi), 4) = c.edx;
    if ((GP4_FIELD(uint8_t, bytesAt(c.esi), 0x8d) & 2u) != 0u) {
        GP4_GLOBAL(uint8_t, 0x00639650) = 0;
    }

    selectRecords(c);
    c.eax = GP4_FIELD(uint32_t, bytesAt(c.esi), 0) - GP4_FIELD(uint32_t, bytesAt(c.ebx), 0);
    c.edx = GP4_FIELD(uint32_t, bytesAt(c.esi), 4) - GP4_FIELD(uint32_t, bytesAt(c.ebx), 4);
    gp4::call_regs(0x00401315, c);

    int32_t correction = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00638030) + GP4_GLOBAL(uint32_t, 0x00638dd0)) >> 16;
    if ((GP4_FIELD(uint8_t, bytesAt(c.esi), 0x71) & 4u) != 0u) {
        correction = -correction;
    }
    c.edx = static_cast<uint32_t>(correction);
    int32_t angle = static_cast<int16_t>(c.eax - c.edx);
    if (angle < 0) {
        uint32_t magnitude = 0u - static_cast<uint32_t>(angle);
        if (magnitude >= 0x2000u) {
            magnitude = 0x2000u;
        }
        angle = static_cast<int32_t>(0u - magnitude);
    } else if (static_cast<uint32_t>(angle) >= 0x2000u) {
        angle = 0x2000;
    }
    c.eax = static_cast<uint32_t>(angle);
    GP4_GLOBAL(uint16_t, 0x00601c60) = static_cast<uint16_t>(angle);
    gp4::call_regs(0x00412eeb, c);

    r->eax = GP4_GLOBAL(uint32_t, 0x006034f0);
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00412d5d, FUN_00412d5d, "regs(eax:u32, edx:u32, esi:ptr[0x420]:layout=car_00438aac+rec_00412b88) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+widths_00852c44+state_004127b3+work_00639204+fixed_0041284a+ranges_004322a6+ring_00405a12+links_00405a12+sign_00639652+state_00412b88+state_00431ffb+state_00412c29")
