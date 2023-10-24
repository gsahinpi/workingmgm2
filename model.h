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

#include <magmio/magmio.h>
#include <thread>
#include "order-sender/order_sender.h"
#include "packet-filter/packet-filter.h"
#include "channel-arbiter/arbiter_top.h"
#include "recovery-buffer/recovery_top.h"
#include "decoder/decoder_top.h"
#include "recovery-filter/seqfilter_top.h"
#include "book-handler/book_handler.h"
#include "book-channel/book_channel.h"
#include "book-uncross/book_uncross.h"
#include "strategy-engine/strategy_engine.h"
#include "strategy-prefilter/strategy_prefilter.h"
#include "generator/pcap_reader.h"
#include "generator/mcast_reader.h"
#include "comparator/comparator.h"
#include "pretrade_risk_check/pretrade_risk_check.h"
#include "common.h"
#include "counters/counters_model.h"
#include "active_order_memory/active_order_memory.h"

/**
 * @brief Magmio model
 * This model simulate PCAP file and generate formatted output using
 * Magmio API.
 * Supported outputs:
 *  - Decoder
 *  - Enhanced Top of Book
 *  - Strategy engine
 */
class MagmioModel {

public:
    const unsigned MI32_ADDR_AFTER_SPLITTER = 18; //!< MI32 Address bits after top splitter

    typedef struct TModelConf {
        bool statsEnabled;
        bool bypass_enabled;
        bool recovery_inhibit;
        bool decodePackets;
        Decoder::DummyEopMode dummyMode;
        unsigned int pltLevels;
        unsigned int strategyOutputs;
        std::string apiMapping;
        unsigned int apiGlobalParams;
        unsigned int apiPersymbolroParams;
        unsigned int apiPerGroupParams;
        unsigned int apiPersymbolrwParams;
        unsigned int apiPretradeParams;
        unsigned int apiCurrTobLevels;
        unsigned int apiPrevTobLevels;
        bool stratPrefilterEnabled;
        unsigned int stratPrefilterMsgTypeWidth;
        bool etobChannelEnabled;
        bool bookChannelEnabled;
        unsigned int bookChannelPrevExportedLevels;
        unsigned int bookChannelCurrExportedLevels;
        bool uncrossPrevBook;
        bool uncrossCurrBook;

        TModelConf(): statsEnabled(false), bypass_enabled(false), recovery_inhibit(false), decodePackets(false), dummyMode(Decoder::DummyEopMode::DISABLED),
                        pltLevels(24), strategyOutputs(0), apiMapping(""), apiGlobalParams(0), apiPersymbolroParams(0),
                        apiPersymbolrwParams(0), apiPretradeParams(0), apiCurrTobLevels(0), apiPrevTobLevels(0),
                        stratPrefilterEnabled(false), stratPrefilterMsgTypeWidth(12), etobChannelEnabled(false),
                        bookChannelEnabled(false), bookChannelPrevExportedLevels(0), bookChannelCurrExportedLevels(0),
                        uncrossPrevBook(false), uncrossCurrBook(false) {};

    } TModelConf;

    /**
     * @brief Constructor
     * @param sw_api   Pointer to Magmio API. Used for simulation & verification
     * @param fw_api   Optional pointer to Magmio API. Used when verification is enabled
     */
    MagmioModel(Magmio::MagmioApi *sw_api, Magmio::MagmioApi *fw_api=NULL);
    ~MagmioModel();


    /**
     * @brief Initialize model, also checks consistency of SW strategies against FW (if FW API is supplied)
     * @param stats_enabled Compute statistics on etob output?
     * @param debug_width Width of debug array in strategy interface
     * @param strategy_output_coun How many strategy outputs should be in export / pretrade unit?
     * @param order_protocol Which protocol is used to configure strategies?
     */
    int init(TModelConf conf);


    /**
     * @brief Check consistency of SW strategies against FW
     * @param sw_hash SHA1 checksum of SW strategy file
     * @return true on success
     */
    bool checkStrategiesConsistency(const std::string &sw_hash);


    /**
    * @brief Run simulation model - replay PCAP, non-blocking, automatically executed in separate thread
    * @param pcapfile Path to a PCAP file
    * @param phy_port Simulate from which port are data originated
    */
    void replayPcap(const std::string &pcapfile, uint8_t phy_port=0);


    /**
    * @brief Verify FW decoder output against SW model, blocking method
    * @param quiet true to display errors only
    * @return zero on success
    */
    int verifyDecoder(bool quiet = false);


    /**
    * @brief Verify FW Enhanced feed output against SW model, blocking method
    * @param quiet true to display errors only
    * @return zero on success
    */
    int verifyEtob(bool quiet = false);


    /**
    * @brief Verify FW Strategy engine output against SW model, blocking method
    * @param quiet true to display errors only
    * @return zero on success
    */
    int verifyStrategyEngine(bool quiet = false);


    /**
    * @brief Verify data statistics from SW (and FW)
    * @return zero on success
    */
    int verifyStatistics();


    /**
     * @brief Subscribe and start reading from multicast feeds
    * @return True on success
     */
    bool startMulticast();


    /**
     * @brief Stop reading and unscubscribe multicast feeds
     */
    void stopMulticast();


    /**
     * @brief Stop model simulation / verification
     */
    void stop();


    /**
     * @brief Returns true when end of pcap file has been reached (simulation ended).
     */
    bool eof();

    /**
     * @brief Returns 0 when all verifications were good
     */
    int returnCode();

    /**
     * @brief Set Strategy input/output recording
     * @param Set of streams for destination files
     */
    bool recordStrategyInouts(TDatasetStreams *dataset_streams);

    /**
     * @brief Return number of price levels in Book
     * @return unsigned int Book price Level count
     */
    unsigned int getPriceLevelCount() const;

private:
    Magmio::MagmioApi *sw_api, *fw_api;
    Common common;
    Comparator *comparator;

    OrderSender *order_sender;
    PacketFilter *filter;
    ArbiterTop *arbiter;
    RecoveryTop *recovery;
    DecoderTop *decoder;
    SeqfilterTop *seqfilter;
    BookHandler *book;
    StrategyPrefilter *strategy_prefilter;
    StrategyEngine *strategies;
    PreTradeRiskCheck *riskcheck;
    ActiveOrderMemory *active_order_memory;
    BookChannel *book_channel;
    BookUncross *book_uncross;

    Framelink       *generator_filter_fl;
    DecoderInIfc    *filter_arbiter_fl;
    RecoveryInIfc   *arbiter_recovery_fl;
    DecoderInIfc    *recovery_decoder_fl;
    OrdDecoderInIfc *filter_ord_decoder_fl;
    Framelink       *decoder_sidout_fl;
    DecoderOutIfc   *decoder_seqfilter_fl;
    DecoderOutIfc   *seqfilter_book_fl;
    DecoderOutIfc   *active_order_mem_out_bus;
    Framelink       *book_out_fl;
    StrategyDataBus *book_prefilter_bus;
    StrategyDataBus *prefilter_strategy_bus;
    StrategyOutBus  *strategy_out_bus;
    StrategyOutBus  *riskcheck_out_bus;

    McastReader *reader;
    PcapReader *pcap_gen;

    ModelCounters counters;

    bool _stop;
    bool eof_flag;
    int return_code;
    bool book_bypass_on;

    std::thread *sim_thread;
    int start_reading_from_fw();
    void _replayPcap(const std::string &pcapfile, uint8_t phy_port); //executed in thread
};

