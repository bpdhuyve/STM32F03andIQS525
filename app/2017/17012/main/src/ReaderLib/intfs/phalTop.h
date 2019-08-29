/*
*         Copyright (c), NXP Semiconductors Bangalore / India
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
* Generic Tag Operation Application Layer Component of Reader Library Framework.
* $Author: nxp66431 $
* $Revision: 336 $
* $Date: 2013-08-08 20:14:50 +0530 (Thu, 08 Aug 2013) $
*/

#ifndef PHALTOP_H
#define PHALTOP_H

#include <ph_Status.h>
#include <phacDiscLoop.h>
#include <phalT1T.h>
#include <phalMful.h>
#include <phalMfdf.h>
#define __DEBUG

#ifdef __cplusplus
extern "C" {
#endif    /* __cplusplus */


#ifdef NXPBUILD__PHAL_TOP_SW

/** \defgroup phalTop_Sw Component : Software
* @{
*/
#define PHAL_TOP_SW_ID                           0x01U       /**< ID for Software Tag Operation layer */

#define PHAL_TOP_T1T_MAX_MEM_CTRL_TLV            0x01U        /**< Max Memory control TLV count for T1T */
#define PHAL_TOP_T1T_MAX_LOCK_CTRL_TLV           0x01U        /**< Max Lock control TLV count for T1T */
#define PHAL_TOP_T1T_MAX_PROPRIETARY_TLV         0x01U        /**< Max Proprietary TLV count for T1T */
#define PHAL_TOP_T1T_SEGMENT_SIZE                0x80U        /**< T1T Segment size */
#define PHAL_TOP_T1T_BLOCKS_PER_SEGMENT          0x10U        /**< Number of blocks per segment for T1T */
#define PHAL_TOP_T1T_NDEF_NMN                    0xE1U        /**< NDEF Magical Number for T1T */
#define PHAL_TOP_T1T_NDEF_SUPPORTED_VNO          0x10U        /**< Version Number for T1T */

#define PHAL_TOP_T2T_MAX_MEM_CTRL_TLV            0x01U        /**< Max Memory control TLV count for T2T */
#define PHAL_TOP_T2T_MAX_LOCK_CTRL_TLV           0x01U        /**< Max Lock control TLV count for T2T */
#define PHAL_TOP_T2T_MAX_PROPRIETARY_TLV         0x01U        /**< Max Proprietary TLV count for T2T */
#define PHAL_TOP_T2T_SECTOR_SIZE                 0x400U       /**< T2T Segment size */
#define PHAL_TOP_T2T_BLOCKS_PER_SECTOR           0xFFU        /**< Number of blocks per sector for T2T */
#define PHAL_TOP_T2T_NDEF_MAGIC_NUMBER           0xE1U        /**< NDEF Magical Number for T2T */
#define PHAL_TOP_T2T_NDEF_SUPPORTED_VNO          0x10U        /**< Version Number for T2T */

#define PHAL_TOP_T3T_BLOCK_SIZE                  0x10U        /**< Number of blocks per segment for T3T */
#define PHAL_TOP_T3T_WRITE_FLAG_ADDRESS          0x09U        /**< Write Flag Byte Location  in Attribute Information Block */
#define PHAL_TOP_T3T_NDEF_SUPPORTED_VNO1         0x10U        /**< Version Number 1 for T3T */
#define PHAL_TOP_T3T_NDEF_SUPPORTED_VNO2         0x11U        /**< Version Number 2 for T3T */

#define PHAL_TOP_T4T_NDEF_SUPPORTED_VNO1         0x10U        /**< Version Number 1 for T4T */
#define PHAL_TOP_T4T_NDEF_SUPPORTED_VNO2         0x20U        /**< Version Number 2 for T4T */

/** \name phalTop Custom Error Codes
*/
/** @{ */
#define PHAL_TOP_ERR_NO_VALID_NDEF_ON_TAG          (PH_ERR_CUSTOM_BEGIN + 0)     /**< No valid NDEF Message on Tag */
#define PHAL_TOP_ERR_READONLY_TAG                  (PH_ERR_CUSTOM_BEGIN + 1)     /**< Tag is Read Only */
#define PHAL_TOP_ERR_INVALID_TAG                   (PH_ERR_CUSTOM_BEGIN + 2)     /**< Tag is not NDEF Compatible. */
#define PHAL_TOP_ERR_NOT_FORMATTED                 (PH_ERR_CUSTOM_BEGIN + 3)     /**< Tag Not Formatted. */
#define PHAL_TOP_T3T_ERR_ALREADY_FORMATTED         (PH_ERR_CUSTOM_BEGIN + 4)     /**< T3T Tag already Formatted. */
#define PHAL_TOP_T4T_ERR_NO_NDEF_TLV               (PH_ERR_CUSTOM_BEGIN + 5)     /**< No NDEF TLV on T4T Tag. Use FormatNDEF. */
#define PHAL_TOP_T4T_ERR_ALREADY_FORMATTED         (PH_ERR_CUSTOM_BEGIN + 6)     /**< T4T Tag already Formatted and cannot create same Application. */
#define PHAL_TOP_T4T_ERR_INVALID_LENGHT            (PH_ERR_CUSTOM_BEGIN + 7)     /**< T4T Tag is Read Only */
/** @} */

/**
* \brief T1T Lock Control TLV parameter structure
*/
typedef struct phalTop_T1T_LockCtrlTlv
{
    uint16_t wOffset;                                             /**< TLV offset */
    uint16_t wByteAddr;                                           /**< Lock bytes start address/offset */
    uint8_t bSizeInBits;                                          /**< Lock bytes size in bits */
    uint8_t bBytesLockedPerBit;                                   /**< Number of bytes locked per lock bit */
}phalTop_T1T_LockCtrlTlv_t;

/**
* \brief T2T Lock Control TLV parameter structure
*/
typedef struct phalTop_T2T_LockCtrlTlv
{
    uint16_t wOffset;                                             /**< TLV offset */
    uint16_t wByteAddr;                                           /**< Lock bytes start address/offset */
    uint8_t bSizeInBits;                                          /**< Lock bytes size in bits */
    uint8_t bBytesLockedPerBit;                                   /**< Number of bytes locked per lock bit */
}phalTop_T2T_LockCtrlTlv_t;

/**
* \brief T1T Memory Control TLV parameter structure
*/
typedef struct phalTop_T1T_MemCtrlTlv
{
    uint16_t wOffset;                                             /**< TLV offset */
    uint16_t wByteAddr;                                           /**< Reserved memory start address/offset */
    uint8_t bSizeInBytes;                                         /**< Reserved memory size in bytes */
}phalTop_T1T_MemCtrlTlv_t;

/**
* \brief T2T Memory Control TLV parameter structure
*/
typedef struct phalTop_T2T_MemCtrlTlv
{
    uint16_t wOffset;                                             /**< TLV offset */
    uint16_t wByteAddr;                                           /**< Reserved memory start address/offset */
    uint8_t bSizeInBytes;                                         /**< Reserved memory size in bytes */
}phalTop_T2T_MemCtrlTlv_t;

/**
* \brief T1T Proprietary TLV parameter structure
*/
typedef struct phalTop_T1T_ProprietaryTlv
{
    uint16_t wOffset;                                             /**< TLV address/offset */
    uint16_t wLength;                                             /**< TLV Length */
}phalTop_T1T_ProprietaryTlv_t;

/**
* \brief T2T Proprietary TLV parameter structure
*/
typedef struct phalTop_T2T_ProprietaryTlv
{
    uint16_t wOffset;                                             /**< TLV address/offset */
    uint16_t wLength;                                             /**< TLV Length */
}phalTop_T2T_ProprietaryTlv_t;

/**
* \brief T1T Segment parameters
*/
typedef struct phalTop_TIT_Segment
{
    uint8_t bAddress;                                             /**< Current active segment address */
    uint8_t bBlockAddress;                                        /**< Current active block address */
    uint8_t bLockReservedOtp[PHAL_TOP_T1T_BLOCKS_PER_SEGMENT];    /**< Lock/reserved/OTP bytes in current segment */
    uint8_t bValidity;                                            /**< Read data validity */
}phalTop_TIT_Segment_t;

/**
* \brief T2T Sector parameters
*/
typedef struct phalTop_T2T_Sector
{
    uint8_t bAddress;                                            /**< Current active Sector address */
    uint8_t bBlockAddress;                                       /**< Current active block address */
    uint8_t bLockReservedOtp[32];                                /**< Lock/reserved/OTP bytes in current Sector */
    uint8_t bValidity;                                           /**< Read data validity */
}phalTop_T2T_Sector_t;

/**
* \brief T1T parameter structure
*/
typedef struct phalTop_T1T
{
    void * phalT1TDataParams;                                                   /**< Pointer to T1T data parameters */
    uint8_t bUid[4];                                                            /**< T1T UID */
    uint8_t bNdefSupport;                                                       /**< NDEF support by T1T tag; 1 - supported, 0 - not supported */
    uint8_t bNdefPresence;                                                      /**< NDEF message presence in tag; 1 - present, 0 - not present */
    uint8_t bVno;                                                               /**< NDEF version number */
    uint8_t bTms;                                                               /**< Tag Memory size */
    uint8_t bRwa;                                                               /**< Tag read/write size */
    uint16_t wNdefHeaderAddr;                                                   /**< Header offset of first NDEF message */
    uint16_t wNdefMsgAddr;                                                      /**< NDEF message start address */
    uint16_t wNdefLength;                                                       /**< NDEF message length */
    uint16_t wMaxNdefLength;                                                    /**< Max. supported NDEF length */
    uint8_t bMemoryTlvCount;                                                    /**< Number of memory TLV present in tag */
    phalTop_T1T_MemCtrlTlv_t asMemCtrlTvl[PHAL_TOP_T1T_MAX_MEM_CTRL_TLV];       /**< Memory TLV details for each TLV present */
    uint8_t bLockTlvCount;                                                      /**< Number of lock TLV present in tag */
    phalTop_T1T_LockCtrlTlv_t asLockCtrlTvl[PHAL_TOP_T1T_MAX_LOCK_CTRL_TLV];    /**< Lock TLV details for each TLV present */
    uint8_t bProprietaryTlvCount;                                               /**< Number of proprietary TLV present in tag */
    phalTop_T1T_ProprietaryTlv_t asPropTvl[PHAL_TOP_T1T_MAX_PROPRIETARY_TLV];   /**< Proprietary TLV details for each TLV present */
    uint8_t bTerminatorTlvPresence;                                             /**< Terminator TLV presence; 1 -present, 0 - not present */
    uint8_t bNdefFormatted;                                                     /**< Tag is NDEF formated or not; 1 - formatted, 0 - not formatted */
    uint8_t bTagState;                                                          /**< T1T tag state */
    uint8_t bTagMemoryType;                                                     /**< Tag memory type (static or dynamic) */
    phalTop_TIT_Segment_t sSegment;                                             /**< Current segment details */
}phalTop_T1T_t;

/**
* \brief T2T parameter structure
*/
typedef struct phalTop_T2T
{
    void * phalT2TDataParams;                                                    /**< Pointer to T2T data parameters */
    uint8_t bNdefPresence;                                                       /**< NDEF message presence in tag; 1 - present, 0 - not present */
    uint8_t bVno;                                                                /**< NDEF version number */
    uint8_t bTms;                                                                /**< Tag Memory size */
    uint8_t bRwa;                                                                /**< Tag read/write size */
    uint16_t wNdefHeaderAddr;                                                    /**< Header offset of first NDEF message */
    uint16_t wNdefMsgAddr;                                                       /**< NDEF message start address */
    uint16_t wNdefLength;                                                        /**< NDEF message length */
    uint16_t wMaxNdefLength;                                                     /**< Max. supported NDEF length */
    uint8_t bMemoryTlvCount;                                                     /**< Number of memory TLV present in tag */
    phalTop_T2T_MemCtrlTlv_t asMemCtrlTvl[PHAL_TOP_T2T_MAX_MEM_CTRL_TLV];        /**< Memory TLV details for each TLV present */
    uint8_t bLockTlvCount;                                                       /**< Number of lock TLV present in tag */
    phalTop_T2T_LockCtrlTlv_t asLockCtrlTvl[PHAL_TOP_T2T_MAX_LOCK_CTRL_TLV];     /**< Lock TLV details for each TLV present */
    uint8_t bProprietaryTlvCount;                                                /**< Number of proprietary TLV present in tag */
    phalTop_T2T_ProprietaryTlv_t asPropTvl[PHAL_TOP_T2T_MAX_PROPRIETARY_TLV];    /**< Proprietary TLV details for each TLV present */
    uint8_t bTerminatorTlvPresence;                                              /**< Terminator TLV presence; 1 -present, 0 - not present */
    uint8_t bEraseProprietaryTlv;                                                /**< Erase proprietary TLV during write; 1 - erase, 0 - don't erase */
    uint8_t bNdefFormatted;                                                      /**< Tag is NDEF formated or not; 1 - formatted, 0 - not formatted */
    uint8_t bTagState;                                                           /**< T2T tag state */
    uint8_t bTagMemoryType;                                                      /**< Tag memory type (static or dynamic) */
    phalTop_T2T_Sector_t sSector;                                                /**< Current segment details */
}phalTop_T2T_t;

/**
* \brief T3T parameter structure
*/
typedef struct phalTop_T3T
{
    void * phalT3TDataParams;                                                    /**< Pointer to Felica data parameters */
    uint8_t bNdefSupport;                                                        /**< NDEF support by T3T tag; 1 - supported, 0 - not supported */
    uint8_t bUid[16];                                                            /**< UID to address T3T */
    uint8_t bVno;                                                                /**< NDEF version number */
    uint8_t bNbr;                                                                /**< Number of Blocks that can be read at one time*/
    uint8_t bNbw;                                                                /**< Number of Blocks that can be written at one time*/
    uint8_t bRwa;                                                                /**< Tag read/write access; 1 - Read/Write, 0 - Read Only */
    uint8_t bNdefPresence;                                                       /**< NDEF message presence in tag; 1 - present, 0 - not present */
    uint8_t bNdefFormatted;                                                      /**< Tag is NDEF formated or not; 1 - formatted, 0 - not formatted */
    uint8_t bTagState;                                                           /**< T3T tag state */
    uint16_t bNmaxb;                                                             /**< Maximum number of 'Blocks * 16' available for NDEF data */
    uint32_t wNdefLength;                                                        /**< NDEF message length */
}phalTop_T3T_t;

/**
* \brief T4T parameter structure
*/
typedef struct phalTop_T4T
{
    void * phalT4TDataParams;                                                    /**< Pointer to T4T data parameters */
    uint8_t bNdefFormatted;                                                      /**< Tag is NDEF formated or not; 1 - formatted, 0 - not formatted */
    uint8_t bNdefSupport;                                                        /**< NDEF support by T1T tag; 1 - supported, 0 - not supported */
    uint8_t bNdefPresence;                                                       /**< NDEF message presence in tag; true - present,false - not present */
    uint8_t bTagState;                                                           /**< T4T tag state */
    uint8_t bNdef_FileID[2];                                                     /**< NDEF File ID to Create */
    uint8_t bVno;                                                                /**< Mapping version number. */
    uint8_t bRa;                                                                 /**< NDEF file read access condition size */
    uint8_t bWa;                                                                 /**< NDEF file write access condition size */
    uint16_t wCCLEN;                                                             /**< Size of CCr,Valid CCLEN range 000Fh-FFFEh.*/
    uint16_t bMaxFileSize;                                                       /**< Maximum NDEF file size in bytes */
    uint16_t wNLEN;                                                              /**< Size of the NDEF message stored in the NDEF file. */
}phalTop_T4T_t;

/**
* \brief Tag Operations parameter structure
*/
typedef struct  phalTop_Sw_DataParams
{
    uint16_t wId;                                 /**< Layer ID for this component, NEVER MODIFY! */
    uint8_t bTagType;                             /**< Tag type */
    void * pT1T;                                  /**< Pointer to T1T parameter structure */
    void * pT2T;                                  /**< Pointer to T2T parameter structure */
    void * pT3T;                                  /**< Pointer to T3T parameter structure */
    void * pT4T;                                  /**< Pointer to T4T parameter structure */
    void * pT5T;                                  /**< RFU : Pointer to T5T parameter structure */
}phalTop_Sw_DataParams_t;

/**
* \brief Initialize this layer.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on error type
*/
phStatus_t phalTop_Sw_Init(
    phalTop_Sw_DataParams_t * pDataParams,        /**< [In] Pointer to this layer's parameter structure. */
    uint16_t wSizeOfDataParams,                   /**< [In] Specifies the size of the data parameter structure. */
    void * pTopT1T,                               /**< [In] Pointer to T1T parameter structure. */
    void * pTopT2T,                               /**< [In] Pointer to T2T parameter structure. */
    void * pTopT3T,                               /**< [In] Pointer to T3T parameter structure. */
    void * pTopT4T,                               /**< [In] Pointer to T4T parameter structure. */
    void * pTopT5T                                /**< [In] RFU : Pointer to T5T parameter structure. */
    );
/** @} */
#endif /* NXPBUILD__PHAL_TOP_SW */

#ifdef NXPBUILD__PHAL_TOP

/** \defgroup phalTop Tag Operation Layer
* \brief This Component implements Tag Operations for different NFC Tag types.
* @{
*/
/**
* \name Tag types
* \brief Tag types are used to set Tag used using \ref phalTop_SetConfig with \ref PHAL_TOP_CONFIG_TAG_TYPE Type
*/
/*@{*/
#define PHAL_TOP_TAG_TYPE_T1T_TAG                   0x01U                /**< Type 1 Tag. */
#define PHAL_TOP_TAG_TYPE_T2T_TAG                   0x02U                /**< Type 2 Tag. */
#define PHAL_TOP_TAG_TYPE_T3T_TAG                   0x04U                /**< Type 3 Tag. */
#define PHAL_TOP_TAG_TYPE_T4T_TAG                   0x08U                /**< Type 4 Tag. */
/*@}*/

/**
* \name Configuration types
* \brief Use these Macro in \ref phalTop_SetConfig to configure Tags before calling \ref phalTop_FormatNdef, \ref phalTop_WriteNdef etc.
*/
/*@{*/
#define PHAL_TOP_CONFIG_TAG_TYPE                    0x00U                /**< Set/Get Tag type. Should be configured before calling any API. */
#define PHAL_TOP_CONFIG_TAG_FORMATTABLE             0x01U                /**< Get T1T Tag Formattable or not. */

#define PHAL_TOP_CONFIG_T1T_NDEF_VNO                0x02U                /**< Set/Get T1T NDEF Version Number. Set before Format Operation. */
#define PHAL_TOP_CONFIG_T1T_TMS                     0x03U                /**< Set/Get Tag memory size. Set before Format Operation. */
#define PHAL_TOP_CONFIG_T1T_RW_ACCESS               0x04U                /**< Set/Get Tag R/W access. */
#define PHAL_TOP_CONFIG_T1T_TERMINATOR_TLV          0x05U                /**< Set/Get Terminator TLV presence. */
#define PHAL_TOP_CONFIG_T1T_MAX_NDEF_LENGTH         0x06U                /**< Get Max NDEF Length. */
#define PHAL_TOP_CONFIG_T1T_GET_TAG_STATE           0x07U                /**< Get T1T state. */

#define PHAL_TOP_CONFIG_T2T_NDEF_VNO                0x08U                /**< Set/Get T2T NDEF Version Number. Set before Format Operation. */
#define PHAL_TOP_CONFIG_T2T_TMS                     0x09U                /**< Set/Get Tag memory size. Set before Format Operation.*/
#define PHAL_TOP_CONFIG_T2T_RW_ACCESS               0x0AU                /**< Set/Get Tag R/W access. */
#define PHAL_TOP_CONFIG_T2T_TERMINATOR_TLV          0x0BU                /**< Set/Get Terminator TLV presence. */
#define PHAL_TOP_CONFIG_T2T_PROPRIETARY_TLV         0x0CU                /**< Set/Get Proprietary TLV erase. */
#define PHAL_TOP_CONFIG_T2T_MAX_NDEF_LENGTH         0x0DU                /**< Get Max NDEF Length. */
#define PHAL_TOP_CONFIG_T2T_GET_TAG_STATE           0x0EU                /**< Get T2T state. */

#define PHAL_TOP_CONFIG_T3T_NDEF_VNO                0x0FU                /**< Set/Get T3T NDEF Version Number. Set before Format Operation. */
#define PHAL_TOP_CONFIG_T3T_NBR                     0x10U                /**< Set/Get T3T NBR(No. Blocks Read Simultaneoulsy). Set before Format Operation. */
#define PHAL_TOP_CONFIG_T3T_NBW                     0x11U                /**< Set/Get T3T NBW(No. Blocks Write Simultaneoulsy). Set before Format Operation. */
#define PHAL_TOP_CONFIG_T3T_MAX_NDEF_LENGTH         0x12U                /**< Set/Get T3T Max NDEF Length-Nmaxb (Maximum NDEF Data supported by Tag). Set before Format Operation. */
#define PHAL_TOP_CONFIG_T3T_RW_ACCESS               0x13U                /**< Get T3T Tag R/W access. */
#define PHAL_TOP_CONFIG_T3T_GET_TAG_STATE           0x14U                /**< Get T3T state. */

#define PHAL_TOP_CONFIG_T4T_NDEF_FILE_ID            0x15U                /**< Set NDEF File ID. Set before Format Operation. */
#define PHAL_TOP_CONFIG_T4T_NDEF_VNO                0x16U                /**< Set/Get T4T NDEF Version Number. Set before Format Operation. */
#define PHAL_TOP_CONFIG_T4T_WRITE_ACCESS            0x17U                /**< Set/Get Tag Write access. Set before Format Operation. */
#define PHAL_TOP_CONFIG_T4T_MAX_NDEF_FILE_SIZE      0x18U                /**< Set/Get Max NDEF length. Set before Format Operation. */
#define PHAL_TOP_CONFIG_T4T_READ_ACCESS             0x19U                /**< Get Tag Read access. */
#define PHAL_TOP_CONFIG_T4T_GET_TAG_STATE           0x1AU                /**< Get T4T state. */
#define PHAL_TOP_CONFIG_T4T_NLEN                    0x1BU                /**< Get NDEF Length. */

/*@}*/

/**
* \name T1T TLV types
*/
/*@{*/
#define PHAL_TOP_T1T_NULL_TLV                       0x00U                /**< NULL TLV. */
#define PHAL_TOP_T1T_LOCK_CTRL_TLV                  0x01U                /**< Lock Control TLV. */
#define PHAL_TOP_T1T_MEMORY_CTRL_TLV                0x02U                /**< Memory Control TLV. */
#define PHAL_TOP_T1T_NDEF_TLV                       0x03U                /**< NDEF Message TLV. */
#define PHAL_TOP_T1T_PROPRIETARY_TLV                0xFDU                /**< Proprietary TLV. */
#define PHAL_TOP_T1T_TERMINATOR_TLV                 0xFEU                /**< Terminator TLV. */
/*@}*/

/**
* \name T2T TLV types
*/
/*@{*/
#define PHAL_TOP_T2T_NULL_TLV                       0x00U                /**< NULL TLV. */
#define PHAL_TOP_T2T_LOCK_CTRL_TLV                  0x01U                /**< Lock Control TLV. */
#define PHAL_TOP_T2T_MEMORY_CTRL_TLV                0x02U                /**< Memory Control TLV. */
#define PHAL_TOP_T2T_NDEF_TLV                       0x03U                /**< NDEF Message TLV. */
#define PHAL_TOP_T2T_PROPRIETARY_TLV                0xFDU                /**< Proprietary TLV. */
#define PHAL_TOP_T2T_TERMINATOR_TLV                 0xFEU                /**< Terminator TLV. */
/*@}*/

/**
* \name T4T TLV types
*/
/*@{*/
#define PHAL_TOP_T4T_NDEF_TLV                       0x04U                /**< NDEF Message TLV. */
#define PHAL_TOP_T4T_PROPRIETARY_TLV                0x05U                /**< Proprietary TLV. */
/*@}*/

/**
* \name T1T Tag States
*/
/*@{*/
#define PHAL_TOP_T1T_STATE_UNKNOWN                  0x00U                /**< Default initial state. */
#define PHAL_TOP_T1T_STATE_INITIALIZED              0x01U                /**< Initialized state. */
#define PHAL_TOP_T1T_STATE_READONLY                 0x02U                /**< Read Only state. */
#define PHAL_TOP_T1T_STATE_READWRITE                0x04U                /**< Read/Write state. */
/*@}*/

/**
* \name T2T Tag States
*/
/*@{*/
#define PHAL_TOP_T2T_STATE_UNKNOWN                  0x00U                /**< Default initial state. */
#define PHAL_TOP_T2T_STATE_INITIALIZED              0x01U                /**< Initialized state. */
#define PHAL_TOP_T2T_STATE_READONLY                 0x02U                /**< Read Only state. */
#define PHAL_TOP_T2T_STATE_READWRITE                0x04U                /**< Read/Write state. */
/*@}*/

/**
* \name T3T Tag States
*/
/*@{*/
#define PHAL_TOP_T3T_STATE_UNKNOWN                  0x00U                /**< Default initial state. */
#define PHAL_TOP_T3T_STATE_INITIALIZED              0x01U                /**< Initialized state. */
#define PHAL_TOP_T3T_STATE_READONLY                 0x02U                /**< Read Only state. */
#define PHAL_TOP_T3T_STATE_READWRITE                0x04U                /**< Read/Write state. */
/*@}*/

/**
* \name T4T Tag States
*/
/*@{*/
#define PHAL_TOP_T4T_STATE_UNKNOWN                  0x00U                /**< Default initial state. */
#define PHAL_TOP_T4T_STATE_INITIALIZED              0x01U                /**< Initialized state. */
#define PHAL_TOP_T4T_STATE_READONLY                 0x02U                /**< Read Only state. */
#define PHAL_TOP_T4T_STATE_READWRITE                0x04U                /**< Read/Write state. */
/*@}*/

/**
* \name T1T Memory type
*/
/*@{*/
#define PHAL_TOP_T1T_TAG_MEMORY_TYPE_STATIC         0x00U                /**< Static memory type */
#define PHAL_TOP_T1T_TAG_MEMORY_TYPE_DYNAMIC        0x01U                /**< Dynamic memory type */
/*@}*/

/**
* \name T2T Memory type
*/
/*@{*/
#define PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC         0x00U                /**< Static memory type */
#define PHAL_TOP_T2T_TAG_MEMORY_TYPE_DYNAMIC        0x01U                /**< Dynamic memory type */
/*@}*/

/**
* \name T1T R/W access type
*/
/*@{*/
#define PHAL_TOP_T1T_CC_RWA_RW                      0x00U                /**< Read/Write access */
#define PHAL_TOP_T1T_CC_RWA_RO                      0x0FU                /**< Read Only access */
/*@}*/

/**
* \name T2T R/W access type
*/
/*@{*/
#define PHAL_TOP_T2T_CC_RWA_RW                      0x00U                /**< Read/Write access */
#define PHAL_TOP_T2T_CC_RWA_RO                      0x0FU                /**< Read Only access */
/*@}*/

/**
* \name T3T R/W access type
*/
/*@{*/
#define PHAL_TOP_T3T_AIB_RWA_RW                     0x01U                /**< Read/Write access */
#define PHAL_TOP_T3T_AIB_RWA_RO                     0x00U                /**< Read Only access */
/*@}*/

/**
* \name T3T Write Flag access type
*/
/*@{*/
#define PHAL_TOP_T3T_WRITE_FLAG_SET                 0x0FU                /**< Write Flag to SET during Write in Progress */
#define PHAL_TOP_T3T_WRITE_FLAG_RESET               0x00U                /**< Write Flag to RESET after Write is Done */
/*@}*/

/**
* \name T4T R/W access type
*/
/*@{*/
#define PHAL_TOP_T4T_NDEF_FILE_READ                 0x00U                /**< Read/Write access */
#define PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS         0x00U                /**< Read/Write access */
#define PHAL_TOP_T4T_NDEF_FILE_NO_WRITE_ACCESS      0xFFU                /**< Read Only access */
/*@}*/

/**
* \brief Format Tag to write NDEF message.
* Used to Format New/Non-NDEF Tag to NDEFTag.
*
* <em>Description :</em> The caller has to ensure that \ref phalTop_CheckNdef is called before
* calling phalTop_FormatNdef and check if the Tag is formattable or not using \ref phalTop_GetConfig.
* If \ref phalTop_CheckNdef returns that Tag is Supported then then this API will format the Tag so that
* it is NDEF Compatible and in case of T2T a Valid NDEF Message is written and in case of other 
* tags the contents of the Tag will not be valid. Caller can call \ref phalTop_WriteNdef to write a new
* NDEF Message on to the Tag.\n\n
* \return Status code
* \retval #PH_ERR_SUCCESS                 Operation successful.
* \retval #PH_ERR_UNSUPPORTED_COMMAND     If Tag is not formattable or in some different state.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_FormatNdef(
    void * pDataParams            /**< [In] Pointer to this layer's parameter structure. */
    );

/**
* \brief Read NDEF message from Tag.
*
* <em>Description :</em> The caller has to ensure that \ref phalTop_CheckNdef is called before
* calling phalTop_ReadNdef. If \ref phalTop_CheckNdef returns valid NDEF Present on Tag
* then phalTop_ReadNdef should be called otherwise error code is returned. User has to
* parse data according to NDEF Spec to validate data as an NDEF.\n\n
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER Parameter value is invalid.
* \retval #PH_ERR_PROTOCOL_ERROR No Valid NDEF Present on Tag.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_ReadNdef(
    void * pDataParams,           /**< [In] Pointer to this layer's parameter structure. */
    uint8_t * pData,              /**< [Out] NDEF data from the Tag. User has to allocate memory accordingly */
    uint16_t * pLength            /**< [Out] NDEF data length. */
    );

/**
* \brief Write NDEF message into Tag.
*
* <em>Description :</em> The caller has to ensure that \ref phalTop_CheckNdef is called before
* calling phalTop_WriteNdef. If it is a New Tag then \ref phalTop_FormatNdef should be called
* to make NDEF Tag then phalTop_WriteNdef should be called otherwise error code is returned.\n
* <em>Note :</em> In case of T2T Tag a valid NDEF must be present before calling this API.
* So to make it convenient \ref phalTop_FormatNdef can be used to write a default NDEF Message
* and then phalTop_WriteNdef API can be used to write NDEF Message.\n\n
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER Parameter value is invalid.
* \retval #PHAL_TOP_ERR_NOT_FORMATTED Non Formatted Tag.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_WriteNdef(
    void * pDataParams,           /**< [In] Pointer to this layer's parameter structure. */
    uint8_t * pData,              /**< [In] NDEF data to be written to tag. User has to allocate memory accordingly */
    uint16_t wLength              /**< [In] Length of NDEF data to be written. */
    );

/**
* \brief Check for NDEF message Presence and check for different TLVs
*
* <em>Description :</em> The caller has to ensure that Tag activation and authentication is done
* before calling phalTop_CheckNdef. Also \ref phalTop_SetConfig should be called to configure the
* Tag used. Only after phalTop_CheckNdef is called any other operation on Tag can be performed.\n\n
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_CheckNdef(
    void * pDataParams,           /**< [In] Pointer to this layer's parameter structure. */
    uint8_t * pNdefPresence       /**< [Out] NDEF data Presence; 1 - NDEF present, 0 - Not present. */
    );

/**
* \brief Reset Tag Operation parameters
*
* <em>Description :</em> User can call phalTop_Reset to reset all the Software parameters which are modified
* when \ref phalTop_CheckNdef, \ref phalTop_WriteNdef or \ref phalTop_ReadNdef API's are called.\n\n
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phalTop_Reset(
    void * pDataParams            /**< [In] Pointer to this layer's parameter structure. */
    );

/**
* \brief Erase NDEF Message
*
* <em>Description :</em> By calling phalTop_EraseNdef NDEF Message stored on the Tag would be erased
* and the NDEF Length would be zero to indicate that there is no NDEF Message on Tag. User can call
* \ref phalTop_CheckNdef to make sure that there is a valid NDEF Present on Tag then
* phalTop_EraseNdef can be called to erase otherwise no operation would be performed.\n
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalTop_EraseNdef(
    void * pDataParams            /**< [In] Pointer to this layer's parameter structure. */
    );

/**
* \brief Set configuration parameter.
*
* <em>Description :</em> Before calling \ref phalTop_CheckNdef, \ref phalTop_WriteNdef, \ref 
* phalTop_ReadNdef API's or \ref phalTop_FormatNdef, user needs to call phalTop_SetConfig to configure
* different parameters as per the Tag used based on User need. After configuring a particular tag
* by calling phalTop_SetConfig, all these parameters are set during further operations (refer Configuration
* Parameters to know how particular parameter used during Top Operations. User can also refer T1T, T2T, T3T
* or T4T Specifications to know in detail).\n\n
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER Parameter value is invalid.
*/
phStatus_t phalTop_SetConfig(
    void * pDataParams,           /**< [In] Pointer to this layer's parameter structure. */
    uint16_t wConfig,             /**< [In] Configuration Identifier. */
    uint16_t wValue               /**< [In] Configuration Value. */
    );

/**
* \brief Get configuration parameter.
*
* <em>Description :</em> After calling \ref phalTop_CheckNdef, \ref phalTop_WriteNdef, \ref 
* phalTop_ReadNdef API's or \ref phalTop_FormatNdef, user can call phalTop_GetConfig to get different
* parameters as per the Tag used based on User need. After a valid opreation on a particular tag all the
* all the parameters of the Tag are updated and user can call phalTop_GetConfig to know the values (refer
* Configuration Parameters to know how particular parameter used during Top Operations. User can also refer
* T1T, T2T, T3T or T4T Specifications to know in detail).\n\n
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_PARAMETER Parameter value is invalid.
*/
phStatus_t phalTop_GetConfig(
    void * pDataParams,           /**< [In] Pointer to this layer's parameter structure. */
    uint16_t wConfig,             /**< [In] Configuration Identifier. */
    uint16_t * pValue             /**< [Out] Configuration Value. */
    );

/** @} */
#endif /* NXPBUILD__PHAL_TOP */

#ifdef __cplusplus
} /* Extern C */
#endif

#ifdef __DEBUG
/* This API would be available only for debugging purpose and test bench */
/**
* \brief   Assigns the component data paramter to one of the filed in 
*          T1T parameter based on the option
*
* \return  Error code
* \retval  #PH_ERR_INVALID_PARAMTER  If invalid option was passed
* \retval  #PH_ERR_SUCCESS           on success
*
*/
phStatus_t phalTop_SetPtr(
    void * pDataParams,           /**< [In] Pointer to this layer's paramter structure */
    void * pT1T,                  /**< [In] Pointer to Tag Type 1 paramter structure */
    void * pT2T,                  /**< [In] Pointer to Tag Type 2 paramter structure */
    void * pT3T,                  /**< [In] Pointer to Tag Type 3 paramter structure */
    void * pT4T                   /**< [In] Pointer to Tag Type 4 paramter structure */
    );
#endif /* __DEBUG */

#endif  /* PHALTOP_H */
