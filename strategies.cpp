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

#include <hls_stream.h>
#include "strategies.h"
#include "hft_types.h"
void print_decoded_msg(const TInfo *tinfo) {
  for (int i = 0; i < MESSAGE_SIZE; i++) {
    printf("%d ", tinfo->decodedMsg[i]);
  }
  printf("\n");
}
void strategy_engine(
   const TToB prevTob[BOOK_LEVELS],
   const TToB currTob[BOOK_LEVELS],
   const uint8_t prevStatus,
   const uint8_t currStatus,

   const uint32_t order_memory_available_capacity,
   const uint64_t symbol_id,
   const uint8_t instrument_in[MAX_IDENT_WIDTH],
   const TInfo info,

   const int32_t global_params[GLOBAL_PARAMS_COUNT],
   const int32_t persymbolro_params[PERSYMBOLRO_PARAMS_COUNT],
   const int32_t persymbolrw_params[PERSYMBOLRW_PARAMS_COUNT],
   const int32_t pergroup_params[PERGROUP_PARAMS_COUNT],

   hls::stream<TOrderID> order_id_in[ORDER_SESSIONS],
   hls::stream<uint8_t> &order_status,
   const uint32_t session_disabled,
   const uint32_t session_fifo_full,
   const uint32_t session_congestion_tresh,
   hls::stream<TFeedback> &feedback_output,
   int32_t persymbolrw_params_out[PERSYMBOLRW_PARAMS_COUNT],
   int32_t pergroup_params_out[PERGROUP_PARAMS_COUNT],
   int32_t global_params_out[GLOBAL_PARAMS_COUNT],
   hls::stream<TOrderOut> &order_output){

   // Instances of classes used to communicate with HLS interface
   OrderOutput order(order_output, order_status, order_id_in);
   FeedbackOutput feedback(feedback_output);

#if (PERSYMBOLRW_PARAMS_COUNT > 0)
   int32_t persymbolrw_params_tmp[PERSYMBOLRW_PARAMS_COUNT];
   memcpy_unroll(persymbolrw_params_tmp, persymbolrw_params, PERSYMBOLRW_PARAMS_COUNT);
#endif

#if (PERGROUP_PARAMS_COUNT > 0)
   int32_t pergroup_params_tmp[PERGROUP_PARAMS_COUNT];
   memcpy_unroll(pergroup_params_tmp, pergroup_params, PERGROUP_PARAMS_COUNT);
#endif

#if (GLOBAL_PARAMS_COUNT > 0)
   int32_t global_params_tmp[GLOBAL_PARAMS_COUNT];
   memcpy_unroll(global_params_tmp, global_params, GLOBAL_PARAMS_COUNT);
#endif

//------------------------------------------------MODIFY THIS SECTION-----------------------------------------------

   // Local variables & initialization
   TOrderOut order_out;
   init_order_msg(order_out, instrument_in);
   TFeedback feedback_out;
   //////////////////////////////////////////////
               /*
         print_decoded_msg( &info) ;
         prints out :
         !!!!!!!!!!!!!in startegy
143 92 159 0 130 101 24 103 146 53 6 0 83 1 0 0 
0 0 0 0 0 65 248 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
         */
  std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!in startegy"<<std::endl;
   std::cout<<"!!!!!!!!symbol_id"<<symbol_id<<std::endl;
 
uint8_t const* decoded_msg = info.decodedMsg;

  // Print the contents of the array
  for (int i = 0; i < MESSAGE_SIZE; ++i) {
    std::cout << (int)decoded_msg[i] << " ";
  }
  std::cout << std::endl<<"-----------!!!!----------"<<std::endl;
  
   ////////////////////////////////////////      
   // uint8_t session_cntr = 0;
   bool make_order = false;

   // Initialization
   if (!(info.flags.ichanNotValid)) {

      order_out.side = SIDE_BUY;
      order_out.position = POSITION_OPEN;
      order_out.time_in_force = TIF_IOC;
      order_out.order_type = TYPE_LIMIT;
      order_out.display = DISPLAY_YES;
      order_out.iso = ISO_NO;
      order_out.message_type = MSG_NEW_ORDER;
      order_out.min_qty = 1;

      // Book bypass
   #if MESSAGE_SIZE == 51

      TMessage_bypass *bypass_data = (TMessage_bypass *)info.decodedMsg;

      if ((bypass_data->price > 0) && (bypass_data->quantity != 0)) {
         order_out.price = bypass_data->price;     // For all non-quote markets
         order_out.size = bypass_data->quantity;   // For all non-quote markets
         if (bypass_data->side) {
            order_out.bid_price = bypass_data->price;
            order_out.bid_size = bypass_data->quantity;
            order_out.side = SIDE_BUY;
         } else {
            order_out.ask_price = bypass_data->price;
            order_out.ask_size = bypass_data->quantity;
            order_out.side = SIDE_SELL;
         }
         make_order = true;
      }
      order_out.min_qty = 10;
      feedback_out.data[0] = bypass_data->TID;

   #else

      if (currTob[0].ask_size != 0) {
         order_out.price = currTob[0].ask;         // For all non-quote markets
         order_out.size = currTob[0].ask_size;     // For all non-quote markets

         order_out.bid_price = currTob[0].ask;
         order_out.bid_size = currTob[0].ask_size;
         order_out.side = SIDE_BUY;
         make_order = true;
      } else if (currTob[0].bid_size != 0) {
         order_out.price = currTob[0].bid;         // For all non-quote markets
         order_out.size = currTob[0].bid_size;     // For all non-quote markets

         order_out.ask_price = currTob[0].bid;
         order_out.ask_size = currTob[0].bid_size;
         order_out.side = SIDE_SELL;
         make_order = true;
      }
   #endif

      // Generate order (for session 0) & Send Feedback
      if (make_order) {
         if (order.send(order_out, 0) != STATUS_OK) {
            // If the order is dropped, increase global_param 0
            global_params_tmp[0]++;
         }
      }
      feedback.send(feedback_out);
   }

//------------------------------------------------------------------------------------------------------------------

#if (PERSYMBOLRW_PARAMS_COUNT > 0)
   memcpy_unroll(persymbolrw_params_out, persymbolrw_params_tmp, PERSYMBOLRW_PARAMS_COUNT);
#endif

#if (PERGROUP_PARAMS_COUNT > 0)
   memcpy_unroll(pergroup_params_out, pergroup_params_tmp, PERGROUP_PARAMS_COUNT);
#endif

#if (GLOBAL_PARAMS_COUNT > 0)
   memcpy_unroll(global_params_out, global_params_tmp, GLOBAL_PARAMS_COUNT);
#endif

   return;
}
