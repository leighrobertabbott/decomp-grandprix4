#include <gp4/gp4.h>

struct Unknown_00405efa_Car {
    uint8_t Unknown00[0x1c];
    uint32_t Segment1C;
    uint8_t Unknown20[0x8d - 0x20];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x90 - 0x8e];
    int16_t Word90;
    uint8_t Unknown92[0x9c - 0x92];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xb0 - 0x9d];
    int16_t WordB0;
    uint8_t UnknownB2[0xe4 - 0xb2];
    uint8_t StateE4;
    uint8_t UnknownE5[0x11c - 0xe5];
    int16_t Word11C;
    int16_t Word11E;
};

struct Unknown_00405efa_Segment {
    uint8_t Unknown00[0x60];
    int16_t Word60;
    int16_t Word62;
};

// FUNCTION: GP4 0x00405efa
// Entered with the car in esi and its new track segment in edi. When bit 1 of byte +0x8d is clear
// it returns at once (flags of that `test`: zero result). Otherwise (pushal ... popal, so only the
// flags of the last callee survive) it clears the state byte +0xe4 and the bits (+0x8d & 2, +0x9c &
// 0x10), adds the offset from 0x00405e45 to the word +0x90, stores the segment's words +0x60/+0x62
// minus 0x100 into +0x11c/+0x11e, picks one of them (+0x11c when +0x90 is negative) for +0xb0, links
// the car to the segment (+0x1c = edi) and runs 0x004322a6, 0x00409416, 0x004036ca and 0x00405f77.
void __cdecl FUN_00405efa(gp4::Regs* r) {
    Unknown_00405efa_Car* car = reinterpret_cast<Unknown_00405efa_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00405efa_Segment* seg = reinterpret_cast<const Unknown_00405efa_Segment*>(static_cast<uintptr_t>(r->edi));

    if ((car->Flags8D & 2u) == 0u) {
        // test byte [esi+0x8d], 2 with a zero result: ZF = PF = 1, the rest clear
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    car->StateE4 = 0;
    car->Flags8D = static_cast<uint8_t>(car->Flags8D & 0xfdu);
    car->Flags9C = static_cast<uint8_t>(car->Flags9C & 0xefu);

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00405e45, call);
    car->Word90 = static_cast<int16_t>(static_cast<uint16_t>(car->Word90) + static_cast<uint16_t>(call.eax));

    car->Word11C = static_cast<int16_t>(static_cast<uint16_t>(seg->Word60) - 0x100u);
    int16_t lane = static_cast<int16_t>(static_cast<uint16_t>(seg->Word62) - 0x100u);
    car->Word11E = lane;
    if (car->Word90 < 0) {
        lane = car->Word11C;
    }
    car->WordB0 = lane;
    call.eax = (call.eax & 0xffff0000u) | static_cast<uint16_t>(lane);
    car->Segment1C = r->edi;

    gp4::call_regs(0x004322a6, call);
    gp4::call_regs(0x00409416, call);
    gp4::call_regs(0x004036ca, call);
    gp4::call_regs(0x00405f77, call);

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00405efa, FUN_00405efa, "regs(esi:ptr[0x420]:bytes, edi:rec[0x8943a0,0x160,0..25]) -> (cf, pf, af, zf, sf, of); globals=layout=state_00403800+ranges_004322a6+ring_00405a12+links_00405a12+state_004059ef+div_00403bb6")
