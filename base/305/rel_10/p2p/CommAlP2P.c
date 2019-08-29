//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// The application layer of the P2P UART communication protocol
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define P2P__COMMALP2P_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COMMALP2P_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               COMMALP2P_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of serices that can be registered.
#ifndef COMMALP2P_SERVICE_COUNT
	#define COMMALP2P_SERVICE_COUNT                     10
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of requests that can be registered.
#ifndef COMMALP2P_REQUEST_COUNT
	#define COMMALP2P_REQUEST_COUNT                     10
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the TX buffer length
#ifndef COMMALP2P_TX_BUFFER_LENGTH
    #define COMMALP2P_TX_BUFFER_LENGTH                 130
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum acknowledge timeout in ms
#ifndef COMMALP2P_ACKNOWLEDGE_TIMEOUT_IN_MS
	#define COMMALP2P_ACKNOWLEDGE_TIMEOUT_IN_MS         200
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum timeout on the service handling in ms
#ifndef COMMALP2P_SERVICEHANDLING_TIMEOUT_IN_MS
	#define COMMALP2P_SERVICEHANDLING_TIMEOUT_IN_MS     5000
#endif
//------------------------------------------------------------------------------------------------//
#ifndef COMMALP2P_INCLUDE_PEERDETECTION
    #define COMMALP2P_INCLUDE_PEERDETECTION             1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section

//STD lib include section

//COM lib include section
#include "CommAlP2P.h"
#include "CommDllP2P.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define P2P_MSG_FUNC_SUBMASK            0x03
#define P2P_MSG_SERVICE_SHIFT           2
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    P2P_MSG_FUNC_REQUEST        = 0,
    P2P_MSG_FUNC_ACKNOWLEDGE    = 1,
    P2P_MSG_FUNC_RESPONSE       = 2,
    P2P_MSG_FUNC_NOTAVAILABLE   = 3,
}
P2P_MSG_FUNC;

typedef struct
{
    P2P_SERVICE                 service;
    U8                          request_id;
    U16                         timeout_countdown;
    COMMALP2P_REQUESTHANDLER    request_handler;
    COMMALP2P_RESPONSEHANDLER   response_handler;
}
COMMALP2P_SERVICE;

typedef struct
{
    P2P_SERVICE                 service;
    U8                          request_id;
    U16                         ack_countdown;
    U16                         response_countdown;
    COMMALP2P_RESPONSECALLBACK  response_callback;
}
COMMALP2P_REQUEST;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CommAlP2PInfo(void);
#endif
static BOOL AlP2P_GetService(P2P_SERVICE service, COMMALP2P_SERVICE** service_ptr_ptr);
static BOOL AlP2P_GetRequest(P2P_SERVICE service, COMMALP2P_REQUEST** request_ptr_ptr);
static void AlP2P_CallResponseCallback(COMMALP2P_REQUEST* request_ptr, RESPONSE_STATE response_state, U8* data_ptr, U8 data_len);
static void AlP2P_OnFrameReceived(U8* data_ptr, U8 data_len);
static void AlP2P_TimeoutTask(VPTR data_ptr);
static BOOL AlP2P_ClaimOutputBuffer(void);
static void AlP2P_ReleaseOutputBuffer(void);
#if (COMMALP2P_INCLUDE_PEERDETECTION == 1)
static void AlP2P_IsPresentTimeout(VPTR data_ptr);
#endif
static void CommAlP2P_Handler(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static COMMALP2P_SERVICE        commalp2p_service[COMMALP2P_SERVICE_COUNT];
static U8                       commalp2p_service_count;

static U8*                      commalp2p_outbuffer_ptr;
static U8                       commalp2p_outbuffer_len;
static BOOL                     commalp2p_outbuffer_claimed;

static U8                       commalp2p_service_na[2];
static U8                       commalp2p_service_na_len;

static U8                       commalp2p_service_ack[2];
static U8                       commalp2p_service_ack_len;

static COMMALP2P_REQUEST        commalp2p_request[COMMALP2P_REQUEST_COUNT];
static U8                       commalp2p_request_count;

#if (COMMALP2P_INCLUDE_PEERDETECTION == 1)
static TASK_HNDL                commalp2p_peerispresent_task;
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
const STRING  p2presponsestatenames[] = {"SENT OK", "RESP OK", "ACK TO", "RESP TO", "SERV N/A"};
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------/
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_CommAlP2PInfo(void)
{
    const STRING false_true[] = {"FALSE", "TRUE"};
    COMMALP2P_SERVICE*  service_ptr;
    COMMALP2P_REQUEST*  request_ptr;
    
    LOG_TRM("COMM P2P Info");
    LOG_TRM("srv |  id | active | rpl.to");
    for(service_ptr = commalp2p_service; service_ptr < &commalp2p_service[commalp2p_service_count]; service_ptr++)
    {
        LOG_TRM("%3d | %3d | %6s | %6d", PU8(service_ptr->service),
                                               PU8(service_ptr->request_id),
                                               PCSTR(false_true[service_ptr->timeout_countdown > 0]),
                                               PU16(service_ptr->timeout_countdown));
    }
    LOG_TRM("req |  id | active | ack.to | rsp.to");
    for(request_ptr = commalp2p_request; request_ptr < &commalp2p_request[commalp2p_request_count]; request_ptr++)
    {
        LOG_TRM("%3d | %3d | %6s | %6d | %6d", PU8(request_ptr->service),
                                                    PU8(request_ptr->request_id),
                                                    PCSTR(false_true[(BOOL)(request_ptr->response_callback != NULL)]),
                                                    PU16(request_ptr->ack_countdown),
                                                    PU16(request_ptr->response_countdown));
    }
    CoreTerm_PrintAcknowledge();
}
#endif
//------------------------------------------------------------------------------------------------/
static BOOL AlP2P_GetService(P2P_SERVICE service, COMMALP2P_SERVICE** service_ptr_ptr)
{
    for(*service_ptr_ptr = commalp2p_service; *service_ptr_ptr < &commalp2p_service[commalp2p_service_count]; (*service_ptr_ptr)++)
    {
        if((*service_ptr_ptr)->service == service)
        {
            return TRUE;
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL AlP2P_GetRequest(P2P_SERVICE service, COMMALP2P_REQUEST** request_ptr_ptr)
{
    for(*request_ptr_ptr = commalp2p_request; *request_ptr_ptr < &commalp2p_request[commalp2p_request_count]; (*request_ptr_ptr)++)
    {
        if((*request_ptr_ptr)->service == service)
        {
            return TRUE;
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void AlP2P_CallResponseCallback(COMMALP2P_REQUEST* request_ptr, RESPONSE_STATE response_state, U8* data_ptr, U8 data_len)
{
    COMMALP2P_RESPONSECALLBACK callback_hook = request_ptr->response_callback;
    
    if(callback_hook == NULL)
    {
        LOG_ERR("request_ptr->response_callback is NULL");
    }
    
    request_ptr->ack_countdown          = 0;
    request_ptr->response_countdown     = 0;
    request_ptr->response_callback      = NULL;
    
    callback_hook(response_state, data_ptr, data_len);
}
//------------------------------------------------------------------------------------------------//
static void AlP2P_OnFrameReceived(U8* data_ptr, U8 data_len)
{
    P2P_SERVICE         service = (P2P_SERVICE)(data_ptr[0] >> P2P_MSG_SERVICE_SHIFT);
    P2P_MSG_FUNC        p2p_msg_func = (P2P_MSG_FUNC)(data_ptr[0] & P2P_MSG_FUNC_SUBMASK);
    U8                  request_id = data_ptr[1];
    COMMALP2P_SERVICE*  service_ptr;
    COMMALP2P_REQUEST*  request_ptr;
    
    // check if message contains enough data
    if(data_len < 2)
    {
        return;
    }
    
#if (COMMALP2P_INCLUDE_PEERDETECTION == 1)
    CoreTask_Start(commalp2p_peerispresent_task);
#endif
    
    LOG_DEV("[P2P RCV] %02x", PU8A(data_ptr, data_len));
    
    if(p2p_msg_func == P2P_MSG_FUNC_REQUEST)   // if request
    {
        // check if service exists and can be handled
        if((AlP2P_GetService(service, &service_ptr) == TRUE) &&
           (service_ptr->request_handler != NULL) &&
           (service_ptr->request_handler(&data_ptr[2], data_len - 2) == TRUE))
        {
            // request service handling
            service_ptr->request_id         = request_id;
            service_ptr->timeout_countdown  = COMMALP2P_SERVICEHANDLING_TIMEOUT_IN_MS;
            
            // report service acknowledge
            commalp2p_service_ack[0] = (service << P2P_MSG_SERVICE_SHIFT) | (U8)P2P_MSG_FUNC_ACKNOWLEDGE;
            commalp2p_service_ack[1] = request_id;
            commalp2p_service_ack_len = 2;
            
            LOG_DEV("[P2P ACK] %02x", PU8A(commalp2p_service_ack, 2));
            
            if(CommDllP2P_SendFrame(commalp2p_service_ack, commalp2p_service_ack_len) == TRUE)
            {
                commalp2p_service_ack_len = 0;
            }
        }
        else
        {
            // report service not available
            commalp2p_service_na[0] = (service << P2P_MSG_SERVICE_SHIFT) | (U8)P2P_MSG_FUNC_NOTAVAILABLE;
            commalp2p_service_na[1] = request_id;
            commalp2p_service_na_len = 2;
            
            LOG_DEV("[P2P N/A] %02x", PU8A(commalp2p_service_na, 2));
            
            if(CommDllP2P_SendFrame(commalp2p_service_na, commalp2p_service_na_len) == TRUE)
            {
                commalp2p_service_na_len = 0;
            }
        }
    }
    else
    {
        // if response to active request
        if((AlP2P_GetRequest(service, &request_ptr) == TRUE) &&
           (request_ptr->request_id == request_id) &&
           (request_ptr->response_callback != NULL))
        {
            switch(p2p_msg_func)
            {
            case P2P_MSG_FUNC_ACKNOWLEDGE:
                // if acknowledge was expected
                if(request_ptr->ack_countdown > 0)
                {
                    request_ptr->ack_countdown = 0;
                    if(request_ptr->response_countdown == 0)
                    {
                        AlP2P_CallResponseCallback(request_ptr, RESPONSE_STATE_MESSAGE_SENT, NULL, 0);
                    }
                }
                break;
                
            case P2P_MSG_FUNC_RESPONSE:
                // if response was expected
                if(request_ptr->response_countdown > 0)
                {
                    request_ptr->ack_countdown = 0;
                    request_ptr->response_countdown = 0;
                    AlP2P_CallResponseCallback(request_ptr, RESPONSE_STATE_RESPONSE_RECEIVED, &data_ptr[2], data_len - 2);
                }
                break;
                
            case P2P_MSG_FUNC_NOTAVAILABLE:
                // if acknowledge or response was expected
                if((request_ptr->ack_countdown > 0) || (request_ptr->response_countdown > 0))
                {
                    request_ptr->ack_countdown = 0;
                    request_ptr->response_countdown = 0;
                    AlP2P_CallResponseCallback(request_ptr, RESPONSE_STATE_SERVICE_NA, NULL, 0);
                }
                break;
            }
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void AlP2P_TimeoutTask(VPTR data_ptr)
{
    COMMALP2P_SERVICE*  service_ptr;
    COMMALP2P_REQUEST*  request_ptr;
    
    // handle service timeout
    for(service_ptr = commalp2p_service; service_ptr < &commalp2p_service[commalp2p_service_count]; service_ptr++)
    {
        if(service_ptr->timeout_countdown > 0)
        {
            service_ptr->timeout_countdown--;
        }
    }
    
    // handle request timeout
    for(request_ptr = commalp2p_request; request_ptr < &commalp2p_request[commalp2p_request_count]; request_ptr++)
    {
        if(request_ptr->response_callback != NULL)
        {
            // if waiting for acknowledge
            if(request_ptr->ack_countdown > 0)
            {
                if(--request_ptr->ack_countdown == 0)
                {
                    AlP2P_CallResponseCallback(request_ptr, RESPONSE_STATE_ACKOWLEDGE_TO, NULL, 0);
                }
            }
            // else if waiting for response
            else if(request_ptr->response_countdown > 0)
            {
                if(--request_ptr->response_countdown == 0)
                {
                    AlP2P_CallResponseCallback(request_ptr, RESPONSE_STATE_RESPONSE_TO, NULL, 0);
                }
            }
        }
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL AlP2P_ClaimOutputBuffer(void)
{
    Core_CriticalEnter();
    if(commalp2p_outbuffer_claimed == FALSE)
    {
        commalp2p_outbuffer_claimed = TRUE;
        Core_CriticalExit();
        return TRUE;
    }
    Core_CriticalExit();
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void AlP2P_ReleaseOutputBuffer(void)
{
    // only release output buffer if emptied
    if(commalp2p_outbuffer_len == 0)
    {
        commalp2p_outbuffer_claimed = FALSE;
    }
}
//------------------------------------------------------------------------------------------------//
#if (COMMALP2P_INCLUDE_PEERDETECTION == 1)
static void AlP2P_IsPresentTimeout(VPTR data_ptr)
{
    CoreTask_Stop(commalp2p_peerispresent_task);
}
#endif
//------------------------------------------------------------------------------------------------//
static void CommAlP2P_Handler(void)
{
    COMMALP2P_SERVICE*  service_ptr;
    U8                  data_len;
    
    MODULE_CHECK();
    
    // if service ACK reply has not been sent yet: retry!
    if(commalp2p_service_ack_len != 0)
    {
        if(CommDllP2P_SendFrame(commalp2p_service_ack, commalp2p_service_ack_len) == TRUE)
        {
            commalp2p_service_ack_len = 0;
        }
    }
    
    // if service NA reply has not been sent yet: retry!
    if(commalp2p_service_na_len != 0)
    {
        if(CommDllP2P_SendFrame(commalp2p_service_na, commalp2p_service_na_len) == TRUE)
        {
            commalp2p_service_na_len = 0;
        }
    }
    
    // if output has not been sent yet: retry!
    if(commalp2p_outbuffer_len != 0)
    {
        if(CommDllP2P_SendFrame(commalp2p_outbuffer_ptr, commalp2p_outbuffer_len) == TRUE)
        {
            commalp2p_outbuffer_len = 0;
            AlP2P_ReleaseOutputBuffer();
        }
    }
    
    // if output buffer is empty check if reponses can be found
    if(AlP2P_ClaimOutputBuffer() == TRUE)
    {
        for(service_ptr = commalp2p_service; service_ptr <= &commalp2p_service[commalp2p_service_count]; service_ptr++)
        {
            if(service_ptr->timeout_countdown > 0)
            {
                if(service_ptr->response_handler(&commalp2p_outbuffer_ptr[2], &data_len) == DECODESTATE_RESPONSEREADY)
                {
                    service_ptr->timeout_countdown = 0;
                    
                    // check on returned length
                    if(data_len > (COMMALP2P_TX_BUFFER_LENGTH - 2))
                    {
                        LOG_ERR("Illegal data_len %d (service %d)", PU8(data_len), PU8(service_ptr->service));
                        break;
                    }
                    
                    // fill output buffer
                    commalp2p_outbuffer_ptr[0] = (service_ptr->service << P2P_MSG_SERVICE_SHIFT) | (U8)P2P_MSG_FUNC_RESPONSE;
                    commalp2p_outbuffer_ptr[1] = service_ptr->request_id;
                    commalp2p_outbuffer_len = data_len + 2;
                    
                    LOG_DEV("[P2P RPL] %02x", PU8A(commalp2p_outbuffer_ptr, commalp2p_outbuffer_len));
                    
                    // try to send it
                    if(CommDllP2P_SendFrame(commalp2p_outbuffer_ptr, commalp2p_outbuffer_len) == TRUE)
                    {
                        commalp2p_outbuffer_len = 0;
                    }
                    break;
                }
            }
        }
        AlP2P_ReleaseOutputBuffer();
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommAlP2P_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)commalp2p_service, 0, SIZEOF(commalp2p_service));
    commalp2p_service_count = 0;
    
    commalp2p_outbuffer_ptr = CoreBuffer_CreateStaticU8(COMMALP2P_TX_BUFFER_LENGTH, "P2P out");
    commalp2p_outbuffer_len = 0;
    commalp2p_outbuffer_claimed = FALSE;
    
    MEMSET((VPTR)commalp2p_service_na, 0, SIZEOF(commalp2p_service_na));
    commalp2p_service_na_len = 0;
    
    MEMSET((VPTR)commalp2p_service_ack, 0, SIZEOF(commalp2p_service_ack));
    commalp2p_service_ack_len = 0;
    
    MEMSET((VPTR)commalp2p_request, 0, SIZEOF(commalp2p_request));
    commalp2p_request_count = 0;
    
    CommDllP2P_RegisterFrameReceivedHook(AlP2P_OnFrameReceived);
    
    CoreTask_Start(CoreTask_RegisterTask(1e3, AlP2P_TimeoutTask, NULL, 128, "P2P TO"));
    
    CoreTerm_RegisterCommand("CommAlP2PInfo", "COMM P2P Info", 0, Command_CommAlP2PInfo, TRUE);
    
#if (COMMALP2P_INCLUDE_PEERDETECTION == 1)
    commalp2p_peerispresent_task = CoreTask_RegisterTask(5e6, AlP2P_IsPresentTimeout, NULL, 128, "P2P present TO");
#endif
    
    Core_RegisterModuleHandler(CommAlP2P_Handler);
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
// @remark  function to register a service handler to handle the received requests
// @return  returns TRUE if registering succeeded.
BOOL CommAlP2P_RegisterServiceHandler(P2P_SERVICE service, COMMALP2P_REQUESTHANDLER request_handler, COMMALP2P_RESPONSEHANDLER response_handler)
{
    COMMALP2P_SERVICE*  service_ptr;
    
    MODULE_CHECK();
    
    // check for legal inputs
    if(service >= 64)
    {
        LOG_ERR("Illegal service");
        return FALSE;
    }
    if(request_handler == NULL)
    {
        LOG_ERR("Illegal request_handler");
        return FALSE;
    }
    if(response_handler == NULL)
    {
        LOG_ERR("Illegal response_handler");
        return FALSE;
    }
    
    // search if already registered
    if(AlP2P_GetService(service, &service_ptr) == FALSE)
    {
        // if not check if there is still one free
        if(commalp2p_service_count >= COMMALP2P_SERVICE_COUNT)
        {
            LOG_ERR("service count overflow");
            return FALSE;
        }
        service_ptr = &commalp2p_service[commalp2p_service_count];
        commalp2p_service_count++;
    }
    
    // register service
    service_ptr->service            = service;
    service_ptr->request_id         = 0;
    service_ptr->timeout_countdown  = 0;
    service_ptr->request_handler    = request_handler;
    service_ptr->response_handler   = response_handler;
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
// @remark  function to send a service request to the other side
// @return  returns TRUE if sending succeeded.
BOOL CommAlP2P_SendServiceRequest(P2P_SERVICE service, U8* data_ptr, U8 data_len, BOOL wait_for_answer, COMMALP2P_RESPONSECALLBACK response_callback)
{
    COMMALP2P_REQUEST*  request_ptr;
    
    MODULE_CHECK();
    
    // check for legal inputs
    if(service >= 64)
    {
        LOG_ERR("Illegal service");
        return FALSE;
    }
	if(data_len > 0)
	{
		if(data_ptr == NULL)
		{
			LOG_ERR("Illegal data_ptr");
			return FALSE;
		}
	}
    if(data_len > (COMMALP2P_TX_BUFFER_LENGTH - 2))
    {
        LOG_ERR("Illegal data_len");
        return FALSE;
    }
    if(response_callback == NULL)
    {
        LOG_ERR("Illegal response_callback");
        return FALSE;
    }
    
    // if output buffer cannot be claimed, deny sending
    if(AlP2P_ClaimOutputBuffer() == FALSE)
    {
        LOG_DEV("Output buffer in use (service %d)", PU8(service));
        return FALSE;
    }
    
    // check if already request has been registered before
    if(AlP2P_GetRequest(service, &request_ptr) == TRUE)
    {
        // if request still active, deny
        if(request_ptr->response_callback != NULL)
        {
            LOG_DEV("Service %d busy", PU8(service));
            AlP2P_ReleaseOutputBuffer();
            return FALSE;
        }
    }
    // else if there are no more free spots, check if an old one can be freed
    else if(commalp2p_request_count >= COMMALP2P_REQUEST_COUNT)
    {
        for(request_ptr = commalp2p_request; request_ptr < &commalp2p_request[COMMALP2P_REQUEST_COUNT]; request_ptr++)
        {
            if(request_ptr->response_callback == NULL)
            {
                break;
            }
        }
        // check if legal spot
        if(request_ptr >= &commalp2p_request[COMMALP2P_REQUEST_COUNT])
        {
            LOG_DEV("No free sport for service %d", PU8(service));
            AlP2P_ReleaseOutputBuffer();
            return FALSE;
        }
    }
    // otherwise take the next free one.
    else
    {
        request_ptr = &commalp2p_request[commalp2p_request_count];
        commalp2p_request_count++;
    }
    
    // register request
    request_ptr->service            = service;
    request_ptr->request_id++;
    request_ptr->ack_countdown      = COMMALP2P_ACKNOWLEDGE_TIMEOUT_IN_MS;
    request_ptr->response_callback  = response_callback;
    request_ptr->response_countdown = 0;
	if(wait_for_answer)
	{
		request_ptr->response_countdown = COMMALP2P_SERVICEHANDLING_TIMEOUT_IN_MS;
	}
    
    // fill output buffer
    commalp2p_outbuffer_ptr[0] = (service << P2P_MSG_SERVICE_SHIFT) | (U8)P2P_MSG_FUNC_REQUEST;
    commalp2p_outbuffer_ptr[1] = request_ptr->request_id;
    if(data_len > 0)
    {
        MEMCPY((VPTR)&commalp2p_outbuffer_ptr[2], data_ptr, data_len);
    }
    commalp2p_outbuffer_len = data_len + 2;
    
    LOG_DEV("[P2P SND] %02x", PU8A(commalp2p_outbuffer_ptr, commalp2p_outbuffer_len));
    
    // try to send it
    if(CommDllP2P_SendFrame(commalp2p_outbuffer_ptr, commalp2p_outbuffer_len) == TRUE)
    {
        commalp2p_outbuffer_len = 0;
        AlP2P_ReleaseOutputBuffer();
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
// @remark  function to check if peer processor is present
// @remark  this check is done based on the reception of a valid message is the last 5 seconds
// @return  returns TRUE if peer was detected.
BOOL CommAlP2P_IsPeerPresent(void)
{
#if (COMMALP2P_INCLUDE_PEERDETECTION == 1)
    return CoreTask_IsTaskRunning(commalp2p_peerispresent_task);
#else
    return TRUE;        // always return TRUE
#endif
}
//================================================================================================//
