/****************************************************************************
 *   $Id:: cards.c 4785 2010-09-03 22:39:27Z nxp21346                    $
 *   Project: NXP LPC11xx I2C example
 *
 *   Description:
 *     This file contains main entry.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/

/* Status code definitions */
#include <ReaderLib/types/ph_Status.h>
#include <phpalI14443p3a.h>
#include <phpalI14443p4.h>
#include <phpalFelica.h>
#include <phpalI14443p3b.h>
#include <phpalI14443p4a.h>
#include <phpalI18092mPI.h>
#include <phacDiscLoop.h>
#include <phbalReg.h>
#include <phOsal.h>
#include <phlnLlcp.h>
#include <phnpSnep.h>
#include <phalT1T.h>
#include <phalTop.h>
#include <phpalMifare.h>
#include <phalMful.h>
#include <phalMfdf.h>
#include <phalFelica.h>
#include <phKeyStore.h>
#include "cards.h"

// Arrays
/* Set the HEX code for the Select App command */
static const uint8_t AppSelection1[20] = {0x00, 0xA4, 0x04, 0x00, 0x0E, 0x31, 0x50, 0x41, 0x59,
		0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};
/* Set the HEX code for the Select App command */
static const uint8_t AppSelection2[20] = {0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59,
		0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};

#ifdef   DEBUG
#include <stdio.h>
#define  DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define  DEBUG_PRINTF(...)
#endif

#ifdef   DEBUG
static void PRINT_BUFF(uint8_t *hex, uint8_t num)
{
    uint32_t	i;

    for(i = 0; i < num; i++)
	{
    	DEBUG_PRINTF(" %02X",hex[i]);
	}
}
#else
#define  PRINT_BUFF(x, y)
#endif /* DEBUG */


/*******************************************************************************
**   DetectMifare
**   This function recognize which kind of mifare card is in field.
**   Card type will be returned.
**   If 0 returned, card isn't recognized.
*******************************************************************************/
uint32_t DetectMifare(uint8_t *pAtqa, uint8_t *bSak)
{
  uint32_t sak_atqa = 0;
  uint16_t detected_card = 0xFFFF;

  sak_atqa = bSak[0] << 24 | pAtqa[0] << 8 | pAtqa[1];
  sak_atqa &= 0xFFFF0FFF;

  /* Detect mini or classic */
  switch (sak_atqa)
  {
    case sak_mfc_1k << 24 | atqa_mfc:
      DEBUG_PRINTF("Product: MIFARE Classic\n");
      detected_card &= mifare_classic;
      break;
    case sak_mfc_4k << 24 | atqa_mfc:
      DEBUG_PRINTF("Product: MIFARE Classic\n");
      detected_card &= mifare_classic;
      break;
    case sak_mfp_2k_sl1 << 24 | atqa_mfp_s:
      DEBUG_PRINTF("Product: MIFARE Classic\n");
      detected_card &= mifare_classic;
      break;
    case sak_mini << 24 | atqa_mini:
      DEBUG_PRINTF("Product: MIFARE Mini\n");
      detected_card &= mifare_mini;
      break;
    case sak_mfp_4k_sl1 << 24 | atqa_mfp_s:
      DEBUG_PRINTF("Product: MIFARE Classic\n");
      detected_card &= mifare_classic;
      break;
    case sak_mfp_2k_sl1 << 24 | atqa_mfp_x:
      DEBUG_PRINTF("Product: MIFARE Classic\n");
      detected_card &= mifare_classic;
      break;
    case sak_mfp_4k_sl1 << 24 | atqa_mfp_x:
      DEBUG_PRINTF("Product: MIFARE Classic\n");
      detected_card &= mifare_classic;
      break;
    default:
      detected_card = 0xFFFF;
      break;
  }

  if (detected_card == 0xFFFF)
  {
    sak_atqa = bSak[0] << 24 | pAtqa[0] << 8 | pAtqa[1];
    switch (sak_atqa)
    {
      case sak_ul << 24 | atqa_ul:
        DEBUG_PRINTF("Product: MIFARE Ultralight\n");
        detected_card &= mifare_ultralight;
        break;
      case sak_mfp_2k_sl2 << 24 | atqa_mfp_s:
        DEBUG_PRINTF("Product: MIFARE Plus\n");
        detected_card &= mifare_plus;
        break;
      case sak_mfp_2k_sl3 << 24 | atqa_mfp_s:
        DEBUG_PRINTF("Product: MIFARE Plus\n");
        detected_card &= mifare_plus;
        break;
      case sak_mfp_4k_sl2 << 24 | atqa_mfp_s:
        DEBUG_PRINTF("Product: MIFARE Plus\n");
        detected_card &= mifare_plus;
        break;
      case sak_mfp_2k_sl2 << 24 | atqa_mfp_x:
        DEBUG_PRINTF("Product: MIFARE Plus\n");
        detected_card &= mifare_plus;
        break;
      case sak_mfp_2k_sl3 << 24 | atqa_mfp_x:
        DEBUG_PRINTF("Product: MIFARE Plus\n");
        detected_card &= mifare_plus;
        break;
      case sak_mfp_4k_sl2 << 24 | atqa_mfp_x:
        DEBUG_PRINTF("Product: MIFARE Plus\n");
        detected_card &= mifare_plus;
        break;
      case sak_desfire << 24 | atqa_desfire:
        DEBUG_PRINTF("Product: MIFARE DESFire\n");
        detected_card &= mifare_desfire;
        break;
      case sak_jcop << 24 | atqa_jcop:
        DEBUG_PRINTF("Payment card:\n");
        detected_card &= jcop;
        break;
      case sak_layer4 << 24 | atqa_nPA:
		DEBUG_PRINTF("German eID (neuer Personalausweis) detected\n");
		detected_card &= nPA;
		break;
      default:
        return 0;
    }
  }

  return detected_card;
}

/*******************************************************************************
**   PaymentCard
**   This function checks whether the card has a PSE or not.
**   If it does, the AID of the card will be read and the
**   card type will be returned.
*******************************************************************************/
int PaymentCard(void *pHal, uint8_t *uid)
{
	phpalI14443p4_Sw_DataParams_t I14443p4;
	phpalMifare_Sw_DataParams_t palMifare;
	phpalI14443p3a_Sw_DataParams_t I14443p3a;
	phpalI14443p4a_Sw_DataParams_t I14443p4a;

	phKeyStore_Sw_DataParams_t SwkeyStore;
	phKeyStore_Sw_KeyEntry_t   pKeyEntries;
	phKeyStore_Sw_KeyVersionPair_t   pKeyVersionPairs;
	phKeyStore_Sw_KUCEntry_t   pKUCEntries;

	uint8_t cryptoEnc[8];
	uint8_t cryptoRng[8];

	phalMful_Sw_DataParams_t alMful;

	uint8_t bUid[10];
	uint8_t bLength;
	uint8_t bMoreCardsAvailable;
	uint32_t sak_atqa = 0;
	uint8_t pAtqa[2];
	uint8_t bSak[1];
	phStatus_t status;
	uint16_t detected_card = 0xFFFF;
	uint8_t bBufferReader[0x60];
	uint8_t **ppRxBuffer = (void *)&bBufferReader[0];
	uint16_t bRxLength;
	uint32_t volatile i;
	uint8_t ansSize = 0;
	uint16_t retValue;
	uint16_t fileName;
	uint8_t aidSize = 0;
	uint8_t cardType = 0;
	int pseFlag = 0;


	/* Initialize the 14443-3A PAL (Protocol Abstraction Layer) component */
	PH_CHECK_SUCCESS_FCT(status, phpalI14443p3a_Sw_Init(&I14443p3a,
			sizeof(phpalI14443p3a_Sw_DataParams_t), pHal));

	/* Initialize the 14443-4 PAL component */
	PH_CHECK_SUCCESS_FCT(status, phpalI14443p4_Sw_Init(&I14443p4,
			sizeof(phpalI14443p4_Sw_DataParams_t), pHal));

	/* Initialize the 14443-4A PAL component */
	PH_CHECK_SUCCESS_FCT(status, phpalI14443p4a_Sw_Init(&I14443p4a,
			sizeof(phpalI14443p4a_Sw_DataParams_t), pHal));

	/* Initialize the Mifare PAL component */
	PH_CHECK_SUCCESS_FCT(status, phpalMifare_Sw_Init(&palMifare,
			sizeof(phpalMifare_Sw_DataParams_t), pHal, &I14443p4));

	/* Initialize the keystore component */
	PH_CHECK_SUCCESS_FCT(status, phKeyStore_Sw_Init(&SwkeyStore, sizeof(phKeyStore_Sw_DataParams_t),
		&pKeyEntries, 1,&pKeyVersionPairs, 1, &pKUCEntries, 1));

	/* Initialize Ultralight(-C) AL component */
	PH_CHECK_SUCCESS_FCT(status, phalMful_Sw_Init(&alMful,
		  sizeof(phalMful_Sw_DataParams_t), &palMifare, &SwkeyStore,
		  &cryptoEnc, &cryptoRng));

	/* Reset the RF field */
	PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldReset(pHal));

	/* Apply the type A protocol settings
	 * and activate the RF field. */
	PH_CHECK_SUCCESS_FCT(status,
			phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A));

	/* Empty the pAtqa */
	memset(pAtqa, '\0', 2);
	status = phpalI14443p3a_RequestA(&I14443p3a, pAtqa);

	/* Reset the RF field */
	PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldReset(pHal));

	/* Empty the bSak */
	memset(bSak, '\0', 1);

	/* Get the right payment card,
	 * use it's UID. */
	bMoreCardsAvailable = 1;
	uint8_t cards = 0;
	while (bMoreCardsAvailable)
	{
		cards++;
		/* Activate the communication layer part 3
		 * of the ISO 14443A standard. */
		status = phpalI14443p3a_ActivateCard(&I14443p3a,
				NULL, 0x00, bUid, &bLength, bSak, &bMoreCardsAvailable);

		/* Check the UID of the active card with
		 * the UID of the payment card detected
		 * in detectMifare(). */
		for(i = 0; i < sizeof(uid); i++)
		{
			if(bUid[0] != uid[0])
			{
				status = phpalI14443p3a_HaltA(&I14443p3a);
				detected_card = 0xFFFF;
				break;
			}
		}
	}

	sak_atqa = bSak[0] << 24 | pAtqa[0] << 8 | pAtqa[1];
	sak_atqa &= 0xFFFF0FFF;

	if (PH_ERR_SUCCESS == status)
	{
		DEBUG_PRINTF("\nCard is ISO-4 compliant");

		/* Iso-4 card, send RATS */
		PH_CHECK_SUCCESS_FCT(status, phpalI14443p4a_Rats(&I14443p4a,
				I14443p4a.bFsdi, I14443p4a.bCid, bBufferReader));
		DEBUG_PRINTF("\nATS successful");

		/* Iso-4 card, set protocol */
		PH_CHECK_SUCCESS_FCT(status, phpalI14443p4_SetProtocol(&I14443p4,
				PH_OFF, I14443p4a.bCid, PH_OFF, PH_OFF,
				I14443p4a.bFwi, I14443p4a.bFsdi, I14443p4a.bFsci));
		DEBUG_PRINTF("\nSet Protocol successful");

		/* Let's perform the "AppSelection" command.
		 * We have to perform one of the following commands to find
		 * out which card this is.
		 * We start with the second one ('2PAY.SYS.DDF01') and when this one
		 * does not work, we check the other one ('1PAY.SYS.DDF01').
		 */

		/* Send the Application Selection code '2PAY.SYS.DDF01' to the card and get the card's answer. */
		DEBUG_PRINTF("\nTry PSE 2PAY.SYS.DDF01.");
		memcpy(&bBufferReader[3], AppSelection2, sizeof(AppSelection2));
		PH_CHECK_SUCCESS_FCT(status, phpalI14443p4_Exchange(&I14443p4,
				PH_EXCHANGE_DEFAULT, &bBufferReader[3], sizeof(AppSelection2),
				(void *)&bBufferReader[0], &bRxLength));

		/* Check the return value. Get the output size and pick the return value. */
		ansSize = ppRxBuffer[0][1] + 2;
		retValue = ppRxBuffer[0][ansSize];
		retValue = (retValue << 8) | ppRxBuffer[0][ansSize + 1];
		DEBUG_PRINTF("\nThe return value is: ");
		PRINT_BUFF((uint8_t *)&retValue, 2);
		/* Check if the return value is an error code. When it is, do the same execution
		 * again, but with '1PAY.SYS.DDF01'. */
		if(retValue != RET_ANSWER)
		{
			DEBUG_PRINTF("\nAn error code was returned. Try PSE 1PAY.SYS.DDF01.");
			memcpy(&bBufferReader[3], AppSelection1, sizeof(AppSelection1));
			PH_CHECK_SUCCESS_FCT(status, phpalI14443p4_Exchange(&I14443p4,
					PH_EXCHANGE_DEFAULT, &bBufferReader[3], sizeof(AppSelection1),
					(void *)&bBufferReader[0], &bRxLength));
			ansSize = ppRxBuffer[0][1] + 2;
			retValue = ppRxBuffer[0][ansSize];
			retValue = (retValue << 8) | ppRxBuffer[0][ansSize + 1];
			DEBUG_PRINTF("\nThe return value is: ");
			PRINT_BUFF((uint8_t *)&retValue, 2);
		}
		/* Check the new return value. It it is an error code again, there is no PSE on the card. */
		if(retValue != RET_ANSWER)
		{
			DEBUG_PRINTF("\nAn error code was returned. There is no PSE on this card.");
			return 0;
		}

		/* START! Let's parse the card's answer. First copy the card output to an array. */
		uint8_t appSelection[255];

		for(i = 0; i < ansSize; i++)
		{
			appSelection[i] = ppRxBuffer[0][i];
		}
		DEBUG_PRINTF("\nThe output is:");
		PRINT_BUFF(&appSelection[0], ansSize);

		/* Check the file name. It's supposed to be 0x840E. 0x0E is the size
		 * of the PSE, it should always be the same. '1PAY.SYS.DDF01' and '2PAY.SYS.DDF01' have 14 symbols. */
		fileName = appSelection[2];
		fileName = (fileName << 8) | appSelection[3];
		DEBUG_PRINTF("\nThe file name is:");
		PRINT_BUFF((uint8_t *)&fileName, 2);
		if(fileName != FILE_NAME)
		{
			DEBUG_PRINTF("\nWrong file name.");
			return 0;
		}

		/* Check with function Compare whether the PSE is correct. */
		pseFlag = Compare(&appSelection[4], appSelection[3]);
		if(pseFlag != 1)
		{
			DEBUG_PRINTF("\nWrong PSE.");
			return 0;
		}

		/* Check the FCI Proprietary Template identifier byte and the its size. */
		if((appSelection[18] != 0xA5) && ((appSelection[1] - 18) != appSelection[20]))
		{
			DEBUG_PRINTF("\nWrong FCI Proprietary Template.");
			return 0;
		}

		/* Check the ADF identifier byte. */
		if(appSelection[23] != 0x61)
		{
			DEBUG_PRINTF("\nWrong ADF.");
			return 0;
		}

		/* Check the AID identifier byte. */
		if(appSelection[25] == 0x4F)
		{
			aidSize = appSelection[26];
		}
		else
		{
			DEBUG_PRINTF("\nNo or wrong AID.");
			return 0;
		}
		/* The AID gets copied into an extra array. */
		uint8_t aid[255];
		for(i = 0; i < aidSize; i++)
		{
			aid[i] = appSelection[27 + i];
		}
		DEBUG_PRINTF("\nThe AID is: ");
		PRINT_BUFF(&aid[0], aidSize);

		/* Check which kind of card is on the field. Card_Scheme returns a uint8_t value.
		 * With this value the card can be identified and use in further progress. */
		DEBUG_PRINTF("\nCheck AID: ");
		cardType = Card_Scheme(&aid[0], aidSize);
		switch(cardType)
			{
			case VISA:
				DEBUG_PRINTF("Visa credit or debit Card");
				break;
			case VISA_ELECTRON:
				DEBUG_PRINTF("Visa Electron Card");
				break;
			case VISA_VPAY:
				DEBUG_PRINTF("Visa V PAY Card");
				break;
			case VISA_PLUS:
				DEBUG_PRINTF("Visa Plus Card");
				break;
			case MASTERCARD:
				DEBUG_PRINTF("MasterCard credit or debit Card");
				break;
			case MASTERCARD_WORLDWIDE:
				DEBUG_PRINTF("MasterCard worldwide Card");
				break;
			case MASTERCARD_MAESTRO:
				DEBUG_PRINTF("MasterCard Maestro (debit) Card");
				break;
			case MASTERCARD_CIRRUS:
				DEBUG_PRINTF("MasterCard Cirrus (ATM only) Card");
				break;
			case MASTERCARD_MAESTRO_UK:
				DEBUG_PRINTF("MasterCard Maestro UK (Switch) Card");
				break;
			case AMERICAN_EXPRESS:
				DEBUG_PRINTF("American Express Card");
				break;
			case LINK:
				DEBUG_PRINTF("LINK (UK) (ATM) Card");
				break;
			case CB:
				DEBUG_PRINTF("CB credit or debit Card");
				break;
			case CB_DEBIT:
				DEBUG_PRINTF("CB debit only Card");
				break;
			case JCB:
				DEBUG_PRINTF("JCB (Japan Credit Bureau) Card");
				break;
			case DANKORT:
				DEBUG_PRINTF("Dankort debit Card");
				break;
			case COGEBAN:
				DEBUG_PRINTF("CoGeBan PagoBANCOMAT (Italy) Card");
				break;
			case DINERSCLUB_DISCOVER:
				DEBUG_PRINTF("Diners Club/Discover Card");
				break;
			case BANRISUL:
				DEBUG_PRINTF("Banrisul (Brasil) Card");
				break;
			case SPAN2:
				DEBUG_PRINTF("SPAn2 (Saudi Arabia) Card");
				break;
			case INTERAC:
				DEBUG_PRINTF("Interac (Canada) debit Card");
				break;
			case DISCOVER:
				DEBUG_PRINTF("Discover ZIP Card");
				break;
			case CHINA_UNIONPAY_DEBIT:
				DEBUG_PRINTF("China UnionPay debit Card");
				break;
			case CHINA_UNIONPAY_CREDIT:
				DEBUG_PRINTF("China UnionPay credit Card");
				break;
			case CHINA_UNIONPAY_QUASI_CREDIT:
				DEBUG_PRINTF("China UnionPay quasi credit Card");
				break;
			case CHINA_UNIONPAY_ELECTRONIC_CASH:
				DEBUG_PRINTF("China UnionPay Electronic Cash Card");
				break;
	/*			case ZKA:
				DEBUG_PRINTF("ZKA (Germany) (Girocard) Card");
				break;
			case EAPS:
				DEBUG_PRINTF("EAPS BANCOMAT (Italy) Card");
				break;*/
			case VERVE:
				DEBUG_PRINTF("Verve (Nigeria) Card");
				break;
			case RUPAY:
				DEBUG_PRINTF("RuPay (India) Card");
				break;
			case JCOP:
				DEBUG_PRINTF("JCOP Card");
				break;
			default:
				DEBUG_PRINTF("Other Card");
				break;
			}

		DEBUG_PRINTF("\n**** Deselect successful");
		PH_CHECK_SUCCESS_FCT(status, phpalI14443p4_Deselect(&I14443p4));
		return detected_card;
	}
	return detected_card;
}

/*******************************************************************************
**   Compare
**   This function checks the PSE of the card.
*******************************************************************************/
int Compare(uint8_t *input, uint8_t length)
{
	uint8_t i = 0;
	for(i = 0; i < length; i++)
	{
		if(input[i] == PSE1[i] || input[i] == PSE2[i])
		{
			//nothing to be done. just check the next PSE value.
		}
		else
		{
			return 0;
		}
	}
	DEBUG_PRINTF("\nPSE okay.");
	return 1;
}

/*******************************************************************************
**   Card_Scheme
**   This function checks the AID of the card.
*******************************************************************************/
int Card_Scheme(uint8_t *input, uint8_t length)
{
	uint8_t i = 0;
	uint8_t j = length;
	uint8_t comp;
	uint8_t card = VISA;
	unsigned long long ref = AID_VISA;

	for(i = 0; i < length; i++)
	{
		comp = (ref  >> (j - (i+1))*8);
		if(input[i] == comp)
		{
			//nothing to be done.
		}
		else
		{
			/* Get next AID and change card via card-number */
			switch(ref)
			{
			case AID_VISA:
				ref = AID_VISA_ELECTRON;
				card = VISA_ELECTRON;
				break;
			case AID_VISA_ELECTRON:
				ref = AID_VISA_VPAY;
				card = VISA_VPAY;
				break;
			case AID_VISA_VPAY:
				ref = AID_VISA_PLUS;
				card = VISA_PLUS;
				break;
			case AID_VISA_PLUS:
				ref = AID_MASTERCARD;
				card = MASTERCARD;
				break;
			case AID_MASTERCARD:
				ref = AID_MASTERCARD_WORLDWIDE;
				card = MASTERCARD_WORLDWIDE;
				break;
			case AID_MASTERCARD_WORLDWIDE:
				ref = AID_MASTERCARD_MAESTRO;
				card = MASTERCARD_MAESTRO;
				break;
			case AID_MASTERCARD_MAESTRO:
				ref = AID_MASTERCARD_CIRRUS;
				card = MASTERCARD_CIRRUS;
				break;
			case AID_MASTERCARD_CIRRUS:
				ref = AID_MASTERCARD_MAESTRO_UK;
				card = MASTERCARD_MAESTRO_UK;
				break;
			case AID_MASTERCARD_MAESTRO_UK:
				ref = AID_AMERICAN_EXPRESS;
				card = AMERICAN_EXPRESS;
				break;
			case AID_AMERICAN_EXPRESS:
				ref = AID_LINK;
				card = LINK;
				break;
			case AID_LINK:
				ref = AID_CB;
				card = CB;
				break;
			case AID_CB:
				ref = AID_CB_DEBIT;
				card = CB_DEBIT;
				break;
			case AID_CB_DEBIT:
				ref = AID_JCB;
				card = JCB;
				break;
			case AID_JCB:
				ref = AID_DANKORT;
				card = DANKORT;
				break;
			case AID_DANKORT:
				ref = AID_COGEBAN;
				card = COGEBAN;
				break;
			case AID_COGEBAN:
				ref = AID_DINERSCLUB_DISCOVER;
				card = DINERSCLUB_DISCOVER;
				break;
			case AID_DINERSCLUB_DISCOVER:
				ref = AID_BANRISUL;
				card = BANRISUL;
				break;
			case AID_BANRISUL:
				ref = AID_SPAN2;
				card = SPAN2;
				break;
			case AID_SPAN2:
				ref = AID_INTERAC;
				card = INTERAC;
				break;
			case AID_INTERAC:
				ref = AID_DISCOVER;
				card = DISCOVER;
				break;
			case AID_DISCOVER:
				ref = AID_CHINA_UNIONPAY_DEBIT;
				card = CHINA_UNIONPAY_DEBIT;
				break;
			case AID_CHINA_UNIONPAY_DEBIT:
				ref = AID_CHINA_UNIONPAY_CREDIT;
				card = CHINA_UNIONPAY_CREDIT;
				break;
			case AID_CHINA_UNIONPAY_CREDIT:
				ref = AID_CHINA_UNIONPAY_QUASI_CREDIT;
				card = CHINA_UNIONPAY_QUASI_CREDIT;
				break;
			case AID_CHINA_UNIONPAY_QUASI_CREDIT:
				ref = AID_VERVE;
				card = AID_VERVE;//CHINA_UNIONPAY_ELECTRONIC_CASH;
				break;
/*			case AID_CHINA_UNIONPAY_ELECTRONIC_CASH:
				ref = AID_ZKA;
				card = ZKA;
				break;
			case AID_ZKA:
				ref = AID_EAPS;
				card = EAPS;
				break;
			case AID_EAPS:
				ref = AID_VERVE;
				card = VERVE;
				break;*/
			case AID_VERVE:
				ref = AID_RUPAY;
				card = RUPAY;
				break;
			case AID_RUPAY:
				ref = AID_JCOP;
				card = JCOP;
				break;
			case AID_JCOP:
				card = 0;
				break;
			default:
				card = 0;
				DEBUG_PRINTF("\nOther Card or Wrong Card.");
				break;
			}
			/* Reset the counter to start from 0 for the new AID */
			if(card != 0)
			{
				i = 0;
				continue;
			}
			else
			{
				break;
			}
		}
	}
	return card;
}
/******************************************************************************
**                            End Of File
******************************************************************************/
