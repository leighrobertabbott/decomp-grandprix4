#include <gp4/gp4.h>

typedef void (__stdcall *OutputDebugStringA_t)(const char* text);

// Debug trace of a core car event (string "CAI_DEBUG_OutputEventType error: Unexpected event type."):
// prints "EVENT (Time: %d): Car Index %d  -  " with the time 0xa96a84 and the car index, then the
// event's name; event 4 adds the car's event mask (0xa95dc8 + car*0x38), event 5 its refuel time
// (0xa95dbc + car*0x38). Event 9 prints nothing more, unknown events print the error line.
// FUNCTION: GP4 0x004e61d0
void __cdecl FUN_004e61d0(int car, int eventType) {
    char text[256];
    OutputDebugStringA_t output = GP4_IAT(OutputDebugStringA_t, 0x005f0290);
    gp4::cdecl_<0x005db088, int>(text, GP4_ARRAY(const char, 0x006640fc), GP4_GLOBAL(int, 0x00a96a84), car);
    output(text);
    switch (eventType) {
    case 0:
        output(GP4_ARRAY(const char, 0x006640c8));
        return;
    case 1:
        output(GP4_ARRAY(const char, 0x006640a0));
        return;
    case 2:
        output(GP4_ARRAY(const char, 0x00664068));
        return;
    case 3:
        output(GP4_ARRAY(const char, 0x00664030));
        return;
    case 4:
        output(GP4_ARRAY(const char, 0x00664004));
        gp4::cdecl_<0x005db088, int>(text, GP4_ARRAY(const char, 0x00663fec), GP4_ARRAY(int, 0x00a95dc8)[car * 14]);
        output(text);
        return;
    case 5:
        output(GP4_ARRAY(const char, 0x00663fc0));
        gp4::cdecl_<0x005db088, int>(text, GP4_ARRAY(const char, 0x00663fac), GP4_ARRAY(int, 0x00a95dbc)[car * 14]);
        output(text);
        return;
    case 6:
        output(GP4_ARRAY(const char, 0x00663f80));
        return;
    case 7:
        output(GP4_ARRAY(const char, 0x00663f58));
        return;
    case 8:
        output(GP4_ARRAY(const char, 0x00663f2c));
        return;
    case 9:
        return;
    case 10:
        output(GP4_ARRAY(const char, 0x00663efc));
        return;
    case 11:
        output(GP4_ARRAY(const char, 0x00663ed0));
        return;
    case 12:
        output(GP4_ARRAY(const char, 0x00663ea4));
        return;
    case 13:
        output(GP4_ARRAY(const char, 0x00663e7c));
        return;
    case 14:
        output(GP4_ARRAY(const char, 0x00663e50));
        return;
    case 15:
        output(GP4_ARRAY(const char, 0x00663e28));
        return;
    default:
        output(GP4_ARRAY(const char, 0x00663dec));
        return;
    }
}
GP4_IMPL(0x004e61d0, FUN_004e61d0, "cdecl(i32[0..21], i32[-2..17]) -> void; gfields={0x00a96a84:i32, 0x00a95dbc*22/0x38:i32, 0x00a95dc8*22/0x38:i32}")
