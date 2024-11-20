#ifndef _CMOS_H_
#define _CMOS_H_

typedef unsigned __int8 BYTE;

#define DATE_BUFFER_SIZE 20

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

char* GetCurrentDate();

#endif
