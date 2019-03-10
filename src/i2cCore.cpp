/*
 ***********************************************************************
 *
 *  i2cCore.cpp - part of eeprom access project
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>

#include "i2cCore.h"


/*
 ***************************************************************************
 * bool isBigEndian()
 * ----------------------------------------------------
 * detect whether system is big endian or not
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * 
 ***************************************************************************
*/
bool isBigEndian()
{
    uint16_t word = 1;
    char* pWord = (char*) &pWord;

    if( pWord[0] == 0 )
    {
fprintf(stderr, "isBigEndian = true\n");
        return( true );
    }
    else
    {
fprintf(stderr, "isBigEndian = false\n");
        return( false );
    }
}

/*
 ***************************************************************************
 * bool isIdValid( uint16_t eeMagic )
 * ----------------------------------------------------
 * detect whether Id is valid magic
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * 
 ***************************************************************************
*/
bool isIdValid( uint16_t eeMagic )
{
    return( eeMagic == I2C_EE_MAGIC );
}

/*
 ***************************************************************************
 * void getWordFromBuffer( uint8_t* pBuf, uint16_t* pWord )
 * ----------------------------------------------------
 * get word from buffer depending on byte order
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * 
 ***************************************************************************
*/
void getWordFromBuffer( uint8_t* pBuf, uint16_t* pWord )
{
    if( pBuf != NULL && pWord != NULL )
    {
        if( isBigEndian() )
        {
            return;
        }
        else
        {
            *pWord = (*pBuf & 0xff) << 8 | *(pBuf+1) & 0xff;
        }
    }
}

/*
 ***************************************************************************
 * i2cConnection::i2cConnection( void )
 * ----------------------------------------------------
 * Create an instance of type i2cConnection
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * 
 ***************************************************************************
*/
i2cConnection::i2cConnection( void )
{
    i2c_devfd = I2C_NULL_FD;
    i2c_addr  = I2C_NULL_ADDR;
    i2c_bus   = I2C_NULL_BUS;
    i2c_force = false;
    i2c_flags = I2C_NULL_FLAGS;
    byte_order_big_endian = isBigEndian();
    i2c_16bit_addressing = false;
}

/*
 ***************************************************************************
 * i2cConnection::i2cConnection(int bus, int addr, bool force, int flags)
 * ----------------------------------------------------
 * Create an instance of type i2cConnection
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * 
 ***************************************************************************
*/
i2cConnection::i2cConnection(int bus, int addr, bool force, int flags)
{
    i2c_devfd = I2C_NULL_FD;
    i2c_addr  = addr;
    i2c_bus   = bus;
    i2c_force = force;
    i2c_flags = flags;
    byte_order_big_endian = isBigEndian();
    i2c_16bit_addressing = false;
}

/*
 ***************************************************************************
 * i2cConnection::~i2cConnection()
 * ----------------------------------------------------
 * Destructor for i2cConnection instance
 * At least close file handle to bus, if any
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * 
 ***************************************************************************
*/
i2cConnection::~i2cConnection()
{
    i2cClose();
}

/*
 ***************************************************************************
 * int i2cConnection::i2cProbe( int bus, int addr )
 * ----------------------------------------------------
 * Probe for the slave device with address addr on bus bus
 * ----------------------------------------------------
 * int bus    : bus to use
 * int addr   : slave addr to connect to
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::i2cProbe( int bus, int addr )
{
    char devName[I2C_MAX_DEVNAME_LEN];
    int devFd;
    int retVal = E_I2C_SUCCESS;
    uint8_t i2cId[I2C_MAX_BLOCK_LEN];
    uint8_t i2cCheckBuf[I2C_EEPROM_ID_LEN];
    uint8_t c;

    uint16_t eeMagic;
    uint16_t eeType;

fprintf(stderr, "probe for slave %02x on bus %d\n", addr, bus );

    sprintf(devName, "/dev/i2c-%d", bus);
    devFd = open(devName, O_RDWR);

    if( devFd > 0 )
    {
        if( (retVal = ioctl(devFd, I2C_SLAVE, addr)) < 0) 
        {
perror("i2cProbe ioctl");
            i2c_lastErrno = errno;
            retVal = E_I2C_IOCTL;
        }
        else
        {
//            retVal = readByte( devFd, 0, &c );

            __s32 res;
            res = i2c_smbus_read_i2c_block_data( devFd, 0x00, 
                                                 I2C_MAX_BLOCK_LEN, i2cId );
            if( res < 0 )
            {
perror("i2c_smbus_read_i2c_block_data");
                i2c_lastErrno = retVal = E_I2C_FAIL;
            }
            else
            {

                if( res == I2C_MAX_BLOCK_LEN )
                {
                    for(int i = 0; i < res; i++ )
                    {
                        fprintf(stderr, "initial read byte %d = %02x\n", 
                                         i, i2cId[i]);
                    }

                    getWordFromBuffer( &i2cId[0], &eeMagic );
                    getWordFromBuffer( &i2cId[2], &eeType );
fprintf(stderr, "Initial read returns %u as magic and %u as type\n", eeMagic, eeType);

                    if( isIdValid( eeMagic ) )
                    {
fprintf(stderr, "Found valid signature!\n");
                        int rc;
                        char i;
                        for( rc = 0, i = 0; rc > 0 && i < I2C_EEPROM_ID_LEN; i++ )
                        {
                            if( (rc = write( devFd, &i, 1 )) > 0 )
                            {
                                rc = read( devFd, &i2cCheckBuf[i], 1 );
                            }
                        }

                        for( i = 0; i < I2C_EEPROM_ID_LEN; i++ )
                        {
fprintf(stderr, "CheckBuffer pos %d is %02x\n", i, i2cCheckBuf[i] );
                        }

                        if( memcmp( i2cId, i2cCheckBuf, 
                                    I2C_EEPROM_ID_LEN ) != 0 )
                        {
fprintf(stderr, " CheckBuffer does not match block buffer. Assuming 16 bit adressing\n");
                            i2c_16bit_addressing = true;
                            i2c_lastErrno = E_I2C_SUCCESS;
                        }
                        else
                        {
                            i2c_16bit_addressing = false;
                            i2c_lastErrno = E_I2C_SUCCESS;
                        }

                        retVal = (int) eeType;
                    }
                    else
                    {
                        i2c_lastErrno = retVal = E_I2C_MAGIC_FAIL;
                    }
                }
                else
                {
                    i2c_lastErrno = retVal = E_I2C_FAIL;
                }
            }
        }
        close( devFd );
    }
    else
    {
        i2c_lastErrno = errno;
        retVal = E_I2C_NODEV;
    }

    return( retVal );
}

#if 0
	// get funcs list
	if((r = ioctl(fd, I2C_FUNCS, &funcs) < 0))
	{
		fprintf(stderr, "Error eeprom_open: %s\n", strerror(errno));
		return -1;
	}
#define CHECK_I2C_FUNC( var, label ) \
	do { 	if(0 == (var & label)) { \
		fprintf(stderr, "\nError: " \
			#label " function is required. Program halted.\n\n"); \
		exit(1); } \
	} while(0);

	
	// check for req funcs
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_BYTE );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_BYTE );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_BYTE_DATA );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_BYTE_DATA );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_READ_WORD_DATA );
	CHECK_I2C_FUNC( funcs, I2C_FUNC_SMBUS_WRITE_WORD_DATA );
#endif

/*
 ***************************************************************************
 * int i2cConnection::int i2cOpen( int bus, int addr, 
 *                                 bool force, int flags )
 * ----------------------------------------------------
 * Open connection to i2c bus
 * ----------------------------------------------------
 * int bus    : bus to use
 * int addr   : slave addr to connect to
 * bool force : force use of address addr
 * int flags  : opening flags, e.g. O_RDWR
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::i2cOpen(int bus, int addr, bool force, int flags)
{
    char devName[I2C_MAX_DEVNAME_LEN];
    int devFd;
    int retVal = E_I2C_SUCCESS;

    if( i2c_devfd > 0 && i2c_devfd != I2C_NULL_FD )
    {
        retVal = E_I2C_IN_USE;
    }
    else
    {
        sprintf(devName, "/dev/i2c-%d", bus);
        devFd = open(devName, flags);

        if( devFd > 0 )
        {
            if (ioctl(devFd, force ? I2C_SLAVE_FORCE : I2C_SLAVE, addr) < 0) 
            {
                i2c_lastErrno = errno;
                retVal = E_I2C_IOCTL;
            }
            else
            {
                i2c_devfd = devFd;
                i2c_bus   = bus;
                i2c_addr  = addr;
                i2c_force = force;
                i2c_flags = flags;
    
                i2c_lastErrno = retVal = E_I2C_SUCCESS;
            }
        }
        else
        {
            i2c_lastErrno = errno;
            retVal = E_I2C_NODEV;
        }
    }

    return( retVal );
}

/*
 ***************************************************************************
 * int i2cConnection::int i2cOpen( void )
 * ----------------------------------------------------
 * Open connection to i2c bus
 * The parameters to use must be set by instantiation
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::i2cOpen( void )
{
    int retVal = E_I2C_SUCCESS;

    if( i2c_bus   != I2C_NULL_BUS &&
        i2c_addr  != I2C_NULL_ADDR &&
        i2c_flags != I2C_NULL_FLAGS )
    {
        retVal = i2cOpen( i2c_bus, i2c_addr, i2c_force, i2c_flags );
    }
    else
    {
        if( i2c_addr  == I2C_NULL_ADDR )
        {
            retVal = E_I2C_NULL_ADDR;
        }
        else
        {
            if( i2c_bus   == I2C_NULL_BUS )
            {
                retVal = E_I2C_NULL_BUS;
            }
            else
            {
                if( i2c_flags == I2C_NULL_FLAGS )
                {
                    retVal = E_I2C_NULL_FLAGS;
                }
                else
                {
                    retVal = E_I2C_OOPS;
fprintf(stderr, "oops: %s -> file %s line %d\n", __FUNCTION__, __FILE__, __LINE__);
                }
            }
        }
    }

    return( retVal );
}

/*
 ***************************************************************************
 * int i2cConnection::i2cClose( void )
 * ----------------------------------------------------
 * Close current connection to i2c bus
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::i2cClose( void )
{
    int retVal = E_I2C_SUCCESS;

    if( i2c_devfd > 0 && i2c_devfd != I2C_NULL_FD )
    {
        if( (retVal = close( i2c_devfd )) == 0 )
        {
            i2c_lastErrno = retVal = E_I2C_SUCCESS;
        }
        else
        {
            i2c_lastErrno = errno;
        }

        i2c_devfd = I2C_NULL_FD;
        i2c_addr  = I2C_NULL_ADDR;
        i2c_bus   = I2C_NULL_BUS;
        i2c_force = false;
        i2c_flags = I2C_NULL_FLAGS;
    }
    else
    {
        retVal = E_I2C_NULL_FD;
    }

    return( retVal );
}






/*
 ***************************************************************************
 * int i2cConnection::readWord( uint16_t addr, void* pData )
 * ----------------------------------------------------
 * read a word value from opened stream at address addr 
 * into storage pointed by pData
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::readWord( uint16_t addr, void* pData )
{
    return( readWord( i2c_devfd, addr, pData ) );
}

/*
 ***************************************************************************
 * int i2cConnection::readWord( int fd, uint16_t addr, void* pData )
 * ----------------------------------------------------
 * read a word value from stream fd at address addr 
 * into storage pointed by pData
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::readWord( int fd, uint16_t addr, void* pData )
{

    int retVal;
    char buf[2];
    uint16_t* pDataOut;

    if( (pDataOut = (uint16_t*) pData) != NULL )
    {
        if( byte_order_big_endian )
        {
            if( (retVal = readByte(fd, addr, &buf[0])) == E_I2C_SUCCESS )
            {
                if( (retVal = readByte(fd, addr+1, &buf[1])) == E_I2C_SUCCESS )
                {
                    *pDataOut = * (uint16_t*)&buf[0];
                }
            }
        }
        else
        {
            if( (retVal = readByte(fd, addr, &buf[1])) == E_I2C_SUCCESS )
            {
                if( (retVal = readByte(fd, addr+1, &buf[0])) == E_I2C_SUCCESS )
                {
                    *pDataOut = * (uint16_t*)&buf[0];
                }
            }
        }
    }
    else
    {
        i2c_lastErrno = retVal = E_I2C_DATA_NULLP;
    }

    return( retVal );
}

/*
 ***************************************************************************
 * int i2cConnection::setAddrPointer( int fd, uint16_t addr )
 * ----------------------------------------------------
 * write address byte(s) to bus related to fd
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::setAddrPointer( int fd, uint16_t addr )
{
//    __s32 res;
    int retVal;
    uint8_t dataBuf[8];
    int bytes2Write;
    dataBuf[0] = (addr >> 8) & 0x00ff;
    dataBuf[1] = addr & 0x00ff;

    if( i2c_16bit_addressing )
    {
// 16 bit adressing
        bytes2Write = 2;
        retVal = write( fd, dataBuf, bytes2Write);
    }
    else
    {
// 8 bit adressing
        bytes2Write = 1;
        retVal = write( fd, dataBuf, bytes2Write);
    }

//    if( res < 0 )
    if( retVal != bytes2Write )
    {
perror("setAddrPointer");
        i2c_lastErrno = retVal = E_I2C_FAIL;
    }
    else
    {
        i2c_lastErrno = retVal = E_I2C_SUCCESS;
    }

    return( retVal );
}

/*
 ***************************************************************************
 * int i2cConnection::readByte( uint16_t addr, void* pData )
 * ----------------------------------------------------
 * read a byte value from opened stream at address addr 
 * into storage pointed by pData
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::readByte( uint16_t addr, void* pData )
{
    return( readByte(i2c_devfd, addr, pData) );
}

/*
 ***************************************************************************
 * int i2cConnection::setAddrPointer( int fd, uint16_t addr )
 * ----------------------------------------------------
 * read a byte value from stream fd at address addr 
 * into storage pointed by pData
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::readByte( int fd, uint16_t addr, void* pData )
{

    __u8 reg = 0x00; /* Device register to access */
    __s32 res;
    int retVal;
    char* pDataOut;
    char rdByte;

    if( (pDataOut = (char*) pData) != NULL )
    {
        if( (retVal = setAddrPointer( fd, addr )) == E_I2C_SUCCESS )
        {

            res = read(fd, &rdByte, 1);
            if (res < 0)
            {
fprintf(stderr, "read failed!, res = %ld\n", res);
                i2c_lastErrno = retVal = E_I2C_FAIL;
            } 
            else 
            {
                /* res contains the read byte */

//                *pDataOut = res & 0xff;
                *pDataOut = rdByte;
                i2c_lastErrno = retVal = E_I2C_SUCCESS;
            }
        }
    }
    else
    {
        i2c_lastErrno = retVal = E_I2C_DATA_NULLP;
    }

    return( retVal );
}

/*
 ***************************************************************************
 * int i2cConnection::writeByte( char addr, char data )
 * ----------------------------------------------------
 * write the byte data to address addr of current stream
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::writeByte( uint16_t addr, char data )
{
    return( writeByte(i2c_devfd, addr, data) );
}

/*
 ***************************************************************************
 * int i2cConnection::writeByte( int fd, uint16_t addr, char data )
 * ----------------------------------------------------
 * write the byte data to address addr of stream fd
 * into storage pointed by pData
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::writeByte( int fd, uint16_t addr, char data )
{

    int retVal;
    int bytes2Write;
    uint8_t dataBuf[8];

    dataBuf[0] = (addr >> 8) & 0x00ff;
    dataBuf[1] = addr & 0x00ff;
    dataBuf[2] = data;

    if( i2c_16bit_addressing )
    {
        // 16 bit adressing
        bytes2Write = 3;
        retVal = write( fd, dataBuf, bytes2Write);
    }
    else
    {
        // 8 bit adressing
        bytes2Write = 2;
        retVal = write( fd, &dataBuf[1], bytes2Write);
    }

    if(retVal != bytes2Write)
    {
fprintf(stderr, "writeByte failed! Wrote %d bytes of %d.\n", retVal, bytes2Write);
        i2c_lastErrno = retVal = E_I2C_FAIL;
    } 
    else 
    {
// fprintf(stderr, "wrote %d bytes of %d.\n", retVal, bytes2Write);
        i2c_lastErrno = retVal = E_I2C_SUCCESS;
        if(i2c_write_cycle_time > 0 )
        {
fprintf(stderr, "write cycle time = %d\n", i2c_write_cycle_time );
            usleep(i2c_write_cycle_time * 1000);
        }

    }

    return( retVal );
}

/*
 ***************************************************************************
 * int i2cConnection::checkID( void )
 * ----------------------------------------------------
 * check for magic and eeprom Id
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::checkID( uint16_t eeMagic )
{

    int retVal;
    uint16_t magic;
    uint16_t eeId;

    int write_cycle_time;
    int bus_frequency_1V8;
    int bus_frequency_4V5;
    int page_size;

    if( (retVal = readWord( (uint16_t) 0, &magic )) == E_I2C_SUCCESS )
    {
        retVal = readWord( (uint16_t) 2, &eeId );
        if( retVal == E_I2C_SUCCESS )
        {
fprintf(stderr, "checkID %02x\n", magic);
fprintf(stderr, "checkID %02x\n", eeId);
            if( magic == eeMagic )
            {
fprintf(stderr, "magic MATCH!\n");
                switch( eeId )
                {
                    case EE_TYPE_24AA65:
                    case EE_TYPE_24LC65:
                        retVal = E_I2C_EE_NOTSUPPORTED;
                        break;
                    case EE_TYPE_24C65:
                        write_cycle_time = 5;
                        bus_frequency_1V8 = 100;
                        bus_frequency_4V5 = 400;
                        page_size = 8;
                        break;
                    default:
                        retVal = E_I2C_EE_INVAL_ID;
                        break;
                }
            }
            else
            {
fprintf(stderr, "magic check FAILED!\n");
                retVal = E_I2C_MAGIC_FAIL;
            }
        }
    }

    return( retVal );
}

/*
 ***************************************************************************
 * int i2cConnection::initID( void )
 * ----------------------------------------------------
 * write current magic and eeprom Id
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::initID( uint16_t eeMagic, uint16_t eeType )
{

    __u8 reg = 0x00; /* Device register to access */
    __s32 res;
    int retVal;
    char wrBuffer[I2C_EEPROM_ID_LEN];

    memcpy(&wrBuffer[0], &eeMagic, 2);
    memcpy(&wrBuffer[2], &eeType, 2);

    if( byte_order_big_endian )
    {
        retVal = writeByte(0, wrBuffer[0] );
        if( retVal < 0 )
        {
            perror("writeByte");
        }
        else
        {
            retVal = writeByte(1, wrBuffer[1] );
            if( retVal < 0 )
            {
                perror("writeByte");
            }
            else
            {
                retVal = writeByte(2, wrBuffer[2] );
                if( retVal < 0 )
                {
                    perror("writeByte");
                }
                else
                {
                    retVal = writeByte(3, wrBuffer[3] );
                    if( retVal < 0 )
                    {
                        perror("writeByte");
                    }
                }
            }
        }
    }
    else
    {
        retVal = writeByte(0, wrBuffer[1] );
        if( retVal < 0 )
        {
            perror("writeByte");
        }
        else
        {
            retVal = writeByte(1, wrBuffer[0] );
            if( retVal < 0 )
            {
                perror("writeByte");
            }
            else
            {
                retVal = writeByte(2, wrBuffer[3] );
                if( retVal < 0 )
                {
                    perror("writeByte");
                }
                else
                {
                    retVal = writeByte(3, wrBuffer[2] );
                    if( retVal < 0 )
                    {
                        perror("writeByte");
                    }
                }
            }
        }
    }
    return( retVal );
}





#ifdef NEVERDEF


Full interface description
==========================

The following IOCTLs are defined:

ioctl(file, I2C_SLAVE, long addr)
  Change slave address. The address is passed in the 7 lower bits of the
  argument (except for 10 bit addresses, passed in the 10 lower bits in this
  case).

ioctl(file, I2C_TENBIT, long select)
  Selects ten bit addresses if select not equals 0, selects normal 7 bit
  addresses if select equals 0. Default 0.  This request is only valid
  if the adapter has I2C_FUNC_10BIT_ADDR.

ioctl(file, I2C_PEC, long select)
  Selects SMBus PEC (packet error checking) generation and verification
  if select not equals 0, disables if select equals 0. Default 0.
  Used only for SMBus transactions.  This request only has an effect if the
  the adapter has I2C_FUNC_SMBUS_PEC; it is still safe if not, it just
  doesn t have any effect.

ioctl(file, I2C_FUNCS, unsigned long *funcs)
  Gets the adapter functionality and puts it in *funcs.

ioctl(file, I2C_RDWR, struct i2c_rdwr_ioctl_data *msgset)
  Do combined read/write transaction without stop in between.
  Only valid if the adapter has I2C_FUNC_I2C.  The argument is
  a pointer to a

  struct i2c_rdwr_ioctl_data {
      struct i2c_msg *msgs;  /* ptr to array of simple messages */
      int nmsgs;             /* number of messages to exchange */
  }

  The msgs[] themselves contain further pointers into data buffers.
  The function will write or read data to or from that buffers depending
  on whether the I2C_M_RD flag is set in a particular message or not.
  The slave address and whether to use ten bit address mode has to be
  set in each message, overriding the values set with the above ioctl s.

ioctl(file, I2C_SMBUS, struct i2c_smbus_ioctl_data *args)
  If possible, use the provided i2c_smbus_* methods described below instead
  of issuing direct ioctls.

You can do plain i2c transactions by using read(2) and write(2) calls.
You do not need to pass the address byte; instead, set it through
ioctl I2C_SLAVE before you try to access the device.

You can do SMBus level transactions (see documentation file smbus-protocol
for details) through the following functions:
  __s32 i2c_smbus_write_quick(int file, __u8 value);
  __s32 i2c_smbus_read_byte(int file);
  __s32 i2c_smbus_write_byte(int file, __u8 value);
  __s32 i2c_smbus_read_byte_data(int file, __u8 command);
  __s32 i2c_smbus_write_byte_data(int file, __u8 command, __u8 value);
  __s32 i2c_smbus_read_word_data(int file, __u8 command);
  __s32 i2c_smbus_write_word_data(int file, __u8 command, __u16 value);
  __s32 i2c_smbus_process_call(int file, __u8 command, __u16 value);
  __s32 i2c_smbus_read_block_data(int file, __u8 command, __u8 *values);
  __s32 i2c_smbus_write_block_data(int file, __u8 command, __u8 length,
                                   __u8 *values);
All these transactions return -1 on failure; you can read errno to see
what happened. The 'write' transactions return 0 on success; the
'read' transactions return the read value, except for read_block, which
returns the number of values read. The block buffers need not be longer
than 32 bytes.

The above functions are made available by linking against the libi2c library,
which is provided by the i2c-tools project.  See:
https://git.kernel.org/pub/scm/utils/i2c-tools/i2c-tools.git/.



#endif // NEVERDEF


