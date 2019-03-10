/*
 ***********************************************************************
 *
 *  i2cEEPROM.cpp - part of eeprom access project
 *
 *  Copyright (C) 2013 Dreamshader (Dirk Schanz)
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



i2cEEPROM::i2cEEPROM()
{
    pBus = (i2cConnection*) NULL;
    autoInit = false;
}

i2cEEPROM::~i2cEEPROM()
{
    if( pBus != (i2cConnection*) NULL )
    {
        pBus->i2cClose();
        delete pBus;
    }
}

int i2cEEPROM::init( int busNo, int slaveAddr, uint16_t magic, uint16_t type )
{
    pBus = (i2cConnection*) NULL;
    autoInit = false;
}


int i2cEEPROM::init( int busNo, int minSlaveAddr, int maxSlaveAddr )
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
fprintf(stderr, "probing bus %d, addr %02x\n", busNo, devAddr);
            if( (retVal = pBus->i2cProbe( busNo, devAddr )) >= 0 )
            {
                devFound = true;
            }
        }
        while( !devFound && devAddr++ < maxSlaveAddr );

        if( devFound )
        {
fprintf(stderr, "found device bus %d, addr %02x\n", busNo, devAddr);
            switch( retVal )
            {
                case EE_TYPE_24AA65:
                case EE_TYPE_24LC65:
                    retVal = E_I2C_EE_NOTSUPPORTED;
                    break;
                case EE_TYPE_24C65:
                    pBus->i2c_write_cycle_time = 5;
                    pBus->i2c_bus_frequency_1V8 = 100;
                    pBus->i2c_bus_frequency_4V5 = 400;
                    page_size = 8;
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
fprintf(stderr, "device successfully opened\n");
            }


        }
    }
    else
    {
        retVal = E_I2C_FAIL;
    }

    return( retVal );
}

void i2cEEPROM::close( void )
{
    if( pBus != (i2cConnection*) NULL )
    {
        pBus->i2cClose();
    }
}


/*
 ***************************************************************************
 * int i2cEEPROM::eeRead( int amount )
 *
 * Initialize the eeprom subsystem
 * Argument is the type of eeprom as defined in eetypes.h
 *
 * returns on success a pointer to a struct i2c_eeprom_t otherwise NULL
 *
 ***************************************************************************
*/
int i2cEEPROM::eeRead( int amount )
{
    int rc = 0;
fprintf(stderr, "called i2cEEPROM::eeRead\n"); 
    return( rc );
}

