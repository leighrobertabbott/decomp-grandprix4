#include <gp4/gp4.h>

struct Unknown_00409793 {
    uint8_t Unknown00[0x278];
    uint16_t Unknown278;
    uint16_t Unknown27A;
    uint16_t Unknown27C;
    uint16_t Unknown27E;
    uint16_t Unknown280;
    uint16_t Unknown282;
};

// FUNCTION: GP4 0x00409793
void __cdecl FUN_00409793(gp4::Regs* r) {
    Unknown_00409793* body = reinterpret_cast<Unknown_00409793*>(r->esi);
    body->Unknown280 = 0;
    body->Unknown27E = 0;
    body->Unknown27A = 0;
    body->Unknown278 = 0;
    body->Unknown282 = 0;
    body->Unknown27C = 0;
    r->eax = 0;
}
GP4_IMPL(0x00409793, FUN_00409793, "regs(esi:ptr[0x28a]:bytes) -> (eax)")
