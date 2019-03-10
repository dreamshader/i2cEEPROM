/*
 ***********************************************************************
 *
 *  i2cCore.h - part of eeprom access project
 *
 *  Copyright (C) 2013 Dreamshader (aka Dirk Schanz)
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

#ifndef I2CCORE_H
#define I2CCORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define I2C_TRACE_IN(f)	fprintf(stderr, "Enter: %s -> file %s line %d\n", __FUNCTION__, __FILE__, __LINE__);
#define I2C_TRACE_OUT(f,r)	fprintf(stderr, "Leave: %s -> file %s line %d [rc=%d]\n", __FUNCTION__, __FILE__, __LINE__, r);


#define E_I2C_SUCCESS              0
#define E_I2C_FAIL                -1
#define E_I2C_NULL                -3
#define E_I2C_MEM                 -4
#define E_I2C_SUPP                -5
#define E_I2C_NODEV               -6
#define E_I2C_IOCTL               -7
#define E_I2C_VERIFY              -8
#define E_I2C_IN_USE              -9
#define E_I2C_NULL_FD            -10
#define E_I2C_NULL_ADDR          -11
#define E_I2C_NULL_BUS           -12
#define E_I2C_NULL_FLAGS         -13
#define E_I2C_OOPS               -14
#define E_I2C_DATA_NULLP         -15
#define E_I2C_MAGIC_FAIL         -16
#define E_I2C_EE_INVAL_ID        -17
#define E_I2C_EE_NOTSUPPORTED    -18


#define I2C_MAX_DEVNAME_LEN       30
#define I2C_NULL_FD               -1
#define I2C_NULL_ADDR              0
#define I2C_NULL_BUS              -1
#define I2C_NULL_FLAGS             0
#define I2C_16BIT_ADDRESS         16
#define I2C_8BIT_ADDRESS           8
#define I2C_MAX_BLOCK_LEN         32

#define I2C_EE_MAGIC          0xf4e1

#define I2C_EEPROM_ID_LEN         4
#define EE_TYPE_24AA65            1
#define EE_TYPE_24LC65            2
#define EE_TYPE_24C65             3

class i2cConnection {

    public:
        bool byte_order_big_endian;

        int i2c_devfd;
        int i2c_bus;
        int i2c_addr;
        bool i2c_force;
        int i2c_flags;
        int i2c_lastErrno;
        bool i2c_16bit_addressing;
        int i2c_write_cycle_time = 5;
        int i2c_bus_frequency_1V8 = 100;
        int i2c_bus_frequency_4V5 = 400;

// write_cycle_time = 5 ms
// bus_frequency 1V8 = 100 kHz
// bus_frequency 4V5 = 400 kHz
// 8-byte page

        uint16_t i2c_eeMagic = I2C_EE_MAGIC;
        uint16_t i2c_eeType = EE_TYPE_24C65;

        i2cConnection( void );
        i2cConnection( int bus, int addr, bool force, int flags );

        ~i2cConnection();

        int i2cProbe( int bus, int addr );

        int i2cOpen( int bus, int addr, bool force, int flags );
        int i2cOpen( void );

        int i2cClose( void );

        int setAddrPointer( int fd, uint16_t addr );

        int readWord( int fd, uint16_t addr, void* pData );
        int readWord( uint16_t addr, void* pData );

        int readByte( int fd, uint16_t addr, void* pData );
        int readByte( uint16_t addr, void* pData );

        int writeByte( int fd, uint16_t addr, char data );
        int writeByte( uint16_t addr, char data );

        int checkID( uint16_t eeMagic );
        int initID( uint16_t eeMagic, uint16_t eeType  );


};


#ifdef __cplusplus
}
#endif

#endif /* I2CCORE_H */

