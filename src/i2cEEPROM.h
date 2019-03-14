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

#define EE_PRIVATE_HDR_LEN   6

#define EE_TYPE_24AA65              1
#define EE_TYPE_24LC65              2

#define EE_TYPE_24C65               3
#define WRITE_CYCLE_TIME_24C65      5
#define BUS_FREQUENCY_1V8_24C65   100
#define BUS_FREQUENCY_4V5_24C65   400
#define PAGE_SIZE_24C65             8
#define TOTAL_PAGES_24C65         (8 * 1024)
#define BLOCK_SIZE_24C65          I2C_MAX_BLOCK_LEN

#define EE_TYPE_24C16               4
#define WRITE_CYCLE_TIME_24C16      5
#define BUS_FREQUENCY_1V8_24C16   100
#define BUS_FREQUENCY_4V5_24C16   400
#define PAGE_SIZE_24C16             8
#define TOTAL_PAGES_24C16         (2 * 1024)
#define BLOCK_SIZE_24C16          I2C_MAX_BLOCK_LEN

class i2cEEPROM {

    private:
        i2cConnection *pBus;
        bool autoInit;
        int byte_offset;

    public:
        unsigned int ee_page_size;
        unsigned int ee_total_pages;
        unsigned int ee_block_size;

        i2cEEPROM();
        ~i2cEEPROM();

        int eeInit( int busNo, int slaveAddr, uint16_t magic, 
                    uint16_t type, bool initialize );
        int eeInit( int busNo, int minSlaveAddr, int maxSlaveAddr );

        void eeClose( void );

        int eeRead( unsigned char* pBuffer, int amount );
        int eeWrite( unsigned char* pBuffer, int amount, uint16_t addr );
};


#ifdef __cplusplus
}
#endif

#endif /* I2CEEPROM_H */

