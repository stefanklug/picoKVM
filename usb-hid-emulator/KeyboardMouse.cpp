
#include "KeyboardMouse.h"
#include "HID.h"



static const uint8_t _hidSingleReportDescriptorKeyboardMouse[] PROGMEM = {
	0x05, 0x01,			/* USAGE_PAGE (Generic Desktop)						*/
	0x09, 0x06,			/* USAGE (Keyboard)									*/
	0xa1, 0x01,			/* COLLECTION (Application)							*/
	0x85, 0x01,         //REPORT_ID(1)
	0x75, 0x01,			/* REPORT_SIZE (1)									*/
	0x95, 0x08,			/* REPORT_COUNT (8)									*/
	0x05, 0x07,			/* USAGE_PAGE (Keyboard)(Key Codes)					*/
	0x19, 0xe0,			/* USAGE_MINIMUM (Keyboard LeftControl)(224)		*/
	0x29, 0xe7,			/* USAGE_MAXIMUM (Keyboard Right GUI)(231)			*/
	0x15, 0x00,			/* LOGICAL_MINIMUM (0)								*/
	0x25, 0x01,			/* LOGICAL_MAXIMUM (1)								*/
	0x81, 0x02,			/* INPUT (Data,Var,Abs) ; Modifier byte				*/
//	0x95, 0x01,			/* REPORT_COUNT (1)									*/
//	0x75, 0x08,			/* REPORT_SIZE (8)									*/
//	0x81, 0x03,			/* INPUT (Cnst,Var,Abs) ; Reserved byte				*/
	0x95, 0x05,			/* REPORT_COUNT (5)									*/
	0x75, 0x01,			/* REPORT_SIZE (1)									*/
	0x05, 0x08,			/* USAGE_PAGE (LEDs)								*/
	0x19, 0x01,			/* USAGE_MINIMUM (Num Lock)							*/
	0x29, 0x05,			/* USAGE_MAXIMUM (Kana)								*/
	0x91, 0x02,			/* OUTPUT (Data,Var,Abs) ; LED report				*/
	0x95, 0x01,			/* REPORT_COUNT (1)									*/
	0x75, 0x03,			/* REPORT_SIZE (3)									*/
	0x91, 0x03,			/* OUTPUT (Cnst,Var,Abs) ; LED report padding		*/
	0x95, 0x06,			/* REPORT_COUNT (6)									*/
	0x75, 0x08,			/* REPORT_SIZE (8)									*/
	0x15, 0x00,			/* LOGICAL_MINIMUM (0)								*/
	0x25, 0x65,			/* LOGICAL_MAXIMUM (101)							*/
	0x05, 0x07,			/* USAGE_PAGE (Keyboard)(Key Codes)					*/
	0x19, 0x00,			/* USAGE_MINIMUM (Reserved (no event indicated))(0)	*/
	0x29, 0x65,			/* USAGE_MAXIMUM (Keyboard Application)(101)		*/
	0x81, 0x00,			/* INPUT (Data,Ary,Abs)								*/
	0xc0,				/*  END_COLLECTION									*/
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x02,                    // USAGE (Mouse)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x09, 0x01,                    //   USAGE (Pointer)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x85, 0x02,                    //REPORT_ID(2)
	0x05, 0x09,                    //     USAGE_PAGE (Button)
	0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
	0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
	0x95, 0x03,                    //     REPORT_COUNT (3)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x75, 0x05,                    //     REPORT_SIZE (5)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //     USAGE (X)
	0x09, 0x31,                    //     USAGE (Y)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x7f,            //     LOGICAL_MAXIMUM (32767)
	0x75, 0x10,                    //     REPORT_SIZE (16)
	0x95, 0x02,                    //     REPORT_COUNT (2)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)

	0x09, 0x38,                    //     USAGE (Wheel)
	0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
	0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x81, 0x06,                    //     INPUT (Data,Var,Rel)

	0xc0,                          //   END_COLLECTION
	0xc0                           // END_COLLECTION 
};


KeyboardMouse_::KeyboardMouse_(void) : PluggableUSBModule(1, 1, epType), protocol(HID_REPORT_PROTOCOL), idle(1)
{
	epType[0] = EP_TYPE_INTERRUPT_IN;
	PluggableUSB().plug(this);
}

int KeyboardMouse_::getInterface(uint8_t* interfaceCount)
{
	*interfaceCount += 1; // uses 1
	HIDDescriptor hidInterface = {
		D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
		D_HIDREPORT(sizeof(_hidSingleReportDescriptorKeyboardMouse)),
		D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
	};
	return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
}

int KeyboardMouse_::getDescriptor(USBSetup& setup)
{
	// In a HID Class Descriptor wIndex cointains the interface number
	if (setup.wIndex != pluggedInterface) { return 0; }

	// Check if this is a HID Class Descriptor request
	if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) { return 0; }

	if (setup.wValueH == HID_HID_DESCRIPTOR_TYPE) {
		// Apple UEFI and USBCV wants it
		HIDDescDescriptor desc = D_HIDREPORT(sizeof(_hidSingleReportDescriptorKeyboardMouse));
		return USB_SendControl(0, &desc, sizeof(desc));
	} else if (setup.wValueH == HID_REPORT_DESCRIPTOR_TYPE) {
		// Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
		// due to the USB specs, but Windows and Linux just assumes its in report mode.
		protocol = HID_REPORT_PROTOCOL;
		return USB_SendControl(TRANSFER_PGM, _hidSingleReportDescriptorKeyboardMouse, sizeof(_hidSingleReportDescriptorKeyboardMouse));
	}

	return 0;
}

bool KeyboardMouse_::setup(USBSetup& setup)
{
	if (pluggedInterface != setup.wIndex) {
		return false;
	}

	uint8_t request = setup.bRequest;
	uint8_t requestType = setup.bmRequestType;

	if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE)
	{
		if (request == HID_GET_REPORT) {
			// TODO: HID_GetReport();
			return true;
		}
		if (request == HID_GET_PROTOCOL) {
			// TODO: Send8(protocol);
			return true;
		}
	}

	if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE)
	{
		if (request == HID_SET_PROTOCOL) {
			protocol = setup.wValueL;
			return true;
		}
		if (request == HID_SET_IDLE) {
			idle = setup.wValueH;
			return true;
		}
		if (request == HID_SET_REPORT)
		{
		}
	}

	return false;
}

typedef struct {
		uint8_t report_id;
		uint8_t modifiers;
		uint8_t keycodes[6];
} KeyboardReport;

typedef struct {
		uint8_t report_id;
		uint8_t buttons;
		int16_t xAxis;
		int16_t yAxis;
		int8_t wheel;
} MouseReport;


void KeyboardMouse_::sendKeyboardReport(uint8_t modifiers, uint8_t keycode1, uint8_t keycode2, uint8_t keycode3, uint8_t keycode4, uint8_t keycode5, uint8_t keycode6) {
	KeyboardReport r;
	r.report_id = 1;
	r.modifiers = modifiers;
	r.keycodes[0] = keycode1;
	r.keycodes[1] = keycode2;
	r.keycodes[2] = keycode3;
	r.keycodes[3] = keycode4;
	r.keycodes[4] = keycode5;
	r.keycodes[5] = keycode6;
	SendReport(&r, sizeof(r));
}


void KeyboardMouse_::sendMouseReport(uint8_t buttons, int16_t x, int16_t y, int8_t wheel) {
	MouseReport r;
	r.report_id = 2;
	r.buttons = buttons;
	r.xAxis = x;
	r.yAxis = y;
	r.wheel = wheel;
	SendReport(&r, sizeof(r));
}

void KeyboardMouse_::SendReport(void* data, int length)
{
	USB_Send(pluggedEndpoint | TRANSFER_RELEASE, data, length);
}

KeyboardMouse_ KeyboardMouse;


