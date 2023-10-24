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

#include <ostream>

//Byte width of generic field. Also used in hft_types.h when running as model
#define MAX_GENERIC_FIELD_LEN 90

/*
 * Vivado HLS doesn't process enum type correctly with specified data type.
 * HLS macro is defined during strategy build by Vivado HLS as additional
 * CFLAGS param -DHLS.
 */
#ifdef HLS
   #define ENUM_DEF typedef enum
#else
   #define ENUM_DEF typedef enum : uint8_t
#endif

/**
 * @brief Order Time In Force enumeration
 */
ENUM_DEF {
   TIF_UNSET           = 0x00,      //!< Not set
   TIF_DAY             = 0x01,      //!< Day order, market hours
   TIF_OPENING         = 0x02,      //!< Opening auction only
   TIF_IOC             = 0x03,      //!< IOC
   TIF_FOK             = 0x04,      //!< FOK
   TIF_FAK             = 0x05,      //!< FAK
   TIF_CLOSING         = 0x06,      //!< Closing auction only
   TIF_EXTENDED        = 0x07,      //!< Extended hours
   TIF_GTC             = 0x08       //!< Good Till Cancelled
} TOrderTIF;


/**
 * @brief Message type selector
 */
ENUM_DEF {
   MSG_NEW_ORDER        = 0x00,     //New order message
   MSG_REPLACE_ORDER    = 0x01,     //Replace order message
   MSG_CANCEL_ORDER     = 0x02,     //Cancel order message
   MSG_NEW_LEG_ORDER    = 0x03,     //New multi-leg order
   MSG_CANCEL_LEG_ORDER = 0x04,     //Cancel multi-leg order
   MSG_MASS_CANCEL      = 0x05,     //Mass quote cancel
   MSG_SPREAD_ORDER     = 0x06,     //Spread order
   MSG_2_LEG_ORDER      = 0x07,     //New order with 2 legs
   MSG_3_LEG_ORDER      = 0x08,     //New order with 3 legs
   MSG_RESET_RISK       = 0x09      //Risk reset request
} TMessageType;


/**
 * @brief Order Type enumeration
 */
ENUM_DEF {
   TYPE_UNSET              = 0x00,  //!< Not set
   TYPE_MARKET             = 0x01,  //!< Market (or market with protection)
   TYPE_LIMIT              = 0x02,  //!< Limit
   TYPE_INSIDE_LIMIT       = 0x03,  //!< Inside limit
   TYPE_MARKET_LIMIT       = 0x04,  //!< Market to limit
   TYPE_MARKET_MAKER       = 0x05,  //!< Market maker
   TYPE_SHORT_QUOTE        = 0x06,  //!< Short quote
   TYPE_MARKET_SWEEP       = 0x07,  //!< Market sweep
   TYPE_AUCTION_RESPONSE   = 0x08,  //!< Auction response
   TYPE_LONG_QUOTE         = 0x09   //!< Long quote
} TOrderType;


/**
 * @brief Order Side enumeration
 */
ENUM_DEF {
    SIDE_UNSET         = 0x00,     //!< Not set
    SIDE_BUY           = 0x01,     //!< Buy
    SIDE_SELL          = 0x02,     //!< Sell
    SIDE_SELL_SHORT    = 0x03,     //!< Sell short
    SIDE_SELL_SHORTEX  = 0x04,     //!< Sell Short Exempt (SSR 201)
    SIDE_SELL_SHORTCCL = 0x05,     //!< Sell Short and Cancel (When SSR 201 is in effect)
    SIDE_CROSS         = 0x06,     //!< Cross
    SIDE_CROSS_SHORT   = 0x07      //!< Cross Short
} TOrderSide;


/**
 * @brief Order Side enumeration
 */
ENUM_DEF {
   POSITION_UNSET      = 0x00,     //!< Not set
   POSITION_OPEN       = 0x01,     //!< Open position
   POSITION_CLOSE      = 0x02      //!< Close position
} TOrderPosition;


/**
 * @brief Order Put Call enumeration
 */
ENUM_DEF {
   OPTION_PUT         = 0x00,    // Put
   OPTION_CALL        = 0x01,    // Call
   NOT_OPTION         = 0x02     //!< Not an option contract (future, maybe?)
} TOrderPutCall;


/**
 * @brief ISO Eligibility
 */
ENUM_DEF {
   ISO_UNSET          = 0x00,   //!< Not set
   ISO_YES            = 0x01,   //!< Eligible
   ISO_NO             = 0x02,   //!< Not eligible
} TIso;


/**
 * @brief Order Display enumeration
 */
ENUM_DEF {
   DISPLAY_UNSET          = 0x00,   //!< Not set
   DISPLAY_YES            = 0x01,   //!< Display
   DISPLAY_POST_ONLY      = 0x02,   //!< Post only
   DISPLAY_IMBALANCE_ONLY = 0x03,   //!< Imbalance only
   DISPLAY_NO             = 0x04,   //!< Do not display
   DISPLAY_REPRICED       = 0x05,   //!< Order was repriced
   DISPLAY_NBBO           = 0x06    //!< NBBO CONFORMANT, displayed one minimum increment away from price specified in the message
} TDisplay;



#if !defined(HLS) || defined(HLS_TB)
/**
 * @brief << Operators for the enums above
 */
inline std::ostream &operator<<(std::ostream &out, const TOrderTIF in) {
   switch (in) {
      case TIF_UNSET:      return out << "TIF_UNSET";
      case TIF_DAY:        return out << "TIF_DAY";
      case TIF_OPENING:    return out << "TIF_OPENING";
      case TIF_IOC:        return out << "TIF_IOC";
      case TIF_FOK:        return out << "TIF_FOK";
      case TIF_FAK:        return out << "TIF_FAK";
      case TIF_CLOSING:    return out << "TIF_CLOSING";
      case TIF_EXTENDED:   return out << "TIF_EXTENDED";
      case TIF_GTC:        return out << "TIF_GTC";
      default:             return out << (int)in;
   }
}


inline std::ostream &operator<<(std::ostream &out, const TMessageType in) {
   switch (in) {
      case MSG_NEW_ORDER:        return out << "MSG_NEW_ORDER";
      case MSG_REPLACE_ORDER:    return out << "MSG_REPLACE_ORDER";
      case MSG_CANCEL_ORDER:     return out << "MSG_CANCEL_ORDER";
      case MSG_NEW_LEG_ORDER:    return out << "MSG_NEW_LEG_ORDER";
      case MSG_CANCEL_LEG_ORDER: return out << "MSG_CANCEL_LEG_ORDER";
      case MSG_MASS_CANCEL:      return out << "MSG_MASS_CANCEL";
      default:                   return out << (int)in;
   }
}


inline std::ostream &operator<<(std::ostream &out, const TOrderType in) {
   switch (in) {
      case TYPE_UNSET:              return out << "TYPE_UNSET";
      case TYPE_MARKET:             return out << "TYPE_MARKET";
      case TYPE_LIMIT:              return out << "TYPE_LIMIT";
      case TYPE_INSIDE_LIMIT:       return out << "TYPE_INSIDE_LIMIT";
      case TYPE_MARKET_LIMIT:       return out << "TYPE_MARKET_LIMIT";
      case TYPE_MARKET_MAKER:       return out << "TYPE_MARKET_MAKER";
      case TYPE_SHORT_QUOTE:        return out << "TYPE_SHORT_QUOTE";
      case TYPE_MARKET_SWEEP:       return out << "TYPE_MARKET_SWEEP";
      case TYPE_AUCTION_RESPONSE:   return out << "TYPE_AUCTION_RESPONSE";
      case TYPE_LONG_QUOTE:         return out << "TYPE_LONG_QUOTE";
      default:                      return out << (int)in;
   }
}


inline std::ostream &operator<<(std::ostream &out, const TOrderSide in) {
   switch (in) {
      case SIDE_UNSET:           return out << "SIDE_UNSET";
      case SIDE_BUY:             return out << "SIDE_BUY";
      case SIDE_SELL:            return out << "SIDE_SELL";
      case SIDE_SELL_SHORT:      return out << "SIDE_SELL_SHORT";
      case SIDE_SELL_SHORTEX:    return out << "SIDE_SELL_SHORTEX";
      case SIDE_SELL_SHORTCCL:   return out << "SIDE_SELL_SHORTCCL";
      case SIDE_CROSS:           return out << "SIDE_CROSS";
      case SIDE_CROSS_SHORT:     return out << "SIDE_CROSS_SHORT";
      default:                   return out << (int)in;
   }
}


inline std::ostream &operator<<(std::ostream &out, const TOrderPosition in) {
   switch (in) {
      case POSITION_UNSET: return out << "POSITION_UNSET";
      case POSITION_OPEN:  return out << "POSITION_OPEN";
      case POSITION_CLOSE: return out << "POSITION_CLOSE";
      default:             return out << (int)in;
   }
}


inline std::ostream &operator<<(std::ostream &out, const TOrderPutCall in) {
   switch (in) {
      case OPTION_PUT:  return out << "OPTION_PUT";
      case OPTION_CALL: return out << "OPTION_CALL";
      default:          return out << (int)in;
   }
}


inline std::ostream &operator<<(std::ostream &out, const TIso in) {
   switch (in) {
      case ISO_UNSET:   return out << "ISO_UNSET";
      case ISO_YES:     return out << "ISO_YES";
      case ISO_NO:      return out << "ISO_NO";
      default:          return out << (int)in;
   }
}


inline std::ostream &operator<<(std::ostream &out, const TDisplay in) {
   switch (in) {
      case DISPLAY_UNSET:           return out << "DISPLAY_UNSET";
      case DISPLAY_YES:             return out << "DISPLAY_YES";
      case DISPLAY_POST_ONLY:       return out << "DISPLAY_POST_ONLY";
      case DISPLAY_IMBALANCE_ONLY:  return out << "DISPLAY_IMBALANCE_ONLY";
      case DISPLAY_NO:              return out << "DISPLAY_NO";
      case DISPLAY_REPRICED:        return out << "DISPLAY_REPRICED";
      case DISPLAY_NBBO:            return out << "DISPLAY_NBBO";
      default:                      return out << (int)in;
   }
}
#endif
