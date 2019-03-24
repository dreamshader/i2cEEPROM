/*
 ***********************************************************************
 *
 *  i2cEEPROM.cpp - part of eeprom access project
 *
 *  Copyright (C) 2013-2019 Dreamshader (Dirk Schanz)
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
    byte_offset = 0;
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
 * int i2cEEPROM::eeInit( void )
 * ----------------------------------------------------
 * initialize EEPROM with params set
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cEEPROM::eeInit( void )
{
    int retVal;

    if( pBus != (i2cConnection*) NULL )
    {
        if( (retVal = pBus->initID( makeMagic(), ee_type )) == E_I2C_SUCCESS )
        {
            byte_offset = EE_PRIVATE_HDR_LEN;
            autoInit = false;
        }

//
//        if( (retVal = pBus->writeWord( 0, makeMagic() )) == E_I2C_SUCCESS )
//        {
//            retVal = pBus->writeWord( 2, ee_type );
//            byte_offset = EE_PRIVATE_HDR_LEN;
//            autoInit = false;
//        }

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

    if( (pBus = new i2cConnection( busNo, slaveAddr, false, O_RDWR )) != NULL )
    {
        // retVal = pBus->i2cOpen( busNo, slaveAddr, false, O_RDWR );
        retVal = pBus->i2cOpen();
    }
    else
    {
        retVal = E_I2C_FAIL;
    }

    return( retVal );
}


/*
 ***************************************************************************
 * int i2cEEPROM::eeTypeDetect( uint16_t* pMagic, uint16_t* pType )
 * ----------------------------------------------------
 * try to get info from EEPROM
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 * the values pointed by pMagic and pType will contain the
 * detected magic and EEPROM type in case of success
 ***************************************************************************
*/
int i2cEEPROM::eeTypeDetect( uint16_t* pMagic, uint16_t* pType )
{
    int retVal;

    if( pBus != (i2cConnection*) NULL )
    {
        if( pMagic == NULL || pType == NULL )
        {
            retVal = E_EE_DATA_NULLP;
        }
        else
        {
            retVal = pBus->check4Magic( pMagic, pType );
        }
    }
    else
    {
        retVal = E_EE_NO_CONNECTION;

    }

    return( retVal );
}

/*
 ***************************************************************************
 * int i2cEEPROM::eeTypeSet( uint16_t type )
 * ----------------------------------------------------
 * initialize internal data to match EEPROM type <type>
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cEEPROM::eeTypeSet( uint16_t type )
{
    int retVal;

    if( pBus != (i2cConnection*) NULL )
    {
        switch( type )
        {
            case EE_TYPE_24AA65:
fprintf(stderr, "Set EEPROM type to %s\n", EE_NAMES_24AA65);
                ee_type        = type;
                ee_page_size   = PAGE_SIZE_24AA65;
                ee_total_pages = TOTAL_PAGES_24AA65;
                ee_block_size  = BLOCK_SIZE_24AA65;
                pBus->i2c_16bit_addressing  = ADRESSING_16_BIT_24AA65;
                pBus->i2c_write_cycle_time  = WRITE_CYCLE_TIME_24AA65;
                pBus->i2c_bus_frequency_1V8 = BUS_FREQUENCY_1V8_24AA65;
                pBus->i2c_bus_frequency_4V5 = BUS_FREQUENCY_4V5_24AA65;
                retVal = E_EE_SUCCESS;
                break;
            case EE_TYPE_24LC65:
fprintf(stderr, "Set EEPROM type to %s\n", EE_NAMES_24LC65);
                ee_type        = type;
                ee_page_size   = PAGE_SIZE_24LC65;
                ee_total_pages = TOTAL_PAGES_24LC65;
                ee_block_size  = BLOCK_SIZE_24LC65;
                pBus->i2c_16bit_addressing  = ADRESSING_16_BIT_24LC65;
                pBus->i2c_write_cycle_time  = WRITE_CYCLE_TIME_24LC65;
                pBus->i2c_bus_frequency_1V8 = BUS_FREQUENCY_1V8_24LC65;
                pBus->i2c_bus_frequency_4V5 = BUS_FREQUENCY_4V5_24LC65;
                retVal = E_EE_SUCCESS;
                break;
            case EE_TYPE_24C65:
fprintf(stderr, "Set EEPROM type to %s\n", EE_NAMES_24C65);
                ee_type        = type;
                ee_page_size   = PAGE_SIZE_24C65;
                ee_total_pages = TOTAL_PAGES_24C65;
                ee_block_size  = BLOCK_SIZE_24C65;
                pBus->i2c_16bit_addressing  = ADRESSING_16_BIT_24C65;
                pBus->i2c_write_cycle_time  = WRITE_CYCLE_TIME_24C65;
                pBus->i2c_bus_frequency_1V8 = BUS_FREQUENCY_1V8_24C65;
                pBus->i2c_bus_frequency_4V5 = BUS_FREQUENCY_4V5_24C65;
                retVal = E_EE_SUCCESS;
                break;
            case EE_TYPE_24C16:
fprintf(stderr, "Set EEPROM type to %s\n", EE_NAMES_24C16);
                ee_type        = type;
                ee_page_size   = PAGE_SIZE_24C16;
                ee_total_pages = TOTAL_PAGES_24C16;
                ee_block_size  = BLOCK_SIZE_24C16;
                pBus->i2c_16bit_addressing  = ADRESSING_16_BIT_24C16;
                pBus->i2c_write_cycle_time  = WRITE_CYCLE_TIME_24C16;
                pBus->i2c_bus_frequency_1V8 = BUS_FREQUENCY_1V8_24C16;
                pBus->i2c_bus_frequency_4V5 = BUS_FREQUENCY_4V5_24C16;
                retVal = E_EE_SUCCESS;
                break;
            default:
fprintf(stderr, "EEPROM type %4x INVALID!\n", type);
                retVal = E_EE_INVAL_TYPE;
                break;

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
 * int i2cEEPROM::eeTypeSet( uint16_t type )
 * ----------------------------------------------------
 * initialize internal data to match EEPROM type <type>
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
void i2cEEPROM::eeInfo( void )
{
    bool validType = true;

    switch( ee_type )
    {
        case EE_TYPE_24AA65:
fprintf(stderr, "EEPROM type %s\n", EE_NAMES_24AA65);
            break;
        case EE_TYPE_24LC65:
fprintf(stderr, "EEPROM type %s\n", EE_NAMES_24LC65);
            break;
        case EE_TYPE_24C65:
fprintf(stderr, "EEPROM type %s\n", EE_NAMES_24C65);
            break;
        case EE_TYPE_24C16:
fprintf(stderr, "EEPROM type %s\n", EE_NAMES_24C16);
            break;
        default:
            validType = false;
fprintf(stderr, "INVALID EEPROM type %4x!\n", ee_type);
            break;

    }

    if( validType )
    {
        fprintf(stderr, "Type ....,,.......: %4d\n", ee_type );
        fprintf(stderr, "Page size ........: %4d byte\n", ee_page_size );
        fprintf(stderr, "Total pages ......: %4d\n", ee_total_pages );
        fprintf(stderr, "Block size .......: %4d byte\n", ee_block_size );

        if( pBus != NULL )
        {
//            fprintf(stderr, "16 bit addressing : %s\n", 
//                    pBus->i2c_16bit_addressing == true ? "true" : "false" );

            fprintf(stderr, "Addressing .......: %4d bit\n", 
                    pBus->i2c_16bit_addressing == true ? 16 : 8 );

            fprintf(stderr, "Write cycle time .: %4d ms\n", 
                             pBus->i2c_write_cycle_time );
            fprintf(stderr, "Bus frequency 1V8 : %4d kHz\n", 
                             pBus->i2c_bus_frequency_1V8 );
            fprintf(stderr, "Bus frequency 4V5 : %4d kHz\n", 
                             pBus->i2c_bus_frequency_4V5 );
        }
    }
}

/*
 ***************************************************************************
 * int i2cEEPROM::eeRead( uint8_t* pBuffer, int amount, uint16_t addr )
 * ----------------------------------------------------
 * read amount of bytes from address addr into buffer pointed by pBuffer
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cEEPROM::eeRead( uint16_t addr, uint8_t* pBuffer, int amount )
{
    int retVal = 0;

    if( pBus != (i2cConnection*) NULL )
    {
fprintf(stderr, "called i2cEEPROM::eeRead\n"); 
fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        if( addr != I2C_CURRENT_ADDRESS )
        {
            addr += byte_offset;
        }

fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        retVal = pBus->readBuf( addr, pBuffer, amount );
    }
    else
    {
        retVal = E_I2C_FAIL;
    }

    return( retVal );
}


int i2cEEPROM::eeReadByte( uint16_t addr, uint8_t* pByteValue )
{
    int retVal = 0;

    if( pBus != (i2cConnection*) NULL )
    {
fprintf(stderr, "called i2cEEPROM::eeReadByte\n"); 
fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        if( addr != I2C_CURRENT_ADDRESS )
        {
            addr += byte_offset;
        }

fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        retVal = pBus->readByte( addr, pByteValue );
    }
    else
    {
        retVal = E_I2C_FAIL;
    }

    return( retVal );
}

int i2cEEPROM::eeReadWord( uint16_t addr, uint16_t* pWordValue )
{
    int retVal = 0;

    if( pBus != (i2cConnection*) NULL )
    {
fprintf(stderr, "called i2cEEPROM::eeReadWord\n"); 
fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        if( addr != I2C_CURRENT_ADDRESS )
        {
            addr += byte_offset;
        }

fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        retVal = pBus->readWord( addr, pWordValue );
    }
    else
    {
        retVal = E_I2C_FAIL;
    }

    return( retVal );
}


/*
 ***************************************************************************
 * int i2cEEPROM::eeWrite( uint8_t* pBuffer, int amount, uint16_t addr )
 * ----------------------------------------------------
 * write amount of bytes pointed by pBuffer to address addr
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cEEPROM::eeWrite( uint16_t addr, uint8_t* pBuffer, int amount )
{
    int retVal = 0;

    if( pBus != (i2cConnection*) NULL )
    {
fprintf(stderr, "called i2cEEPROM::eeWrite\n"); 
fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        if( addr != I2C_CURRENT_ADDRESS )
        {
            addr += byte_offset;
        }

fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        retVal = pBus->writeBuf( addr, pBuffer, amount );
    }
    else
    {
        retVal = E_I2C_FAIL;
    }
    return( retVal );
}

int i2cEEPROM::eeWriteByte( uint16_t addr, uint8_t byteValue )
{
    int retVal = 0;

    if( pBus != (i2cConnection*) NULL )
    {
fprintf(stderr, "called i2cEEPROM::eeWriteByte\n"); 
fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        if( addr != I2C_CURRENT_ADDRESS )
        {
            addr += byte_offset;
        }

fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        retVal = pBus->writeByte( addr, byteValue );
    }
    else
    {
        retVal = E_I2C_FAIL;
    }
    return( retVal );
}

int i2cEEPROM::eeWriteWord( uint16_t addr, uint16_t wordValue )
{
    int retVal = 0;

    if( pBus != (i2cConnection*) NULL )
    {
fprintf(stderr, "called i2cEEPROM::eeWriteWord\n"); 
fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        if( addr != I2C_CURRENT_ADDRESS )
        {
            addr += byte_offset;
        }

fprintf(stderr, "addr = %4x [%u]\n", addr, addr);

        retVal = pBus->writeWord( addr, wordValue );
    }
    else
    {
        retVal = E_I2C_FAIL;
    }
    return( retVal );
}


