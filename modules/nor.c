/**
 * @file nor.c
 * 
 * Copyright (C): SEC Consult Unternehmensberatung GmbH, 2019         \n
 * Web:           https://sec-consult.com/                            \n
 *                                                                    \n
 * This Source Code Form is subject to the terms of the Mozilla Public\n
 * License, v. 2.0. If a copy of the MPL was not distributed with this\n
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.          \n
 * 
 * @authors Thomas Weber, Wolfgang Ettlinger, Steffen Robertz
 */

#include "../secxtractor.h"
#include "../hal.h"

/*
 * PK0 = CE#		0x01 OUTPUT
 * PK1 = OE#		0x02 OUTPUT
 * PK2 = WE#		0x04 OUTPUT
 * PK3 = WP#/ACC	0x08 OUTPUT
 * PK4 = RESET#		0x10 OUTPUT
 * PK5 = BYTE#		0x20 OUTPUT
 * PK6 = RY#/BY#	0x40 INPUT
 * 
 * control BIT0 = 16bit/1 8bit/0
 * control BIT1 = BigEndian/1 LittleEndian/0
 * */

/**
 * @brief Function to dump a NOR flash memory.
 * The content is directly redirected to the main UART in hexadecimal and can be recovered with "xxd -r".
 * @param endianess This varable indicates which endianess type should be set on the outputted firmware.
 * @param word_mode If set to one Word mode will be used during transfers, otherwise Byte Mode will be used
 */
static inline void norDump(uint8_t endianess, uint8_t word_mode)
{
	DEBUG("Entering norDump");
	uartprintf("endiness:%u, word: %u"NL, endianess, word_mode);

	/* NOR flash init */
	portOut(&NOR_CTRL_PORT, NOR_CE | NOR_OE | NOR_RESET);
	portSetDirection(&NOR_CTRL_PORT, ~(NOR_RYBY | NOR_NC) & 0xFF); // set CE#, OE# and WE# to output, RY/BY# input

	/* set all address pins to output */
	portSetDirection(&NOR_ADDR_PORT0, 0xFF);
	portSetDirection(&NOR_ADDR_PORT1, 0xFF);
	portSetDirection(&NOR_ADDR_PORT2, 0xFF);
	portSetDirection(&NOR_ADDR_PORT3, 0xFF);

	/* set all data pins to input */
	portOut(&NOR_DATA_PORT0, 0x00);
	portOut(&NOR_DATA_PORT1, 0x00);
	portSetDirection(&NOR_DATA_PORT0, 0x00);
	portSetDirection(&NOR_DATA_PORT1, 0x00);

	norResetFlash();

	dumpStart();


	for (uint64_t address = 0x0; address < NOR_READ_LENGTH && !isOperationCanceled(); ++address)
	{
		uint16_t data;
		norStartReadData();
		if(word_mode){
			norModeWord();
			data = norRead16DataBits(address);
			norEndReadData();
			uint8_t data_byte1 = (uint8_t)((data & 0xFF00) >> 8);
			uint8_t data_byte2 = (uint8_t)(data & 0x00FF);
			if (endianess & 0x02 == 0x02)
			{
				dumpByte(data_byte2);
				dumpByte(data_byte1);
			}
			else
			{
				dumpByte(data_byte1);
				dumpByte(data_byte2);
			}
		}else{
			norModeByte();
			data = norRead8DataBits(address);
			norEndReadData();
			dumpByte((uint8_t) data);
		}
	}
	dumpEnd();
}

void cmdDumpNor(char *arguments)
{
	if(*arguments == '\0'){
		uartWriteString("Please supply and endianess and word-mode parameter"NL);
		uartWriteString("dump nor <endianess> <word_mode>"NL"<endianess> = 0 or 1"NL"<word_mode>=1 or 0"NL);
		return;
	}
	uint8_t word_mode = 1;
	uint8_t endianess = 1;
	int err = sscanf(arguments, "%u %u", &endianess, &word_mode);
	if (endianess < 2)
	{
		uartWriteString(NL "Starting nor flash dump: ");
		norDump(endianess, word_mode);
	}
	else
	{
		uartWriteString("Value too high. Max 1!" NL);
	}
}

///TODO: Fix pattern write method
/**
 * @brief Writes a pattern to a NOR Flash
 */
// void cmdNorPattern(char *arguments)
// {
// 	uartprintf("Writing NOR Pattern: %s", arguments);
// 	uint16_t pattern = 0xff;
// 	sscanf(arguments, "0x%x", &pattern);

// 	/* NOR flash init */
// 	portOut(&NOR_CTRL_PORT, NOR_CE | NOR_OE | NOR_RESET);
// 	portSetDirection(&NOR_CTRL_PORT, ~(NOR_RYBY | NOR_NC) & 0xFF); // set CE#, OE# and WE# to output, RY/BY# input

// 	/* set all address pins to output */
// 	portSetDirection(&NOR_ADDR_PORT0, 0xFF);
// 	portSetDirection(&NOR_ADDR_PORT1, 0xFF);
// 	portSetDirection(&NOR_ADDR_PORT2, 0xFF);
// 	portSetDirection(&NOR_ADDR_PORT3, 0xFF);

// 	/* set all data pins to output */
// 	portSetDirection(&NOR_DATA_PORT0, 0xFF);
// 	portSetDirection(&NOR_DATA_PORT1, 0xFF);

// 	norResetFlash();

// 	norModeByte();

// 	norEraseChip();

// 	_delay_ms(10000);

// 	for (uint64_t address = 0x0; address < NOR_READ_LENGTH; ++address)
// 	{
// 		norWrite8DataBits(0x0aaa, 0xaa);
// 		norWrite8DataBits(0x0555, 0x55);
// 		norWrite8DataBits(0x0aaa, 0xa0);
// 		norWrite8DataBits(address, pattern);
// 		uartWriteChar('.');
// 	}
// 	uartWriteString(NL);
// 	uartprintf("Pattern written. Pls verify!");
// }
