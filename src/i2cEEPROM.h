/*
 ***********************************************************************
 *
 *  i2cEEPROM.h - part of eeprom access project
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

#ifndef I2CEEPROM_H
#define I2CEEPROM_H

#include <stdint.h>
#include "i2cCore.h"

#ifdef __cplusplus
extern "C" {
#endif


#define E_EE_SUCCESS         0
#define E_EE_FAIL           -1
#define E_EE_NULL           -3
#define E_EE_MEM            -4
#define E_EE_SUPP           -5
#define E_EE_NODEV          -6
#define E_EE_IOCTL          -7
#define E_EE_VERIFY         -8


class i2cEEPROM {

    private:
        i2cConnection *pBus;
        bool autoInit;

    public:

        int page_size;

        i2cEEPROM();
        ~i2cEEPROM();
        int init( int busNo, int slaveAddr, uint16_t magic, uint16_t type );
        int init( int busNo, int minSlaveAddr, int maxSlaveAddr );

        void close( void );

        int eeRead( int amount );
};


#ifdef __cplusplus
}
#endif

#endif /* I2CEEPROM_H */

