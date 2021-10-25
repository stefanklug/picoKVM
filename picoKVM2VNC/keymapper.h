#include <rfb/rfbproto.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct KeyboardStatus_ {
    uint8_t modifiers;
    uint8_t keys[6];
} HIDKeyboardStatus;

/*
keymap: only "de" supported at the moment
return 0 on success,
-1 if the key was not found
-2 if too many keys are pressed at the same time
*/
int applyVncKeyEvent(HIDKeyboardStatus* status, const char* keymap, rfbBool down,rfbKeySym key);

#ifdef __cplusplus
}
#endif