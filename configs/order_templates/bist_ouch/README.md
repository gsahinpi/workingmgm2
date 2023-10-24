# Description of received and decoded BIST_OUCH messages by Magmio

Order decoder for BIST OUCH currently supports the following messages:

- Order Accepted
- Order Rejected
- Order Replaced
- Order Canceled
- Order Executed

The content of BIST OUCH messages is available in strategy_engine in the input argument `decodedMsg`. The *first byte* in `decodedMsg` (`ofeed_flag` member in each structure declaration) is `0xFF` for all received BIST OUCH messages. This field should be used to differentiate received *order data* from *market data* in strategy_engine. The structure of each supported BIST OUCH message is declared in `bist_ouch/messages.h` for user-friendly processing, i.e. you have to only recast `decodedMsg` input array to a specific structure according to a value of the *second byte* in `decodedMsg`.

## Description of data in decodedMsg for various BIST OUCH messages

### Order Accepted

- **TID** = 65 (0x41)
- **StoredTime** = timestamp when the original `Enter Order` message was sent
- **StoredLeavesQty** = value of `Quantity` field used in the original `Enter Order` message
- **StoredSide** = value of `Side` field used in the original `Enter Order` message

Fields `ClOrdID` (Order Token), `Order_book_ID`, `Side`, `Order_ID`, `Quantity`, `Price` are exported directly from the received BIST OUCH message.

### Order Rejected

- **TID** = 74 (0x4A)
- **StoredTime** = timestamp when the original `Enter Order` message was sent
- **StoredLeavesQty** = value of `Quantity` field used in the original `Enter Order` message
- **StoredSide** = value of `Side` field used in the original `Enter Order` message

Fields `ClOrdID` (Order Token), `Reject_Code` are exported directly from the received BIST OUCH message.

### Order Replaced

After a `Order Replaced` message is received in Magmio as a rection to previously sent `Replace Order` message, it is internally split into two separate `Order Replaced` messages, i.e. strategy_engine is called twice, for each internal `Order Replaced` message separatelly. Furthermore, another message can be processed by strategy_engine between the first and the second internal `Order Replaced` msg.

#### The first Order Replaced

- **TID** = 85 (0x55)
- **ClOrdID** = `Order Token` in the original `Enter Order` message (Previous Order Token)
- **StoredTime** = timestamp when the original `Enter Order` message was sent
- **StoredLeavesQty** = original remaining Quantity value (not executed yet)
- **StoredSide** = value of `Side` field used in the original `Enter Order` message.

Other fields (`Order_book_ID`, `Side`, `Order_ID`, `Quantity` and `Price`) are set to zero.

#### The second Order Replaced

- **TID** = 85 (0x55)
- **ClOrdID** = new `Order Token` (Replacement Order Token)
- **StoredTime** = timestamp when the `Replace Order` message was sent
- **StoredLeavesQty** = new remaining Quantity value in the book (not executed yet)
- **StoredSide** = value of `Side` field used in the original `Enter Order` message

Fields `Order_book_ID`, `Side`, `Order_ID`, `Quantity` and `Price` are exported directly from the received BIST OUCH message.

### Order Canceled

- **TID** = 67 (0x43)
- **StoredTime** = timestamp when the original `Enter Order` message was sent
- **StoredLeavesQty** = Remaining `Quantity` of the cancelled order
- **StoredSide** = value of `Side` field used in the original `Enter Order` message

Fields `ClOrdID` (Order Token), `Order_book_ID`, `Side`, `Order_ID`, `Reason` are exported directly from the received BIST OUCH message.

### Order Executed

- **TID** = 69 (0x45)
- **StoredTime** = timestamp when the original `Enter Order` message was sent
- **StoredLeavesQty** = Remaining Quantity (not executed yet, 0 when fully filled order)
- **StoredSide** = value of `Side` field used in the original `Enter Order` message

Fields `ClOrdID` (Order Token), `Order_book_ID`, `Client_Category`, `Traded_Quantity`, `Trade_Price`, `Side`, `Match ID` (split into two fields `Match_ID_Low` and `Match_ID_High` because of the unusual byte length) are exported directly from the received BIST OUCH message.

## Use case (from the strategy_engine viewpoint)

```
   MAGMIO                         MARKET
(strategy_engine)                  (BIST OUCH)

Enter Order ------------------>
   Msg Type: "O"
   Order Token: "20052200002181"
   Order book ID: 74196
   Side: "B"
   Qty: 1000
   Price: 8000
   Timestamp: 1590136315227180690
   Client Category: 1

     <-------------------- Order Accepted
                              TID: 65 [0x41]
                              ClOrdID: "20052200002181" [0x3230303532323030303032313831]
                              StoredTime: 1590136315227180690 [0x16114C8DC5908E92]
                              StoredLeavesQty: 1000 [0x000003E8]
                              StoredSide: "B" [0x66]
                              Order_book_ID: 74196 [0x000121D4]
                              Side: "B" [0x66]
                              Order_ID: 7284707091741747225 [0x65187A8100092819]
                              Quantity: 1000 [0x00000000000003E8]
                              Price: 8000 [0x00001F40]

     <-------------------- Order Executed
                              TID: 69 [0x45]
                              ClOrdID: "20052200002181" [0x3230303532323030303032313831]
                              StoredTime: 1590136315227180690 [0x16114C8DC5908E92]
                              StoredLeavesQty: 800 [0x00000320] = Original Qty (1000) - Traded Qty(200)
                              StoredSide: "B" [0x66]
                              Order_book_ID: 74196 [0x000121D4]
                              Client_Category: 1 [0x01]
                              Traded_Quantity: 200 [0x00000000000000C8]
                              Trade_Price: 8000 [0x00001F40]
                              Match_ID_Low: 0 [0x00000000]
                              Match_ID_High: 728470713039585292 [0x0A1C0C410000000C]

Replace Order ---------------->
   Msg Type: "U"
   Existing Order Token: "20052200002181"
   Replacement Order Token: "20062200004879"
   Quantity: 700
   Price: 8000
   Client Category: 1
   Timestamp: 1590136315227181586

     <-------------------- Order Replaced 1
                              TID: 85 [0x55]
                              ClOrdID: "20052200002181" [0x3230303532323030303032313831]
                              StoredTime: 1590136315227180690 [0x16114C8DC5908E92]
                              StoredLeavesQty: 800 [0x00000320]
                              StoredSide: "B" [0x66]
                              Order_book_ID: 0 [0x00000000]
                              Side: 0 [0x00]
                              Order_ID: 0 [0x0000000000000000]
                              Quantity: 0 [0x0000000000000000]
                              Price: 0 [0x00000000]

     <-------------------- Order Replaced 2
                              TID: 85 [0x55]
                              ClOrdID: "20062200004879" [0x3230303632323030303034383739]
                              StoredTime: 1590136315227181586 [0x16114C8DC5909212]
                              StoredLeavesQty: 500 [0x0000000001F4] = original StoredLeavesQty (800) after partial order execution
                                                                      - [Quantity from `Enter Order` (1000) - Quantity from `Replace Order` (700)]

                              StoredSide: "B" [0x66]
                              Order_book_ID: 74196 [0x000121D4]
                              Side: "B" [0x66]
                              Order_ID: 7284707091741747225 [0x65187A8100092819]
                              Quantity: 500 [0x0000000001F4]
                              Price: 8000 [0x00001F40]
```
