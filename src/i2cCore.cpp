/*
 ***********************************************************************
 *
 *  i2cCore.cpp - part of eeprom access project
 *
 *  Copyright (C) 2013-2019 Dreamshader (aka Dirk Schanz)
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
    char* pWord = (char*) &word;

    if( pWord[0] == 0 )
    {
// fprintf(stderr, "isBigEndian = true\n");
        return( true );
    }
    else
    {
// fprintf(stderr, "isBigEndian = false\n");
        return( false );
    }
}

/*
 ***************************************************************************
 * uint16_t makeMagic( void )
 * ----------------------------------------------------
 * calclate magic
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * 
 ***************************************************************************
*/
uint16_t makeMagic( void )
{
    uint16_t retVal;


    retVal = I2C_EE_MAGIC;
 

fprintf(stderr, "makeMagic: %4x\n", retVal );

    return( retVal );
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
    return( eeMagic == makeMagic() );
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
            *pWord = ((*pBuf & 0xff) << 8) | (*(pBuf+1) & 0xff);
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
fprintf(stderr, "Using device %s\n", devName);
        if( devFd > 0 )
        {
	    if((retVal = ioctl(devFd, I2C_FUNCS, &i2c_funcs) < 0))
	    {
perror("i2cOpen ioctl i2c funcs!");
                i2c_lastErrno = errno;
                retVal = E_I2C_IOCTL;
	    }
            else
            {
                if(ioctl(devFd, force ? I2C_SLAVE_FORCE : I2C_SLAVE, addr) < 0) 
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

    if( addr != I2C_CURRENT_ADDRESS )
    {
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

        if( retVal != bytes2Write )
        {
perror("setAddrPointer");
            i2c_lastErrno = retVal = E_I2C_FAIL;
        }
        else
        {
            i2c_lastErrno = retVal = E_I2C_SUCCESS;
        }
    }
    else
    {
        i2c_lastErrno = retVal = E_I2C_SUCCESS;
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
    int retVal;
    char wrBuffer[I2C_EEPROM_ID_LEN];

    memcpy(&wrBuffer[0], &eeMagic, 2);
    memcpy(&wrBuffer[2], &eeType, 2);

    if( byte_order_big_endian )
    {
        retVal = writeByte(0, wrBuffer[0] );
        if( retVal < 0 )
        {
            perror("writeByte pos 0");
        }
        else
        {
            retVal = writeByte(1, wrBuffer[1] );
            if( retVal < 0 )
            {
                perror("writeByte pos 1");
            }
            else
            {
                retVal = writeByte(2, wrBuffer[2] );
                if( retVal < 0 )
                {
                    perror("writeByte pos 2");
                }
                else
                {
                    retVal = writeByte(3, wrBuffer[3] );
                    if( retVal < 0 )
                    {
                        perror("writeByte pos 3");
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
            perror("writeByte pos 0");
        }
        else
        {
            retVal = writeByte(1, wrBuffer[0] );
            if( retVal < 0 )
            {
                perror("writeByte pos 1");
            }
            else
            {
                retVal = writeByte(2, wrBuffer[3] );
                if( retVal < 0 )
                {
                    perror("writeByte pos 2");
                }
                else
                {
                    retVal = writeByte(3, wrBuffer[2] );
                    if( retVal < 0 )
                    {
                        perror("writeByte pos 3");
                    }
                }
            }
        }
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

    __s32 res;
    int retVal;
    char* pDataOut;
    uint8_t rdByte;

    if( (pDataOut = (char*) pData) != NULL )
    {
        if( (retVal = setAddrPointer( fd, addr )) == E_I2C_SUCCESS )
        {

            res = read(fd, &rdByte, 1);
            if (res < 0)
            {
perror("readByte: read failed!");
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
int i2cConnection::writeByte( uint16_t addr, uint8_t data )
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
int i2cConnection::writeByte( int fd, uint16_t addr, uint8_t data )
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
perror("writeByte failed!");
        i2c_lastErrno = retVal = E_I2C_FAIL;
    } 
    else 
    {
// fprintf(stderr, "wrote %d bytes of %d.\n", retVal, bytes2Write);
        i2c_lastErrno = retVal = E_I2C_SUCCESS;
        if(i2c_write_cycle_time > 0 )
        {
// fprintf(stderr, "write cycle time = %d\n", i2c_write_cycle_time );
            usleep(i2c_write_cycle_time * 1000);
        }
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
 * int i2cConnection::writeWord( uint16_t addr, uint16_t data )
 * ----------------------------------------------------
 * write word value data to opened stream at address addr 
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::writeWord( uint16_t addr, uint16_t data )
{
    return( writeWord(i2c_devfd, addr, data) );
}

/*
 ***************************************************************************
 * int i2cConnection::writeWord( int fd, uint16_t addr, uint16_t data )
 * ----------------------------------------------------
 * write the word data to address addr of stream fd
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::writeWord( int fd, uint16_t addr, uint16_t data )
{
    int retVal;

fprintf(stderr, "%s %s %d: fd=%d, addr=%4x, data=%4x\n",
__FUNCTION__,__FILE__,__LINE__, fd, addr, data );

    i2c_lastErrno = retVal = E_I2C_SUCCESS;
fprintf(stderr, "write Word: %4x\n", data);
    return( retVal );
}


/*
 ***************************************************************************
 * int i2cConnection::readBuf( uint16_t addr, uint8_t* pBuffer, int amount )
 * ----------------------------------------------------
 * read amount number of bytes from open stream from address addr 
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::readBuf( uint16_t addr, uint8_t* pBuffer, int amount )
{
    return( readBuf( i2c_devfd, addr, pBuffer, amount ) );
}

/*
 ***************************************************************************
 * int i2cConnection::readBuf(int fd,uint16_t addr,uint8_t* pBuffer,int amount)
 * ----------------------------------------------------
 * read amount number of bytes from stream fd from address addr 
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::readBuf( int fd, uint16_t addr, uint8_t* pBuffer, int amount )
{
    int retVal;

fprintf(stderr, "%s %s %d: fd=%d, addr=%4x, pBuffer=%4x, amount=%d\n",
__FUNCTION__,__FILE__,__LINE__, fd, addr, (unsigned int) pBuffer, amount);

    if( pBuffer != (uint8_t*) NULL )
    {
        if( amount > 0 )
        {
        }
        else
        {
            i2c_lastErrno = retVal = E_I2C_INVAL_BUFLEN;
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
 * int i2cConnection::writeBuf( uint16_t addr, uint8_t* pBuffer, int amount )
 * ----------------------------------------------------
 * write amount number of data bytes pointed by pBuffer to 
 * opened stream at address addr 
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::writeBuf( uint16_t addr, uint8_t* pBuffer, int amount )
{
    return( writeBuf( i2c_devfd, addr, pBuffer, amount ) );
}

/*
 ***************************************************************************
 * int i2cConnection::writeBuf(int fd,uint16_t addr,uint8_t* pBuffer,int amount)
 * ----------------------------------------------------
 * write amount number of data bytes pointed by pBuffer to 
 * stream fd at address addr 
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 ***************************************************************************
*/
int i2cConnection::writeBuf( int fd, uint16_t addr, uint8_t* pBuffer, int amount )
{
    int retVal;

fprintf(stderr, "%s %s %d: fd=%d, addr=%4x, pBuffer=%4x, amount=%d\n",
__FUNCTION__,__FILE__,__LINE__, fd, addr, (unsigned int) pBuffer, amount);

    if( pBuffer != (uint8_t*) NULL )
    {
        if( amount > 0 )
        {
        }
        else
        {
            i2c_lastErrno = retVal = E_I2C_INVAL_BUFLEN;
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
 * int i2cConnection::check4Magic( uint16_t* pMagic, uint16_t* pType )
 * ----------------------------------------------------
 * check for existing magic, EEPROM type and detect
 * ----------------------------------------------------
 * 
 * ----------------------------------------------------
 * returns an errorcode, E_I2C_SUCCESS on success
 * the values pointed by pMagic and pType will contain the
 * detected magic and EEPROM type in case of success
 ***************************************************************************
*/
int i2cConnection::check4Magic( uint16_t* pMagic, uint16_t* pType )
{
    int retVal = E_I2C_SUCCESS;
    uint8_t i2cId[I2C_MAX_BLOCK_LEN];

    uint16_t eeMagic;
    uint16_t eeType;

    __s32 res;
    res = i2c_smbus_read_i2c_block_data( i2c_devfd, 0x00, 
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
#ifdef TALK_2_ME
            for(int i = 0; i < res; i++ )
            {
                fprintf(stderr, "initial read byte %d = %02x\n", i, i2cId[i]);
            }
#endif // TALK_2_ME

            getWordFromBuffer( &i2cId[0], &eeMagic );
            getWordFromBuffer( &i2cId[2], &eeType );
fprintf(stderr, "Initial read returns %x as magic and %u as type\n", eeMagic, eeType);

            if( isIdValid( eeMagic ) )
            {
                if( pMagic == NULL || pType == NULL )
                {
                    i2c_lastErrno = retVal = E_I2C_DATA_NULLP;
                }
                else
                {
                    *pMagic = eeMagic;
                    *pType = eeType;
                    i2c_lastErrno = retVal = E_I2C_SUCCESS;
                }
            }
            else
            {
                i2c_lastErrno = retVal = E_I2C_MAGIC_FAIL;
            }
        }
        else
        {
perror("i2c_smbus_read_i2c_block_data");
            i2c_lastErrno = retVal = E_I2C_FAIL;
        }
    }

    return( retVal );
}

