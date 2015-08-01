#ifndef _MCI_NAME_H_
#define _MCI_NAME_H_

struct mci_desc {
	unsigned int event;
	char name[32];
};

struct mci_desc mci_desc[] = {
	{ 0x00, "Si0 AR transaction counts" },
	{ 0x01, "Si0 AW transaction counts" },
	{ 0x02, "AC_R transaction counts" },
	{ 0x03, "AC_W transaction counts" },
	{ 0x04, "AC_R snoop hit" },
	{ 0x05, "AC_W snoop hit" },
	{ 0x06, "Si0 AR input queue full" },
	{ 0x07, "Si0 AW input queue full" },
	{ 0x08, "Si0 AR order table full" },
	{ 0x09, "Si0 AW order table full" },
	{ 0x0a, "Reserved" },
	{ 0x0b, "Si0 AR order table stall" },
	{ 0x0c, "Si0 AW order table stall" },
	{ 0x0d, "Si1 AR transaction counts" },
	{ 0x0e, "Si1 AW transaction counts" },
	{ 0x0f, "Si1 AR input queue full" },
	{ 0x10, "Si1 AW input queue full" },
	{ 0x11, "Si1 AR order table full" },
	{ 0x12, "Si1 AW order table full" },
	{ 0x13, "Reserved" },
	{ 0x14, "Si1 AR order table full" },
	{ 0x15, "Si1 AW order table full" },
	{ 0x16, "Si1 ar_tracker full" },
	{ 0x17, "Si0 aw_tracker full" },
	{ 0x18, "Si1 tracker contention" }
};

#define MCI_DESC_COUNT (sizeof(mci_desc) / sizeof(struct mci_desc))

#endif // _MCI_NAME_H_