#include "ata.h"
#include "ata_commands.h"
#include "logging.h"

#include <intrin.h>

int ata_wait_busy_with_timeout(int command_port, int timeout_ms) {
    int delay_count = 0;
    while (__inbyte(command_port + ATA_REG_STATUS) & ATA_STATUS_BSY) {
        if (++delay_count > timeout_ms) {
            LogSerial(0, "Timeout on port %X\n", command_port);
            return -1;
        }
    }
    return 0;
}

int ata_check_error(int command_port) {
    BYTE status = __inbyte(command_port + ATA_REG_STATUS);
    return (status & ATA_STATUS_ERR) ? 1 : 0;
}

int ata_identify_device(int bus, int device) {
    Log("Entered identify\n");
    int command_port = (bus == 0) ? ATA_PRIMARY_COMMAND_PORT : ATA_SECONDARY_COMMAND_PORT;
    int control_port = (bus == 0) ? ATA_PRIMARY_CONTROL_PORT : ATA_SECONDARY_CONTROL_PORT;

    __outbyte(command_port + ATA_REG_DEVICE, 0xA0 | (device << 4));
    if (ata_wait_busy_with_timeout(command_port, 5000) < 0) {
        LogSerial(0, "Timeout waiting device select, bus %D\n", bus);
        return -1;
    }

    Log("Sending identify command\n");
    __outbyte(command_port + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    if (ata_wait_busy_with_timeout(command_port, 5000) < 0) {
        LogSerial(0, "Timeout waiting identify command, bus %D\n", bus);
        return -1;
    }

    if (ata_check_error(command_port)) {
        LogSerial(0, "ATAPI device on bus %D\n", bus);
        return -1;
    }

    BYTE status = __inbyte(command_port + ATA_REG_STATUS);
    if (!(status & ATA_STATUS_DRQ)) {
        LogSerial(0, "No device on bus %D\n", bus);
        return -1;
    }

    ATA_IDENTIFY_RESPONSE identify_data;
    PWORD identify_data_ptr = (PWORD)&identify_data;

    for (int i = 0; i < sizeof(ATA_IDENTIFY_RESPONSE) / 2; i++) {
        identify_data_ptr[i] = __inword(command_port + ATA_REG_DATA);
    }

    BYTE serial_number[ATA_SERIAL_NO_CHARS + 1] = { 0 };
    BYTE model_number[ATA_MODEL_NO_CHARS + 1] = { 0 };

    for (int i = 0; i < sizeof(identify_data.SerialNumbers) && i < ATA_SERIAL_NO_CHARS; i += 2) {
        serial_number[i] = identify_data.SerialNumbers[i + 1];
        serial_number[i + 1] = identify_data.SerialNumbers[i];
    }
    for (int i = 0; i < sizeof(identify_data.ModelNumber) && i < ATA_MODEL_NO_CHARS; i += 2) {
        model_number[i] = identify_data.ModelNumber[i + 1];
        model_number[i + 1] = identify_data.ModelNumber[i];
    }


    serial_number[ATA_SERIAL_NO_CHARS] = '\0';
    model_number[ATA_MODEL_NO_CHARS] = '\0';

    LogSerial(0, "Device detected on bus %D\n", bus);
    LogSerial(0, "Serial number - %s\n", serial_number);
    LogSerial(0, "Model number - %s\n", model_number);

    return 0;
}

int ata_write_sectors(int bus, int device, DWORD lba, WORD sector_count, PWORD data) {
    int command_port = (bus == 0) ? ATA_PRIMARY_COMMAND_PORT : ATA_SECONDARY_COMMAND_PORT;
    int control_port = (bus == 0) ? ATA_PRIMARY_CONTROL_PORT : ATA_SECONDARY_CONTROL_PORT;

    if (lba > 0x0FFFFFFF) {
        LogSerial(0, "LBA out of range\n");
        return -1;
    }

    __outbyte(command_port + ATA_REG_DEVICE, 0xE0 | (device << 4) | ((lba >> 24) & 0x0F));

    if (ata_wait_busy_with_timeout(command_port, 5000) < 0) {
        // should never happen if operation on working device
        LogSerial(0, "Timeout waiting select, bus %D\n", bus);
        return -1;
    }

    __outbyte(command_port + ATA_REG_SECTOR_COUNT, (BYTE)sector_count);
    __outbyte(command_port + ATA_REG_LBA_LOW, (BYTE)(lba & 0xFF));
    __outbyte(command_port + ATA_REG_LBA_MID, (BYTE)((lba >> 8) & 0xFF));
    __outbyte(command_port + ATA_REG_LBA_HIGH, (BYTE)((lba >> 16) & 0xFF));

    __outbyte(command_port + ATA_REG_COMMAND, ATA_CMD_WRITE_SECTORS);

    for (int sector = 0; sector < sector_count; sector++) {
        if (ata_wait_busy_with_timeout(command_port, 5000) < 0 || !(__inbyte(command_port + ATA_REG_STATUS) & ATA_STATUS_DRQ)) {
            return -1;
        }

        for (int i = 0; i < 256; i++) {
            __outword(command_port + ATA_REG_DATA, data[sector * 256 + i]);
        }

        if (ata_wait_busy_with_timeout(command_port, 5000) < 0) {
            return -1;
        }
    }

    if (ata_check_error(command_port)) {
        LogSerial(0, "Error during write operation, bus %D\n", bus);
        return -1;
    }

    LogSerial(0, "Write successful on bus %D, device %D, LBA %d, sectors %d\n", bus, device, lba, sector_count);
    return 0;
}

int ata_read_sectors(int bus, int device, DWORD lba, WORD sector_count, PWORD buffer) {
    int command_port = (bus == 0) ? ATA_PRIMARY_COMMAND_PORT : ATA_SECONDARY_COMMAND_PORT;
    int control_port = (bus == 0) ? ATA_PRIMARY_CONTROL_PORT : ATA_SECONDARY_CONTROL_PORT;

    if (lba > 0x0FFFFFFF) {
        LogSerial(0, "LBA out of range\n");
        return -1;
    }

    __outbyte(command_port + ATA_REG_DEVICE, 0xE0 | (device << 4) | ((lba >> 24) & 0x0F));

    if (ata_wait_busy_with_timeout(command_port, 5000) < 0) {
        // should never happen if operation on working device
        LogSerial(0, "Timeout waiting device select, bus %D\n", bus);
        return -1;
    }

    __outbyte(command_port + ATA_REG_SECTOR_COUNT, (BYTE)sector_count);
    __outbyte(command_port + ATA_REG_LBA_LOW, (BYTE)(lba & 0xFF));
    __outbyte(command_port + ATA_REG_LBA_MID, (BYTE)((lba >> 8) & 0xFF));
    __outbyte(command_port + ATA_REG_LBA_HIGH, (BYTE)((lba >> 16) & 0xFF));

    __outbyte(command_port + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    for (int sector = 0; sector < sector_count; sector++) {
        if (ata_wait_busy_with_timeout(command_port, 5000) < 0 || !(__inbyte(command_port + ATA_REG_STATUS) & ATA_STATUS_DRQ)) {
            return -1;
        }

        for (int i = 0; i < 256; i++) {
            WORD raw_data = __inword(command_port + ATA_REG_DATA);
            buffer[sector * 256 + i] = (raw_data >> 8) | (raw_data << 8);
        }

        if (ata_wait_busy_with_timeout(command_port, 5000) < 0) {
            return -1;
        }
    }

    if (ata_check_error(command_port)) {
        LogSerial(0, "Error during read operation, bus %D\n", bus);
        return -1;
    }

    LogSerial(0, "Read successful on bus %D, device %D, LBA %d, sectors %d\n", bus, device, lba, sector_count);
    return 0;
}
