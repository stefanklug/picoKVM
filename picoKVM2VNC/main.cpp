/* V4L2 video picture grabber
    Copyright (C) 2009 Mauro Carvalho Chehab <mchehab@infradead.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <rfb/rfb.h>
#include <rfb/keysym.h>
#include "usb_hid_keys.h"

#include <set>
#include <map>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

int serial_fd = 0;

struct buffer {
    void   *start;
    size_t length;
};

std::map<uint32_t, int> vncScanToHID = {
    {XK_Escape, KEY_ESC},

    {XK_dead_circumflex, KEY_GRAVE},
    {XK_1, KEY_1},
    {XK_2, KEY_2},
    {XK_3, KEY_3},
    {XK_4, KEY_4},
    {XK_5, KEY_5},
    {XK_6, KEY_6},
    {XK_7, KEY_7},
    {XK_8, KEY_8},
    {XK_9, KEY_9},
    {XK_0, KEY_0},
    {XK_ssharp, KEY_MINUS},
    {XK_dead_acute, KEY_EQUAL},


    {XK_BackSpace, KEY_BACKSPACE},

    {XK_Tab, KEY_TAB},
    {XK_q, KEY_Q},
    {XK_w, KEY_W},
    {XK_e, KEY_E},
    {XK_r, KEY_R},
    {XK_t, KEY_T},
    {XK_z, KEY_Y},
    {XK_u, KEY_U},
    {XK_i, KEY_I},
    {XK_o, KEY_O},
    {XK_p, KEY_P},
    {XK_udiaeresis, KEY_LEFTBRACE},
    {XK_plus, KEY_RIGHTBRACE},
    
    {XK_Return, KEY_ENTER},

    {XK_a, KEY_A},
    {XK_s, KEY_S},
    {XK_d, KEY_D},
    {XK_f, KEY_F},
    {XK_g, KEY_G},
    {XK_h, KEY_H},
    {XK_j, KEY_J},
    {XK_k, KEY_K},
    {XK_l, KEY_L},
    {XK_odiaeresis, KEY_SEMICOLON},
    {XK_adiaeresis, KEY_APOSTROPHE},
    {XK_numbersign, KEY_BACKSLASH},

    {XK_less, KEY_102ND},
    {XK_y, KEY_Z},
    {XK_x, KEY_X},
    {XK_c, KEY_C},
    {XK_v, KEY_V},
    {XK_b, KEY_B},
    {XK_n, KEY_N},
    {XK_m, KEY_M},
    {XK_comma, KEY_COMMA},
    {XK_period, KEY_DOT},
    {XK_minus, KEY_SLASH},

    {XK_KP_Multiply, KEY_KPASTERISK},

    {XK_Caps_Lock, KEY_CAPSLOCK},
    {XK_F1, KEY_F1},
    {XK_F2, KEY_F2},
    {XK_F3, KEY_F3},
    {XK_F4, KEY_F4},
    {XK_F5, KEY_F5},
    {XK_F6, KEY_F6},
    {XK_F7, KEY_F7},
    {XK_F8, KEY_F8},
    {XK_F9, KEY_F9},
    {XK_F10, KEY_F10},
    {XK_F11, KEY_F11},
    {XK_F12, KEY_F12},

    {XK_Num_Lock, KEY_NUMLOCK},
    {XK_Scroll_Lock, KEY_SCROLLLOCK},
    {XK_KP_7, KEY_KP7},

    {XK_KP_8, KEY_KP8},
    {XK_KP_9, KEY_KP9},
    {XK_KP_Subtract, KEY_KPMINUS},
    {XK_KP_4, KEY_KP4},
    {XK_KP_5, KEY_KP5},
    {XK_KP_6, KEY_KP6},
    {XK_KP_Add, KEY_KPPLUS},
    {XK_KP_1, KEY_KP1},
    {XK_KP_2, KEY_KP2},
    {XK_KP_3, KEY_KP3},
    {XK_KP_0, KEY_KP0},
    {XK_KP_Separator, KEY_KPDOT},

    {XK_KP_Enter, KEY_KPENTER},

    {XK_KP_Divide, KEY_KPSLASH},
    {XK_Sys_Req, KEY_SYSRQ},

    {XK_Home, KEY_HOME},
    {XK_Up, KEY_UP},
    {XK_Page_Up, KEY_PAGEUP},
    {XK_Left, KEY_LEFT},
    {XK_Right, KEY_RIGHT},
    {XK_End, KEY_END},
    {XK_Down, KEY_DOWN},
    {XK_Page_Down, KEY_PAGEDOWN},
    {XK_Insert, KEY_INSERT},
    {XK_Delete, KEY_DELETE},
    {XK_Pause, KEY_PAUSE},

    {XK_space, KEY_SPACE}
};

std::map<uint32_t, int> vncShiftScanToHID = {
    {XK_Escape, KEY_ESC},

    {XK_exclam, KEY_1},
    {XK_quotedbl, KEY_2},
    {XK_section, KEY_3},
    {XK_dollar, KEY_4},
    {XK_percent, KEY_5},
    {XK_ampersand, KEY_6},
    {XK_slash, KEY_7},
    {XK_parenleft, KEY_8},
    {XK_parenright, KEY_9},
    {XK_equal, KEY_0},
    {XK_question, KEY_MINUS},
    {XK_dead_grave, KEY_EQUAL},


    {XK_BackSpace, KEY_BACKSPACE},

    {XK_ISO_Left_Tab, KEY_TAB},
    {XK_Q, KEY_Q},
    {XK_W, KEY_W},
    {XK_E, KEY_E},
    {XK_R, KEY_R},
    {XK_T, KEY_T},
    {XK_Z, KEY_Y},
    {XK_U, KEY_U},
    {XK_I, KEY_I},
    {XK_O, KEY_O},
    {XK_P, KEY_P},
    {XK_Udiaeresis, KEY_LEFTBRACE},
    {XK_asterisk, KEY_RIGHTBRACE},
    
    {XK_Return, KEY_ENTER},

    {XK_A, KEY_A},
    {XK_S, KEY_S},
    {XK_D, KEY_D},
    {XK_F, KEY_F},
    {XK_G, KEY_G},
    {XK_H, KEY_H},
    {XK_J, KEY_J},
    {XK_K, KEY_K},
    {XK_L, KEY_L},
    {XK_Odiaeresis, KEY_SEMICOLON},
    {XK_Adiaeresis, KEY_APOSTROPHE},
    {XK_apostrophe, KEY_BACKSLASH},

    {XK_greater, KEY_102ND},
    {XK_Y, KEY_Z},
    {XK_X, KEY_X},
    {XK_C, KEY_C},
    {XK_V, KEY_V},
    {XK_B, KEY_B},
    {XK_N, KEY_N},
    {XK_M, KEY_M},
    {XK_semicolon, KEY_COMMA},
    {XK_colon, KEY_DOT},
    {XK_underscore, KEY_SLASH},

    {XK_KP_Multiply, KEY_KPASTERISK},


    {XK_Num_Lock, KEY_NUMLOCK},
    {XK_Scroll_Lock, KEY_SCROLLLOCK},
    {XK_KP_7, KEY_KP7},

    {XK_KP_8, KEY_KP8},
    {XK_KP_9, KEY_KP9},
    {XK_KP_Subtract, KEY_KPMINUS},
    {XK_KP_4, KEY_KP4},
    {XK_KP_5, KEY_KP5},
    {XK_KP_6, KEY_KP6},
    {XK_KP_Add, KEY_KPPLUS},
    {XK_KP_1, KEY_KP1},
    {XK_KP_2, KEY_KP2},
    {XK_KP_3, KEY_KP3},
    {XK_KP_0, KEY_KP0},
    {XK_KP_Separator, KEY_KPDOT},

    {XK_KP_Enter, KEY_KPENTER},

    {XK_KP_Divide, KEY_KPSLASH},
    {XK_Sys_Req, KEY_SYSRQ},

    {XK_Home, KEY_HOME},
    {XK_Up, KEY_UP},
    {XK_Page_Up, KEY_PAGEUP},
    {XK_Left, KEY_LEFT},
    {XK_Right, KEY_RIGHT},
    {XK_End, KEY_END},
    {XK_Down, KEY_DOWN},
    {XK_Page_Down, KEY_PAGEDOWN},
    {XK_Insert, KEY_INSERT},
    {XK_Delete, KEY_DELETE},
    {XK_Pause, KEY_PAUSE},

    {XK_space, KEY_SPACE}
};

std::map<uint32_t, int> vncAltGrScanToHID = {
    {XK_Escape, KEY_ESC},

    {XK_onesuperior, KEY_1},
    {XK_quotedbl, KEY_2},
    {XK_section, KEY_3},
    {XK_dollar, KEY_4},
    {XK_percent, KEY_5},
    {XK_ampersand, KEY_6},
    {XK_braceleft, KEY_7},
    {XK_bracketleft, KEY_8},
    {XK_bracketright, KEY_9},
    {XK_braceright, KEY_0},
    {XK_backslash, KEY_MINUS},
    {XK_dead_grave, KEY_EQUAL},


    {XK_BackSpace, KEY_BACKSPACE},

    {XK_ISO_Left_Tab, KEY_TAB},
    {XK_at, KEY_Q},
    {XK_W, KEY_W},
    {XK_E, KEY_E},
    {XK_R, KEY_R},
    {XK_T, KEY_T},
    {XK_Z, KEY_Y},
    {XK_U, KEY_U},
    {XK_I, KEY_I},
    {XK_O, KEY_O},
    {XK_P, KEY_P},
    {XK_Udiaeresis, KEY_LEFTBRACE},
    {XK_asterisk, KEY_RIGHTBRACE},
    
    {XK_Return, KEY_ENTER},

    {XK_A, KEY_A},
    {XK_S, KEY_S},
    {XK_D, KEY_D},
    {XK_F, KEY_F},
    {XK_G, KEY_G},
    {XK_H, KEY_H},
    {XK_J, KEY_J},
    {XK_K, KEY_K},
    {XK_lstroke, KEY_L},
    {XK_Odiaeresis, KEY_SEMICOLON},
    {XK_Adiaeresis, KEY_APOSTROPHE},
    {XK_apostrophe, KEY_BACKSLASH},

    {XK_bar, KEY_102ND},
    {XK_Y, KEY_Z},
    {XK_X, KEY_X},
    {XK_C, KEY_C},
    {XK_V, KEY_V},
    {XK_B, KEY_B},
    {XK_N, KEY_N},
    {XK_M, KEY_M},
    {XK_semicolon, KEY_COMMA},
    {XK_colon, KEY_DOT},
    {XK_underscore, KEY_SLASH},

    {XK_KP_Multiply, KEY_KPASTERISK},


    {XK_Num_Lock, KEY_NUMLOCK},
    {XK_Scroll_Lock, KEY_SCROLLLOCK},
    {XK_KP_7, KEY_KP7},

    {XK_KP_8, KEY_KP8},
    {XK_KP_9, KEY_KP9},
    {XK_KP_Subtract, KEY_KPMINUS},
    {XK_KP_4, KEY_KP4},
    {XK_KP_5, KEY_KP5},
    {XK_KP_6, KEY_KP6},
    {XK_KP_Add, KEY_KPPLUS},
    {XK_KP_1, KEY_KP1},
    {XK_KP_2, KEY_KP2},
    {XK_KP_3, KEY_KP3},
    {XK_KP_0, KEY_KP0},
    {XK_KP_Separator, KEY_KPDOT},

    {XK_KP_Enter, KEY_KPENTER},

    {XK_KP_Divide, KEY_KPSLASH},
    {XK_Sys_Req, KEY_SYSRQ},

    {XK_Home, KEY_HOME},
    {XK_Up, KEY_UP},
    {XK_Page_Up, KEY_PAGEUP},
    {XK_Left, KEY_LEFT},
    {XK_Right, KEY_RIGHT},
    {XK_End, KEY_END},
    {XK_Down, KEY_DOWN},
    {XK_Page_Down, KEY_PAGEDOWN},
    {XK_Insert, KEY_INSERT},
    {XK_Delete, KEY_DELETE},
    {XK_Pause, KEY_PAUSE},

    {XK_space, KEY_SPACE}
};

static void xioctl(int fh, int request, void *arg)
{
    int r;

    do {
        r = v4l2_ioctl(fh, request, arg);
    } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

    if (r == -1) {
        fprintf(stderr, "error %d, %s\\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int
set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 0;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf ("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void
set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                printf ("error %d setting term attributes", errno);

        fcntl(fd, F_SETFL, should_block ? 0 : FNDELAY);
}

class ClientData {
public:
  uint8_t hid_modifiers = 0;
  std::set<uint8_t> pressed_keys;
  uint8_t mouseButtonMask;
  uint16_t mouseX;
  uint16_t mouseY;
  int8_t mouseWheel;
  bool mouseChanged;
};

ClientData* gcl = NULL; //uhuoh


static void clientgone(rfbClientPtr cl)
{
  free(cl->clientData);
  gcl = NULL;
  cl->clientData = NULL;
}

static enum rfbNewClientAction newclient(rfbClientPtr cl)
{
  cl->clientData = new ClientData;
  gcl = (ClientData*)cl->clientData;
  cl->clientGoneHook = clientgone;
  return RFB_CLIENT_ACCEPT;
}

/* Here the pointer events are handled */

static void sendString(int fd, const char* str) {
    char buf[64];
    int l;
    write (fd, str, strlen(str));
    printf("%s\n", str);
}

static void sendMouseEventIfNeeded(ClientData* cd) {
    char buf[64];
    if(!cd) return;
    if(!cd->mouseChanged) return;
    snprintf(buf, 64, "M %d %d %d %d\r", cd->mouseButtonMask, cd->mouseX, cd->mouseY, cd->mouseWheel);
    sendString(serial_fd, buf); 
    cd->mouseChanged = false;
    //printf("Mouse mask: %hd x:%d y:%d Msg: %s\n", buttonMask, x, y, buf);
}

static void doptr(int buttonMask,int x,int y,rfbClientPtr cl)
{
    char buf[64];
    ClientData* cd = (ClientData*)cl->clientData;

    int w = 1920;
    int h = 1080;
    int8_t wheel = 0;
    uint8_t mask = buttonMask & 0x7;

    int ox = (int)((x/(float)w)*32767.0);
    int oy = (int)((y/(float)h)*32767.0);

    if(buttonMask & 0x8) wheel = 127;
    else if(buttonMask & 0x10) wheel = -127;

    uint8_t oldMask = cd->mouseButtonMask;

    cd->mouseButtonMask = mask;
    cd->mouseX = ox;
    cd->mouseY = oy;
    cd->mouseWheel = wheel;

    //decide if we need to send or can defer
    cd->mouseChanged = true;
    if(oldMask != mask || wheel != 0) {
        sendMouseEventIfNeeded(cd);
    }
    sendMouseEventIfNeeded(cd);
}

static void updateBits(uint8_t* data, uint8_t mask, int8_t enable) {
    if(enable) {
        *data = *data | mask;
    } else {
        *data = *data & ~mask;
    }
}

static void dokey(rfbBool down,rfbKeySym key,rfbClientPtr cl)
{
    ClientData* cd = (ClientData*)cl->clientData;

    switch(key) {
        case XK_Control_L: updateBits(&cd->hid_modifiers, KEY_MOD_LCTRL, down); break;
        case XK_Shift_L:   updateBits(&cd->hid_modifiers, KEY_MOD_LSHIFT, down); break;
        case XK_Alt_L:     updateBits(&cd->hid_modifiers, KEY_MOD_LALT, down); break;
        case XK_Meta_L:    updateBits(&cd->hid_modifiers, KEY_MOD_LMETA, down); break;
        case XK_Control_R: updateBits(&cd->hid_modifiers, KEY_MOD_RCTRL, down); break;
        case XK_Shift_R:   updateBits(&cd->hid_modifiers, KEY_MOD_RSHIFT, down); break;
        case XK_Alt_R:     updateBits(&cd->hid_modifiers, KEY_MOD_RALT, down); break;
        case XK_Meta_R:    updateBits(&cd->hid_modifiers, KEY_MOD_RMETA, down); break;

        case XK_ISO_Level3_Shift:    updateBits(&cd->hid_modifiers, KEY_MOD_RALT, down); break;
    }
    
    //find the usb key
    int usbKey = 0;


    if(key != XK_Control_L &&
       key != XK_Shift_L && 
       key != XK_Alt_L && 
       key != XK_Meta_L && 
       key != XK_Control_R && 
       key != XK_Shift_R && 
       key != XK_Alt_R && 
       key != XK_Meta_R &&
       key != XK_ISO_Level3_Shift
    ) {
        if(cd->hid_modifiers & KEY_MOD_RALT || cd->hid_modifiers & KEY_MOD_RALT) {
            auto res = vncAltGrScanToHID.find(key);
            if(res != vncAltGrScanToHID.end()) {
                usbKey = res->second;
            }
        }
        
        if(cd->hid_modifiers & KEY_MOD_LSHIFT || cd->hid_modifiers & KEY_MOD_RSHIFT) {
            auto res = vncShiftScanToHID.find(key);
            if(res != vncShiftScanToHID.end()) {
                usbKey = res->second;
            }
        }

        if(usbKey == 0) {
            auto res = vncScanToHID.find(key);
            if(res != vncScanToHID.end()) {
                usbKey = res->second;
            }
        }

        if(usbKey != 0) {
            if(down) {
                cd->pressed_keys.insert(usbKey);
            } else {
                cd->pressed_keys.erase(usbKey);
            }
        } else {
            printf("Failed to deduce Usb Key for vnc key code %x\n", key);
        }
    }

    uint8_t keys[6];
    if(cd->pressed_keys.size() > 6) {
        memset(keys, KEY_ERR_OVF, 6);
    } else {
        int i=0;
        memset(keys, KEY_NONE, 6);
        for (const auto &v : cd->pressed_keys) {
            keys[i++] = v;
        }
    }

    char buf[64];

    snprintf(buf, 64, "D %d %d %d %d %d %d %d\r", (cd->hid_modifiers) & 0xff, keys[0], keys[1], keys[2], keys[3], keys[4], keys[5]);
    sendString(serial_fd, buf); 

    //printf("%x %d  %s", key, down, buf);
}

static void readSerial(int fd) {
    char buf[64];
    int l;
    while((l = read(fd, buf, 63)) > 0) {
        buf[l+1] = 0;
        printf("%s", buf);
    }
}


int main(int argc, char **argv)
{
    struct v4l2_format              fmt;
    struct v4l2_buffer              buf;
    struct v4l2_requestbuffers      req;
    enum v4l2_buf_type              type;
    fd_set                          fds;
    struct timeval                  tv;
    int                             r, fd = -1;
    unsigned int                    i, n_buffers;
    const char*                     dev_name = "/dev/video0";
    char                            out_name[256];
    FILE                            *fout;
    struct buffer                   *buffers;
    long                            usec;
    int                             width = 1920;
    int                             height = 1080;

    fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
        perror("Cannot open device");
        exit(EXIT_FAILURE);
    }

    
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = width;
    fmt.fmt.pix.height      = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    xioctl(fd, VIDIOC_S_FMT, &fmt);
    if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {
        printf("Libv4l didn't accept RGB24 format. Can't proceed.\\n");
        exit(EXIT_FAILURE);
    }
    if ((fmt.fmt.pix.width != width) || (fmt.fmt.pix.height != height)) {
        printf("Warning: The requested size is not available. Size was adjusted to: %dx%d\\n",
            fmt.fmt.pix.width, fmt.fmt.pix.height);
        width = fmt.fmt.pix.width;
        height = fmt.fmt.pix.height;
    }


    /* init VNC Server */
    int bpp = 3; /* tis is considered slow by libvnc but is guaranteed by libv4lconvert */
    rfbScreenInfoPtr rfbScreen = rfbGetScreen(&argc,argv,width ,height ,8,3,bpp);
    if(!rfbScreen)
        return 0;
    rfbScreen->desktopName = "picoKVM";
    rfbScreen->frameBuffer = (char*)malloc(width*height*bpp);
    rfbScreen->alwaysShared = TRUE;
    rfbScreen->ptrAddEvent = doptr;
    rfbScreen->kbdAddEvent = dokey;
    rfbScreen->newClientHook = newclient;
    rfbScreen->httpEnableProxyConnect = TRUE;

    rfbInitServer(rfbScreen);

    /* init serial port */
    const char *portname = "/dev/ttyUSB0";
    serial_fd = open (portname, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd < 0) {
        printf ("error %d opening %s: %s", errno, portname, strerror (errno));
        exit(EXIT_FAILURE);
    }

    fcntl(serial_fd, F_SETFL, FNDELAY);


    set_interface_attribs (serial_fd, B57600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    //set_blocking (serial_fd, 0);                // set no blocking

    CLEAR(req);
    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    xioctl(fd, VIDIOC_REQBUFS, &req);

    buffers = (buffer*)calloc(req.count, sizeof(*buffers));
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        CLEAR(buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;

        xioctl(fd, VIDIOC_QUERYBUF, &buf);

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
                PROT_READ | PROT_WRITE, MAP_SHARED,
                fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < n_buffers; ++i) {
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        xioctl(fd, VIDIOC_QBUF, &buf);
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    xioctl(fd, VIDIOC_STREAMON, &type);

    while (rfbIsActive(rfbScreen)) {
        int r;
        sendMouseEventIfNeeded(gcl);
        readSerial(serial_fd);

        

        //simply try to deque a buffer
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        do {
            r = v4l2_ioctl(fd, VIDIOC_DQBUF, &buf);
        } while (r == -1 && (errno == EINTR));

        if(r == 0) {
            memcpy(rfbScreen->frameBuffer, buffers[buf.index].start, width*height*bpp);
            rfbMarkRectAsModified(rfbScreen,0,0,width,height);
            xioctl(fd, VIDIOC_QBUF, &buf);
        }
          
        usec = rfbScreen->deferUpdateTime*1000;
        rfbProcessEvents(rfbScreen,usec);
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_STREAMOFF, &type);
    for (i = 0; i < n_buffers; ++i)
        v4l2_munmap(buffers[i].start, buffers[i].length);
    v4l2_close(fd);

    return 0;
}
