#include <cstdio>
#include <cstring>
#include <rfb/keysym.h>
#include <set>


#include "keymaps.h"
#include "keymapper.h"


std::map<uint32_t, uint8_t> createKeyMap() {
    std::map<uint32_t, uint8_t> result;
    for (const auto &v : vncScanToHIDList_de) {
        for(int i=0; i<v.size()-1; i++) {
            result[v[i]] = v.back();
        }
    }
    return result;
}

static std::map<uint32_t, uint8_t> vncScanToHIDMap = createKeyMap();


static void updateBits(uint8_t* data, uint8_t mask, int8_t enable) {
    if(enable) {
        *data = *data | mask;
    } else {
        *data = *data & ~mask;
    }
}

/*
keymap: only "de" supported at the moment
return 0 on success,
-1 if the key was not found
*/
int applyVncKeyEvent(HIDKeyboardStatus* status, const char* keymap, rfbBool down, rfbKeySym key) {
    if(strcmp("de", keymap) != 0) {
        fprintf( stderr, "Only keymap \"de\" is supported at the moment");
        return -2;
    }

    //some sanity changes
    if(key == XK_ISO_Level3_Shift) {
        key = XK_Alt_R;
    }

    switch(key) {
        case XK_Control_L: updateBits(&status->modifiers, KEY_MOD_LCTRL, down); break;
        case XK_Shift_L:   updateBits(&status->modifiers, KEY_MOD_LSHIFT, down); break;
        case XK_Alt_L:     updateBits(&status->modifiers, KEY_MOD_LALT, down); break;
        case XK_Meta_L:    updateBits(&status->modifiers, KEY_MOD_LMETA, down); break;
        case XK_Control_R: updateBits(&status->modifiers, KEY_MOD_RCTRL, down); break;
        case XK_Shift_R:   updateBits(&status->modifiers, KEY_MOD_RSHIFT, down); break;
        case XK_Alt_R:     updateBits(&status->modifiers, KEY_MOD_RALT, down); break;
        case XK_Meta_R:    updateBits(&status->modifiers, KEY_MOD_RMETA, down); break;
    }
    
    //find the usb key
    int usbKey = 0;
    std::set<uint8_t> keyset(std::begin(status->keys), std::end(status->keys));


    if(key != XK_Control_L &&
       key != XK_Shift_L && 
       key != XK_Alt_L && 
       key != XK_Meta_L && 
       key != XK_Control_R && 
       key != XK_Shift_R && 
       key != XK_Alt_R && 
       key != XK_Meta_R
    ) {
        uint32_t mods = 0;
        if(status->modifiers & KEY_MOD_LALT || status->modifiers & KEY_MOD_RALT) {
            mods |= KM_ALTGR;
        }

        if(status->modifiers & KEY_MOD_LSHIFT || status->modifiers & KEY_MOD_RSHIFT) {
            mods |= KM_SHIFT;
        }

        auto res = vncScanToHIDMap.find(key | mods);
        if(res != vncScanToHIDMap.end()) {
            usbKey = res->second;
        } else {
            //fallback without modifiers
            res = vncScanToHIDMap.find(key);
            if(res != vncScanToHIDMap.end()) {
                usbKey = res->second;
            }
        }

        if(usbKey != 0) {
            if(down) {
                keyset.insert(usbKey);
            } else {
                keyset.erase(usbKey);
            }
        } else {
            fprintf(stderr, "Failed to deduce Usb Key for vnc key code %x\n", key);
            return -1;
        }
    }

    if(keyset.size() > 6) {
        memset(status->keys, KEY_ERR_OVF, 6);
        return -2;
    } else {
        int i=0;
        memset(status->keys, KEY_NONE, 6);
        for (const auto &v : keyset) {
            status->keys[i++] = v;
        }
    }

    return 0;
}