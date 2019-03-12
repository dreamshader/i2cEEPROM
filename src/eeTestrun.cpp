/*
 ***********************************************************************
 *
 *  eeTestrun.cpp - part of eeprom access project
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "i2cEEPROM.h"

#define I2C_MIN_SLAVE_ADDR     0x50
#define I2C_MAX_SLAVE_ADDR     0x57

int main( int argc, char *argv[] )
{
    int busNo = 1;
    i2cEEPROM *pDevice;

    if( (pDevice = new i2cEEPROM()) != NULL )
    {
        if( pDevice->eeInit( busNo, I2C_MIN_SLAVE_ADDR, 
                             I2C_MAX_SLAVE_ADDR ) == E_I2C_SUCCESS )

//        if( pDevice->eeInit( busNo, 0x51, I2C_EE_MAGIC, 0, false ) == E_I2C_SUCCESS )


//        if( pDevice->eeInit( busNo, 0x51, I2C_EE_NO_MAGIC, 3, false ) == E_I2C_SUCCESS )
        {
fprintf(stderr, "device init success\n");
pDevice->eeRead(1);
        }

//             if( pDevice->checkID( I2C_EE_MAGIC ) == E_I2C_MAGIC_FAIL )
//             {
// fprintf(stderr, "device magic failed\n");
//                     pDevice->initID( I2C_EE_MAGIC, EE_TYPE_24C65) ;
//             }
// 
 fprintf(stderr, "close device\n");
             pDevice->eeClose();

        delete pDevice;
    }

    return(0);

}

// class ee24LC65 : public i2cEEPROM {
