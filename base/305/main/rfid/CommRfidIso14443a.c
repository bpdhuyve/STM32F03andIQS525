//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of ISO/IEC14443 RFID protocol (especially IEC14443-3)
// Asumption: The specific transceiver driver should implement timeouts.
//            Multiple chips have a timer on board to do this, so that's why
//            this isn't implemented in this module
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//
//  ALL PAGE REFERENCES IN COMMENTS IN THIS SECTION REFER TO IEC-14443-3/2016
//
//================================================================================================//
#define COMMRFIDISO14443A_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COMMRFIDISO14443A_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               COMMRFIDISO14443A_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef LEAVE_IDLE_STATE_WITH_REQ_A
    #define LEAVE_IDLE_STATE_WITH_REQ_A         0
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section
//#include "rfid\DrvRfidTransceiverNxpPn512.h"

//STD lib include section

//COM lib include section

//APP include section
#include "CommRfidIso14443a.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define REQA                    0x26   /**defined on p.16 (request PICCs of appropriate type to respond if they are avalilable for initialization)*/
#define WUPA                    0x52   /**defined on p.16 */
#define CASCADE_LEVEL_1         0x93    //(defined in IEC14443-3:2016 p21)
#define CASCADE_LEVEL_2         0x95
#define CASCADE_LEVEL_3         0x97
#define CASCADE_TAG             0x88    //indicates that Uid is not complete see p24 'CT'
#define MIFARE_READ             0x30    //Mifare read command; see MF1S50YYX_V1.pdf
#define MIFARE_WRITE            0xa0
#define MIFARE_HALT             0x50
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
/**
*@brief	Three of the five commands defined in IEC14443-3:2016 p16
*/
static BOOL Iso14443a_RequestA(void); //IEC: REQA
static BOOL Iso14443a_CardAnticollisionLoop(U8 cascade_code, U8 number_of_valid_bits); //IEC: ANTICOLLISOIN
static BOOL Iso14443a_Select(U8 cascade_code, U8 number_of_valid_bits, U8* uid_ptr, U8* sak); //IEC: SELECT
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static RFID_ISO14443A_CALLBACK_HOOK     callback_hook;
static RFID_TRANSCEIVER_HNDL            transceiver_hndl;
static U8                               tx_buffer[50];
static U8                               tx_buffer_length;
static U8                               rx_buffer[50];
static U8                               rx_buffer_length;
static U8                               complete_uid[10];
static U8                               complete_uid_length;
static U8                               key_buffer[6];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//


//  ALL PAGE REFERENCES IN COMMENTS IN THIS SECTION REFER TO IEC-14443-3/2016
//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
/**
*@brief	Implements the REQA command(REQuest command type A) or WUPA (WakeUp type A)
*       Type A = communication signal interface defined in IEC14443-2
*                determines bitrate, modulation etc..
*@note  mandatory function acc IEC14443-3, to be sent by the PCD to probe the field
*       for PICCs of type A. PCCs of type A are requested to answer.
*       !!! Must be called before the ISO14443A Anticollision/Activation Loop!!!
*       !!! See flowchart p.18
*       The actual content of ATQA is not read. AN10833 p.8 states:
*       In real application the content details of the ATQA are recommended to be ignored
*       So although the UiD length is in the ATQA, we do not read it.
*@retval TRUE when one PICC answered correctly
*/
static BOOL Iso14443a_RequestA(void)
{ 
    // PROCESS:	[1] send REQA or WUPA command (Wake uP Type A)
    //			[2] receive ATQA (Answer To reQuest type A)
    //          (first two blocks of flowchart p18.
    // -------------------------------------------------------------
    
    // Prepare the driver for initiating the REQA/ WUPA command
    // 7 bit commands!!!! 
    // CRC is switched off in DrvRfidTransceiver_Iso14443aPrepareRequestA(transceiver_hndl)
    DrvRfidTransceiver_Iso14443aPrepareRequestA(transceiver_hndl);
    
    //determine whether WUPA or REQA must be sent
#if LEAVE_IDLE_STATE_WITH_REQ_A
    tx_buffer[0] = REQA;
#else
    tx_buffer[0] = WUPA;
#endif
    tx_buffer_length = 1;
    
    // [1]Send WUPA or REQA and 
    // [2]receive the ATQA (Answer To reQuest type A) in rx_buffer
    // PICCs answer with 16 bits: b8-b7=[7:6]=UID size; 
    // b5-b1[4:0]=bit frame anticollision (one hot); IEC14443-3 p19
    DrvRfidTransceiver_SetTimer(transceiver_hndl, 300); //allow 300 us for this
    switch(DrvRfidTransceiver_Transceive(transceiver_hndl, tx_buffer, tx_buffer_length, rx_buffer, &rx_buffer_length))
    {
    case RFID_NO_ERROR:
        // ico RFID_NO_ERROR check the ATQA in rx_buffer
        // acc IEC14443A-3 ATQA is 16 bits long. The least significant byte is sent first
        if(rx_buffer_length == 2) //rx_buffer_length is in bytes!
        {
            LOG_DBG("ATQA: %02h", PU8A(rx_buffer, rx_buffer_length)); //ATQA = Answer To reQuestA (REQA)
            break;
        }
        else if(rx_buffer_length != 0)
        {
            LOG_DBG("[RFID]: Invalid ATQA length - %d", PU8(rx_buffer_length));
        }
        return FALSE;
    case RFID_COLLISION_ERROR:
        return FALSE;
    case RFID_TIMEOUT_ERROR:
        return FALSE;
    default:
        return FALSE;
    }
    
    if(callback_hook != NULL)
    {
        callback_hook(RFID_MSSG_TYPE_ATQA, rx_buffer, rx_buffer_length);
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//

/**
*@brief	The anticollisionloop
*@note  See p.17 & p19-21 of IEC14443-3(partial implementation!)
*       'step' comments refer to flowchart on p21
*       This function is recursive and is normally first called with
*       cascade_code = CASCADE_LEVEL_1
*       Call Iso14443a_RequestA (with valid ATQA response) before calling this function!!!
*@param	param: cascade_code = cascade_level (IEC p21)('0x93','0x95','0x97')
*			   number_of_valid_bits = NVB defined on p22
*@retval when returning TRUE, the PICC is selected	(=ACTIVE state)
*/
static BOOL Iso14443a_CardAnticollisionLoop(U8 cascade_code, U8 number_of_valid_bits)
{
	U8	i = 0;
    U8 sak; //SAK = Select AcKnowledge Type A = selected card has acknowledged
     
    //alignment of bits
    DrvRfidTransceiver_Iso14443aPrepareAnticollision(transceiver_hndl);
    
    //switch off CRC
    DrvRfidTransceiver_DisableCrc(transceiver_hndl);

    //prepare step 3
    tx_buffer[0] = cascade_code;    //SEL
    tx_buffer[1] = number_of_valid_bits; //NVB
    tx_buffer_length = 2;
    
    //step: 3 & 4
    DrvRfidTransceiver_SetTimer(transceiver_hndl, 300); //allow 300 us for this
    switch(DrvRfidTransceiver_Transceive(transceiver_hndl, tx_buffer, tx_buffer_length, rx_buffer, &rx_buffer_length))
    {
    case RFID_NO_ERROR:
        //transceive OK
        if(rx_buffer_length != 5) //uid0  uid1  uid2  uid3  BCC
        {
            LOG_WRN("[RFID]: Invalid length - %d", PU8(rx_buffer_length));
            return FALSE;
        }
        break;
    default:
        //transceive NOK
        return FALSE;
    }
    
    LOG_DBG("[RFID]: ANTICOLLISION FRAME %02h", PU8A(rx_buffer, rx_buffer_length));
    
    switch(cascade_code)
    {
    case CASCADE_LEVEL_1: //0x93
        if(rx_buffer[0] == CASCADE_TAG)
        {
            // UID not complete (see p24)
            // store the partial UID and recall the anticollision with CASCADE_LEVEL_2
            // The CT (= Cascade Tag, Type A) byte indicates that the UID is not received completely yet. 
            // It indicates that another anticollision loop on the next higher cascade level is required 
            // to get the complete UID. (AN10833 p.7 fig 2
            for(i = 0; i < 3; i++)
            {
                //store first part of UID (bytes 0 to 2) (p24)
                complete_uid[i] = rx_buffer[i + 1];
            }
            complete_uid_length = 3; //this is the preliminary length, since UID is not complete
            
            // select the device to enable further communication
            // rx_buffer contains: '88'|uid0|uid1|uid2|BCC (so 5 bytes) (p24)
            // Iso14443a_Select(..) will extend these to 7 bytes acc IEC
            // PICC should answer with UID not complete bit[2] in SAK = 1
            if((!Iso14443a_Select(CASCADE_LEVEL_1, 0x50, rx_buffer, &sak)) || ((sak & 0x04) != 0x04))
            {
                LOG_WRN("[RFID]: Invalid SAK");
                return FALSE;
            }
            
            // perform a next anticollision for level 2
            return Iso14443a_CardAnticollisionLoop(CASCADE_LEVEL_2, 0x20); //cascade_code: 95, 2 bytes
        }
        else
        {
            // UID complete
            // rx_buffer_length is 5 since the BCC is also sent.
            // So one could recalculate the BCC by exoring the 4 uid's and comparing the result
            // with rx_buffer[4]. Is not done here.
            if ((rx_buffer[0]^rx_buffer[1]^rx_buffer[2]^rx_buffer[3]) != rx_buffer[4] )
                return FALSE;
            
            for(i = 0; i < 4; i++)
            {
                //store uid bytes 0 to 3 (p24)
                complete_uid[i] = rx_buffer[i];
            }
            complete_uid_length = 4;
            
            // step 11 & 12 Select the PICC and receive SAK
            // rx_buffer contains: |uid0|uid1|uid2|uid3|BCC (so 5 bytes) (p24)
            // Iso14443a_Select(..) will extend these to 7 bytes acc IEC 
            if(!Iso14443a_Select(CASCADE_LEVEL_1, 0x50, rx_buffer, &sak))
            {
                LOG_WRN("[RFID]: Invalid SAK");
                return FALSE;
            }
        }
        break;
    case CASCADE_LEVEL_2:
        if(rx_buffer[0] == CASCADE_TAG)
        {
            // UID not complete
            for(i = 0; i < 3; i++)
            {
                // store second part of UID (bytes 3 to 5) (p24)
                complete_uid[i + 3] = rx_buffer[i + 1];
            }
            complete_uid_length += 3; // this is the preliminary length, since UID is not complete
            
            // select the device to enable further communication
            // rx_buffer contains: '88'|uid3|uid4|uid5|BCC (so 5 bytes) (p24)
            // Iso14443a_Select(..) will extend these to 7 bytes acc IEC
            // PICC should answer with UID not complete bit[2] in SAK = 1
            if((!Iso14443a_Select(CASCADE_LEVEL_2, 0x50, rx_buffer, &sak)) || ((sak & 0x04) != 0x04))
            {
                LOG_WRN("[RFID]: Invalid SAK");
                return FALSE;
            }
            
            // perform a next anticollision for level 3
            return Iso14443a_CardAnticollisionLoop(CASCADE_LEVEL_3, 0x20);
        }
        else
        {
            // UID complete
            for(i = 0; i < 4; i++)
            {
                // store second part of UID (bytes 3 to 6) (p24)
                complete_uid[i + 3] = rx_buffer[i];
            }
            complete_uid_length = 7;
            
            // select the device to enable further communication
            if(!Iso14443a_Select(CASCADE_LEVEL_2, 0x50, rx_buffer, &sak))
            {
                LOG_WRN("[RFID]: Invalid SAK");
                return FALSE;
            }
        }
        break;
    case CASCADE_LEVEL_3:
        if(rx_buffer[0] == CASCADE_TAG)
        {
            LOG_WRN("[RFID]: Invalid CASCADE level");
            return FALSE;
        }
        else
        {
            // UID complete
            for(i = 0; i < 4; i++)
            {
                // store third part of UID (bytes 6 to 9) (p24)
                complete_uid[i + 6] = rx_buffer[i];
            }
            complete_uid_length = 10;
            
            // select the device to enable further communication
            if(!Iso14443a_Select(CASCADE_LEVEL_3, 0x50, rx_buffer, &sak))
            {
                LOG_WRN("[RFID]: Invalid SAK");
                return FALSE;
            }
        }
        break;
    default:
        return FALSE;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//

/**
*@brief	The purpose of the Select command (sequence) is to get the UID from one PICC 
*       and to select this PICC for further communication. Used during an anticollision loop.
*@note  Select sequence flowchart: IEC 14443A p18
*       In case of no errors, the CRC-block is on, in case of errors it is undefined if the 
*       CRC-block is switched on or not (see code)
*@param	param: cascade_code = cascade_level (p21)
*			   number_of_valid_bits = as defined on p22 (special way of coding!)
*              uid_ptr = the uid of the selected PICC, (used as in- and output of this function)
*              sak = Select AcKnowledge byte of PICC
*@retval	
*/
static BOOL Iso14443a_Select(U8 cascade_code, U8 number_of_valid_bits, U8* uid_ptr, U8* sak)
{    
    // According IEC14443-3:2016 sec 6.4
    // The format of the select frame is: (values between ( ) are number of bits)
    // | SEL (8) | NVB (8) | UID CLn (5*8) |
    //
    // SEL = Select code, here called cascade code(table7 p21)
    // NVB = Number of Valid Bits (table 8 p22 special way of coding!)
    // UID Cln = Unique IDentifier of Class n; class = cascade level
    
    tx_buffer[0] = cascade_code;
    tx_buffer[1] = 0x70; //flowchart p21 step 11 (0x70 = NVB = 7 bytes acc table 8 p22)
    MEMSET(&tx_buffer[2], 0x00, 5); //5 extra bytes for the UID + BCC
    // set tx_buffer_length equal to NVB bytes PRELIMINARY, +1 byte optionally for possible extra bits
    tx_buffer_length = (U8)((number_of_valid_bits & 0xF0) >> 4); 
    if((number_of_valid_bits & 0x0F) > 0)
    {
        tx_buffer_length++; //add an extra byte for possible extra bits
    }
    
    // Set UID data in the tx_buffer
	MEMCPY(&tx_buffer[2], uid_ptr, tx_buffer_length);
    //increment the tx_buffer_length by 2 to take SEL and NVB into account 
	tx_buffer_length += 2;
    // Append BCC (Block Check Character: p12)
    tx_buffer[6] = uid_ptr[0] ^ uid_ptr[1] ^ uid_ptr[2] ^ uid_ptr[3];
    
    LOG_DBG("[RFID] SELECT FRAME: %02h", PU8A(tx_buffer, tx_buffer_length));
    
    // alignment of bits
    DrvRfidTransceiver_Iso14443aPrepareSelect(transceiver_hndl);
    
    // CRC needs to be enabled when sending the SELECT command
    DrvRfidTransceiver_EnableCrc(transceiver_hndl);
    
    // send the Select command, the PN512 should answer with gthe received SAK 
    // (Select AcKnowledge) in rx_buffer[0]
    DrvRfidTransceiver_SetTimer(transceiver_hndl, 300); //allow 300 us for this
    switch(DrvRfidTransceiver_Transceive(transceiver_hndl, tx_buffer, tx_buffer_length, rx_buffer, &rx_buffer_length))
    {
    case RFID_NO_ERROR:
        LOG_DBG("SAK: %02h", PU8A(rx_buffer, rx_buffer_length));
        if(rx_buffer_length != 1)
        {
            LOG_WRN("[RFID]: Invalid SAK length");
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
    
    // Reset RxAlignment
    DrvRfidTransceiver_Iso14443aExitSelect(transceiver_hndl);
    
    // implementation of Table 9 IEC14443-3 p23
    if((rx_buffer[0] & 0x04) == 0x04) //= uid not complete
    {
        // another cascade level is coming, except if the current cascade level
        // is equal to 0x97. In that case, there is an error.
        if((cascade_code == CASCADE_LEVEL_3) && (uid_ptr[0] == CASCADE_TAG))
        {
            LOG_WRN("[RFID]: Protocol error");
            return FALSE;
        }
    }
    else
    {
        if(uid_ptr[0] == CASCADE_TAG)
        {
            //uid_ptr[0] == 0x88 means it's a cascadetag, but SAK says uid complete
            LOG_WRN("[RFID]: Protocol error");
            return FALSE;
        }
    }
    
    // Copy received SAK
    *sak = rx_buffer[0];
    
    if(callback_hook != NULL)
    {
        callback_hook(RFID_MSSG_TYPE_SAK, rx_buffer, 1);
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//




//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommRfidIso14443a_Init(RFID_TRANSCEIVER_HNDL rfid_transceiver_hndl)
{
    MODULE_INIT_ONCE();
    
    callback_hook    = NULL;
    transceiver_hndl = rfid_transceiver_hndl;
    
    tx_buffer_length = 0;
    rx_buffer_length = 0;
    
    MEMSET(tx_buffer, 0, SIZEOF(tx_buffer));
    MEMSET(rx_buffer, 0, SIZEOF(rx_buffer));
    
    complete_uid_length = 0; //the current uid is not valid anymore
    
    //default key is 6 times 0xff
    //See MF1S50YYX_V1 (Rev. 3.2) sec 8.6.3
    MEMSET(key_buffer, 0xff, SIZEOF(key_buffer));     
    
    //Acc AN1304; ico NDEF format:
    //Application directory key: 0xA0A1A2A3A4A5 blocks0-3
    //NFC Forum key: 0xd3f7d3f7d3f7 for the following sectors
//    key_buffer[0] = 0xA0;
//    for (int i = 0; i < 5; i++)
//    {
//        key_buffer[i+1] = key_buffer[i] + 1;
//    }
//    key_buffer[0] = 0xd3;
//    key_buffer[1] = 0xf7;
//    key_buffer[2] = 0xd3;
//    key_buffer[3] = 0xf7;
//    key_buffer[4] = 0xd3;
//    key_buffer[5] = 0xf7;
    
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//

/**
*@brief	Register function for hook that will be called when an iso14443a compliant tag
        enters the RF-field of the reader
*/
void CommRfidIso14443a_RegisterCallbackHook(RFID_ISO14443A_CALLBACK_HOOK tag_found_hook)
{
    callback_hook = tag_found_hook;
}
//------------------------------------------------------------------------------------------------//
/**
*@brief	Used to detect ISO14443A conform tags
*       partially according flowchart IEC14443-3:2016 fig9 p21
*@note	Background handler called by AppMainCheckRfid(void* dataPtr) to detect ISO14443A conform tags 
*/
void CommRfidIso14443a_Handler(void)
{
    BOOL tag_found = FALSE;
    
    // Is used to detect ISO14443A conform tags
    // PROCESS:	[1] load ISO14443A protocol
    //			[2] turn on RF driver
    //			[3] do a complete anticollision sequence
    //			[4] turn off RF driver
    // --------------------------------------------------
    
    // [1] load ISO14443A protocol
    DrvRfidTransceiver_SetProtocol(transceiver_hndl, RFID_PROTOCOL_ISO_14443A);
    
    // [2] turn on RF driver
    DrvRfidTransceiver_SetRfField(transceiver_hndl, TRUE);
    
    // [3] probe the field for cards of type A
    //     do a complete anticollision sequence 
    //     and in case a card is detected, select it.
    if(Iso14443a_RequestA())
    {
        if(Iso14443a_CardAnticollisionLoop(CASCADE_LEVEL_1, 0x20)) //0x20 = 2 bytes
        {
            tag_found = TRUE;
            LOG_DBG("[RFID]: Complete UID: %02h", PU8A(complete_uid, complete_uid_length));
            if(callback_hook != NULL)
            {
                callback_hook(RFID_MSSG_TYPE_ID, complete_uid, complete_uid_length);
            }
        }
    }
    
    if(callback_hook != NULL)
    {
        callback_hook(RFID_MSSG_TYPE_TAG_PRESENT, (U8*)(&tag_found), 1);
    }
    
    // [4] turn off RF driver
    DrvRfidTransceiver_SetRfField(transceiver_hndl, FALSE);
}
//------------------------------------------------------------------------------------------------//

/**
*@brief	Reads an entire sector of the selected PICC (= 4 * 16 bytes)
*       The PICC should be selected before calling this function.
*       CommRfidIso14443a_Handler() is responsible for the selection.
*/  
BOOL CommRfidIso14443a_ReadCardData(U8 address, U8* card_data_ptr, U8* card_data_length)
{
    RFID_ERROR status = RFID_NO_ERROR;
     
    //check if a PICC is selected
    if (complete_uid_length == 0)
    {
        return FALSE;
    }
    
    DrvRfidTransceiver_EnableCrc(transceiver_hndl);  
    
    // authenticate the card ; DOES NOT WORK at this moment; returns a protocolErr (p29 spec PN512)
//    status = DrvRfidTransceiver_Authenticate(transceiver_hndl, address, key_buffer, complete_uid, complete_uid_length);
    if (status != RFID_NO_ERROR)
    {
        return FALSE;
    }
    
    tx_buffer[0] = MIFARE_READ;  
    tx_buffer[1] = address;
    tx_buffer_length = 2;
    
    DrvRfidTransceiver_SetTimer(transceiver_hndl, 5000); //allow 5 ms for this
    status = DrvRfidTransceiver_TransceiveRx(transceiver_hndl, tx_buffer, tx_buffer_length, rx_buffer, &rx_buffer_length);
    
    //stop the encrypted communication with the card
    DrvRfidTransceiver_Stop_Encryption(transceiver_hndl);
    
    if (status != RFID_NO_ERROR)
    {
        return FALSE;
    }
    
    switch(status)
    {
    case RFID_NO_ERROR:
        MEMCPY(card_data_ptr, rx_buffer, rx_buffer_length);
        *card_data_length = rx_buffer_length;
        LOG_DBG("[RFID]: Page %d: %02h", PU8(address), PU8A(card_data_ptr, *card_data_length));
        break;
    case RFID_COLLISION_ERROR:
        return FALSE;
    case RFID_TIMEOUT_ERROR:
        return FALSE;
    case RFID_CRC_ERROR:
        return FALSE;
    default:
        LOG_WRN("[RFID]: Unknown ERROR");
        return FALSE;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
/**
*@brief	Write data to the card
*/    
BOOL CommRfidIso14443a_WriteCardData(U8 address, U8* data_ptr, U8 data_length)
{
    RFID_ERROR status;
        
    //check if a PICC is selected
    if (complete_uid_length == 0)
    {
        return FALSE;
    }
    
    //set the crc on for the authentication
    DrvRfidTransceiver_EnableCrc(transceiver_hndl);
    
//    //Authenticate the card
//    status = DrvRfidTransceiver_Authenticate(transceiver_hndl, address, &key_buffer[0], &complete_uid[0], complete_uid_length);
//    if (status != RFID_NO_ERROR)
//    {
//        return FALSE;
//    }
    
    //++++++++++++THE ACTUAL WRITING MUST BE DONE HERE++++++++++++++/
    //acc MF1S50YYX_V1 p22
    // part 1:
    // fill a tx_buffer:
    tx_buffer[0] = MIFARE_WRITE;  //0xa0
    tx_buffer[1] = address; //address 0x10 = 16dec (= block 2)
    tx_buffer_length = 2;
    // two extra crc bytes must be added by the PN512
    DrvRfidTransceiver_EnableCrc(transceiver_hndl);
    // send this to the PICC
    // PICC must ack or nack (ack = 0xa; nack: 0x0=invalid operation; 0x1=parity or crc err; 0x4 invalid oper; 0x5 parity or crc err
    DrvRfidTransceiver_SetTimer(transceiver_hndl, 10000); //allow 10 ms for this
    status = DrvRfidTransceiver_Transceive(transceiver_hndl, tx_buffer, tx_buffer_length, rx_buffer, &rx_buffer_length);
    
    // part 2:
    // fill a tx_buffer with 16 bytes of data
    MEMCPY(tx_buffer, data_ptr, data_length);
    tx_buffer_length = data_length;
    // two extra crc bytes must be added by the PN512
    DrvRfidTransceiver_EnableCrc(transceiver_hndl);
    DrvRfidTransceiver_SetTimer(transceiver_hndl, 5000); //allow 5 ms for this
    status = DrvRfidTransceiver_Transceive(transceiver_hndl, tx_buffer, tx_buffer_length, rx_buffer, &rx_buffer_length);
    // send this to the PICC
    // PICC must ack or nack
    
    
    
    
    //stop the encrypted communication with the card
    status = DrvRfidTransceiver_Stop_Encryption(transceiver_hndl);
    if (status != RFID_NO_ERROR)
    {
        return FALSE;
    }
    
    return TRUE; 
}
//================================================================================================//