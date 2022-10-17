#ifndef INC_flash_FLASH_H_
#define INC_flash_FLASH_H_

#define PAGE_SIZE					256
#define SECTOR_SIZE					4096

#define FLASH_CS_PIN				15
#define SPI_BUS						spi0

struct flash_info
{
	uint8_t manufacturer_ID;
	uint8_t memory_type;
	uint8_t capacity;
};

typedef enum flash_status_t
{
	FLASH_OK = 0,
	FLASH_ERROR,
	FLASH_BUSY,
} FLASH_Status;

uint8_t flash_Init(void);

void flash_eraseChip(void);

void flash_WriteEnable(void);
void flash_WriteDisable(void);

uint8_t flash_readStatReg1(void);
uint8_t flash_readStatReg2(void);
void flash_writeStatReg(uint8_t, uint8_t);

FLASH_Status flash_SectorErase(uint16_t);
FLASH_Status flash_ChipErase(void);

FLASH_Status flash_PageProgram(uint32_t, uint8_t *, uint16_t);
FLASH_Status flash_ReadDataBytes(uint32_t adress, uint8_t *data, uint16_t size);

void flash_get_JEDEC_ID(struct flash_info *info);

void flash_WriteEnable_and_WaitForWriteEnableLatch(void);
void flash_WaitForWriteEnableLatch(void);
void flash_WaitForWriteInProgressClear(void);

FLASH_Status flash_SPI_Transmit_Data(uint8_t *data, uint16_t size);
FLASH_Status flash_SPI_Receive_Data(uint8_t *data, uint16_t size);
void flash_cs_low(void);
void flash_cs_high(void);

#endif /* INC_flash_FLASH_H_ */
