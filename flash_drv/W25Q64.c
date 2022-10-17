#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "W25Q64.h"

#define flash_WRITE_IN_PROGRESS  	0x01
#define flash_WRITE_ENABLE_LATCH 	0x02

#define WRITE_ENABLE				0x06
#define WRITE_DISABLE				0x04
#define READ_STATUS_REG1			0x05
#define READ_STATUS_REG2			0x35
#define WRITE_STATUS_REG			0x01
#define READ_DATA					0x03
#define PAGE_PROGRAM				0x02
#define SECTOR_ERASE				0x20
#define CHIP_ERASE					0xC7
#define ENABLE_RESET				0x66
#define RESET						0x99

#define	WIBOND_ID					0xEF
#define DEVICE_ID					0x40
#define W25Q64_ID					0x17

uint8_t flash_Init(void)
{
	struct flash_info flash_memory;

	flash_eraseChip();
	flash_get_JEDEC_ID(&flash_memory);

	if(WIBOND_ID == flash_memory.manufacturer_ID && DEVICE_ID == flash_memory.memory_type && W25Q64_ID == flash_memory.capacity)
		return FLASH_OK;
	else
		return FLASH_ERROR;
}

void flash_eraseChip(void)
{
	uint8_t data_to_send[] = { ENABLE_RESET, RESET };

	flash_cs_low();
	flash_SPI_Transmit_Data(data_to_send, 2);
	flash_cs_high();
}

void flash_WriteEnable(void)
{
	uint8_t data_to_send =  WRITE_ENABLE;

	flash_cs_low();
	flash_SPI_Transmit_Data(&data_to_send, 1);
	flash_cs_high();
}

void flash_WriteDisable(void)
{
	uint8_t data_to_send =  WRITE_DISABLE;

	flash_cs_low();
	flash_SPI_Transmit_Data(&data_to_send, 1);
	flash_cs_high();
}

uint8_t flash_readStatReg1(void)
{
	uint8_t data_to_send = READ_STATUS_REG1;
	uint8_t receive_data = 0;

	flash_cs_low();
	flash_SPI_Transmit_Data(&data_to_send, 1);
	flash_SPI_Receive_Data(&receive_data, 1);
	flash_cs_high();

	return receive_data;
}

uint8_t flash_readStatReg2(void)
{
	uint8_t data_to_send = READ_STATUS_REG2;
	uint8_t receive_data = 0;

	flash_cs_low();
	flash_SPI_Transmit_Data(&data_to_send, 1);
	flash_SPI_Receive_Data(&receive_data, 1);
	flash_cs_high();

	return receive_data;
}

void flash_writeStatReg(uint8_t reg1, uint8_t reg2)
{
	uint8_t data_to_send[] = { 0, 0, 0 };

	flash_WriteEnable_and_WaitForWriteEnableLatch();

	data_to_send[0] = WRITE_STATUS_REG;
	data_to_send[1] = reg1;
	data_to_send[2] = reg2;

	flash_cs_low();
	flash_SPI_Transmit_Data(data_to_send, 2);
	flash_cs_high();
}

FLASH_Status flash_ReadDataBytes(uint32_t adress, uint8_t *data, uint16_t size)
{
	uint8_t data_to_send[] = { 0, 0, 0, 0 };
	FLASH_Status status;

	flash_WaitForWriteInProgressClear();

	data_to_send[0] = READ_DATA;
	data_to_send[1] = (adress >> 16) & 0xff;
	data_to_send[2] = (adress >> 8) & 0xff;
	data_to_send[3] = adress & 0xff;

	flash_cs_low();
	flash_SPI_Transmit_Data(data_to_send, 4);
	status = flash_SPI_Receive_Data(data, size);
	flash_cs_high();

	return status;
}

FLASH_Status flash_PageProgram(uint32_t page_adress, uint8_t *data, uint16_t size)
{
	uint8_t data_to_send[] = { 0, 0, 0, 0 };
	FLASH_Status status;

	flash_WaitForWriteInProgressClear();
	flash_WriteEnable_and_WaitForWriteEnableLatch();

	data_to_send[0] = PAGE_PROGRAM;
	data_to_send[1] = (page_adress >> 16) & 0xff;
	data_to_send[2] = (page_adress >> 8) & 0xff;
	data_to_send[3] = page_adress & 0xff;

	flash_cs_low();
	flash_SPI_Transmit_Data(data_to_send, 4);
	status = flash_SPI_Transmit_Data(data, size);
	flash_cs_high();

	return status;
}

FLASH_Status flash_SectorErase(uint16_t sector_number)
{
	uint32_t adress;
	adress = sector_number * SECTOR_SIZE;
	uint8_t data_to_send[] = { 0, 0, 0, 0 };
	FLASH_Status status;

	flash_WaitForWriteInProgressClear();
	flash_WriteEnable_and_WaitForWriteEnableLatch();

	data_to_send[0] = SECTOR_ERASE;
	data_to_send[1] = (adress >> 16) & 0xff;
	data_to_send[2] = (adress >> 8) & 0xff;
	data_to_send[3] = adress & 0xff;

	flash_cs_low();
	status = flash_SPI_Transmit_Data(data_to_send, 4);
	flash_cs_high();

	flash_WaitForWriteInProgressClear();

	return status;
}

FLASH_Status flash_ChipErase(void)
{
	uint8_t data_to_send =  CHIP_ERASE;
	FLASH_Status status;

	flash_WaitForWriteInProgressClear();
	flash_WriteEnable_and_WaitForWriteEnableLatch();

	flash_cs_low();
	status = flash_SPI_Transmit_Data(&data_to_send, 1);
	flash_cs_high();

	flash_WaitForWriteInProgressClear();

	return status;
}

void flash_get_JEDEC_ID(struct flash_info *info)
{
	uint8_t data_to_send = 0x9F;
	uint8_t receive_data[3] = { 0, 0, 0 };

	flash_cs_low();

	flash_SPI_Transmit_Data(&data_to_send, 1);
	flash_SPI_Receive_Data(receive_data, 3);

	flash_cs_high();

	info->manufacturer_ID = receive_data[0];
	info->memory_type = receive_data[1];
	info->capacity = receive_data[2];
}

void flash_WriteEnable_and_WaitForWriteEnableLatch(void)
{
	while(!(flash_readStatReg1() & flash_WRITE_ENABLE_LATCH))
	{
		flash_WriteEnable();
	}
}

void flash_WaitForWriteEnableLatch(void)
{
	while(!(flash_readStatReg1() & flash_WRITE_ENABLE_LATCH));
}

void flash_WaitForWriteInProgressClear(void)
{
	while((flash_readStatReg1() & flash_WRITE_IN_PROGRESS));
}

FLASH_Status flash_SPI_Transmit_Data(uint8_t *data, uint16_t size)
{
	FLASH_Status status;
	uint32_t written_bytes = 0;

	written_bytes = spi_write_blocking(SPI_BUS, data, size);

	if(written_bytes == size)
	{
		status = FLASH_OK;
	}
	else
	{
		status = FLASH_ERROR;
	}

	return status;
}

FLASH_Status flash_SPI_Receive_Data(uint8_t *data, uint16_t size)
{
	FLASH_Status status;
	uint32_t readed_bytes = 0;

	readed_bytes = spi_read_blocking(SPI_BUS, 0xFF, data, size);

	if(readed_bytes == size)
	{
		status = FLASH_OK;
	}
	else
	{
		status = FLASH_ERROR;
	}

	return status;
}

void flash_cs_low(void)
{
	gpio_put(FLASH_CS_PIN, 0);
}

void flash_cs_high(void)
{
	gpio_put(FLASH_CS_PIN, 1);
}
