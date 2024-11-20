#ifndef _ATA_H_
#define _ATA_H_

// Define some ATA I/O ports for Primary and Secondary channels
#define ATA_PRIMARY_COMMAND_PORT       0x1F0
#define ATA_PRIMARY_CONTROL_PORT       0x3F6
#define ATA_SECONDARY_COMMAND_PORT     0x170
#define ATA_SECONDARY_CONTROL_PORT     0x376

// Command ports offsets
#define ATA_REG_DATA                   0x00
#define ATA_REG_ERROR                  0x01
#define ATA_REG_FEATURES               0x01
#define ATA_REG_SECTOR_COUNT           0x02
#define ATA_REG_LBA_LOW                0x03
#define ATA_REG_LBA_MID                0x04
#define ATA_REG_LBA_HIGH               0x05
#define ATA_REG_DEVICE                 0x06
#define ATA_REG_STATUS                 0x07
#define ATA_REG_COMMAND                0x07

// Control ports offsets
#define ATA_REG_ALT_STATUS             0x00
#define ATA_REG_CONTROL                0x00

// Status register flags
#define ATA_STATUS_BSY                 0x80
#define ATA_STATUS_DRDY                0x40
#define ATA_STATUS_DRQ                 0x08
#define ATA_STATUS_ERR                 0x01

typedef unsigned __int8     BYTE, * PBYTE;
typedef unsigned __int16    WORD, * PWORD;
typedef unsigned __int32    DWORD, * PDWORD;
typedef unsigned __int64    QWORD, * PQWORD;
typedef signed __int8       INT8;
typedef signed __int16      INT16;
typedef signed __int32      INT32;
typedef signed __int64      INT64;

int ata_identify_device(int bus, int device);
int ata_wait_busy_with_timeout(int command_port, int timeout_ms);
int ata_check_error(int command_port);
int ata_write_sectors(int bus, int device, DWORD lba, WORD sector_count, PWORD data);
int ata_read_sectors(int bus, int device, DWORD lba, WORD sector_count, PWORD buffer);

#endif