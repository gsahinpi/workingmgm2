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

std::thread *event_thread = NULL;
std::thread *command_thread = NULL;
bool stopSenderLoops = false;
int tmp = 0;

#if USE_ORDER_TCP_API==0

//thread for reading events from market
void order_event_thread() {
   while (!stopSenderLoops) {
      Magmio::TEventItem event;

      //get event from order sender
      if (api_sw->order.getEvent(event) != Magmio::TError::SUCCESS) { break; }

      std::cout << "Received event from order sender " << event.sessionID << " (port #" << event.physicalPort << ")" << std::endl;

      //resolve events
      switch (event.type) {
          case Magmio::TEventType::EVT_ORDER_SEND:
              std::cout << "Order #" << event.order.orderID << " SENT" << std::endl;
              std::cout << "Price: " << std::setprecision(10) << event.order.price << std::endl;
              std::cout << "Size: " << event.order.size << std::endl;
              std::cout << "Min QTY: " << event.order.minimumQty << std::endl;
              std::cout << "Side: " << event.order.side << std::endl;
              std::cout << "Time in force: " << event.order.timeInForce << std::endl;
              std::cout << "Order type: " << event.order.orderType << std::endl;
              std::cout << "Display: " << event.order.displayType << std::endl;
              std::cout << "ISO: " << event.order.iso << std::endl;
              std::cout << "Symbol: " << event.order.symbol << std::endl;
              std::cout << "Symbol (num): " << event.order.symbolNum << std::endl;
              std::cout << "SID: " << event.order.sid << std::endl;
              std::cout << "Strike price: " << event.order.strikePrice << std::endl;
              std::cout << "Strike date: " << event.order.strikeDate << std::endl;
              std::cout << "PutCall: " << event.order.putCall << std::endl;
              std::cout << "Position: " << event.order.position << std::endl;
              std::cout << "User tag: " << (unsigned int)event.order.userTag << std::endl << std::endl;
              std::cout << "Latency: " << event.latency << "ns" << std::endl << std::endl;
          break;

          case Magmio::TEventType::EVT_CANCEL_SEND:
              std::cout << "CANCEL of Order #" << event.order.orderID << " sent to market" << std::endl << std::endl;
          break;

          case Magmio::TEventType::EVT_ORDER_ACK:
              std::cout << "Order #" << event.order.orderID << " ACKED (market ID #" << event.order.marketID <<  ")" << std::endl << std::endl;
          break;

          case Magmio::TEventType::EVT_CANCEL_ACK:
              std::cout << "Cancel #" << event.order.orderID << " ACKED" << std::endl << std::endl;
          break;
          case Magmio::TEventType::EVT_ORDER_FILLED:
              std::cout << "Order #" << event.order.orderID << " FILLED (increment: " << event.order.size <<")" << std::endl << std::endl;
          break;

          case Magmio::TEventType::EVT_ORDER_KILLED:
              std::cout << "Order #" <<  event.order.orderID << " KILLED (decrement: " << event.order.size <<")" << std::endl << std::endl;
          break;

          case Magmio::TEventType::EVT_BROKEN_TRADE:
              std::cout << "Order #" <<  event.order.orderID << " BROKEN TRADE (price: " << event.order.price <<")" << std::endl << std::endl;
          break;

          case Magmio::TEventType::EVT_DONE_FOR_DAY:
              std::cout << "Order #" << event.order.orderID << " IS DONE FOR DAY" << std::endl << std::endl;
          break;

          case Magmio::TEventType::EVT_MESSAGE_REJECTED:
              std::cout << "Message with order id #" << event.order.orderID << " REJECTED" << std::endl;
              std::cout << "Description: " << event.infoText <<std::endl;
              std::cout << "Code: " << event.reasonCode <<std::endl << std::endl;
          break;

          case Magmio::TEventType::EVT_ERROR:
              std::cout << "Error: " << event.infoText << std::endl << std::endl;
              stop = true;
          break;

          default:
          break;
      }
   }
}


//thread for reading the commands
void handler_thread() {

    uint32_t decrement;
    uint16_t count;
    int handle, readed;
    std::string value;
    int session_id = 0;
    char buf[256];
    Magmio::TError err;
    struct pollfd fdp;
    int retpoll;

   //remove previous pipe if any
    remove("/tmp/magmio");

    //crate named pipe
    if (mkfifo("/tmp/magmio", 0777) != 0) {
        std::cerr << "ERROR: could not create named pipe /tmp/magmio" << std::endl;
        return;
    }

    //open it for reading (and writing, because we don't want our application to hang on this call)
    if ((handle = open("/tmp/magmio", O_RDWR)) < 0) {
        return;
    }
    fdp.fd = handle;
    fdp.events = POLLIN;

    //wait until there is something to read, in a loop
    while (!stop && !stopSenderLoops) {
        retpoll = poll(&fdp, 1, 500);
        if (retpoll > 0) {
            //something happened
            if ((readed = read(handle, buf, sizeof(buf)-1)) < 1) {
                  continue;
            }
        }
        else {
           //timeout / error -> continue
           continue;
        }
        buf[readed] = '\0'; //add terminator to proper place
        std::stringstream ss(buf);

        //the example is using first available session
        std::string session;// = api_sw->order.hwIDToSessionID(0);

        // Choose Action
        getline(ss, value,';');
        switch (value[0]) {

            case 'S':
               //stop the solution
               std::cout << "Stopping application..." << std::endl;
               stop = true;
               api_sw->signal_handler(SIGINT);
            break;

            case 'E':
               //enable order sender E <NUM>
               getline(ss, value,';');
               std::cout << "Enabling order sender, shoots: " << value << std::endl;
               count = std::stoul(value);

               getline(ss, value,';');
               session_id = std::stoi(value);
               session = api_sw->order.hwIDToSessionID(session_id);
               std::cout << "Session: " << session << std::endl;

               if ((err=api_sw->order.enable(session, count)) != Magmio::SUCCESS) {
                  std::cerr << "Error enabling order sender: " << Magmio::Log::getErrorMessage(err) << std::endl;
                  stop = true;
               }
            break;

            case 'C': // order_id;session;
               //cancel given order
               getline(ss, value,';');
               std::cout << "Cancel order: " << value << std::endl;
               decrement = std::stoull(value);

               getline(ss, value,';');
               session_id = std::stoi(value);
               session = api_sw->order.hwIDToSessionID(session_id);
               std::cout << "Session: " << session << std::endl;

               if ((err=api_sw->order.cancel(session, decrement)) != Magmio::SUCCESS) {
                  std::cerr << "Error cancelling order: " << Magmio::Log::getErrorMessage(err) << std::endl;
               }
            break;

            case 'F': {
               //update price // order_id;sid;price;size;session;
               std::cout << "Sending SW order" << std::endl;
               getline(ss, value,';');
               uint64_t orderid = std::stoull(value);
               Magmio::TSendOrder sw_ord;
               getline(ss, value,';');
               sw_ord.sid = std::stoi(value);
               getline(ss, value,';');
               sw_ord.askPrice = std::stoull(value);
               getline(ss, value,';');
               sw_ord.askSize = std::stoi(value);
               getline(ss, value,';');
               sw_ord.side = Magmio::TOrderSide(std::stoi(value));
               sw_ord.orderType = Magmio::TYPE_LIMIT; // TYPE_LIMIT // TYPE_MARKET // TYPE_MARKET_LIMIT
               sw_ord.timeInForce = Magmio::TIF_DAY;  // TIF_FAK //  TIF_DAY
               sw_ord.minimumQty = -1; //unset

               //getline(ss, value,';');
               session_id = 0;// std::stoi(value);
               session = api_sw->order.hwIDToSessionID(session_id);

               if ((err=api_sw->order.sendOrder(session, orderid, sw_ord)) != Magmio::SUCCESS) {
                  std::cerr << "Error sending SW order: " << Magmio::Log::getErrorMessage(err) << std::endl;
               }
            }
            break;

            case 'G': {
               //update price // order_id;sid;price;size;session;
               std::cout << "Sending SW order" << std::endl;
               getline(ss, value,';');
               uint64_t orderid = std::stoull(value);
               Magmio::TSendOrder sw_ord;
               getline(ss, value,';');
               sw_ord.sid = std::stoi(value);
               getline(ss, value,';');
               sw_ord.askPrice = -1 ;//std::stoull(value); unset for MARKET ORDERS
               getline(ss, value,';');
               sw_ord.askSize = std::stoi(value);
               getline(ss, value,';');
               sw_ord.side = Magmio::TOrderSide(std::stoi(value));
               sw_ord.orderType = Magmio::TYPE_MARKET_LIMIT; // TYPE_LIMIT // TYPE_MARKET // TYPE_MARKET_LIMIT
               sw_ord.timeInForce = Magmio::TIF_DAY;  // TIF_FAK //  TIF_DAY
               sw_ord.minimumQty = -1; //unset

               //getline(ss, value,';');
               session_id = 0;// std::stoi(value);
               session = api_sw->order.hwIDToSessionID(session_id);

               if ((err=api_sw->order.sendOrder(session, orderid, sw_ord)) != Magmio::SUCCESS) {
                  std::cerr << "Error sending SW order: " << Magmio::Log::getErrorMessage(err) << std::endl;
               }
            }
            break;

            case 'H': {
               //update price // order_id;sid;price;size;session;
               std::cout << "Sending SW order" << std::endl;
               getline(ss, value,';');
               uint64_t orderid = std::stoull(value);
               Magmio::TSendOrder sw_ord;
               getline(ss, value,';');
               sw_ord.sid = std::stoi(value);
               getline(ss, value,';');
               sw_ord.askPrice = std::stoull(value);
               getline(ss, value,';');
               sw_ord.askSize = std::stoi(value);
               getline(ss, value,';');
               sw_ord.side = Magmio::TOrderSide(std::stoi(value));
               sw_ord.orderType = Magmio::TYPE_LIMIT; // TYPE_LIMIT // TYPE_MARKET // TYPE_MARKET_LIMIT
               sw_ord.timeInForce = Magmio::TIF_FAK;  // TIF_FAK //  TIF_DAY
               sw_ord.minimumQty = -1; //unset

               //getline(ss, value,';');
               session_id = 0;// std::stoi(value);
               session = api_sw->order.hwIDToSessionID(session_id);

               if ((err=api_sw->order.sendOrder(session, orderid, sw_ord)) != Magmio::SUCCESS) {
                  std::cerr << "Error sending SW order: " << Magmio::Log::getErrorMessage(err) << std::endl;
               }
            }
            break;

            case 'W': {
               //update price // order_id;sid;Price;Size;minQtty;side;session
               std::cout << "Sending SW order" << std::endl;
               getline(ss, value,';');
               uint64_t orderid = std::stoull(value);
               Magmio::TSendOrder sw_ord;
               getline(ss, value,';');
               sw_ord.sid = std::stoi(value);
               getline(ss, value,';');
               sw_ord.askPrice = std::stoull(value);
               getline(ss, value,';');
               sw_ord.askSize = std::stoi(value);
               // sw_ord.side = Magmio::SIDE_SELL;
               sw_ord.orderType = Magmio::TYPE_LIMIT; // TYPE_LIMIT // TYPE_MARKET
               sw_ord.timeInForce = Magmio::TIF_FAK; // TIF_FAK //  TIF_DAY

               getline(ss, value,';');
               sw_ord.minimumQty = std::stoi(value);

               getline(ss, value,';');
               tmp = std::stoi(value);

               switch (tmp)
               {
                  case 1: sw_ord.side = Magmio::SIDE_SELL; break;
                  case 2: sw_ord.side = Magmio::SIDE_BUY; break;
                  default: std::cout << "ERROR: Choose 1-> sell 2-> buy !!!" << std::endl; break;
               }

               getline(ss, value,';');
               session_id = std::stoi(value);
               session = api_sw->order.hwIDToSessionID(session_id);
               std::cout << "Session: " << session << std::endl;

               if ((err=api_sw->order.sendOrder(session, orderid, sw_ord)) != Magmio::SUCCESS) {
                  std::cerr << "Error sending SW order: " << Magmio::Log::getErrorMessage(err) << std::endl;
               }
            }
            break;

            case 'A': { // cliOrdID;orderID;sid;Price;Size;side;
               Magmio::TSendOrder sw_rep;

               // replaceOrder() via sendOrderFpga, tailored for CME
               std::cout << "Sending Cancel/Replace request...(Limit-TIF)" << std::endl;

               sw_rep.msgType = Magmio::MSG_REPLACE_ORDER;

               // order ID to modify
               getline(ss, value,';');
               uint64_t cliOrdID = std::stoull(value);

               //fill generic values (for CME only)
               /*
               Magmio::TIlink3Cancel *genStruct = (Magmio::TIlink3Cancel*)sw_rep.generic;
               getline(ss, value,';');
               genStruct->orderID = std::stoull(value);
               genStruct->ifmFlag = true;
               */

               getline(ss, value,';');
               sw_rep.sid = std::stoi(value);

               // askPrice
               getline(ss, value,';');
               sw_rep.askPrice = std::stoull(value);

               // askSize
               getline(ss, value,';');
               sw_rep.askSize = std::stoi(value);

               // orderType
               sw_rep.orderType = Magmio::TYPE_LIMIT;

               // timeInForce
               sw_rep.timeInForce = Magmio::TIF_DAY;

               //side
               getline(ss, value,';');
               sw_rep.side = Magmio::TOrderSide(std::stoi(value));

               //minQty unset
               sw_rep.minimumQty = -1; //unset

               session = api_sw->order.hwIDToSessionID(0);
               if ((err=api_sw->order.sendOrderFpga(session, cliOrdID, sw_rep)) != Magmio::TError::SUCCESS) {
                  std::cerr << "Error sending SW replace: " << Magmio::Log::getErrorMessage(err) << std::endl;
               }
            }
            break;

            default:
            break;
        }
   }
}

#endif

bool startSender(bool enable_pipe) {
#if USE_ORDER_TCP_API==0

   Magmio::TError errs = api_sw->order.start();
   if (errs != Magmio::TError::SUCCESS) {
      std::cerr << "Error starting order sender: " << Magmio::Log::getErrorMessage(errs) << std::endl;
      return false;
   }
   else {
      std::cout << "Connected to server." << std::endl;
   }
   //create threads for controlling order sender
   event_thread = new std::thread(order_event_thread);
   if (enable_pipe) command_thread = new std::thread(handler_thread);
   
   return true;
#else
   std::cerr << "Order sender model not available for TCP API" << std::endl;
   return false;
#endif
}


void stopSender() {
#if USE_ORDER_TCP_API==0
   //Stop order sender
   api_sw->order.stop();

   //Destroy order sender threads
   stopSenderLoops = true;
   if (event_thread != NULL) {
      event_thread->join();
      delete event_thread;
      event_thread = NULL;
   }

   if (command_thread != NULL) {
      command_thread->join();
      delete command_thread;
      command_thread = NULL;
   }
#endif
}
