//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the MQTT application layer
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define COMMMQTT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COMMMQTT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               COMMMQTT_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// Maximum payload size (including variable header) for incoming messages
#ifdef MQTT_CONF_MAX_PAYLOAD_SIZE
    #define MQTT_MAX_PAYLOAD_SIZE       MQTT_CONF_MAX_PAYLOAD_SIZE
#else
    #define MQTT_MAX_PAYLOAD_SIZE       512
#endif
//------------------------------------------------------------------------------------------------//
#ifdef MQTT_CONF_DEFAULT_KEEPALIVE
    #define MQTT_DEFAULT_KEEPALIVE      MQTT_CONF_DEFAULT_KEEPALIVE
#else
    #define MQTT_DEFAULT_KEEPALIVE      30
#endif
//------------------------------------------------------------------------------------------------//
#ifndef MQTT_RX_BUFSIZE
    #define MQTT_RX_BUFSIZE             1024
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// COM
#include "picotcp\CommAlPicoTcp.h"
#include "CommMqtt.h"

// FREERTOS
#define MUTEX_PREFIX(postfix)                                   send_connect_mutex##postfix
#include "PsiMutexTpl.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define configMQTT_STACK_SIZE               500
//------------------------------------------------------------------------------------------------//
#define MQTT_REMAINING_LENGTH_BYTES(r)      (r < 128 ? 1 : (r < 16384 ? 2 : (r < 2097152 ? 3 : 4)))
//------------------------------------------------------------------------------------------------//
#define MQTT_IBUF_SIZE                      (MQTT_MAX_PAYLOAD_SIZE + 5)
#define MQTT_CONVERT_BUFFER_SIZE            10
//------------------------------------------------------------------------------------------------//
#define MQTT_PROTOCOL_NAME                  "MQIsdp"
#define MQTT_PROTOCOL_VERSION               3
//------------------------------------------------------------------------------------------------//
// Connect (we don't support will)
#define MQTT_CONNECT_FLAG_CLEAN_SESSION     0x02
#define MQTT_CONNECT_FLAG_PASSWORD          0x40
#define MQTT_CONNECT_FLAG_USERNAME          0x80
//------------------------------------------------------------------------------------------------//
// Connack
#define MQTT_CONNACK_RESPONSE_CODE_INDEX        3
#define MQTT_CONNACK_ACCEPTED                   0x00
#define MQTT_CONNACK_REFUSED_PROTOCOLVERSION    0x01
#define MQTT_CONNACK_REFUSED_IDENTIFIER         0x02
#define MQTT_CONNACK_REFUSED_UNAVAILABLE        0x03
#define MQTT_CONNACK_REFUSED_BAD_USER_PASSWORD  0x04
#define MQTT_CONNACK_REFUSED_NOT_AUTHORIZED     0x05
#define MQTT_GET_MSGTYPE(byte)                  ((byte & 0xF0) >> 4)
#define MQTT_SET_MSGTYPE(byte, type)            (byte = (byte & 0x0F) + ((type << 4) & 0xF0))
//------------------------------------------------------------------------------------------------//
#define MQTT_STRLEN(s)                          (2 + CoreString_GetLength((STRING)s))
#define MQTT_FIRST_BYTE(msgtype, dup, qos, retain) (((msgtype & 0x0F)<<4) \
		+ (dup ? 0x80 : 0x00) + ((qos & 0x03) << 1) + (retain ? 1 : 0))
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    MQTT_MSGTYPE_CONNECT            = 1,
    MQTT_MSGTYPE_CONNACK            = 2,
    MQTT_MSGTYPE_PUBLISH            = 3,
    MQTT_MSGTYPE_PUBACK             = 4,
    MQTT_MSGTYPE_PUBREC             = 5,
    MQTT_MSGTYPE_PUBREL             = 6,
    MQTT_MSGTYPE_PUBCOMP            = 7,
    MQTT_MSGTYPE_SUBSCRIBE          = 8,
    MQTT_MSGTYPE_SUBACK             = 9,
    MQTT_MSGTYPE_UNSUBSCRIBE        = 10,
    MQTT_MSGTYPE_UNSUBACK           = 11,
    MQTT_MSGTYPE_PINGREQ            = 12,
    MQTT_MSGTYPE_PINGRESP           = 13,
    MQTT_MSGTYPE_DISCONNECT         = 14
}
MQTT_MSG_TYPE;
//------------------------------------------------------------------------------------------------//
typedef enum
{
    MQTT_DISCONNECTED               = 0,
    MQTT_PENDING                    = 1,
    MQTT_CONNECTING                 = 2,
    MQTT_NETWORK_CONNECTED          = 3,
    MQTT_SENDING_CONNECT            = 4,
    MQTT_AWAITING_CONNACK           = 5,
    MQTT_CONNECTED                  = 6,
    MQTT_DISCONNECTING              = 7
}
MQTT_PROCESS_STATE;
//------------------------------------------------------------------------------------------------//
typedef enum
{
    MQTT_KA_IDLE                    = 0,
    MQTT_KA_PENDING                 = 1,
    MQTT_KA_SENDING_PINGREQ         = 2,
    MQTT_KA_AWAITING_PINGRESP       = 3
}
MQTT_KEEPALIVE_STATE;
//------------------------------------------------------------------------------------------------//
typedef struct
{
	U16         mid;
	const CHAR* topic;
	const CHAR* content;
	U32         len;
	U8          qos;
	BOOL        retain;
}
MQTT_PUBSTATE;
//------------------------------------------------------------------------------------------------//
typedef struct
{
	// Our state
	MQTT_PROCESS_STATE  state;
	// Configuration
	U16                 keepalive;
	// Mid counter
	U16                 mid;
	// For connect
	const U8*           ip;
	U16 port;
	const CHAR*         client_id;
	const CHAR*         username;
	const CHAR*         password;
	// For publishing
	MQTT_PUBSTATE pub;
	U16                 last_completed_mid;
	// For keepalive
	U8                  ka_state;
	// Buffers
	U32                 readlen;
	U8                  convert_buf[MQTT_CONVERT_BUFFER_SIZE];
	// Our connection
	struct uip_conn*    conn;
}
MQTT_STATE;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void Mqtt_Task(void* p_arg);
static void SendMqttStringLength(const CHAR* string);
static void send_connect(void);
static void send_disconnect(void);
static void send_publish(void);
static void mqtt_handle_message(void);
static void mqtt_handle_connack(void);
static void reset_pub_state(MQTT_PUBSTATE* pub);
static void reset_state();
static U32 allocate_mid(MQTT_STATE* s);
static U8 mqtt_encode_remaining_length(U32 len, U8* buffer);
static BOOL mqtt_decode_remaining_length(U8* buffer, S32 buflen, U32* remaining_length);
static U32 mqtt_is_message_complete(U8* buffer, U32 len);
static void ka_start(void);
static void ka_send_ping(void);
static void ka_timer_expired(VPTR data_ptr);
static void ka_pingresp_arrived(void);
static void SendFrameOfConstChar(const CHAR* string);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static CHAR                         copy[50];
static TASK_HNDL                    keep_alive_timer;
static MQTT_STATE                   mqtt_state;
static Q_HNDL                       rx_queue;
static U8                           rx_buffer[MQTT_IBUF_SIZE];
static U16                          rx_buffer_length;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void Mqtt_Task(void* p_arg)
{
    while(1)
    {
        vTaskDelay(10 * OS_TICKS_PER_100MS);
        
        if(mqtt_state.state == MQTT_NETWORK_CONNECTED)
        {
            mqtt_state.state = MQTT_SENDING_CONNECT;
            send_connect();
            // handle_input could have changed it
            mqtt_state.state = MQTT_AWAITING_CONNACK;
        }
        else if(mqtt_state.state == MQTT_DISCONNECTING)
        {
            send_disconnect();
            reset_state();
        }
        else if(mqtt_state.pub.topic != NULL  && mqtt_state.pub.content != NULL)
        {
            // We support only one at a time right now
            send_publish();
            // Publish done (we only support QoS 0 for now)
            mqtt_state.last_completed_mid = mqtt_state.pub.mid;
            reset_pub_state(&mqtt_state.pub);
        }
        else if(mqtt_state.ka_state == MQTT_KA_PENDING)
        {
            ka_send_ping();
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void send_connect(void)
{
    MQTT_STATE* s = &mqtt_state;
    U8 i;
    
    send_connect_mutex_Take(portMAX_DELAY);
    
	LOG_TRM("Sending CONNECT");
    
	// Send the connect fixed header
	s->convert_buf[0] = 0; // Initialise: all flags 0
	MQTT_SET_MSGTYPE(s->convert_buf[0], MQTT_MSGTYPE_CONNECT);  // 0x10
    
	// Send the header
	LOG_TRM("Send fixed CONNECT header");
    CommAlPicoTcp_SendFrame(s->convert_buf, 1 + mqtt_encode_remaining_length(
				// The variable header
				+ MQTT_STRLEN(MQTT_PROTOCOL_NAME) // the protocol name
				+ 4                               // version + flags + keepalive
				// The payload
				+ MQTT_STRLEN(s->client_id)       // clientid 
				+ (s->username != NULL ? MQTT_STRLEN(s->username) : 0)   // username (if not NULL) 
				+ (s->password != NULL ? MQTT_STRLEN(s->password) : 0),  // password (if not NULL)
				s->convert_buf+1));
	LOG_TRM("CONNECT: header sent");
    
	// Send the variable header
    s->convert_buf[0] = MSB(CoreString_GetLength(MQTT_PROTOCOL_NAME));
    s->convert_buf[1] = LSB(CoreString_GetLength(MQTT_PROTOCOL_NAME));
    for(i = 0; i < CoreString_GetLength(MQTT_PROTOCOL_NAME); i++)
    {
        s->convert_buf[2 + i] = MQTT_PROTOCOL_NAME[i];
    }
    CommAlPicoTcp_SendFrame(s->convert_buf, 2 + CoreString_GetLength(MQTT_PROTOCOL_NAME));
	LOG_TRM("CONNECT: protocol name sent");
	s->convert_buf[0] = MQTT_PROTOCOL_VERSION;
	s->convert_buf[1] = MQTT_CONNECT_FLAG_CLEAN_SESSION; 
	if(s->username != NULL)
    {
		s->convert_buf[1] |= MQTT_CONNECT_FLAG_USERNAME; 
	}
	if(s->password != NULL) 
    {
		s->convert_buf[1] |= MQTT_CONNECT_FLAG_PASSWORD; 
	}
    s->convert_buf[2] = MSB(s->keepalive);
    s->convert_buf[3] = LSB(s->keepalive);
    CommAlPicoTcp_SendFrame(s->convert_buf, 4);
	LOG_TRM("CONNECT: protocol version and flags sent");
    
	// Send the client id
    //   First the length
    SendMqttStringLength(s->client_id);
	//   Then the value
    SendFrameOfConstChar(s->client_id);
	LOG_TRM("CONNECT: client id sent");

	if(s->username != NULL)
    {
		// Send the username
	  //   First the length
		SendMqttStringLength(s->username);
		//   Then the value
        SendFrameOfConstChar(s->username);
		LOG_TRM("CONNECT: username sent");
	}

	if(s->password != NULL) 
    {
		// Send the password
		//   First the length
		SendMqttStringLength(s->password);
		//   Then the value
        SendFrameOfConstChar(s->password);
		LOG_TRM("CONNECT: password sent");
	}
    
    send_connect_mutex_Give();
}
//------------------------------------------------------------------------------------------------//
static void send_disconnect(void)
{
	mqtt_state.convert_buf[0] = 0;
	MQTT_SET_MSGTYPE(mqtt_state.convert_buf[0], MQTT_MSGTYPE_DISCONNECT);
	mqtt_state.convert_buf[1] = 0; // remaining lenght is 0
    
    CommAlPicoTcp_SendFrame(mqtt_state.convert_buf, 2);
}
//------------------------------------------------------------------------------------------------//
static void send_publish(void)
{
    // Send the header
	mqtt_state.convert_buf[0] = MQTT_FIRST_BYTE(MQTT_MSGTYPE_PUBLISH, FALSE, mqtt_state.pub.qos, mqtt_state.pub.retain);
	CommAlPicoTcp_SendFrame(mqtt_state.convert_buf, 1 + mqtt_encode_remaining_length(
				MQTT_STRLEN(mqtt_state.pub.topic)  // the topic
				+ (mqtt_state.pub.qos == 0 ? 0 : 2) // mid if QoS is not zero
				+ mqtt_state.pub.len,				
				mqtt_state.convert_buf+1));
    
	// Send the topic
	SendMqttStringLength(mqtt_state.pub.topic);
	SendFrameOfConstChar(mqtt_state.pub.topic);
    
	// Send the mid if needed
	if(mqtt_state.pub.qos != 0) 
    {
		// TODO: implement, we don't support QoS 1 and 2 yet
	}
    
	// Send the payload
	SendFrameOfConstChar(mqtt_state.pub.content);
}
//------------------------------------------------------------------------------------------------//
static void ReadCallback(U8* data_ptr, U16 length)
{
    if(CoreQ_Write(rx_queue, data_ptr, (U32)length))
    {
        LOG_TRM("Writing to RX Q: %02h", PU8A(data_ptr, (U32)length));
    }
    else
    {
        LOG_TRM("Failed writing to RX Q: %02h", PU8A(data_ptr, (U32)length));
    }
}
//------------------------------------------------------------------------------------------------//
// Read packet in buffer and call message dispatcher
static void handle_input(void)
{
    U32 to_read;
    
    while(CoreQ_Peek(rx_queue, rx_buffer, 2))
    {
        rx_buffer_length = 2;
        
        LOG_TRM("RX Q: 2 bytes read - %02h", PU8A(rx_buffer, 2));
        
        do
        {
            to_read = mqtt_is_message_complete(rx_buffer, rx_buffer_length);
            if(to_read == 0)
            {
                LOG_TRM("RX Q: incoming mssg complete - %02h", PU8A(rx_buffer, rx_buffer_length));
                // message is completely read
                CoreQ_Drop(rx_queue, rx_buffer_length);
                mqtt_handle_message();
            }
            else
            {
                if(CoreQ_Peek(rx_queue, rx_buffer, 2 + to_read))
                {
                    LOG_TRM("RX Q: some bytes more read");
                    rx_buffer_length += to_read;
                }
                else
                {
                    LOG_TRM("not all RX bytes available in Q yet");
                    return;
                }
            }
        }
        while(to_read > 0);
    }
}
//------------------------------------------------------------------------------------------------//
// Incoming message dispatching
static void mqtt_handle_message(void)
{
    LOG_TRM("Mqtt handle RX mssg");
    
	switch(MQTT_GET_MSGTYPE(rx_buffer[0])) 
    {
    case MQTT_MSGTYPE_CONNACK:
        mqtt_handle_connack();
        break;
    case MQTT_MSGTYPE_PINGRESP:
        ka_pingresp_arrived();
        break;
    default:
        LOG_TRM("Got MQTT msg type we didn't implement yet: 0x%02h", PU8(rx_buffer[0]));
        break;
	}
}
//------------------------------------------------------------------------------------------------//
static void mqtt_handle_connack(void)
{
	LOG_TRM("Handling CONNACK");
    
	if(rx_buffer[MQTT_CONNACK_RESPONSE_CODE_INDEX] == MQTT_CONNACK_ACCEPTED) 
    {
		LOG_TRM("Connected to broker!");
		mqtt_state.state = MQTT_CONNECTED;
		ka_start();
        // warning: HOOK to upper layer may be needed here
	}
    else 
    {
		LOG_TRM("Broker refused connection: 0x%02h", rx_buffer[MQTT_CONNACK_RESPONSE_CODE_INDEX]);
		reset_state();
		// Main process loop will take care of updating state and notifying caller
	}
}
//------------------------------------------------------------------------------------------------//
// MQTT utility functions
//------------------------------------------------------------------------------------------------//
// buffer should be at least 4 bytes (that's the maximum for the remaining length)
static U8 mqtt_encode_remaining_length(U32 len, U8* buffer)
{
	U8 i = 0;
    
	do 
    {
		buffer[i] = len % 128;
		len = len / 128;
		if( len > 0 ) 
        {
			buffer[i] = buffer[i] | 0x80;
		}
		++i;
	} 
    while ( len > 0 );
    
	return i;
}
//------------------------------------------------------------------------------------------------//
static BOOL mqtt_decode_remaining_length(U8* buffer, S32 buflen, U32* remaining_length)
{
	U32 multiplier = 1;
    S32 value = 0;
	U8 i = 0;
	U8 digit;
	
    do
    {
		digit = buffer[i];
		value += (digit & 127) * multiplier;
		multiplier *= 128;
		++i;
	} 
    while((digit & 128) && i < buflen);
	
    if(digit & 128) 
    {
		// Algorithm terminated prematurely
        return FALSE;
	}
    else 
    {
        *remaining_length = (U32)value;
        return TRUE;
	}
}
//------------------------------------------------------------------------------------------------//
// Check if the given buffer with the given length contains a complete mqtt message.
// If not enough bytes in the buffer: return the number of bytes that are at least needed to
// possibly have a complete message.
static U32 mqtt_is_message_complete(U8* buffer, U32 len)
{
    U32 remaining_length;
    
	if(len < 2)
    {
		return 2 - len; // We need at least 2 bytes to start
	}
    
	if(mqtt_decode_remaining_length(buffer+1, len-1, &remaining_length) == FALSE)
    {
		// We are lacking at least one byte of remaining_length
		return 1;
	}
    
	if(len < 1 + MQTT_REMAINING_LENGTH_BYTES(remaining_length) + remaining_length)
    {
		return (1 + MQTT_REMAINING_LENGTH_BYTES(remaining_length) + remaining_length) - len;
	}
	// Okay, we have the whole message!
	return 0;
}
//------------------------------------------------------------------------------------------------//
// Mqtt Block specific functions
//------------------------------------------------------------------------------------------------//
static void reset_pub_state(MQTT_PUBSTATE* pub)
{
	pub->mid        = 0;
	pub->topic      = NULL;
	pub->content    = NULL;
	pub->len        = 0;
	pub->qos        = 0;
	pub->retain     = FALSE;
}
//------------------------------------------------------------------------------------------------//
static void reset_state()
{
    LOG_TRM("Reset MQTT state");
    
	mqtt_state.state = MQTT_DISCONNECTED;
	mqtt_state.mid = 0;
	mqtt_state.ip = NULL;
	mqtt_state.port = 0;
	mqtt_state.client_id = NULL;
	mqtt_state.username = NULL;
	mqtt_state.password = NULL;
	reset_pub_state(&mqtt_state.pub);
	mqtt_state.last_completed_mid = 0;
	mqtt_state.ka_state = MQTT_KA_IDLE;
	mqtt_state.readlen = 0;
    // Q's resetten
    CommAlPicoTcp_Disconnect();
	mqtt_state.conn = NULL;
}
//------------------------------------------------------------------------------------------------//
static U32 allocate_mid(MQTT_STATE* s)
{
	s->mid++;
	if(s->mid == 0)
    {
        s->mid++; // Wrapped around, skip invalid 0
    }
	return s->mid;
}
//------------------------------------------------------------------------------------------------//
// Keepalives
//------------------------------------------------------------------------------------------------//
static void ka_start(void)
{
	mqtt_state.ka_state = MQTT_KA_IDLE;
    CoreTask_SetPeriod(keep_alive_timer, MAX(1, (mqtt_state.keepalive - 10) * 1000000));
    CoreTask_Start(keep_alive_timer);
}
//------------------------------------------------------------------------------------------------//
static void ka_send_ping(void)
{
	LOG_TRM("Sending PINGREQ");
	mqtt_state.ka_state = MQTT_KA_SENDING_PINGREQ;
    
	mqtt_state.convert_buf[0] = 0;
	MQTT_SET_MSGTYPE(mqtt_state.convert_buf[0], MQTT_MSGTYPE_PINGREQ);
	mqtt_state.convert_buf[1] = 0; // remaining lenght is 0
    
    CommAlPicoTcp_SendFrame(mqtt_state.convert_buf, 2);
    
	if(mqtt_state.ka_state == MQTT_KA_SENDING_PINGREQ)
    {
		mqtt_state.ka_state = MQTT_KA_AWAITING_PINGRESP;
        CoreTask_SetPeriod(keep_alive_timer, mqtt_state.keepalive * 1000000);
        CoreTask_Start(keep_alive_timer);
	}
}
//------------------------------------------------------------------------------------------------//
static void ka_timer_expired(VPTR data_ptr)
{
    LOG_TRM("Keep alive time error");
    
    if(mqtt_state.ka_state == MQTT_KA_IDLE)
    {
		mqtt_state.ka_state = MQTT_KA_PENDING;
	}
    else if(mqtt_state.ka_state == MQTT_KA_AWAITING_PINGRESP)
    {
		LOG_TRM("PINGRESP too late, closing connection");
        reset_state();
	}
}
//------------------------------------------------------------------------------------------------//
static void ka_pingresp_arrived(void)
{
	LOG_TRM("Handling PINGRESP");
	// We need to restart our cycle
	ka_start();
    return;
}
//------------------------------------------------------------------------------------------------//
// 
//------------------------------------------------------------------------------------------------//
static void SendFrameOfConstChar(const CHAR* string)
{
    U16 length = CoreString_GetLength((STRING)string);
    U8 i;
    
    for(i = 0; (i < 50) && (i < length); i++)
    {
        copy[i] = string[i];
    }
    CommAlPicoTcp_SendFrame((U8*)copy, i);
}
//------------------------------------------------------------------------------------------------//
static void SendMqttStringLength(const CHAR* string)
{
    U16 length = CoreString_GetLength((STRING)string);
    // LSB first
    length = (LSB(length) << 8) | MSB(length);
    
    CommAlPicoTcp_SendFrame((U8*)(&length), 2);
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommMqtt_Init(void)
{
    xTaskHandle task;
    U32 server_ip = ((U32)(10) | ((U32)(0) << 8) | ((U32)(182) << 16) | ((U32)(182) << 24));
    
    rx_queue = CoreQ_Register(MQTT_RX_BUFSIZE, 1, "MQTT Rx");
    MEMSET((VPTR)rx_buffer, 0, MQTT_IBUF_SIZE);
    rx_buffer_length = 0;
    
    keep_alive_timer = CoreTask_RegisterTask(MQTT_DEFAULT_KEEPALIVE * 1000000, ka_timer_expired, NULL, 120, "Mqtt keep alive timer task");
    
    CommAlPicoTcp_RegisterReadEventHook(ReadCallback);
    
    xTaskCreate(Mqtt_Task, (signed portCHAR*)"Mqtt Task", configMQTT_STACK_SIZE, NULL, 2, &task);
    CoreFreeRtos_Register(task);
}
//------------------------------------------------------------------------------------------------//
void CommMqtt_Handler(void)
{
    if(mqtt_state.state != MQTT_DISCONNECTED)
    {
        handle_input();
    }
}
//------------------------------------------------------------------------------------------------//
BOOL CommMqtt_Connect(const U8* ip, U16 port, const CHAR* client_id, const CHAR* username, const CHAR* password)
{
    LOG_TRM("Mqtt Connecting");
    
	if(mqtt_state.state != MQTT_DISCONNECTED)
    {
        LOG_TRM("MQTT not DISCONNECTED");
        return FALSE;
    }
    
    if((ip == NULL) || (client_id == NULL) || (username == NULL) || (password == NULL))
    { 
		LOG_TRM("Connection inputs invalid");
		return FALSE;
	}
    
	mqtt_state.state     = MQTT_PENDING;
	mqtt_state.ip        = ip;
	mqtt_state.port      = port;
	mqtt_state.client_id = client_id;
	mqtt_state.username  = username;
	mqtt_state.password  = password;
    
    /* open a TCP socket with the appropriate callback */
    LOG_TRM("Opening socket...");
    mqtt_state.state = MQTT_CONNECTING;
    
    if(CommAlPicoTcp_Connect((void*)(mqtt_state.ip), mqtt_state.port) == FALSE)
    {
        LOG_TRM("Connecting to socket failed");
        reset_state();
        return FALSE;
    }
    
    mqtt_state.state = MQTT_NETWORK_CONNECTED;
    LOG_TRM("Socket opened and connected to");
    
	return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CommMqtt_Disconnect(void)
{
	if(mqtt_state.state == MQTT_CONNECTED) 
    {
		mqtt_state.state = MQTT_DISCONNECTING;
		return TRUE;
	}
    else 
    {
		// TODO: support disconnect in other states?
		return FALSE;
	}
}
//------------------------------------------------------------------------------------------------//
U16 CommMqtt_Publish(const CHAR* topic, const CHAR* content, U32 content_len, U8 qos, BOOL retain)
{
	if(topic == NULL || content == NULL) 
    {
		LOG_TRM("MQTT publish called with invalid arguments: topic | content");
		return 0;
	}
    
	if(content_len > (268435455 - 2 - CoreString_GetLength((STRING)topic) - (qos > 0 ? 2 : 0))) 
    {
		LOG_TRM("MQTT publish called with too large content length: %d", PU32(content_len));
		return 0;
	}
    
	if(mqtt_state.state != MQTT_CONNECTED) 
    {
		LOG_TRM("Cannot publish when we are not connected");
		return 0;
	}
    
	if(qos != 0)
    {
		LOG_TRM("We only support QoS 0 for now!");
		return 0;
	}
    
	// In the future we might replace this with an allocate function
	MQTT_PUBSTATE * pstate;
    
	if(mqtt_state.pub.topic != NULL || mqtt_state.pub.content != NULL || mqtt_state.pub.len != 0) 
    {
		LOG_TRM("We support only one publish at a time!");
		pstate = NULL;
	}
    else 
    {
		pstate = &mqtt_state.pub;
        // JLG2: moved line from here to after the next if statement (if(pstate == NULL) ... for safety reason)
	}
    
	// End of to replace by allocate
	if(pstate == NULL) 
    {
		LOG_TRM("Could not allocate a publish state structure");
		return 0;
	}
    
    pstate->mid = allocate_mid(&mqtt_state);
    
	// Store everything
	pstate->len     = content_len;
	pstate->qos     = qos;
	pstate->retain  = retain;
	pstate->topic   = topic;
	pstate->content = content;
    
 	return pstate->mid;
}
//------------------------------------------------------------------------------------------------//
void CommMqtt_SetKeepalive(U16 keepalive)
{
	mqtt_state.keepalive = keepalive;
}
//------------------------------------------------------------------------------------------------//
BOOL CommMqtt_Connected()
{
	return (BOOL)(mqtt_state.state == MQTT_CONNECTED);
}
//------------------------------------------------------------------------------------------------//
BOOL CommMqtt_Disconnected()
{
	return (BOOL)(mqtt_state.state == MQTT_DISCONNECTED);
}
//------------------------------------------------------------------------------------------------//
U16 CommMqtt_Completed()
{
	return mqtt_state.last_completed_mid;
}
//================================================================================================//
