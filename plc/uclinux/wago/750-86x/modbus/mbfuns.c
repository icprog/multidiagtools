/*---------------------------------------------------------------------------
Filename: mbfuns.c  -  Functions for Modbus like access to data areas

Description:            Functions for Modbus like access to data areas
---------------------------------------------------------------------------*/

/* the following sequence will declare a character constant containing module
 * name and version in the name and in the contents. A "." in the file name
 * must be replaced by "_".
 */
#define PEP_MODNAME mbfuns_c
#define PEP_VER_ID 01a

#define PEP_VER_VARNAME_H(v,m,u) PEP_VER_##m##u##v
#define PEP_VER_VARNAME(v,m) PEP_VER_VARNAME_H(v,m,_)

#define PEP_VER_STR_H(m,v) "$PEPVERSION: " #m " " #v " $"
#define PEP_VER_STR(m,v) PEP_VER_STR_H(m,v)

#ifndef _ASMLANGUAGE
volatile static char PEP_VER_VARNAME(PEP_MODNAME,PEP_VER_ID)[]
    __attribute__ ((unused)) = PEP_VER_STR(PEP_MODNAME,PEP_VER_ID);
#endif

#undef PEP_MODNAME
#undef PEP_VER_ID
#undef PEP_VER_VARNAME_H
#undef PEP_VER_VARNAME
#undef PEP_VER_STR_H
#undef PEP_VER_STR

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Include-Dateien                                                       */
/*                                                                       */
/*-----------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "mbtypes.h"
#include "mbfuns.h"
#include "kbusapi.h"
           

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Globale Variablen                                                     */
/*                                                                       */
/*-----------------------------------------------------------------------*/
UINT32 uiNumberOfTableEntries; /* Anzahl der FC-Eintraege */
UINT32 uiRegisterInputEntries, uiRegisterOutputEntries, uiDigitalInputEntries, uiDigitalOutputEntries;
T_MODBUS_STATISTIK stModbusStatistik;
/*
 * Addons: MDT, 25.11.2012, PS
 */
int globalDigitalInputOffset;
int globalDigitalOutputOffset;

/*
 * MDt: read the Kbus configuration to extract digital I/O's offsets (ugly "hack")..
 * (Used to get digital I/O offsets, because KbusGetBinaryXXXXXOffset() seems to return wrong results ???)
 */
int readKbusConfig()
{
  FILE *cfgFile;
  char line[256];
  char articleNumber[256];
  char c, moduleType;
  int fakeValue;
  int i, convertedItemsCount;
  int dinOffset, doutOffset;
  char analogType;
  
  /* Init global variables with known values */
  globalDigitalInputOffset = 0;
  globalDigitalOutputOffset = 0;
  
  /* Open and read configuration file */
  cfgFile = fopen("/proc/driver/kbus/config.csv", "r");
  if(cfgFile == NULL){
    printf("Could not retrieve KBus driver module data. KBus module loaded?\n");
    return -1;
  }
  while(fgets(line, 255, cfgFile) != 0){
    // Parse line
    convertedItemsCount = sscanf(line, "%d\t%s\t%d\t%d\t%c\t%d\t%d\t%d\t%d", 
				      &fakeValue, articleNumber, &fakeValue, &fakeValue, &analogType, &doutOffset, &fakeValue, &fakeValue, &dinOffset);
    if(convertedItemsCount < 9){
      printf("Could not parse a line in KBus driver configuration file (converted items: %d)\n", convertedItemsCount);
      fclose(cfgFile);
      return -1;
    }
    // We want only digital I/O's
    if(analogType != 'y'){
      // Check if we have a input or output
      if(dinOffset > 0){
	if(globalDigitalInputOffset == 0){
	  globalDigitalInputOffset = dinOffset;
	}else{
	  /* We search the offset of the first digital input */
	  if(dinOffset < globalDigitalInputOffset){
	    globalDigitalInputOffset = dinOffset;
	  }
	}
      }
      if(doutOffset > 0){
	if(globalDigitalOutputOffset == 0){
	  globalDigitalOutputOffset = doutOffset;
	}else{
	  /* We search the offset of the first digital output */
	  if(doutOffset < globalDigitalOutputOffset){
	    globalDigitalOutputOffset = doutOffset;
	  }
	}
      }
    }
  }
  
  fclose(cfgFile);
  return 0;
}

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Modulinterne Funktionen                                               */
/*                                                                       */
/*-----------------------------------------------------------------------*/
INT32 CheckForBkError ( void );
void MbRegisterCopy ( UINT8 * , UINT8 *, UINT32 );

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Tabellen                                                              */
/*                                                                       */
/*-----------------------------------------------------------------------*/
// Funktionstabelle (Functioncode,Funktionsaufruf)
const MODBUS_FUNCTION_SELECT stModbusFunctionTable[] =
{
  { WRITE_MULTIPLE_REGISTERS, WriteMultipleRegisters }, //  = 16
  { WRITE_SINGLE_REGISTER,    WriteSingleRegister    }, //PRESET SINGLE REGISTER    = 06
  { MASK_WRITE_REGISTER,      MaskWriteRegister      }, //  = 22
  { READ_INPUT_REGISTERS,     ReadMultipleRegisters  }, //(READ) INPUT REGISTER     = 04
  { READ_MULTIPLE_REGISTERS,  ReadMultipleRegisters  }, //(READ) HOLDING REGISTERS  = 03
  { FORCE_MULTIPLE_COILS,     ForceMultipleCoils     }, //  = 15
  { READ_COILS,               ReadInputBits          }, //(READ) COIL STATUS        = 01
  { READ_INPUT_DISCRETES,     ReadInputBits          }, //(READ) INPUT STATUS       = 02
  { WRITE_COIL,               WriteCoil              }, //FORCE SINGLE COIL         = 05
  { GET_COMM_EVENT_COUNTER,   GetCommEventCounters   }  // = 11
/*
  { READ_EXCEPTION_STATUS,    ReadExceptionStatus    },
*/
};

static UINT16 MacId[3]; 
extern UINT16* getMAC();

/* Steuerdaten f�r Modbus-Zugriffe auf die Abbilddaten und andere interne Daten. */
/* Klaert den Zusammenhang zwischen */
/* - Zugriffsart (s. Feldnamen, unten) */
/* - Modbus-Adressbereich f�r den Datenbereich. */
/* - Datenbereichanfangsadresse. Kann mehreren Modbus-Adressbereichen zugeordnet sein. */
/* - Adressierungstyp f�r Pr�fungen und Auswahl des Transferverfahrens. */

/* Zugriffe wortweise auf Ausgangsdaten (Register, Out) */
/* Adressen in den ersten beiden Spalten entsprechen Wortnummern. */
static MODBUS_MEMORY_MAP astModbusRegisterOutput[] =
{
  { 0x0000, 0x01FF, (UINT16 *)(0xFFE060+2040), PROZESS_IMAGE_ADDRESS_OUT       },
  { 0x0200, 0x03FF, (UINT16 *)(0xFFE060+2040), PROZESS_IMAGE_ADDRESS_OUT       },
};

/* Zugriffe wortweise auf Eingangsdaten (Register, In) */
/* Adressen in den ersten beiden Spalten entsprechen Wortnummern. */
static MODBUS_MEMORY_MAP astModbusRegisterInput[] =
{
  { 0x0000, 0x01FF, (UINT16 *)0xFFE060,        PROZESS_IMAGE_ADDRESS_IN        },
  { 0x0200, 0x03FF, (UINT16 *)(0xFFE060+2040), PROZESS_IMAGE_ADDRESS_OUT       },
  { 0x1031, 0x1034, (UINT16 *)MacId,           INTERNAL_VAR_ADDRESS_OUT        },
};

/* Zugriffe bitweise auf Ausgangsdaten (Digital, Out) */
/* Adressen in den ersten beiden Spalten entsprechen Bitnummern. */
static MODBUS_MEMORY_MAP astModbusDigitalOutput[] =
{
  { 0x0000, 0x01FF, (UINT16 *)(0xFFE060+2040), PROZESS_IMAGE_ADDRESS_OUT       },/*TODO Bits 0...511*/
  { 0x0200, 0x03FF, (UINT16 *)(0xFFE060+2040), PROZESS_IMAGE_ADDRESS_OUT       },/*TODO Bits 0...511*/
};

/* Zugriffe bitweise auf Eingangsdaten (Digital, In) */
/* Adressen in den ersten beiden Spalten entsprechen Bitnummern. */
static MODBUS_MEMORY_MAP astModbusDigitalInput[] =
{
  { 0x0000, 0x01FF, (UINT16 *)0xFFE060,      PROZESS_IMAGE_ADDRESS_IN        },/*TODO Bits 0...511*/
  { 0x0200, 0x03FF, (UINT16 *)(0xFFE060+2040), PROZESS_IMAGE_ADDRESS_OUT       },/*TODO Bits 0...511*/
};

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:    ProcessOneModbusRequest                                  */
/*                                                                       */
/* Aufgabe:     Bearbeitung einer Modbus-Anfrage                         */
/*                                                                       */
/* Eingang:     Zeiger auf Modbus-Paket                                  */
/*                                                                       */
/* Ausgabe:     -                                                        */
/*                                                                       */
/* Rueckgabe:   Laenge der zu sendenen Daten                             */
/*                                                                       */
/*-----------------------------------------------------------------------*/
INT32 ProcessOneModbusRequest ( char *pcModbusPacket )
{
  UINT32 uiCount;
  INT32 iLength;
  MODBUS_HEADER *pstModbusHeader;

  pstModbusHeader = (MODBUS_HEADER *)pcModbusPacket; /*lint !e826 */

  /* Suchen nach der auszufuehrenden Funktion anhand des Function-Codes */
  for (uiCount=0; uiCount < uiNumberOfTableEntries; uiCount++)
  {
    if (pstModbusHeader->ucFunctionCode == stModbusFunctionTable[uiCount].cFunctionCode) 
      break; /* Richtige Funktion gefunden */
  }
  if (uiCount < uiNumberOfTableEntries)
  {
    /* Aufruf der zu bearbeitenden Funktion */
    iLength = stModbusFunctionTable[uiCount].pFunction ( pcModbusPacket );  
  }
  else
  {  
    /* Ungueltiger Funktions-Code => Exception*/
    iLength = ReturnException (pcModbusPacket, ILLEGAL_FUNCTION);
  }
  return ( iLength ); 
}



/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:    ReadMultipleRegisters                                    */
/*                                                                       */
/* Aufgabe:     Lesen von mehreren Registern ( FC3 und FC4 )             */
/*                                                                       */
/* Eingang:     Zeiger auf Modbus-Paket                                  */
/*                                                                       */
/* Ausgabe:     -                                                        */
/*                                                                       */
/* Rueckgabe:   Laenge der zu sendenen Daten                             */
/*                                                                       */
/*-----------------------------------------------------------------------*/
INT32 ReadMultipleRegisters ( char *pcModbusPacket )
{
  UINT16 *pusPhysAddress, usRegisterOffset;
  UINT16 usRefNr, usByteCnt;
  UINT32 uiAddress;
  UINT8  *pusPAB;
  INT32  iResponseLength, iError;
  READ_MULTIPLE_REGISTERS_REQUEST  *pstRmrRequest;
  READ_MULTIPLE_REGISTERS_RESPONSE *pstRmrResponse;

  iResponseLength = 0;  /* Initialisieren der Response-Laenge */

  stModbusStatistik.usModbusMessageCounter++; /* Statistik : Modbus Message Counter */

  /* Aufsetzen der Strukturen f�r Request und Response */
  pstRmrRequest  = (READ_MULTIPLE_REGISTERS_REQUEST  *)pcModbusPacket; /*lint !e826 */
  pstRmrResponse = (READ_MULTIPLE_REGISTERS_RESPONSE *)pcModbusPacket; /*lint !e826 */

  /* Laengenbeschraenkung fuer Read Multiple Registers pruefen */
  if (__cpu_to_be16(pstRmrRequest->usWordCount) > READ_REGS_MAX )
  {
    iResponseLength = ReturnException ( pcModbusPacket, ILLEGAL_DATA_VALUE );
  }
  else /* Laenge der zu lesenden Register in Ordnung */
  {
    uiAddress = CheckAddress ( &astModbusRegisterInput[0], uiRegisterInputEntries,
                               __cpu_to_be16(pstRmrRequest->usReferenceNumber),
                               __cpu_to_be16(pstRmrRequest->usWordCount),
                               &pusPhysAddress, &usRegisterOffset );
                               
    switch (uiAddress)
    {
      case INTERNAL_VAR_ADDRESS_OUT:
	  
	/* read MAC-ID */
	memcpy(MacId, getMAC(), 6);

        iError = CheckForBkError ();  /* Buskoppler-Status abfragen ( KBus-Fehler oder FBus-Fehler ) */
        if ( ( iError & FBUS_ERROR) || ( iError & KBUS_ERROR) )
        {
          iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
        }
        else
        {
          usRefNr = __cpu_to_be16(pstRmrRequest->usReferenceNumber) <<1;
          usByteCnt = __cpu_to_be16(pstRmrRequest->usWordCount) << 1;

          iError = 0;
          if ( 0 == iError )
          {
            pstRmrResponse->ucByteCount = usByteCnt;
            
            pusPAB = (UINT8 *)(pusPhysAddress + usRefNr/2 - usRegisterOffset);
            MbRegisterCopy(pstRmrResponse->aucRegisterValues, (UINT8*) MacId, usByteCnt);
            
            iResponseLength = READ_MULTIPLE_REGISTERS_LENGTH + pstRmrResponse->ucByteCount;
            pstRmrResponse->stModbusHeader.usLengthField = (UINT16) (__cpu_to_be16(iResponseLength - LENGTH_TO_UNIT_ID));
          }
          else
          {
            iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
          }
        }
     
      break;

      case PROZESS_IMAGE_ADDRESS_IN:
      case PROZESS_IMAGE_ADDRESS_OUT:
        iError = CheckForBkError ();  /* Buskoppler-Status abfragen ( KBus-Fehler oder FBus-Fehler ) */
        if ( ( iError & FBUS_ERROR) || ( iError & KBUS_ERROR) )
        {
          iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
        }
        else
        {
          usRefNr = __cpu_to_be16(pstRmrRequest->usReferenceNumber) <<1;
          usByteCnt = __cpu_to_be16(pstRmrRequest->usWordCount) << 1;

          iError = 0;
          if ( 0 == iError )
          {
            pstRmrResponse->ucByteCount = usByteCnt;
            
            KbusUpdate();            

            pusPAB = (UINT8 *)(pusPhysAddress + usRefNr/2 - usRegisterOffset);
            MbRegisterCopy(pstRmrResponse->aucRegisterValues, pusPAB, usByteCnt);
            
            iResponseLength = READ_MULTIPLE_REGISTERS_LENGTH + pstRmrResponse->ucByteCount;
            pstRmrResponse->stModbusHeader.usLengthField = (UINT16) (__cpu_to_be16(iResponseLength - LENGTH_TO_UNIT_ID));
          }
          else
          {
            iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
          }
        }
      break;

      case PFC_VAR_ADDRESS_IN:
      case PFC_VAR_ADDRESS_OUT:
      case NOVRAM_ADDRESS_IN:
      break;

      default :
	  iResponseLength = ReturnException ( pcModbusPacket, ILLEGAL_DATA_ADDRESS );
      break;

    } /* Ende Switch Anweisung */
  } /* Ende else : Adressen in Ordnung */

  return (iResponseLength);
}


/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:    WriteMultipleRegisters                                   */
/*                                                                       */
/* Aufgabe:     Schreiben von mehreren Registern                         */
/*                                                                       */
/* Eingang:     pcModbusPacket                                           */
/*                                                                       */
/* Ausgabe:     -                                                        */
/*                                                                       */
/* Rueckgabe:   Laenge der zu sendenen Daten                             */
/*                                                                       */
/*-----------------------------------------------------------------------*/
INT32 WriteMultipleRegisters ( char *pcModbusPacket )
{
  INT32 iResponseLength, iError;
  UINT32 uiAddress;
  UINT16 *pusPhysAddress, usRegisterOffset;
  UINT16 usRefNr, usByteCnt;
  UINT8 *pusPAB;
  
  WRITE_MULTIPLE_REGISTERS_REQUEST  *pstWmrRequest;
  WRITE_MULTIPLE_REGISTERS_RESPONSE *pstWmrResponse;

  /* Initialisieren der Response-Laenge */
  iResponseLength = 0;
  /* Statistik : Modbus Message Counter */
  stModbusStatistik.usModbusMessageCounter++;

  /* Aufsetzen der Strukturen f�r Request und Response */
  pstWmrRequest = (WRITE_MULTIPLE_REGISTERS_REQUEST *)pcModbusPacket; /*lint !e826 */
  pstWmrResponse = (WRITE_MULTIPLE_REGISTERS_RESPONSE *)pcModbusPacket; /*lint !e826 */

  /* Laengenbeschraenkung fuer Read Multiple Registers pruefen */
  if (__cpu_to_be16(pstWmrRequest->usWordCount) > WRITE_REGS_MAX )
  {
    iResponseLength = ReturnException ( pcModbusPacket, ILLEGAL_DATA_VALUE );
  }
  else /* Laenge der zu lesenden Register in Ordnung */
  {
    uiAddress = CheckAddress ( &astModbusRegisterOutput[0], uiRegisterOutputEntries,
                               __cpu_to_be16(pstWmrRequest->usReferenceNumber),
                               __cpu_to_be16(pstWmrRequest->usWordCount), &pusPhysAddress,
                              &usRegisterOffset );

    switch (uiAddress)
    {
      case PROZESS_IMAGE_ADDRESS_OUT:
        iError = CheckForBkError ();  /* Buskoppler-Status abfragen ( KBus-Fehler oder FBus-Fehler ) */

        if ( ( iError & FBUS_ERROR) || ( iError & KBUS_ERROR) )
        {
          iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
        }
        else
        {
          usRefNr = __cpu_to_be16(pstWmrRequest->usReferenceNumber) <<1;
          usByteCnt = pstWmrRequest->ucByteCount;
          
          pusPAB = (UINT8 *)(pusPhysAddress + usRefNr/2 - usRegisterOffset);
          MbRegisterCopy(pusPAB, pstWmrRequest->aucRegisterValues, usByteCnt);

          KbusUpdate();

          iError = 0;
          if ( 0 == iError )
          {
            pstWmrResponse->usWordCount = __cpu_to_be16(usByteCnt >> 1);
            iResponseLength = WRITE_MULTIPLE_REGISTERS_RESPONSE_LENGTH;
            pstWmrResponse->stModbusHeader.usLengthField = (UINT16) (__cpu_to_be16(iResponseLength - LENGTH_TO_UNIT_ID));
          }
          else
          {
            iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
          }

        }
      break;

      case PFC_VAR_ADDRESS_IN:
      case PFC_VAR_ADDRESS_OUT:
      case NOVRAM_ADDRESS_OUT:
      break;

      default :
        iResponseLength = ReturnException ( pcModbusPacket, ILLEGAL_DATA_ADDRESS );
      break;
    } /* Ende Switch Anweisung */
  } /* Ende else : Adressen in Ordnung */
  return (iResponseLength);
}

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:    WriteSingleRegister                                      */
/*                                                                       */
/* Aufgabe:                                                              */
/*                                                                       */
/* Eingang:                                                              */
/*                                                                       */
/* Ausgabe:     -                                                        */
/*                                                                       */
/* Rueckgabe:   Laenge der zu sendenen Daten                             */
/*                                                                       */
/*-----------------------------------------------------------------------*/
INT32 WriteSingleRegister ( char *pcModbusPacket )
{
  INT32 iResponseLength, iError;
  UINT16 usRegisterOffset, *pusPhysAddress;
  UINT32 uiAddress;
  WRITE_SINGLE_REGISTER_REQUEST  *pstWsrRequest;
  WRITE_SINGLE_REGISTER_RESPONSE *pstWsrResponse;
  UINT16 usRefNr;

  /* Initialisieren der Response-Laenge */
  iResponseLength = 0;
  /* Statistik : Modbus Message Counter */
  stModbusStatistik.usModbusMessageCounter++;

  /* Aufsetzen der Strukturen f�r Request und Response */
  pstWsrRequest = (WRITE_SINGLE_REGISTER_REQUEST *)pcModbusPacket; /*lint !e826 */
  pstWsrResponse = (WRITE_SINGLE_REGISTER_RESPONSE *)pcModbusPacket; /*lint !e826 */

  uiAddress = CheckAddress ( &astModbusRegisterOutput[0], uiRegisterOutputEntries,
                             __cpu_to_be16(pstWsrRequest->usReferenceNumber), 1, &pusPhysAddress, &usRegisterOffset );

  switch (uiAddress)
  {
    case PROZESS_IMAGE_ADDRESS_OUT:
      iError = CheckForBkError ();  /* Buskoppler-Status abfragen ( KBus-Fehler oder FBus-Fehler ) */

      if ( ( iError & FBUS_ERROR) || ( iError & KBUS_ERROR) )

      {
        iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
      }
      else
      {
        usRefNr = __cpu_to_be16(pstWsrRequest->usReferenceNumber) <<1;

        *(pusPhysAddress + usRefNr/2 - usRegisterOffset) = __cpu_to_be16(pstWsrRequest->usRegisterValue);
        
        KbusUpdate();
        
        iError = 0;
        if ( 0 == iError )
        {
          iResponseLength = WRITE_SINGLE_REGISTER_RESPONSE_LENGTH;
          pstWsrResponse->stModbusHeader.usLengthField = (UINT16) (__cpu_to_be16(iResponseLength - LENGTH_TO_UNIT_ID));
        }
        else
        {
          iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
        }
      }
    break;
    case PFC_VAR_ADDRESS_IN:
    case PFC_VAR_ADDRESS_OUT:
    case NOVRAM_ADDRESS_OUT:
    break;

    default :
      iResponseLength = ReturnException ( pcModbusPacket, ILLEGAL_DATA_ADDRESS );
    break;
  } /* Ende Switch Anweisung */
  return (iResponseLength);
}

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:    MaskWriteRegister                                        */
/*                                                                       */
/* Aufgabe:                                                              */
/*                                                                       */
/* Eingang:                                                              */
/*                                                                       */
/* Ausgabe:     -                                                        */
/*                                                                       */
/* Rueckgabe:   Laenge der zu sendenen Daten                             */
/*                                                                       */
/*-----------------------------------------------------------------------*/
INT32 MaskWriteRegister ( char *pcModbusPacket )
{
  INT32 iResponseLength, iError;
  UINT32 uiAddress;
  UINT16 usRegisterOffset, *pusPhysAddress;
  UINT16 usCurrentValue;
  MASK_WRITE_REGISTER_REQUEST  *pstMwrRequest;
  MASK_WRITE_REGISTER_RESPONSE  *pstMwrResponse;
  UINT16 usRefNr;
  transfer_data	msg_data;

  /* Initialisieren der Response-Laenge */
  iResponseLength = 0;
  /* Statistik : Modbus Message Counter */
  stModbusStatistik.usModbusMessageCounter++;

  /* Aufsetzen der Strukturen f�r Request und Response */
  pstMwrRequest = (MASK_WRITE_REGISTER_REQUEST *)pcModbusPacket; /*lint !e826 */
  pstMwrResponse = (MASK_WRITE_REGISTER_RESPONSE *)pcModbusPacket; /*lint !e826 */

  uiAddress = CheckAddress ( &astModbusRegisterOutput[0], uiRegisterOutputEntries,
                             __cpu_to_be16(pstMwrRequest->usReferenceNumber), 1,
                             &pusPhysAddress, &usRegisterOffset );

  switch (uiAddress)
  {
    case PROZESS_IMAGE_ADDRESS_OUT:
      iError = CheckForBkError ();  /* Buskoppler-Status abfragen ( KBus-Fehler oder FBus-Fehler ) */
      if ( ( iError & FBUS_ERROR) || ( iError & KBUS_ERROR) )
      {
        iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
      }
      else
      {
        usRefNr = __cpu_to_be16(pstMwrRequest->usReferenceNumber) <<1;
        memset(&msg_data,0,sizeof(msg_data));

        msg_data.op_offset = usRefNr;
        msg_data.op_size = 2;
        MbRegisterCopy_WITH_SWAP_IF_LITTLE((UINT16 *)&msg_data.data[usRefNr], &usCurrentValue, 1);
        iError = 0;
        if ( 0 == iError )
        {
          usCurrentValue = ( usCurrentValue & __cpu_to_be16(pstMwrRequest->usAndMask) )
                           | (__cpu_to_be16 (pstMwrRequest->usOrMask)
                           & (__cpu_to_be16 (pstMwrRequest->usAndMask)));

          memset(&msg_data,0,sizeof(msg_data));

          msg_data.op_offset = usRefNr;
          msg_data.op_size = 2;
          MbRegisterCopy_WITH_SWAP_IF_LITTLE(&usCurrentValue, (UINT16 *)&msg_data.data[usRefNr], 1);
          iError = 0;
          if ( 0 == iError )
          {
            iResponseLength = MASK_WRITE_REGISTER_RESPONSE_LENGTH;
            pstMwrResponse->stModbusHeader.usLengthField = (UINT16) (__cpu_to_be16(iResponseLength - LENGTH_TO_UNIT_ID));
          }
          else
          {
            iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
          }
        }
        else
        {
          iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
        }
      }
    break;

    case PFC_VAR_ADDRESS_IN:
    case PFC_VAR_ADDRESS_OUT:
    break;

    case NOVRAM_ADDRESS_OUT:
    break;

    default :
      iResponseLength = ReturnException ( pcModbusPacket, ILLEGAL_DATA_ADDRESS );
    break;
  } /* Ende Switch Anweisung */
  return (iResponseLength);
}


/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:    ReadInputBits                                            */
/*                                                                       */
/* Aufgabe:     Lesen von mehreren Bits ( FC1 und FC2 )                  */
/*                                                                       */
/* Eingang:     Zeiger auf Modbus-Paket                                  */
/*                                                                       */
/* Ausgabe:     -                                                        */
/*                                                                       */
/* Rueckgabe:   Laenge der zu sendenen Daten                             */
/*                                                                       */
/*-----------------------------------------------------------------------*/
/* TODO will fail in a sneaky way when READ_COILS_BYTE_MAX is uneven */
INT32 ReadInputBits ( char *pcModbusPacket )
{
  UINT32 uiAddress;
  INT32  iResponseLength, iError;
  UINT32 uiBitOffset;
  UINT16 *pusPhysAddress, usRegisterOffset, usBitCount;
  UINT16 ausTempBuffer[READ_COILS_BYTE_MAX/2];

  READ_COILS_REQUEST  *pstRcRequest;
  READ_COILS_RESPONSE *pstRcResponse;

  /* Initialisieren der Response-Laenge */
  iResponseLength = 0;
  /* Statistik : Modbus Message Counter */
  stModbusStatistik.usModbusMessageCounter++;

  /* Aufsetzen der Strukturen f�r Request und Response */
  pstRcRequest  = (READ_COILS_REQUEST  *)pcModbusPacket; /*lint !e826 */
  pstRcResponse = (READ_COILS_RESPONSE *)pcModbusPacket; /*lint !e826 */


  /* Laengenbeschraenkung fuer Read Multiple Registers pruefen */
  if (__cpu_to_be16(pstRcRequest->usBitCount) > READ_COILS_MAX )
  {
    iResponseLength = ReturnException ( pcModbusPacket, ILLEGAL_DATA_VALUE );
  }
  else /* Laenge der zu lesenden Register in Ordnung */
  {
    uiAddress = CheckAddress (&astModbusDigitalInput[0], uiDigitalInputEntries,
                              __cpu_to_be16(pstRcRequest->usReferenceNumber),
                              __cpu_to_be16(pstRcRequest->usBitCount),
                              &pusPhysAddress, &usRegisterOffset );
    

    switch (uiAddress)
    {
      case PROZESS_IMAGE_ADDRESS_IN:
      case PROZESS_IMAGE_ADDRESS_OUT:
        iError = CheckForBkError ();  /* Buskoppler-Status abfragen ( KBus-Fehler oder FBus-Fehler ) */
        if ( ( iError & FBUS_ERROR) || ( iError & KBUS_ERROR) )
        {
          iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
        }
        else
        {
          /* Berechnung des BitOffsets */
          ///uiBitOffset = KbusGetBinaryInputOffset() + __cpu_to_be16(pstRcRequest->usReferenceNumber) - usRegisterOffset;
          uiBitOffset = globalDigitalInputOffset + __cpu_to_be16(pstRcRequest->usReferenceNumber) - usRegisterOffset;

          usBitCount = __cpu_to_be16(pstRcRequest->usBitCount);

          iError = 0;
          if ( 0 == iError )
          {
            KbusUpdate();
            
            memset(ausTempBuffer, 0, READ_COILS_BYTE_MAX);
            CopyDigitalData(pusPhysAddress, uiBitOffset, ausTempBuffer, 0, usBitCount);
            memcpy(pstRcResponse->ucBitValues, ausTempBuffer, usBitCount/8+1);

            pstRcResponse->ucByteCount = (UINT8) (usBitCount/8);
            if ( usBitCount % 8 )
            {
              pstRcResponse->ucByteCount++;
            }
            iResponseLength = READ_COIL_LENGTH + pstRcResponse->ucByteCount;
            pstRcResponse->stModbusHeader.usLengthField = (UINT16) (__cpu_to_be16(iResponseLength - LENGTH_TO_UNIT_ID));
          }
          else
          {
            iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
          }
        }
      break;

      default :
        iResponseLength = ReturnException ( pcModbusPacket, ILLEGAL_DATA_ADDRESS );
      break;

    } /* Ende Switch Anweisung */
  } /* Ende else : Adressen in Ordnung */
  return (iResponseLength);
}

/*-----------------------------------------------------------------------*/
/*                                                                       */

/* Funktion:    ForceMultipleCoils                                       */
/*                                                                       */
/* Aufgabe:     Schreiben von mehreren Bits ins PA                       */
/*                                                                       */
/* Eingang:     pcModbusPacket                                           */
/*                                                                       */
/* Ausgabe:     -                                                        */
/*                                                                       */

/* Rueckgabe:   Laenge der zu sendenen Daten                             */
/*                                                                       */
/*-----------------------------------------------------------------------*/
INT32 ForceMultipleCoils ( char *pcModbusPacket )
{
  INT32 iResponseLength, iError;
  UINT32 uiAddress;
  UINT16 *pusPhysAddress, usRegisterOffset, uiBitOffset;
  UINT16 ausTempBuffer[FORCE_COILS_BYTE_MAX/2];

  T_TERMINAL_OFFSET stDigitalOffset;

  FORCE_MULTIPLE_COILS_REQUEST  *pstFmcRequest;
  FORCE_MULTIPLE_COILS_RESPONSE *pstFmcResponse;
  
  memset(ausTempBuffer, 0, FORCE_COILS_BYTE_MAX/2);
    

  /* Initialisieren der Response-Laenge */
  iResponseLength = 0;
  /* Statistik : Modbus Message Counter */
  stModbusStatistik.usModbusMessageCounter++;

  /* Aufsetzen der Strukturen f�r Request und Response */
  pstFmcRequest  = (FORCE_MULTIPLE_COILS_REQUEST  *)pcModbusPacket; /*lint !e826 */
  pstFmcResponse = (FORCE_MULTIPLE_COILS_RESPONSE *)pcModbusPacket; /*lint !e826 */

  /* Laengenbeschraenkung fuer Read Multiple Registers pruefen */
  if (__cpu_to_be16(pstFmcRequest->usBitCount) > FORCE_COILS_MAX )
  {
    iResponseLength = ReturnException ( pcModbusPacket, ILLEGAL_DATA_VALUE );
  }
  else /* Laenge der zu lesenden Register in Ordnung */
  {
    uiAddress = CheckAddress ( &astModbusDigitalOutput[0], uiDigitalOutputEntries,
                               __cpu_to_be16(pstFmcRequest->usReferenceNumber),
                               __cpu_to_be16(pstFmcRequest->usBitCount)-1,
                               &pusPhysAddress, &usRegisterOffset );

    switch (uiAddress)
    {
      case PROZESS_IMAGE_ADDRESS_OUT:
        iError = CheckForBkError ();  /* Buskoppler-Status abfragen ( KBus-Fehler oder FBus-Fehler ) */
        if ( ( iError & FBUS_ERROR) || ( iError & KBUS_ERROR) )
        {
          iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
        }
        else
        {
          /* Berechnung des BitOffsets */
          ///uiBitOffset = KbusGetBinaryOutputOffset() + __cpu_to_be16(pstFmcRequest->usReferenceNumber) - usRegisterOffset;
          uiBitOffset = globalDigitalOutputOffset + __cpu_to_be16(pstFmcRequest->usReferenceNumber) - usRegisterOffset;

          memcpy(ausTempBuffer, pstFmcRequest->aucData, pstFmcRequest->ucByteCount);
          CopyDigitalData(ausTempBuffer, 0, pusPhysAddress, uiBitOffset, __cpu_to_be16(pstFmcRequest->usBitCount));

          KbusUpdate();

          iError = 0; /* Schreiben des Bits auf den KBUS */
          if ( 0 == iError )
          {
            /* Berechnen der Response Laengen */
            iResponseLength = FORCE_MULTIPLE_COILS_RESPONSE_LENGTH;
            pstFmcResponse->stModbusHeader.usLengthField = (UINT16) __cpu_to_be16(FORCE_MULTIPLE_COILS_RESPONSE_LENGTH - LENGTH_TO_UNIT_ID);
          }
          else
          {
            iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
          }
        }
      break;

      default:
        iResponseLength = ReturnException ( pcModbusPacket, ILLEGAL_DATA_ADDRESS );
      break;
    } /* Ende Switch Anweisung */
  } /* Ende else : Adressen in Ordnung */
  return (iResponseLength);
}

/*-----------------------------------------------------------------------*/
/*                                                                      */
/* Funktion:    WriteCoil                                                */
/*                                                                       */
/* Aufgabe:     Schreiben eines Bits ins PA                              */
/*                                                                       */
/* Eingang:     pcModbusPacket                                           */
/*                                                                       */
/* Ausgabe:     -                                                        */
/*                                                                       */
/* Rueckgabe:   Laenge der zu sendenen Daten                             */
/*                                                                       */
/*-----------------------------------------------------------------------*/
/*
 * Updated by MDT (Philippe Steinmann) - 25.11.2012:
 *  - Use of alternate digital output offset calculation
 *  - Make this function simply work (hope it's real..)
 */
INT32 WriteCoil ( char *pcModbusPacket )
{
  INT32 iResponseLength, iError;
  UINT32 uiAddress;
  UINT16 *pusPhysAddress, usRegisterOffset;
  UINT8 *regAddress;
  UINT16 mbQueryAddress, mbQueryValue;
  UINT8 bitOffset;
  WRITE_COIL_REQUEST  *pstWcRequest;
  WRITE_COIL_RESPONSE *pstWcResponse;

  /* Initialisieren der Response-Laenge */
  iResponseLength = 0;
  /* Statistik : Modbus Message Counter */
  stModbusStatistik.usModbusMessageCounter++;

  /* Aufsetzen der Strukturen f�r Request und Response */
  pstWcRequest  = (WRITE_COIL_REQUEST  *)pcModbusPacket; /*lint !e826 */
  pstWcResponse = (WRITE_COIL_RESPONSE *)pcModbusPacket; /*lint !e826 */

  uiAddress = CheckAddress ( &astModbusDigitalOutput[0], uiDigitalOutputEntries,
                             __cpu_to_be16(pstWcRequest->usReferenceNumber),
                             0, &pusPhysAddress, &usRegisterOffset );

  switch (uiAddress)
  {
    //case PROZESS_IMAGE_ADDRESS_IN:
    case PROZESS_IMAGE_ADDRESS_OUT:
      iError = CheckForBkError ();  /* Buskoppler-Status abfragen ( KBus-Fehler oder FBus-Fehler ) */
      if ( ( iError & FBUS_ERROR) || ( iError & KBUS_ERROR) )
      {
        iResponseLength = ReturnException ( pcModbusPacket, SERVER_FAILURE );
      }
      else
      {
	/* Extract MODBUS query address and value */
	mbQueryAddress = __cpu_to_be16(pstWcRequest->usReferenceNumber);
	mbQueryValue = __cpu_to_be16(pstWcRequest->usValue);
	
	/* Select correct register */
	regAddress = pusPhysAddress;
	regAddress += globalDigitalOutputOffset;
	regAddress += (mbQueryAddress - usRegisterOffset) / 8;
	/* Select correct bit */
	bitOffset = mbQueryAddress % 8;
	/* Apply query value */
	switch(mbQueryValue){
	  case 0x0000:
	    *regAddress &= ~(1<<bitOffset);
	    break;
	  case 0xFF00:
	    *regAddress |= (1<<bitOffset);
	    break;
	  default:
	    return ReturnException( pcModbusPacket, ILLEGAL_DATA_VALUE);
	}
	// Update I/O's bus
	if(KbusUpdate() != 0){
	  return ReturnException( pcModbusPacket, SERVER_FAILURE);
	}
	// Check back and build reply
	if(*regAddress & (1<<bitOffset)){
	  pstWcResponse->usValue = __cpu_to_be16(0xFF00);
	}else{
	  pstWcResponse->usValue = __cpu_to_be16(0x0000);
	}
	pstWcResponse->stModbusHeader.usLengthField = __cpu_to_be16((UINT16)(WRITE_COIL_RESPONSE_LENGTH - LENGTH_TO_UNIT_ID));
	iResponseLength = WRITE_COIL_RESPONSE_LENGTH;
      }
      break;
    default :
      iResponseLength = ReturnException ( pcModbusPacket, ILLEGAL_DATA_ADDRESS );
    break;
  } /* Ende Switch Anweisung */

  return (iResponseLength);
}

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:    GetCommEventCounters                                     */
/*                                                                       */
/* Aufgabe:     Lesen des Event-Counters  (FC 11)                        */
/*                                                                       */
/* Eingang:     Zeiger auf Modbus-Paket                                  */
/*                                                                       */
/* Ausgabe:     -                                                        */
/*                                                                       */
/* Rueckgabe:   Laenge der zu sendenen Daten                             */
/*                                                                       */
/*-----------------------------------------------------------------------*/
INT32 GetCommEventCounters ( char *pcModbusPacket )
{
  INT32 iResponseLength;
  GET_COMM_EVENT_COUNTER_RESPONSE *pstGcecResp;

  pstGcecResp = (GET_COMM_EVENT_COUNTER_RESPONSE *) pcModbusPacket; /*lint !e826 */

  pstGcecResp->usStatus = 0;
  pstGcecResp->usEventCount = (UINT16) __cpu_to_be16(stModbusStatistik.usModbusMessageCounter);

  iResponseLength = GET_COMM_EVENT_COUNTER_LENGTH;
  pstGcecResp->stModbusHeader.usLengthField = __cpu_to_be16(GET_COMM_EVENT_COUNTER_DATA_LENGTH);

  return (iResponseLength);
}



/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:    CheckAddress                                             */
/*                                                                       */
/* Aufgabe:     Bestimmung des Modbus-Adress-Bereiches                   */
/*                                                                       */
/* Eingang:     Zeiger auf Memory-Map                                    */
/*              Startadresse                                             */
/*              Anzahl der Adressen                                      */
/*                                                                       */
/* Ausgabe:     Zeiger auf Datenbereich                                  */
/*                                                                       */
/* Rueckgabe:   Art des Zugriffs                                         */
/*                                                                       */
/*-----------------------------------------------------------------------*/
UINT32 CheckAddress ( MODBUS_MEMORY_MAP *pstMemoryMap, UINT32 uiNumberOfEntries,
                      UINT16 usStartAddress, UINT16 usWordCount,
                      UINT16 **pusPhysAddress, UINT16 *pusRegisterOffset )
{
  UINT32 uiAddress, uiEndaddress, uiCount;
  MODBUS_MEMORY_MAP *pstAddressRange;

  /* Setzen des Adress-Bereichs auf Anfang der Memory-Map */
  pstAddressRange = pstMemoryMap;

  /* Suchen der Adresse in der Memory-Map */
  for (uiCount = 0; uiCount < uiNumberOfEntries; uiCount++, pstAddressRange++)
  {
    if (( usStartAddress >= pstAddressRange->usModbusStartAddress)
      && (usStartAddress <= pstAddressRange->usModbusEndAddress))
    {
      break; /* Adresse gefunden */
    }
  }
  if ( uiCount < uiNumberOfEntries ) /* Startadresse liegt innerhalb der Memory-Map */
  {
    /* Berechnung der Endadresse */
    uiEndaddress = usStartAddress + usWordCount;
    /* Endadresse liegt auch im Bereich */
    if (uiEndaddress <= pstAddressRange->usModbusEndAddress)
    {
      *pusPhysAddress = pstAddressRange->pusPhysicalAddress;
      uiAddress = pstAddressRange->usArtOfAddress;
      *pusRegisterOffset = pstAddressRange->usModbusStartAddress;
    }
    else /* Adressbereich ueberscritten */
    {
      uiAddress = UNKNOWN_ADDRESS;
    }
  }
  else /* Adresse nicht gefunden */
  {
    uiAddress = UNKNOWN_ADDRESS;
  }

  return ( uiAddress );
}

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:    ReturnException                                          */
/*                                                                       */
/* Aufgabe:     Exception Response                                       */
/*                                                                       */
/* Eingang:     Zeiger auf Modbus-Header                                 */

/*              ExceptionNr.                                             */
/*                                                                       */
/* Ausgabe:     -                                                        */
/*                                                                       */
/* Rueckgabe:   Laenge der zu sendenen Daten                             */
/*                                                                       */
/*-----------------------------------------------------------------------*/
INT32 ReturnException ( char *pcModbusPacket, UINT8 ucException )
{
  EXCEPTION *pstException;

  switch (ucException)
  {
    case ILLEGAL_DATA_ADDRESS :
      stModbusStatistik.usBadAddress++;
    break;

    case ILLEGAL_DATA_VALUE :
      stModbusStatistik.usBadData++;
    break;

    case ILLEGAL_FUNCTION :
      stModbusStatistik.usBadFunction++;
    break;

    case SERVER_FAILURE :
      stModbusStatistik.usSlaveDeviceFailure++;
    break;
    
    default :
    break;
  }
  /* Auf Modbus-Header casten */
  pstException = (EXCEPTION *)pcModbusPacket; /*lint !e826 */

  pstException->stModbusHeader.ucFunctionCode |= 0x80;
  pstException->ucException = ucException;
  pstException->stModbusHeader.usLengthField = __cpu_to_be16(0x0003);

  return ( EXCEPTION_LENGTH );
}

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:  CheckForBkError                                            */
/*                                                                       */
/* Aufgabe:   Ueberpruefen auf Feldbusfehler und K-Busfehler             */
/*                                                                       */
/* Eingang:   -                                                          */
/*                                                                       */
/* Ausgabe:   -                                                          */
/*                                                                       */
/* Rueckgabe: NU_SUCCESS                                                 */
/*                                                                       */
/*-----------------------------------------------------------------------*/
INT32 CheckForBkError ( void )
{
#ifdef ALLES    //TODO!
  INT32 iStatus;
  UINT32 uiFlags;

  /* Liegt ein Klemmenbusfehler vor */
  iStatus = NU_Retrieve_Events ( &egBkStatus, KBUS_ERROR, NU_OR, &uiFlags, NU_NO_SUSPEND );


  if (iStatus != NU_NOT_PRESENT) /* Fehler in der Funktion oder Ein Bit gesetzt */
  {
    if (iStatus == NU_SUCCESS)  /* Es liegt ein Kbus oder Fbus-Fehler vor */
    {
      return (KBUS_ERROR);
    }
    else
    {
//      LOGERR( iStatus, ERR_TRACE, NU_SYS_ERR, 0, 0 );
    }
  }

  /* Liegt ein Feldbusfehler vor */
  iStatus = NU_Retrieve_Events ( &egFbStatus, SET_MB_WDG_EXPIRED, NU_OR, &uiFlags, NU_NO_SUSPEND );

  if (iStatus != NU_NOT_PRESENT) /* Fehler in der Funktion oder Ein Bit gesetzt */
  {
    if (iStatus == NU_SUCCESS)  /* Es liegt ein Kbus oder Fbus-Fehler vor */
    {
      return (FBUS_ERROR);
    }
    else
    {

//      LOGERR( iStatus, ERR_TRACE, NU_SYS_ERR, 0, 0 );
    }
  }
  return (NU_SUCCESS);
#else
  return (0);
#endif
}

/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:    InitModbusVariables                                      */
/*                                                                       */
/* Aufgabe:     Initialisierung von Variablen                            */
/*                                                                       */
/* Eingang:     -                                                        */
/*                                                                       */
/* Ausgabe:     -                                                        */
/*                                                                       */
/* Rueckgabe:   Laenge der zu sendenen Daten                             */
/*                                                                       */
/*-----------------------------------------------------------------------*/
void InitModbusVariables ( void )
{
  /* Anzahl der Eintaege in der Function Code Tabelle speichern */
  uiNumberOfTableEntries =  sizeof(stModbusFunctionTable) / sizeof(MODBUS_FUNCTION_SELECT);
  uiRegisterInputEntries =  sizeof(astModbusRegisterInput) / sizeof (MODBUS_MEMORY_MAP);
  uiRegisterOutputEntries = sizeof(astModbusRegisterOutput) / sizeof (MODBUS_MEMORY_MAP);
  uiDigitalInputEntries =  sizeof(astModbusDigitalInput) / sizeof (MODBUS_MEMORY_MAP);
  uiDigitalOutputEntries = sizeof(astModbusDigitalOutput) / sizeof (MODBUS_MEMORY_MAP);
  /* Modbus-Statistik auf 0 setzen */
  memset ( &stModbusStatistik, 0, sizeof (stModbusStatistik) );

}

/* Deprecated fuer Linux - der GCC arbeitet fehlerhaft beim packed Attribut
   daher auf die "haessliche" 8 Bit Kopierroutine ausweichen  
*/
void MbRegisterCopy_WITH_SWAP_IF_LITTLE ( __packed UINT16 *pusDest, __packed UINT16 *pusSrc, UINT32 uiNrOfRegs )
{
  while ( uiNrOfRegs-- )  /* fuer alle 16-Bit-Werte */
  { 
    *pusDest = __cpu_to_be16 ( *pusSrc ); /* 16 Bit kopieren, dabei Bytes tauschen */
    ++pusDest;                        /* Zielzeiger weiter stellen */
    ++pusSrc;                         /* Quellzeiger weiter stellen */
  }
  return;
}



/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:  MbRegisterCopy                                             */
/*                                                                       */
/* Aufgabe:   High- und Low-Bytes eines Datenbereiches tauschen wenn CPU */
/*            im little endian Format laeuft.                            */
/*                                                                       */
/* Eingang:   uiLen = Anzahl der zu bearbeitenden Bytes                  */
/*                                                                       */
/* Ausgabe:   Bytes getauscht                                            */
/*                                                                       */
/* Rueckgabe: --                                                         */
/*                                                                       */
/*-----------------------------------------------------------------------*/
void MbRegisterCopy ( UINT8 * pusDest, UINT8 *pusSrc, UINT32 uiLength )
{
   int i;
   UINT16 usSwap;
   
   for(i=0; i < uiLength; i=i+2){
     usSwap = pusSrc[i];
     usSwap |= pusSrc[i+1] << 8;
     usSwap = __cpu_to_be16(usSwap);
     pusDest[i] = usSwap;
     pusDest[i+1] = usSwap >> 8;
   }
   return;
}


/*-----------------------------------------------------------------------*/
/*                                                                       */
/* Funktion:  CopyDigitalData                                            */
/*                                                                       */
/* Aufgabe:   Kopieren von Bits von Source nach Destination              */
/*                                                                       */
/* Eingang:   -                                                          */
/*                                                                       */
/* Ausgabe:   -                                                          */
/*                                                                       */
/* Rueckgabe: -                                                          */
/*                                                                       */
/*-----------------------------------------------------------------------*/
void CopyDigitalData ( UINT16 *ppusSource, UINT32 uiSourceBitOffset, UINT16 *ppusDestination, 
                       UINT32 uiDestinationBitOffset, UINT16 usBitCount )
{
  UINT16 usSrcBitMask, usDestBitMask;  /* Bitmasken zum kopieren */
  UINT16 *pusSource = ppusSource;
  UINT16 *pusDest   = ppusDestination;
  UINT32 i;
  
  
  /* Zeiger auf richtigen Word Offset stellen */
  pusSource += uiSourceBitOffset / 16;
  pusDest += uiDestinationBitOffset / 16;
  /* Bit Offset im aktuellen Wort berechnen */
  usSrcBitMask =  ( 0x1 << (uiSourceBitOffset % 16));
  usDestBitMask = ( 0x1 << (uiDestinationBitOffset % 16));

  for ( i = 0; i < usBitCount; i++ )
  {
    if (*pusSource & usSrcBitMask) /* Bit gesetzt ? */
    {
      *pusDest |= usDestBitMask; /* Bit gesetzt */
    }
    else
    {
      *pusDest &= ~usDestBitMask; /* Bit nicht gesetzt */
    }
    usSrcBitMask <<=1; /* naechstes Source-Bit */
    usDestBitMask <<=1; /* naechstes Destination-Bit */
    if (usSrcBitMask == 0) /* naechstes Wort ? */
    {
      usSrcBitMask = 0x1; /* Source Maske zuruecksetzen */
      pusSource++; /* Zeiger auf naechstes Wort */
    }
    if (usDestBitMask == 0) /* naechstes Wort ? */
    {
      usDestBitMask = 0x1; /* Destination Maske zuruecksetzen */
      pusDest++;  /* Zeiger auf naechstes Wort */
    }
  }
}
