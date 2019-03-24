/*
 ***********************************************************************
 *
 *  eeInit.cpp - part of eeprom access project
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
 *
 * Options:
 *
 * ------------------------- EEPROM type -------------------------------
 *
 * --type <type> (same as --type=<type> resp. -t <type>)
 *
 * ------------------------- magic word --------------------------------
 *
 * --magic <word> (same as --magic=<word> resp. -m <word>)
 *
 * --------------------- I2C slave address -----------------------------
 *
 * --address <addr> (same as --address=<addr> resp. -a <addr>)
 *
 * ---------------------- I2C bus number -------------------------------
 *
 * --bus <bus #> (same as --bus=<bus #> resp. -b <bus #>)
 *
 * ---------------------- list known types -----------------------------
 *
 * --list (same as -l)
 *
 * ------------------------ display info -------------------------------
 *
 * --info (same as -i)
 *
 * ------------------------ force action--------------------------------
 *
 * --force (same as -f)
 *
 * ------------------------ verbose output------------------------------
 *
 * --verbose (same as -v)
 *
 * ---------------------------- check ----------------------------------
 *
 * --check (same as -c)
 *
 * ---------------------------- help -----------------------------------
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
#include "i2cEEPROM.h"

#define OPTION_TYPE_SET     0x0001
#define OPTION_MAGIC_SET    0x0002
#define OPTION_ADDR_SET     0x0004
#define OPTION_BUS_SET      0x0008
#define OPTION_LIST_SET     0x0010
#define OPTION_INFO_SET     0x0020
#define OPTION_FORCE_SET    0x0040
#define OPTION_VERBOSE_SET  0x0080
#define OPTION_CHECK_SET    0x0100

struct _caller_options {
    uint16_t eeTypeOpt;
    uint16_t eeMagicOpt;
    uint16_t eeSlaveAddrOpt;
    uint16_t eeBusNoOpt;
    uint16_t eeOptFlags;
    bool     eeListOpt;
    bool     eeInfoOpt;
    bool     eeForceOpt;
    bool     eeVerboseOpt;
    bool     eeCheckOopt;
};

/* -------------------------------------------------------------------------
 | void help( void )
 |
 | print help screen and exit
 ---------------------------------------------------------------------------
*/
void help( void )
{
    fprintf(stderr, "HELP!\n");
    exit(0);
}

/* -------------------------------------------------------------------------
 | void dumpArgs( struct _caller_options *pParam )
 |
 | print given option and argument settings
 ---------------------------------------------------------------------------
*/
void dumpArgs( struct _caller_options *pParam )
{
    if( pParam != NULL )
    {
        fprintf(stderr, "Type .......: %d\n",   pParam->eeTypeOpt );
        fprintf(stderr, "Magic ......: %04x\n", pParam->eeMagicOpt );
        fprintf(stderr, "Slave addr. : %04x\n", pParam->eeSlaveAddrOpt );
        fprintf(stderr, "Bus No. ....: %d\n",   pParam->eeBusNoOpt );
        fprintf(stderr, "Flags ......: %04x\n", pParam->eeOptFlags);
        fprintf(stderr, "List .......: %s\n", 
                pParam->eeListOpt == true ? "true" : "false" );
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


/* -------------------------------------------------------------------------
 | void resetArgs( struct _caller_options *pParam )
 |
 | reset options to defaults
 ---------------------------------------------------------------------------
*/

void resetArgs( struct _caller_options *pParam )
{
    if( pParam != NULL )
    {
        pParam->eeTypeOpt      = 0xffff;
        pParam->eeMagicOpt     = 0xffff;

        pParam->eeMagicOpt = makeMagic();
        pParam->eeOptFlags = OPTION_MAGIC_SET;

        pParam->eeSlaveAddrOpt = 0xffff;
        pParam->eeBusNoOpt     = 0xff;
        pParam->eeOptFlags     = 0;
        pParam->eeListOpt      = false;
        pParam->eeInfoOpt      = false;
        pParam->eeForceOpt     = false;
        pParam->eeVerboseOpt   = false;
        pParam->eeCheckOopt    = false;
    }
}

/* -------------------------------------------------------------------------
 | void get_arguments(int argc, char **argv, struct _caller_options *pParam)
 |
 | scan commandline for arguments an set the corresponding value
 ---------------------------------------------------------------------------
*/
void get_arguments ( int argc, char **argv, struct _caller_options *pParam )
{

    int failed = 0;
    int next_option;
    /* valid short options letters */
    const char* const short_options = "t:m:a:b:lifvch?";
    unsigned long scanValue;

    if( pParam != NULL )
    {

        /* valid long options */
        const struct option long_options[] = {
             { "type",    1, NULL, 't' },
             { "magic",   1, NULL, 'm' },
             { "address", 1, NULL, 'a' },
             { "bus",     1, NULL, 'b' },
             { "list",    0, NULL, 'l' },
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
                    pParam->eeOptFlags |= OPTION_TYPE_SET;
                    break;
                case 'm':
                    sscanf(optarg, "%x", &scanValue);
                    pParam->eeMagicOpt = (uint16_t) scanValue;
                    pParam->eeOptFlags |= OPTION_MAGIC_SET;
                    break;
                case 'a':
                    sscanf(optarg, "%x", &scanValue);
                    pParam->eeSlaveAddrOpt = (uint16_t) scanValue;
                    pParam->eeOptFlags |= OPTION_ADDR_SET;
                    break;
                case 'b':
                    pParam->eeBusNoOpt = atoi(optarg);
                    pParam->eeOptFlags |= OPTION_BUS_SET;
                    break;
                case 'l':
                    pParam->eeListOpt = true;
                    pParam->eeOptFlags |= OPTION_LIST_SET;
                    break;
                case 'i':
                    pParam->eeInfoOpt = true;
                    pParam->eeOptFlags |= OPTION_INFO_SET;
                    break;
                case 'f':
                    pParam->eeForceOpt = true;
                    pParam->eeOptFlags |= OPTION_FORCE_SET;
                    break;
                case 'v':
                    pParam->eeVerboseOpt = true;
                    pParam->eeOptFlags |= OPTION_VERBOSE_SET;
                    break;
                case 'c':
                    pParam->eeCheckOopt = true;
                    pParam->eeOptFlags |= OPTION_CHECK_SET;
                    break;
                case 'h':
                case '?':
                    dumpArgs( pParam );
                    help();
                    // help();
                    break;
                default:
                    break;
            }
        } while (next_option != -1);
    }

}

#define ERROR_NULL          -1
#define ERROR_NO_TYPE_MAGIC -2
#define ERROR_I2C_PARAM     -3
#define USER_ABORT          -4

int listKnownTypes( i2cEEPROM *pDevice, struct _caller_options *pParam )
{
    int retVal = 0;
    int eeType;

    for( eeType = 0; eeType < EE_TYPE_MAX_TYPE; eeType++ )
    {
        switch( eeType )
        {  
            case EE_TYPE_24AA65:
//                printf("Type %2d:\n", eeType);
//                printf("\t%s\n", EE_NAMES_24AA65);
                printf("Type %2d: %s\n", eeType, EE_NAMES_24AA65);
                break;
            case EE_TYPE_24LC65:
//                printf("Type %2d:\n", eeType);
//                printf("\t%s\n", EE_NAMES_24LC65);
                printf("Type %2d: %s\n", eeType, EE_NAMES_24LC65);
                break;
            case EE_TYPE_24C65:
//                printf("Type %2d:\n", eeType);
//                printf("\t%s\n", EE_NAMES_24C65);
                printf("Type %2d: %s\n", eeType, EE_NAMES_24C65);
                break;
            case EE_TYPE_24C16:
//                printf("Type %2d:\n", eeType);
//                printf("\t%s\n", EE_NAMES_24C16);
                printf("Type %2d: %s\n", eeType, EE_NAMES_24C16);
                break;
            default:
                break;
        }
    }
    return( retVal );
}



int check4EEPROM( i2cEEPROM *pDevice, struct _caller_options *pParam )
{
    int retVal = 0;

    if( pDevice != NULL && pParam != (struct _caller_options*) NULL )
    {
        if( (pParam->eeOptFlags & (OPTION_ADDR_SET|OPTION_BUS_SET)) ==
                (OPTION_ADDR_SET|OPTION_BUS_SET) )
        {
            if( pDevice->eeOpen( pParam->eeBusNoOpt, 
                                 pParam->eeSlaveAddrOpt ) == E_EE_SUCCESS )
            {
// if( (pParam->eeOptFlags & OPTION_TYPE_SET) == OPTION_TYPE_SET )
// if( (pParam->eeOptFlags & OPTION_MAGIC_SET) == OPTION_MAGIC_SET )
                    // pParam->eeTypeOpt
                    // pParam->eeMagicOpt
                    // pParam->eeSlaveAddrOpt
                    // pParam->eeBusNoOpt
                    // pParam->eeListOpt
                    // pParam->eeInfoOpt
                    // pParam->eeForceOpt
                    // pParam->eeVerboseOpt
                    // pParam->eeCheckOopt

                fprintf(stderr, "close device\n");
                pDevice->eeClose();
            }
        }
    }
    else
    {
        retVal = ERROR_NULL;
    }
    return( retVal );
}


int infoOnEEPROM( i2cEEPROM *pDevice, struct _caller_options *pParam )
{
    int retVal = 0;
    uint16_t rdMagic;
    uint16_t rdType;

    if( pDevice != NULL && pParam != (struct _caller_options*) NULL )
    {
        if( (pParam->eeOptFlags & (OPTION_ADDR_SET|OPTION_BUS_SET)) ==
                (OPTION_ADDR_SET|OPTION_BUS_SET) )
        {
            if( retVal = pDevice->eeOpen( pParam->eeBusNoOpt, 
                                 pParam->eeSlaveAddrOpt ) == E_EE_SUCCESS )
            {
                if( (retVal = pDevice->eeTypeDetect( &rdMagic, &rdType )) == 
                    E_EE_SUCCESS)
                {
                    if( (retVal = pDevice->eeTypeSet( rdType )) ==
                        E_EE_SUCCESS )
                    {
fprintf(stderr, "Got magic = %4x and type = %d\n", rdMagic, rdType);
                        pDevice->eeInfo();
                    }
                    else
                    {
fprintf(stderr, "type %d invalid?\n", rdType);
fprintf(stderr, "retVal = %d\n", retVal);
                    }
                }
                else
                {
fprintf(stderr, "there seems to be no valid ID!\n");
fprintf(stderr, "retVal = %d\n", retVal);
                }

                fprintf(stderr, "close device\n");
                pDevice->eeClose();
            }
        }
    }
    else
    {
        retVal = ERROR_NULL;
    }
    return( retVal );
}

bool confirm_YN(char defaultValue)
{
    bool retVal = false;
    char answer;

    printf("[y]es/[n]o: (%c) ", defaultValue); 
    answer = fgetc(stdin);

// printf("\nanswer -> %c[%x]\n", answer, answer);

    if( answer == 0x0a )
    {
        answer = defaultValue;
    }

    switch( answer )
    {
        case 'y':
        case 'Y':
            retVal = true;
            break;
        default:
            retVal = false;
            break;
    }

    return( retVal );
}

int initializeEEPROM( i2cEEPROM *pDevice, struct _caller_options *pParam )
{
    int retVal = 0;

    if( pDevice != NULL && pParam != (struct _caller_options*) NULL )
    {
        if( (pParam->eeOptFlags & (OPTION_ADDR_SET|OPTION_BUS_SET)) ==
                (OPTION_ADDR_SET|OPTION_BUS_SET) )
        {
            if( (pParam->eeOptFlags & OPTION_TYPE_SET) == OPTION_TYPE_SET )
            {
                if( (retVal = pDevice->eeOpen( pParam->eeBusNoOpt, 
                                 pParam->eeSlaveAddrOpt )) == E_EE_SUCCESS )
                {
                    if( (retVal = pDevice->eeTypeSet( pParam->eeTypeOpt )) ==
                        E_EE_SUCCESS )
                    {
                        if( !pParam->eeForceOpt )
                        {
printf("Write magic=%x and type=%x to EEPROM at address=%x on i2c-bus %d?\n", 
       pParam->eeMagicOpt, pParam->eeTypeOpt, pParam->eeSlaveAddrOpt, 
       pParam->eeBusNoOpt );

                            if( confirm_YN('n') )
                            {
                                retVal = pDevice->eeInit();
                            }
                            else
                            {
                                printf("\nabgebrochen!\n");
                                retVal = USER_ABORT;
                            }
                        }
                        else
                        {
                            retVal = pDevice->eeInit();
                        }
                    }

                    pDevice->eeClose();
                }
            }
            else
            {
                retVal = ERROR_NO_TYPE_MAGIC;
            }
        }
        else
        {
            retVal = ERROR_I2C_PARAM;
        }
    }
    else
    {
        retVal = ERROR_NULL;
    }
    return( retVal );
}

/* -------------------------------------------------------------------------
 | int main( int argc, char *argv[] )
 |
 | ...
 ---------------------------------------------------------------------------
*/
int main( int argc, char *argv[] )
{
    struct _caller_options param;
    i2cEEPROM *pDevice;
    int retVal;

    get_arguments( argc, argv, &param );

    if( (pDevice = new i2cEEPROM()) != NULL )
    {
        if( (param.eeOptFlags & OPTION_LIST_SET) == OPTION_LIST_SET )
        {
            retVal = listKnownTypes( pDevice, &param );
        }
        else
        {
            if( (param.eeOptFlags & OPTION_CHECK_SET) == OPTION_CHECK_SET )
            {
                retVal = check4EEPROM( pDevice, &param );
            }
            else
            {
                if( (param.eeOptFlags & OPTION_INFO_SET) == OPTION_INFO_SET )
                {
                    retVal = infoOnEEPROM( pDevice, &param );
                }
                else
                {
                    retVal = initializeEEPROM( pDevice, &param );
                }
            }
        }
        delete pDevice;
    }
    else
    {
        retVal = ERROR_NULL;
    }
    return(retVal);
}


