/*
*         Copyright (c), NXP Semiconductors Gratkorn / Austria
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is 
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

/** \file
* Build System Definitions for Reader Library Framework.
* $Author: nxp69453 $
* $Revision: 417 $
* $Date: 2013-11-14 13:45:43 +0530 (Thu, 14 Nov 2013) $
*
* History:
*  MK: Generated 15. October 2009
*
*/

#ifndef PH_NXPBUILD_H
#define PH_NXPBUILD_H

/** \defgroup ph_NxpBuild NXP Build
* \brief Controls build behaviour of components.
* @{
*/

/* NXPBUILD_DELETE: included code lines should be always removed from code */

/* NXP Build defines                            */
/* use #define to include components            */
/* comment out #define to exclude components    */

/* DEBUG build mode */


/* BAL components */

#define NXPBUILD__PHBAL_REG_STUB                /**< Stub BAL definition */
/* Commenting as this BAL would be available only for LPC platform */

#if defined (NXPBUILD__PHBAL_REG_STUB)          || \
    defined (NXPBUILD__PHBAL_REG_LPC1768SPI)    || \
    defined (NXPBUILD__PHBAL_REG_LPC1768I2C)
    #define NXPBUILD__PHBAL_REG                 /**< Generic BAL definition */
#endif

/* HAL components */

#define NXPBUILD__PHHAL_HW_RC523                /**< Rc523 HAL definition */
//#define NXPBUILD__PHHAL_HW_CALLBACK             /**< Callback HAL definition */
//#define NXPBUILD__PHHAL_HW_RC663                /**< Rc663 HAL definition */

#if defined (NXPBUILD__PHHAL_HW_RC523)        || \
    defined (NXPBUILD__PHHAL_HW_CALLBACK)     || \
    defined (NXPBUILD__PHHAL_HW_RC663)
    #define NXPBUILD__PHHAL_HW                  /**< Generic HAL definition */
#endif

/* PAL ISO 14443-3A components */

#define NXPBUILD__PHPAL_I14443P3A_SW            /**< Software PAL ISO 14443-3A definition */

#if defined (NXPBUILD__PHPAL_I14443P3A_SW)
    #define NXPBUILD__PHPAL_I14443P3A           /**< Generic PAL ISO 14443-3A definition */
#endif

/* PAL ISO 14443-3B components */

//#define NXPBUILD__PHPAL_I14443P3B_SW            /**< Software PAL ISO 14443-3B definition */

#if defined (NXPBUILD__PHPAL_I14443P3B_SW)
    #define NXPBUILD__PHPAL_I14443P3B           /**< Generic PAL ISO 14443-3B definition */
#endif

/* PAL ISO 14443-4A components */

#define NXPBUILD__PHPAL_I14443P4A_SW            /**< Software PAL ISO 14443-4A definition */
        
#if defined (NXPBUILD__PHPAL_I14443P4A_SW)
    #define NXPBUILD__PHPAL_I14443P4A           /**< Generic PAL ISO 14443-4A definition */
#endif

/* PAL ISO 14443-4 components */

#define NXPBUILD__PHPAL_I14443P4_SW             /**< Software PAL ISO 14443-4 definition */

#if defined (NXPBUILD__PHPAL_I14443P4_SW)
    #define NXPBUILD__PHPAL_I14443P4            /**< Generic PAL ISO 14443-4 definition */
#endif

/* PAL MIFARE components */

#define NXPBUILD__PHPAL_MIFARE_SW               /**< Software PAL MIFARE */

#if defined (NXPBUILD__PHPAL_MIFARE_SW)
    #define NXPBUILD__PHPAL_MIFARE              /**< Generic PAL MIFARE definition */
#endif

/* PAL ISO15693 ICode SLI components */


/* PAL Felica components */

//#define NXPBUILD__PHPAL_FELICA_SW               /**< Software PAL Felica */

#if defined (NXPBUILD__PHPAL_FELICA_SW)
    #define NXPBUILD__PHPAL_FELICA              /**< Generic PAL Felica definition */
#endif 

/* PAL ICode EPC/UID components */



/* PAL I18000p3m3 components */


/* PAL ISO 18092 components */

//#define NXPBUILD__PHPAL_I18092MPI_SW            /**< Software PAL ISO 18092 definition */

#if defined (NXPBUILD__PHPAL_I18092MPI_SW)
    #define NXPBUILD__PHPAL_I18092MPI           /**< Generic PAL ISO 18092 definition */
#endif

/* PAL ISO 18092 Target component */
//#define NXPBUILD__PHPAL_I18092MT_SW              /**< Software implementation of ISO 18092 target mode */

#if defined (NXPBUILD__PHPAL_I18092MT_SW)
    #define NXPBUILD__PHPAL_I18092MT             /**< Generic PAL ISO 18092 target definition */
#endif

/* AC dicover loop component */
//#define NXPBUILD__PHAC_DISCLOOP_TARGET
#define NXPBUILD__PHAC_DISCLOOP_SW              /**< Discovery loop software componenet */

#if defined (NXPBUILD__PHAC_DISCLOOP_SW)
    #define NXPBUILD__PHAC_DISCLOOP             /**< Generic dicovery loop component */
#endif

#define NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P3_TAGS  /**< Detect type A I3p3 cards */
#define NXPBUILD__PHAC_DISCLOOP_TYPEA_JEWEL_TAGS /**< Detect type A jewel cards */
#define NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS  /**< Detect type A I3p4 cards */
//#define NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_TAGS   /**< Detect type A P2P tags */

#if defined (NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P3_TAGS) || \
    defined (NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS) || \
    defined (NXPBUILD__PHAC_DISCLOOP_TYPEA_JEWEL_TAGS)
    #define NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS
#endif

//#define NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS /**< Detect Type F tags */

//#define NXPBUILD__PHAC_DISCLOOP_TYPEF_P2P_TAGS      /**< Detect Type F P2P tags */
#if defined (NXPBUILD__PHAC_DISCLOOP_TYPEF_P2P_TAGS)
   #define NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS
#endif

//#define NXPBUILD__PHAC_DISCLOOP_TYPEB_I3P3B_TAGS    /**< Detect TypeB, I3P3B tags */
//#define NXPBUILD__PHAC_DISCLOOP_TYPEB_I3P4B_TAGS    /**< Detect TypeB, I3P4B tags */
//#define NXPBUILD__PHAC_DISCLOOP_TYPEB_BPRIME_TAGS   /**< Detect TypeB, B Prime tags */

#if defined (NXPBUILD__PHAC_DISCLOOP_TYPEB_I3P3B_TAGS) || \
    defined (NXPBUILD__PHAC_DISCLOOP_TYPEB_I3P4B_TAGS) || \
    defined (NXPBUILD__PHAC_DISCLOOP_TYPEB_BPRIME_TAGS)
    #define NXPBUILD__PHAC_DISCLOOP_TYPEB_TAGS          /**< Detect TypeB Tags */
#endif

/* OSAL components */
#define  NXPBUILD__PH_OSAL_STUB                  /**< Software OSAL for STUB */

#if defined (NXPBUILD__PH_OSAL_STUB)
    #define NXPBUILD__PH_OSAL                           /**< Generic OSAL definition */
#endif

/* AL Felica components */

#define NXPBUILD__PHAL_FELICA_SW                /**< Software AL Felica */

#if defined (NXPBUILD__PHAL_FELICA_SW)
    #define NXPBUILD__PHAL_FELICA               /**< Generic AL Felica definition */
#endif 

/* AL MIFARE Classic components */

#define NXPBUILD__PHAL_MFC_SW                   /**< Software MIFARE Classic */

#if defined (NXPBUILD__PHAL_MFC_SW)
    #define NXPBUILD__PHAL_MFC                  /**< Generic AL MIFARE Classic definition */
#endif

/* AL MIFARE Plus components */


#define NXPBUILD__PHAL_MFDF_SW                  /**< Software MIFARE DESFire */

#if defined (NXPBUILD__PHAL_MFDF_SW)
    #define NXPBUILD__PHAL_MFDF                  /**< Generic AL MIFARE Desfire definition */
#endif


/* AL MIFARE Ultralight components */

#define NXPBUILD__PHAL_MFUL_SW                  /**< Software MIFARE Ultralight */

#if defined (NXPBUILD__PHAL_MFUL_SW)
    #define NXPBUILD__PHAL_MFUL                 /**< Generic AL MIFARE Ultralight definition */
#endif

/* AL T1T components */

#define NXPBUILD__PHAL_T1T_SW                  /**< Software Type 1 Tag */

#if defined (NXPBUILD__PHAL_T1T_SW)
    #define NXPBUILD__PHAL_T1T                 /**< Generic AL Type 1 Tag definition */
#endif

/* AL TOP components */

#define NXPBUILD__PHAL_TOP_SW                  /**< Software Tag OPerations */

#if defined (NXPBUILD__PHAL_TOP_SW)
    #define NXPBUILD__PHAL_TOP                 /**< Generic AL Tag Operations */
#endif

/* AL Virtual Card Architecture components */


/* DL FPGA Box components */


/* DL Amplifier components */


/* DL Thermostream components */


/* DL Oscilloscope components */


/* DL Master Amplifier Oscilloscope components */


/* DL Stepper components */


/* AL I15693 components */


/* AL Sli components */


/* AL ISO 18000P3M3 components */


/* AL P40 PrivateCommands components */

/* AL P40 PublicCommand components */

/* CID Manager components */

#define NXPBUILD__PH_CIDMANAGER_SW              /**< Software CID Manager */

#if defined (NXPBUILD__PH_CIDMANAGER_SW)
    #define NXPBUILD__PH_CIDMANAGER             /**< Generic CID Manager definition */
#endif

/* Crypto components */


/* CryptoRng components */


/* KeyStore components */

#define NXPBUILD__PH_KEYSTORE_SW                /**< Software KeyStore */
#define NXPBUILD__PH_KEYSTORE_RC663             /**< RC663 KeyStore */

#if defined (NXPBUILD__PH_KEYSTORE_SW)      || \
    defined (NXPBUILD__PH_KEYSTORE_RC663)
    #define NXPBUILD__PH_KEYSTORE               /**< Generic KeyStore definition */
#endif

/* LN components */

//#define NXPBUILD__PHLN_LLCP_FRI                /**< Software Link layer */

#if defined (NXPBUILD__PHLN_LLCP_FRI)
    #define NXPBUILD__PHLN_LLCP               /**< Generic Link layer definition */
#endif

/* SNEP components */

//#define NXPBUILD__PHNP_SNEP_FRI                /**< SNEP FRI definition */

#if defined (NXPBUILD__PHNP_SNEP_FRI)
    #define NXPBUILD__PHNP_SNEP                  /**< Generic HAL definition */
#endif

/* TMIUtils component */
/* Log component */

/*#define NXPBUILD__PH_LOG*/

/** @}
* end of ph_NxpBuild
*/

#endif /* PH_NXPBUILD_H */
