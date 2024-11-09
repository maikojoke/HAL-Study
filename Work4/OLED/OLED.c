#include "stm32f1xx_hal.h"
#include "OLED_Font.h"

#define OLED_W_SCL(x)		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (x))
#define OLED_W_SDA(x)		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, (x))

void OLED_I2C_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	OLED_W_SCL(GPIO_PIN_SET);
	OLED_W_SDA(GPIO_PIN_SET);
}

void OLED_I2C_Start(void)
{
	OLED_W_SDA(GPIO_PIN_SET);
	OLED_W_SCL(GPIO_PIN_SET);
	OLED_W_SDA(GPIO_PIN_RESET);
	OLED_W_SCL(GPIO_PIN_RESET);
}

void OLED_I2C_Stop(void)
{
	OLED_W_SDA(GPIO_PIN_RESET);
	OLED_W_SCL(GPIO_PIN_SET);
	OLED_W_SDA(GPIO_PIN_SET);
}

void OLED_I2C_SendByte(uint8_t Byte)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		OLED_W_SDA((Byte & (0x80 >> i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
		OLED_W_SCL(GPIO_PIN_SET);
		OLED_W_SCL(GPIO_PIN_RESET);
	}
	OLED_W_SCL(GPIO_PIN_SET); // Extra clock pulse
	OLED_W_SCL(GPIO_PIN_RESET);
}

void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		// Slave address
	OLED_I2C_SendByte(0x00);		// Write command
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}

void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		// Slave address
	OLED_I2C_SendByte(0x40);		// Write data
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}

void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					// Set Y position
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	// Set X high 4 bits
	OLED_WriteCommand(0x00 | (X & 0x0F));			// Set X low 4 bits
}

void OLED_Clear(void)
{  
	for (uint8_t j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for (uint8_t i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    // 检查行和列的范围
    if (Line < 1 || Line > 4 || Column < 1 || Column > 16) return;

    // 设置光标到字符的上半部分
    OLED_SetCursor(Line * 2 - 2, (Column - 1) * 8);  // 行乘以 2 - 2（上半部分行）

    // 显示字符的上半部分
    for (uint8_t i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]); // 显示上半部分
    }

    // 设置光标到字符的下半部分
    OLED_SetCursor(Line * 2 - 1, (Column - 1) * 8); // 行乘以 2 - 1（下半部分行）

    // 显示字符的下半部分
    for (uint8_t i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]); // 显示下半部分
    }
}





void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	for (uint8_t i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	for (uint8_t i = 0; i < Length; i++)
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (uint8_t i = 0; i < Length; i++)
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	for (uint8_t i = 0; i < Length; i++)
	{
		uint8_t SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	for (uint8_t i = 0; i < Length; i++)
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

void OLED_Init(void)
{
	HAL_Delay(1000); // Power on delay
	
	OLED_I2C_Init(); // Port initialization
	
	OLED_WriteCommand(0xAE); // Turn off display
	OLED_WriteCommand(0xD5); // Set display clock divide ratio/oscillator frequency
	OLED_WriteCommand(0x80);
	OLED_WriteCommand(0xA8); // Set multiplex ratio
	OLED_WriteCommand(0x3F);
	OLED_WriteCommand(0xD3); // Set display offset
	OLED_WriteCommand(0x00);
	OLED_WriteCommand(0x40); // Set start line
	OLED_WriteCommand(0xA1); // Set segment remap
	OLED_WriteCommand(0xC8); // Set COM output scan direction
	OLED_WriteCommand(0xDA); // Set COM pins hardware configuration
	OLED_WriteCommand(0x12);
	OLED_WriteCommand(0x81); // Set contrast control
	OLED_WriteCommand(0xCF);
	OLED_WriteCommand(0xD9); // Set pre-charge period
	OLED_WriteCommand(0xF1);
	OLED_WriteCommand(0xDB); // Set VCOMH deselect level
	OLED_WriteCommand(0x30);
	OLED_WriteCommand(0xA4); // Entire display ON
	OLED_WriteCommand(0xA6); // Normal display
	OLED_WriteCommand(0x8D); // Charge pump
	OLED_WriteCommand(0x14);
	OLED_WriteCommand(0xAF); // Turn on display
		
	OLED_Clear(); // Clear OLED
}
