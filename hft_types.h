/*------------------------------------------------------------------------
 *
 *   Company: Magmio a.s.
 *
 *
 *   Project: Magmio
 *
 *
 * -----------------------------------------------------------------------
 *
 *
 *   (c) Copyright 2013-2023 Magmio a.s.
 *   All rights reserved.
 *
 *   Please review the terms of the license agreement before using this
 *   file. If you are not an authorized user, please destroy this
 *   source code file and notify Magmio a.s. immediately
 *   that you inadvertently received an unauthorized copy.
 *
 *------------------------------------------------------------------------
 */

#ifndef _HFT_INCL_H_
#define _HFT_INCL_H_

#define MAX_IDENT_WIDTH 58
#include <inttypes.h>
#include "order_enums.h"
#include <hls_stream.h>

#ifdef MODEL
#undef GENERIC_OUT_WIDTH
#define GENERIC_OUT_WIDTH MAX_GENERIC_FIELD_LEN
#undef FEEDBACK_WIDTH
#define FEEDBACK_WIDTH 128
#undef ORDER_ID_WIDTH
#define ORDER_ID_WIDTH 12
#undef ORDER_FEEDBACK_LEN
#define ORDER_FEEDBACK_LEN 0
#undef MESSAGE_SIZE
#define MESSAGE_SIZE 200
#endif

#if defined(MODEL) || defined(HLS_TB) || defined(FW_ORDER_STATUS_OFF)
#define ORDER_STATUS_OFF
#endif

#ifdef ORDER_STATUS_OFF
#undef ap_wait
#define ap_wait void
#endif

// When lenght zero, make it at least 1 in interface definition
#define GENERIC_WIDTH_INT (GENERIC_OUT_WIDTH > 0 ? GENERIC_OUT_WIDTH : 1)
#define ORDER_FEEDBACK_INT (ORDER_FEEDBACK_LEN > 0 ? ORDER_FEEDBACK_LEN : 1)

#define POSITION_TWO_SIDED_ASK_BID(a, b) TOrderPosition((a << 4) | b)
#define POSITION_ONE_SIDED(value) value

typedef struct tob {
   int64_t ask;           // Ask price
   int64_t bid;           // Bid price
   uint64_t ask_size;     // Ask quantity
   uint64_t bid_size;     // Bid quantity
   uint32_t ask_cussize;  // Ask customer quantity (used only for specific protocols)
   uint32_t bid_cussize;  // Bid customer quantity (used only for specific protocols)
   uint32_t ask_prosize;  // Ask professional quantity (used only for specific protocols)
   uint32_t bid_prosize;  // Bid professional quantity (used only for specific protocols)
   uint64_t ask_lvl_ts;   // Ask book level timestamp
   uint64_t bid_lvl_ts;   // Bid book level timestamp
   bool ask_empty;   // Ask book level empty
   bool bid_empty;   // Bid book level empty
   bool ask_valid;   // Ask book level valid
   bool bid_valid;   // Bid book level valid
} TToB;

typedef struct stat { // Obsolete (not used)
   uint32_t ema;
   uint32_t imid;
   uint32_t cmid;
   uint32_t mid;
} TStats;

typedef struct TFlags {
   bool eop;            // End of Packet indicator, the update msg is the last in packet
   bool seqError;       // Packet miss detected in arbiter unit
   bool swUpdate;       // Trigger caused by triggerStrategy api call
   bool ichanNotValid;  // SW synchronisation, info channel data are not valid
   bool obRstToggle;    // Toogle event when Order Book is full and internal reset is complete
} TFlags;

typedef struct TInfo {
   uint64_t timestamp;                 //!< Strategy start timestamp, nanoseconds since epoch
   uint64_t hwTimestamp;               //!< Packet timestamp (when received on IBUF), nanoseconds since epoch
   uint64_t exchangeTimestamp;         //!< Exchange timestamp, assigned by market
   TFlags flags;                       //!< Flags
   uint8_t feedType;                   //!< Type of data feed
   uint16_t msgType;                   //!< Message type (Template ID)
   uint8_t channelId;                  //!< Channel ID (rule ID in feed), if (flags.ichanNotValid), then channelId is 255 (0xFF), i.e. update was triggered by book sync.
   uint64_t packetSeqnum;              //!< Packet Sequence Number
   uint64_t msgSeqnum;                 //!< Message Sequence Number
   uint8_t decoderSeqnum;              //!< Decoder Sequence Number
   uint8_t decodedMsg[MESSAGE_SIZE];   //!< Decoded Message data array
} TInfo;

typedef int64_t TPrice;

typedef uint32_t TSize; // Quantity

typedef enum {
   NORMAL_FEED        = 0x00, //!< Usual multicast feed
   INCREMENTAL_FEED   = 0x01, //!< Multicast feed, indexed
   SNAPSHOT_FEED      = 0x02, //!< Feed with snapshot recovery data for given incremental feed
   REQUEST_FEED       = 0x03, //!< Through this feed Magmio can request for message replay
   ODEC_FEED          = 0x04  //!< Data for order sender (ie. fill events)
} TFeedType;

typedef enum {
   STATUS_OK           = 0x00,    // Order passed to order sender unit
   SESSION_DISABLED    = 0x01,    // Order dropped because target session is disabled
   FIFO_FULL           = 0x02,    // Too many requests for given session, order is dropped
   TCP_WINDOW_FULL     = 0x04,    // TCP windowing machanism dropped the order to prevent congestion on remote TCP side
   RISK_CHECK          = 0x08,    // Risk check prevented the order request from comming to order sender unit
   OUT_OF_RANGE        = 0x80     // Invalid session index, order was dropped
} TOrderStatus;

// Struct for internal use
typedef struct TOrderID {
   uint8_t data[ORDER_ID_WIDTH]; // Internal Order ID memory (the format of stored data depends on the protocol)
} TOrderID;

// Struct for order message configuration
// The types are defined as enums in order_enums.h
typedef struct TOrderOut {
   TMessageType message_type; // Which type of message send to market (new order, cancel, replace)
   TOrderType order_type;     // Type of order: market, limit, ...
   int64_t price;             // Price for non-quote protocols
   uint32_t size;             // Quantity for non-quote protocols
   int64_t ask_price;         // Ask price for quote protocols
   int64_t bid_price;         // Bid price used only for quote protocols (SQF, ArcaQuote)
   uint32_t ask_size;         // Ask quantity for quote protocols
   uint32_t bid_size;         // Bid quantity used only for quote protocols (SQF, ArcaQuote)
   uint32_t min_qty;          // Minimum quantity to trade
   TOrderSide side;           // Side of order
   TOrderPosition position;   // Position: open, close
   TOrderTIF time_in_force;   // Time in fore: day, ioc, fok, ...
   uint8_t order_id[ORDER_ID_WIDTH]; // Desired ID of the order
   TDisplay display;          // Display
   TIso iso;                  // ISO
   uint8_t user_tag;          // User assigned tag in strategy, can be used to track sent order through its life
   uint8_t session_index;     // Session identifier
   uint8_t generic_out[GENERIC_WIDTH_INT]; // Used to pass additional fields in the order specific for the protocol (if required)
   uint8_t instrument[MAX_IDENT_WIDTH]; // Instrument identification
   char feedback[ORDER_FEEDBACK_INT];
   bool feedback_vld;

   void getOrderID(uint64_t &order_id);
   void getOrderID(char *order_id);
} TOrderOut;

// Initializers of the struct above
void init_order_msg(TOrderOut *msgs, const uint8_t *instrument_in, unsigned int msg_count);
void init_order_msg(TOrderOut &msg, const uint8_t *instrument_in);

// Helper HLS function(s)
bool is_ascii_digit(const int num);

// Struct holding feedback data
typedef struct TFeedback{
   uint8_t data[FEEDBACK_WIDTH];
   uint8_t index;
   
   TFeedback();
} TFeedback;

// Class for passing order msgs to output
class OrderOutput {
public:
   OrderOutput(hls::stream<TOrderOut> &outstream, hls::stream<uint8_t> &status, hls::stream<TOrderID> *idstream);

   /**
    * @brief Send order message to given session, internal order ID is used
    * @param msg Order message body
    * @param session Session ID to write order to
    * @param fb Optional strategy feedback via order sender
    * @param fb_len Len of feedback. Assumed ORDER_FEEDBACK_LEN if unspecified
    * @return Zero if order was passed to order sender, see TOrderStatus
    */
   uint8_t send(TOrderOut &msg, uint8_t session, const char *fb = NULL, uint16_t fb_len = ORDER_FEEDBACK_LEN);

   /**
    * @brief Send order message to all sessions available, internal order IDs are used
    * @param msg Order message body
    * @param fb Optional strategy feedback via order sender
    * @param fb_len Len of feedback. Assumed ORDER_FEEDBACK_LEN if unspecified
    */
   void sendToAll(TOrderOut &msg, const char *fb = NULL, uint16_t fb_len = ORDER_FEEDBACK_LEN);

   /**
    * @brief Send order message to given session, using manually specified order id (binary)
    * @details In this case the internal order ID is NOT incremented
    * @param msg Order message body
    * @param session Session ID to write order to
    * @param order_id Order ID to use. Binary value
    * @param fb Optional strategy feedback via order sender
    * @param fb_len Len of feedback. Assumed ORDER_FEEDBACK_LEN if unspecified
    * @return Zero if order was passed to order sender, see TOrderStatus
    */
   uint8_t send(TOrderOut &msg, uint8_t session, uint64_t order_id, const char *fb = NULL, uint16_t fb_len = ORDER_FEEDBACK_LEN);

   /**
    * @brief Send order message to given session, using manually specified order id (ascii)
    * @details In this case the internal order ID is NOT incremented
    *          String is padded on the left with '0' if it's too short
    * @param msg Order message body
    * @param session Session ID to write order to
    * @param order_id Order ID to use. Arrays of char
    * @param order_id_len Legth of order_id ascii array. Shorter arrays padded with '0' on upper digits automatically
    * @param fb Optional strategy feedback via order sender
    * @param fb_len Len of feedback. Assumed ORDER_FEEDBACK_LEN if unspecified
    * @return Zero if order was passed to order sender, see TOrderStatus
    */
   uint8_t send(TOrderOut &msg, uint8_t session, const char *order_id, unsigned char order_id_len, const char *fb = NULL, uint16_t fb_len = ORDER_FEEDBACK_LEN);

   // Manually increment order ID. This is not needed, as send() does it automatically
   // Returns value of order ID before it is incremented
   TOrderID incrementOrderID(uint8_t session);

private:
   hls::stream<TOrderOut> &outstream;
   hls::stream<uint8_t> &status;
   hls::stream<TOrderID> *idstream;
};

// Class for passing feedback information to output
class FeedbackOutput {
public:
   FeedbackOutput(hls::stream<TFeedback> &dstream);

   // Send feedback data to API
   void send(TFeedback &data, const uint8_t index = 0);

private:
   hls::stream<TFeedback> &dstream;
};

// Macro for latency optimized memcpy
#define memcpy_unroll(dest, src, len)\
         for (unsigned int _ii = 0; _ii < len; _ii++) {\
            _Pragma("HLS unroll") \
            dest[_ii] = src[_ii];\
         }


// Copy string of given len, pad in destination with specified char if needed
#define strcpy_pad(dest, src, pad, dest_len, src_len)\
         for (unsigned int _ii = 0; _ii < dest_len; _ii++) {\
            _Pragma("HLS unroll") \
            if (_ii<src_len) \
               dest[_ii] = src[_ii]; \
            else \
               dest[_ii] = pad; \
         }

// Copy string of given len, pad in destination with zeros if needed
#define fill_dynamic_string(dest, src, dest_len, src_len)\
         for (unsigned int _ii = 0; _ii < dest_len; _ii++) {\
            _Pragma("HLS unroll") \
            if (_ii<src_len) \
               dest[_ii+1] = src[_ii]; \
            else \
               dest[_ii+1] = 0;\
         }\
         dest[0] = src_len;

#endif
