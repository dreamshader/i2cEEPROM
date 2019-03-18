/*
 ***********************************************************************
 *
 *  i2cEEPROM.cpp - part of eeprom access project
 *
 *  Copyright (C) 2019 Dreamshader (Dirk Schanz)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 ***********************************************************************
*
 * history:
 *
 * in 2013 ....: initial check in / -ds-
 * 2019/02/17 .: begin of complete rework / -ds-
 *
 ***********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>

#include "i2cEEPROM.h"


/*
 ***************************************************************************
 * i2cEEPROM::i2cEEPROM()
 * ----------------------------------------------------
 * create instance of i2cEEPROM
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns nothing
 ***************************************************************************
*/
i2cEEPROM::i2cEEPROM()
{
    pBus = (i2cConnection*) NULL;
    autoInit = false;
}

/*
 ***************************************************************************
 * i2cEEPROM::~i2cEEPROM()
 * ----------------------------------------------------
 * destructor - used to cleanup
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns nothing
 ***************************************************************************
*/
i2cEEPROM::~i2cEEPROM()
{
    if( pBus != (i2cConnection*) NULL )
    {
        pBus->i2cClose();
        delete pBus;
    }
}

/*
 ***************************************************************************
 * int i2cEEPROM::eeInit(int busNo, int slaveAddr, uint16_t magic,
*                      uint16_t type, bool initialize)
 * ----------------------------------------------------
 * initialize an i2cEEPROM object with given parameters
 * note: to handle regular i2c EEPROMs without any
 *       private header use this initializer with parameter
 *       magic set to I2C_EE_NO_MAGIC!
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cEEPROM::eeInit( int busNo, int slaveAddr, uint16_t magic, uint16_t type, bool initialize )
{
    int retVal;

    pBus = (i2cConnection*) NULL;
    autoInit = false;

    if( (pBus = new i2cConnection()) != NULL )
    {
        if( (retVal = pBus->i2cProbe( busNo, slaveAddr, magic, type )) >= 0 )
        {
// fprintf(stderr, "found device bus %d, addr %02x\n", busNo, slaveAddr);
            switch( retVal )
            {
                case EE_TYPE_24AA65:
                case EE_TYPE_24LC65:
                    retVal = E_I2C_EE_NOTSUPPORTED;
                    break;
                case EE_TYPE_24C65:
                    pBus->i2c_write_cycle_time  = WRITE_CYCLE_TIME_24C65;
                    pBus->i2c_bus_frequency_1V8 = BUS_FREQUENCY_1V8_24C65;
                    pBus->i2c_bus_frequency_4V5 = BUS_FREQUENCY_4V5_24C65;
                    ee_page_size   = PAGE_SIZE_24C65;
                    ee_total_pages = TOTAL_PAGES_24C65;
                    ee_block_size  = BLOCK_SIZE_24C65;

                    if( magic == I2C_EE_NO_MAGIC )
                    {
// fprintf(stderr, "generic eeprom -> no private header!\n");
                        byte_offset = 0;
                    }
                    else
                    {
// fprintf(stderr, "special eeprom -> private header!\n");
                        byte_offset = EE_PRIVATE_HDR_LEN;
                    }
                    retVal = E_I2C_SUCCESS;
                    break;
                case EE_TYPE_24C16:
                    pBus->i2c_write_cycle_time  = WRITE_CYCLE_TIME_24C16;
                    pBus->i2c_bus_frequency_1V8 = BUS_FREQUENCY_1V8_24C16;
                    pBus->i2c_bus_frequency_4V5 = BUS_FREQUENCY_4V5_24C16;
                    ee_page_size   = PAGE_SIZE_24C16;
                    ee_total_pages = TOTAL_PAGES_24C16;
                    ee_block_size  = BLOCK_SIZE_24C16;

                    if( magic == I2C_EE_NO_MAGIC )
                    {
// fprintf(stderr, "generic eeprom -> no private header!\n");
                        byte_offset = 0;
                    }
                    else
                    {
// fprintf(stderr, "special eeprom -> private header!\n");
                        byte_offset = EE_PRIVATE_HDR_LEN;
                    }
                    retVal = E_I2C_SUCCESS;
                    break;
                default:
                    retVal = E_I2C_EE_INVAL_ID;
                    break;
            }

            retVal = pBus->i2cOpen( busNo, slaveAddr, false, O_RDWR );
        }
    }
    else
    {
        retVal = E_I2C_FAIL;
    }

    return( retVal );
}

/*
 ***************************************************************************
 * int i2cEEPROM::eeInit( int busNo, int minSlaveAddr, int maxSlaveAddr )
 * ----------------------------------------------------
 * search for an i2c EEPROM on the bus
 * note: to handle special i2c EEPROMs with private header
 *       use this initializer!
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cEEPROM::eeInit( int busNo, int minSlaveAddr, int maxSlaveAddr )
{
    unsigned char devAddr;
    bool devFound;
    int retVal;

    if( (pBus = new i2cConnection()) != NULL )
    {
        devAddr = minSlaveAddr;
        devFound = false;

        do
        {
// fprintf(stderr, "probing bus %d, addr %02x\n", busNo, devAddr);
            if( (retVal = pBus->i2cProbe( busNo, devAddr )) >= 0 )
            {
                devFound = true;
            }
        }
        while( !devFound && devAddr++ < maxSlaveAddr );

        if( devFound )
        {
// fprintf(stderr, "found device bus %d, addr %02x\n", busNo, devAddr);
            switch( retVal )
            {
                case EE_TYPE_24AA65:
                case EE_TYPE_24LC65:
                    retVal = E_I2C_EE_NOTSUPPORTED;
                    break;
                case EE_TYPE_24C65:
                    pBus->i2c_write_cycle_time  = WRITE_CYCLE_TIME_24C65;
                    pBus->i2c_bus_frequency_1V8 = BUS_FREQUENCY_1V8_24C65;
                    pBus->i2c_bus_frequency_4V5 = BUS_FREQUENCY_4V5_24C65;
                    ee_page_size   = PAGE_SIZE_24C65;
                    ee_total_pages = TOTAL_PAGES_24C65;
                    ee_block_size  = BLOCK_SIZE_24C65;

                    // pBus->i2c_write_cycle_time = 5;
                    //  pBus->i2c_bus_frequency_1V8 = 100;
                    // pBus->i2c_bus_frequency_4V5 = 400;
                    // ee_page_size = 8;
                    // ee_total_pages = 8 * 1024;
                    // ee_block_size = I2C_MAX_BLOCK_LEN;
// fprintf(stderr, "special eeprom -> private header!\n");
                    byte_offset = EE_PRIVATE_HDR_LEN;
                    retVal = E_I2C_SUCCESS;
                    break;
                default:
                    retVal = E_I2C_EE_INVAL_ID;
                    break;
            }

            retVal = pBus->i2cOpen( busNo, devAddr, false, O_RDWR );

            if( retVal == E_I2C_SUCCESS )
            {
                autoInit = true;
// fprintf(stderr, "device successfully opened\n");
            }
        }
    }
    else
    {
        retVal = E_I2C_FAIL;
    }

    return( retVal );
}

/*
 ***************************************************************************
 * void i2cEEPROM::eeClose( void )
 * ----------------------------------------------------
 * close handle to i2c device
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns nothing
 ***************************************************************************
*/
void i2cEEPROM::eeClose( void )
{
    if( pBus != (i2cConnection*) NULL )
    {
        pBus->i2cClose();
    }
}

/*
 ***************************************************************************
 * int i2cEEPROM::eeOpen( int busNo, int slaveAddr )
 * ----------------------------------------------------
 * open handle to an i2c device
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns E_EE_SUCCESS or an error code
 ***************************************************************************
*/
int i2cEEPROM::eeOpen( int busNo, int slaveAddr )
{
    int retVal;

    if( (pBus = new i2cConnection()) != NULL )
    {
        retVal = pBus->i2cOpen( busNo, slaveAddr, false, O_RDWR );
    }
    else
    {
        retVal = E_I2C_FAIL;
    }

    return( retVal );
}

/*
 ***************************************************************************
 * int i2cEEPROM::eeRead( unsigned char* pBuffer, int amount )
 * ----------------------------------------------------
 * read amount of bytes into buffer pointed by pBuffer
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cEEPROM::eeRead( unsigned char* pBuffer, int amount )
{
    int retVal = 0;
fprintf(stderr, "called i2cEEPROM::eeRead\n"); 
    return( retVal );
}

/*
 ***************************************************************************
 * int i2cEEPROM::eeWrite( unsigned char* pBuffer, int amount, uint16_t addr )
 * ----------------------------------------------------
 * write amount of bytes pointed by pBuffer to address addr
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cEEPROM::eeWrite( unsigned char* pBuffer, int amount, uint16_t addr )
{
    int retVal = 0;
fprintf(stderr, "called i2cEEPROM::eeWrite\n"); 
    return( retVal );
}

