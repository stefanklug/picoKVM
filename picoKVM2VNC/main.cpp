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
#include <getopt.h>
#include "keymapper.h"
#include "argparse.hpp"

#include <string>

using namespace std;

#define CLEAR(x) memset(&(x), 0, sizeof(x))

int serial_fd = 0;

struct buffer {
    void   *start;
    size_t length;
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
  HIDKeyboardStatus keyboardStatus = {0};
  uint8_t mouseButtonMask = 0;
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



static void dokey(rfbBool down,rfbKeySym key,rfbClientPtr cl)
{
    char buf[64];
    ClientData* cd = (ClientData*)cl->clientData;
    HIDKeyboardStatus* ks = &cd->keyboardStatus;
    applyVncKeyEvent(ks, "de", down, key);

    snprintf(buf, 64, "D %d %d %d %d %d %d %d\r", ks->modifiers, ks->keys[0], ks->keys[1], ks->keys[2], ks->keys[3], ks->keys[4], ks->keys[5]);
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
    string                          video_dev;
    string                          hid_emu_dev;
    char                            out_name[256];
    FILE                            *fout;
    struct buffer                   *buffers;
    long                            usec;
    int                             width = 1920;
    int                             height = 1080;

    argparse::ArgumentParser program("picoKVM2VNC", "0.0.1", argparse::default_arguments::help);
    program.add_argument("-v", "--verbose")
           .help("Be more verbose")
           .default_value(false)
           .implicit_value(true)
           .nargs(0);
    program.add_argument("-d", "--video-device")
           .help("Use video device <dev> instead of /dev/video0")
           .default_value(string("/dev/video0"));
    program.add_argument("-e", "--hid-emu-device")
           .help("Use HID emu device <dev> instead of /dev/ttyUSB0")
           .default_value(string("/dev/ttyUSB0"));

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        std::exit(0);
    }

    video_dev = program.get<string>("--video-device");
    hid_emu_dev = program.get<string>("--hid-emu-device");


    fd = v4l2_open(video_dev.c_str(), O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
        fprintf(stderr, "Failed to open video device '%s'\n", video_dev.c_str());
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
    serial_fd = open (hid_emu_dev.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd < 0) {
        printf ("error %d opening %s: %s", errno, hid_emu_dev.c_str(), strerror (errno));
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
