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

#include "strategies.h"
#include "hft_types.h"

TFeedback::TFeedback() {
   index = 0;
   for (unsigned int i = 0; i < FEEDBACK_WIDTH; i++) {
      #pragma HLS unroll
      data[i] = 0;
   }
}


void init_order_msg(TOrderOut &msg, const uint8_t *instrument_in) {
   #pragma HLS inline
   msg.message_type = MSG_NEW_ORDER;
   msg.order_type = TYPE_UNSET;
   msg.price = 0;
   msg.size = 0;
   msg.ask_price = 0;
   msg.bid_price = 0;
   msg.ask_size = 0;
   msg.bid_size = 0;
   msg.min_qty = 0;
   msg.side = SIDE_UNSET;
   msg.position = POSITION_UNSET;
   msg.time_in_force = TIF_UNSET;
   msg.display = DISPLAY_UNSET;
   msg.iso = ISO_UNSET;
   msg.user_tag = 0;
   for (unsigned int _i = 0; _i<GENERIC_WIDTH_INT; _i++) {
      #pragma HLS unroll
      msg.generic_out[_i] = 0;
   }
   for (unsigned int _i = 0; _i<MAX_IDENT_WIDTH; _i++) {
      #pragma HLS unroll
      msg.instrument[_i] = instrument_in[_i];
   }
   for (unsigned int _i = 0; _i<ORDER_FEEDBACK_INT; _i++) {
      #pragma HLS unroll
      msg.feedback[_i] = 0;
   }
   msg.feedback_vld = false;
   msg.session_index = 0;
}


void init_order_msg(TOrderOut *msgs, const uint8_t *instrument_in, unsigned int msg_count) {
   #pragma HLS inline
   for (unsigned int i = 0; i<msg_count; i++) {
      #pragma HLS unroll
      init_order_msg(msgs[i], instrument_in);
   }
}


bool is_ascii_digit(const int num) {
   #pragma HLS inline
   if ((num >= '0') && (num <= '9')) return true; else return false;
}


void TOrderOut::getOrderID(uint64_t &order_id) {
   #pragma HLS inline
   order_id = *((uint64_t*)this->order_id);
}


void TOrderOut::getOrderID(char *order_id) {
   #pragma HLS inline
   if (order_id != NULL) {
      for (unsigned int i = 0; i < ORDER_ID_WIDTH; i++) {
         #pragma HLS unroll
         order_id[i] = this->order_id[i];
      }
   }
}


FeedbackOutput::FeedbackOutput(hls::stream<TFeedback> &dstream) : dstream(dstream) {

}


void FeedbackOutput::send(TFeedback &data, const uint8_t index) {
   #pragma HLS inline
   data.index = index;
   dstream.write(data);
}


OrderOutput::OrderOutput(hls::stream<TOrderOut> &outstream, hls::stream<uint8_t> &status, hls::stream<TOrderID> *idstream)
   : outstream(outstream),
     status(status),
     idstream(idstream) {

}


void OrderOutput::sendToAll(TOrderOut &msg, const char *fb, uint16_t fb_len) {
   #pragma HLS inline
   if (fb != NULL) {
      uint8_t safe_fb_len = (fb_len > ORDER_FEEDBACK_LEN ? ORDER_FEEDBACK_LEN : fb_len);
      for (unsigned int i = 0; i < safe_fb_len; i++) {
         #pragma HLS unroll
         msg.feedback[i] = fb[i];
      }
      msg.feedback_vld = true;
   }
   for (unsigned int i = 0; i<ORDER_SESSIONS; i++) {
      #pragma HLS unroll
      TOrderID tmp_id;
      idstream[i].read(tmp_id);
      for (unsigned int j = 0; j < ORDER_ID_WIDTH; j++) {
         #pragma HLS unroll
         msg.order_id[j] = tmp_id.data[j];
      }
      msg.session_index = i;
      outstream.write(msg);
   }
}


uint8_t OrderOutput::send(TOrderOut &msg, uint8_t session, const char *fb, uint16_t fb_len) {
   uint8_t tmp = OUT_OF_RANGE;
   #pragma HLS inline
   if (session<ORDER_SESSIONS) {
      msg.session_index = session;
      TOrderID tmp_id;
      idstream[session].read(tmp_id);
      for (unsigned int i = 0; i < ORDER_ID_WIDTH; i++) {
         #pragma HLS unroll
         msg.order_id[i] = tmp_id.data[i];
      }
      if (fb != NULL) {
         uint8_t safe_fb_len = (fb_len > ORDER_FEEDBACK_LEN ? ORDER_FEEDBACK_LEN : fb_len);
         for (unsigned int i = 0; i < safe_fb_len; i++) {
            #pragma HLS unroll
            msg.feedback[i] = fb[i];
         }
         msg.feedback_vld = true;
      }
      {
         #pragma HLS protocol floating
         outstream.write(msg);
         ap_wait();
         status.read(tmp);
      }
   }
   return tmp;
}


uint8_t OrderOutput::send(TOrderOut &msg, uint8_t session, uint64_t order_id, const char *fb, uint16_t fb_len) {
   uint8_t tmp = OUT_OF_RANGE;
   #pragma HLS inline
   if (session<ORDER_SESSIONS) {
      if (fb != NULL) {
         uint8_t safe_fb_len = (fb_len > ORDER_FEEDBACK_LEN ? ORDER_FEEDBACK_LEN : fb_len);
         for (unsigned int i = 0; i < safe_fb_len; i++) {
            #pragma HLS unroll
            msg.feedback[i] = fb[i];
         }
         msg.feedback_vld = true;
      }
      msg.session_index = session;
      *((uint64_t*)(msg.order_id)) = order_id;
      {
         #pragma HLS protocol floating
         outstream.write(msg);
         ap_wait();
         status.read(tmp);
      }
   }
   return tmp;
}


uint8_t OrderOutput::send(TOrderOut &msg, uint8_t session, const char *order_id, unsigned char order_id_len, const char *fb, uint16_t fb_len) {
   uint8_t tmp = OUT_OF_RANGE;
   #pragma HLS inline
   if (session<ORDER_SESSIONS) {
      msg.session_index = session;
      unsigned char pad_len;
      if (order_id_len < ORDER_ID_WIDTH) {
         pad_len = ORDER_ID_WIDTH - order_id_len;
      } else {
         pad_len = 0;
      }
      for (unsigned int i = 0; i < ORDER_ID_WIDTH; i++) {
         #pragma HLS unroll
         if (i < pad_len) {
            msg.order_id[i] = '0';
         } else {
            msg.order_id[i] = order_id[i-pad_len];
         }
      }
      if (fb != NULL) {
         uint8_t safe_fb_len = (fb_len > ORDER_FEEDBACK_LEN ? ORDER_FEEDBACK_LEN : fb_len);
         for (unsigned int i = 0; i < safe_fb_len; i++) {
            #pragma HLS unroll
            msg.feedback[i] = fb[i];
         }
         msg.feedback_vld = true;
      }
      {
         #pragma HLS protocol floating
         outstream.write(msg);
         ap_wait();
         status.read(tmp);
      }
   }
   return tmp;
}


TOrderID OrderOutput::incrementOrderID(uint8_t session) {
   #pragma HLS inline
   TOrderID tmp_id;
   if (session<ORDER_SESSIONS) {
      idstream[session].read(tmp_id);
   }
   return tmp_id;
}
