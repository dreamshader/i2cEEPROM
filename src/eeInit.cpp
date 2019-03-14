/*
 ***********************************************************************
 *
 *  eeInit.cpp - part of eeprom access project
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
 *
 * Options:
 *
 * -------------- communication port for connection --------------------
 *
 * --com devicename (same as --com=devicename resp. -c devicename)
 *
 *   Default is --com=/dev/ttyUSB0
 *
 * ---------------- baudrate for serial connection ---------------------
 *
 * --baud baudrate (same as --baud=baudrate resp. -b baudrate)
 *
 *   Default is --baud=38400
 *
 * ---------------- databit for serial connection ----------------------
 *
 * --data bits (same as --data=bits resp. -d bits)
 *            may be 5 up to 8
 *
 *   Default is --data=8
 *
 * ---------------- parity for serial connection -----------------------
 *
 * --parity parity (same as --parity=parity resp. -p parity)
 *            may be e/E (even), o/O (odd), n/N (none)
 *
 *   Default is --parity=n
 *
 * --------------- stoppbits for serial connection ---------------------
 *
 * --stop stoppbits (same as --stop=stoppbits resp. -s stoppbits)
 *            may be 1 or 2
 *
 *   Default is --stop=1
 *
 * --------------- handshake for serial connection ---------------------
 *
 * --handshake handshake (same as --handshake=handshake resp. -h handshake)
 *            may be n/N (no handshake), x/X (XON/XOFF)
 *
 *   Default is --handshake=n
 *
 * ----------------------------- help ----------------------------------
 *
 * --help     (same as -? )
 *
 *   Show options and exit
 *
 ***********************************************************************

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "i2cEEPROM.h"

#define I2C_MIN_SLAVE_ADDR     0x50
#define I2C_MAX_SLAVE_ADDR     0x57

struct _caller_options {
    uint16_t eeTypeOpt;
    uint16_t eeMagicOpt;
    uint16_t eeSlaveAddrOpt;
    uint8_t  eeBusNoOpt;
    bool     eeInfoOpt;
    bool     eeForceOpt;
    bool     eeVerboseOpt;
    bool     eeCheckOopt;
};

int main( int argc, char *argv[] )
{
    struct _caller_options param;
    i2cEEPROM *pDevice;

    resetArgs( &param );
    get_arguments( argc, argv, &param );

    if( (pDevice = new i2cEEPROM()) != NULL )
    {
        if( pDevice->eeInit( busNo, I2C_MIN_SLAVE_ADDR, 
                             I2C_MAX_SLAVE_ADDR ) == E_I2C_SUCCESS )

//        if( pDevice->eeInit( busNo, 0x51, I2C_EE_MAGIC, 0, false ) == E_I2C_SUCCESS )


//        if( pDevice->eeInit( busNo, 0x51, I2C_EE_NO_MAGIC, 3, false ) == E_I2C_SUCCESS )
        {
fprintf(stderr, "device init success\n");
unsigned char c;
pDevice->eeRead(&c, 1);
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


/* ---------------------------------------------------------------------------------
 | void help( struct serial_param_t *ctl_param, short failed )
 |
 | show options. If failed, print an error message, too
 -----------------------------------------------------------------------------------
*/

void help( void )
{
    fprintf(stderr, "HELP!\n");
    exit(0);
}

void dumpArgs( struct _caller_options *pParam )
{
    if( pParam != NULL )
    {
        fprintf(stderr, "Type .......: %d\n",   pParam->eeTypeOpt );
        fprintf(stderr, "Magic ......: %04x\n", pParam->eeMagicOpt );
        fprintf(stderr, "Slave addr. : %02x\n", pParam->eeSlaveAddrOpt );
        fprintf(stderr, "Bus No. ....: %d\n",   pParam->eeBusNoOpt );
        fprintf(stderr, "Info .......: %s\n", 
                pParam->eeInfoOpt == true ? "true" : "false" );
        fprintf(stderr, "Force ......: %s\n", 
                pParam->eeForceOpt == true ? "true" : "false" );
        fprintf(stderr, "Verbose ....: %s\n", 
                pParam->eeVerboseOpt == true ? "true" : "false" );
        fprintf(stderr, "Check only .: %s\n", 
                pParam->eeCheckOopt == true ? "true" : "false" );

    }
}


void resetArgs( struct _caller_options *pParam )
{
    if( pParam != NULL )
    {
        pParam->eeTypeOpt      = 0xffff;
        pParam->eeMagicOpt     = 0xffff;
        pParam->eeSlaveAddrOpt = 0xffff;
        pParam->eeBusNoOpt     = 0xff;
        pParam->eeInfoOpt      = false;
        pParam->eeForceOpt     = false;
        pParam->eeVerboseOpt   = false;
        pParam->eeCheckOopt    = false;
    }
}

/* ---------------------------------------------------------------------------------
 | void get_arguments ( int argc, char **argv, struct serial_param_t *ctl_param, 
 |                      short *myst)
 |
 | scan commandline for arguments an set the corresponding value
 | myst is a short pointer to a secret flag
 -----------------------------------------------------------------------------------
*/

void get_arguments ( int argc, char **argv, struct _caller_options *pParam )
{

    int failed = 0;
    int next_option;
    /* valid short options letters */
    const char* const short_options = "t:m:a:b:ifvch?";

    if( pParam != NULL )
    {

        /* valid long options */
        const struct option long_options[] = {
             { "type",    1, NULL, 't' },
             { "magic",   1, NULL, 'm' },
             { "address", 1, NULL, 'a' },
             { "bus",     1, NULL, 'b' },
             { "info",    0, NULL, 'i' },
             { "force",   0, NULL, 'f' },
             { "verbose", 0, NULL, 'v' },
             { "check",   0, NULL, 'c' },
             { "help",    0, NULL, 'h' },
            { NULL,       0, NULL,  0  }
        };
    
        resetArgs( pParam );
    
        do
        {
            next_option = getopt_long (argc, argv, short_options,
                long_options, NULL);
    
            switch (next_option) {
                case 't':
                    pParam->eeTypeOpt = atoi(optarg);
                    break;
                case 'm':
                    sscanf(optarg, "%x", &pParam->eeMagicOpt);
                    break;
                case 'a':
                    sscanf(optarg, "%x", &pParam->eeSlaveAddrOpt);
                    break;
                case 'b':
                    pParam->eeBusNoOpt = atoi(optarg);
                    break;
                case 'i':
                    pParam->eeInfoOpt = true;
                    break;
                case 'f':
                    pParam->eeForceOpt = true;
                    break;
                case 'v':
                    pParam->eeVerboseOpt = true;
                    break;
                case 'c':
                    pParam->eeCheckOopt = true;
                    break;
                case 'h':
                case '?':
                    dumpArgs( pParam );
                    // help();
                    break;
                default:
                    fprintf(stderr, "Invalid option %c! \n", next_option);
                    help();
            }
        } while (next_option != -1);
    }

}


