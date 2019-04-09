/**********************************************************************************************
 * Filename:       myData.c
 *
 * Description:    This file contains the implementation of the service.
 *
 * Copyright (c) 2015-2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include <icall.h>

/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include "myData.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
* GLOBAL VARIABLES
*/

// myData Service UUID
CONST uint8_t myDataUUID[ATT_UUID_SIZE] =
{
  TI_BASE_UUID_128(MYDATA_SERV_UUID)
};

// data1 UUID
CONST uint8_t myData_Data1UUID[ATT_UUID_SIZE] =
{
  TI_BASE_UUID_128(MYDATA_DATA1_UUID)
};
// data2 UUID
CONST uint8_t myData_Data2UUID[ATT_UUID_SIZE] =
{
  TI_BASE_UUID_128(MYDATA_DATA2_UUID)
};

/*********************************************************************
 * LOCAL VARIABLES
 */

static myDataCBs_t *pAppCBs = NULL;

/*********************************************************************
* Profile Attributes - variables
*/

// Service declaration
static CONST gattAttrType_t myDataDecl = { ATT_UUID_SIZE, myDataUUID };

// Characteristic "Data1" Properties (for declaration)
static uint8_t myData_Data1Props = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic "Data1" Value variable
static uint8_t myData_Data1Val[MYDATA_DATA1_LEN] = {0};
// Characteristic "Data2" Properties (for declaration)
static uint8_t myData_Data2Props = GATT_PROP_NOTIFY;

// Characteristic "Data2" Value variable
static uint8_t myData_Data2Val[MYDATA_DATA2_LEN] = {0};

// Characteristic "Data2" CCCD
static gattCharCfg_t *myData_Data2Config;

/*********************************************************************
* Profile Attributes - Table
*/

static gattAttribute_t myDataAttrTbl[] =
{
  // myData Service Declaration
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID },
    GATT_PERMIT_READ,
    0,
    (uint8_t *)&myDataDecl
  },
    // Data1 Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &myData_Data1Props
    },
      // Data1 Characteristic Value
      {
        { ATT_UUID_SIZE, myData_Data1UUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        myData_Data1Val
      },
    // Data2 Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &myData_Data2Props
    },
      // Data2 Characteristic Value
      {
        { ATT_UUID_SIZE, myData_Data2UUID },
        GATT_PERMIT_READ,
        0,
        myData_Data2Val
      },
      // Data2 CCCD
      {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)&myData_Data2Config
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t myData_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                           uint16_t maxLen, uint8_t method );
static bStatus_t myData_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                            uint8_t *pValue, uint16_t len, uint16_t offset,
                                            uint8_t method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t myDataCBs =
{
  myData_ReadAttrCB,  // Read callback function pointer
  myData_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
* PUBLIC FUNCTIONS
*/

/*
 * MyData_AddService- Initializes the MyData service by registering
 *          GATT attributes with the GATT server.
 *
 */
extern bStatus_t MyData_AddService( uint8_t rspTaskId )
{
  uint8_t status;

  // Allocate Client Characteristic Configuration table
  myData_Data2Config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * linkDBNumConns );
  if ( myData_Data2Config == NULL )
  {
    return ( bleMemAllocError );
  }

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( LINKDB_CONNHANDLE_INVALID, myData_Data2Config );
  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( myDataAttrTbl,
                                        GATT_NUM_ATTRS( myDataAttrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &myDataCBs );

  return ( status );
}

/*
 * MyData_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
bStatus_t MyData_RegisterAppCBs( myDataCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    pAppCBs = appCallbacks;

    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*
 * MyData_SetParameter - Set a MyData parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t MyData_SetParameter( uint8_t param, uint16_t len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case MYDATA_DATA1_ID:
      if ( len == MYDATA_DATA1_LEN )
      {
        memcpy(myData_Data1Val, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case MYDATA_DATA2_ID:
      if ( len == MYDATA_DATA2_LEN )
      {
        memcpy(myData_Data2Val, value, len);

        // Try to send notification.
        GATTServApp_ProcessCharCfg( myData_Data2Config, (uint8_t *)&myData_Data2Val, FALSE,
                                    myDataAttrTbl, GATT_NUM_ATTRS( myDataAttrTbl ),
                                    INVALID_TASK_ID,  myData_ReadAttrCB);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  return ret;
}


/*
 * MyData_GetParameter - Get a MyData parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
bStatus_t MyData_GetParameter( uint8_t param, uint16_t *len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case MYDATA_DATA1_ID:
      memcpy(value, myData_Data1Val, MYDATA_DATA1_LEN);
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  return ret;
}


/*********************************************************************
 * @fn          myData_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t myData_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                       uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                       uint16_t maxLen, uint8_t method )
{
  bStatus_t status = SUCCESS;

  // See if request is regarding the Data1 Characteristic Value
if ( ! memcmp(pAttr->type.uuid, myData_Data1UUID, pAttr->type.len) )
  {
    if ( offset > MYDATA_DATA1_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, MYDATA_DATA1_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  // See if request is regarding the Data2 Characteristic Value
else if ( ! memcmp(pAttr->type.uuid, myData_Data2UUID, pAttr->type.len) )
  {
    if ( offset > MYDATA_DATA2_LEN )  // Prevent malicious ATT ReadBlob offsets.
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      *pLen = MIN(maxLen, MYDATA_DATA2_LEN - offset);  // Transmit as much as possible
      memcpy(pValue, pAttr->pValue + offset, *pLen);
    }
  }
  else
  {
    // If we get here, that means you've forgotten to add an if clause for a
    // characteristic value attribute in the attribute table that has READ permissions.
    *pLen = 0;
    status = ATT_ERR_ATTR_NOT_FOUND;
  }

  return status;
}


/*********************************************************************
 * @fn      myData_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t myData_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                        uint8_t *pValue, uint16_t len, uint16_t offset,
                                        uint8_t method )
{
  bStatus_t status  = SUCCESS;
  uint8_t   paramID = 0xFF;

  // See if request is regarding a Client Characterisic Configuration
  if ( ! memcmp(pAttr->type.uuid, clientCharCfgUUID, pAttr->type.len) )
  {
    // Allow only notifications.
    status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                             offset, GATT_CLIENT_CFG_NOTIFY);
  }
  // See if request is regarding the Data1 Characteristic Value
  else if ( ! memcmp(pAttr->type.uuid, myData_Data1UUID, pAttr->type.len) )
  {
    if ( offset + len > MYDATA_DATA1_LEN )
    {
      status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
      // Copy pValue into the variable we point to from the attribute table.
      memcpy(pAttr->pValue + offset, pValue, len);

      // Only notify application if entire expected value is written
      if ( offset + len == MYDATA_DATA1_LEN)
        paramID = MYDATA_DATA1_ID;
    }
  }
  else
  {
    // If we get here, that means you've forgotten to add an if clause for a
    // characteristic value attribute in the attribute table that has WRITE permissions.
    status = ATT_ERR_ATTR_NOT_FOUND;
  }

  // Let the application know something changed (if it did) by using the
  // callback it registered earlier (if it did).
  if (paramID != 0xFF)
    if ( pAppCBs && pAppCBs->pfnChangeCb )
      pAppCBs->pfnChangeCb(connHandle, paramID, len, pValue); // Call app function from stack task context.

  return status;
}
