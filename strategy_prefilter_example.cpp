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

#include <list>

Magmio::TError setStrategyPrefilter(unsigned int setupIndex) {
    switch (setupIndex) {
        case 1: {
            std::cout << "INFO: Running setup for MsgType filter..." << std::endl;
            Magmio::TError error;
            bool msgTypeStat;
            std::list<uint16_t> msgTypes{65, 68, 70, 79};

            if ((error = api_sw->strategyPrefilter.dropMsgType(msgTypes.front())) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to drop MsgType: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            msgTypeStat = api_sw->strategyPrefilter.getMsgTypeStatus(msgTypes.front());
            std::cout << "Strategy Prefilter: MsgType " << msgTypes.front() << " is " << ((msgTypeStat) ? "passed" : "dropped") << std::endl;

            if ((error = api_sw->strategyPrefilter.passMsgType(msgTypes.front())) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to pass MsgType: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            msgTypeStat = api_sw->strategyPrefilter.getMsgTypeStatus(msgTypes.front());
            std::cout << "Strategy Prefilter: MsgType " << msgTypes.front() << " is " << ((msgTypeStat) ? "passed" : "dropped") << std::endl;

            if ((error = api_sw->strategyPrefilter.dropMsgTypes(msgTypes)) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to drop MsgTypes: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            for (auto msgType : msgTypes) {
                msgTypeStat = api_sw->strategyPrefilter.getMsgTypeStatus(msgType);
                std::cout << "Strategy Prefilter: MsgType " << msgType << " is " << ((msgTypeStat) ? "passed" : "dropped") << std::endl;
            }

            if ((error = api_sw->strategyPrefilter.passMsgType(msgTypes.front())) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to pass MsgType: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            msgTypeStat = api_sw->strategyPrefilter.getMsgTypeStatus(msgTypes.front());
            std::cout << "Strategy Prefilter: MsgType " << msgTypes.front() << " is " << ((msgTypeStat) ? "passed" : "dropped") << std::endl;

            if ((error = api_sw->strategyPrefilter.enableMsgTypeFilter()) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to enable MsgType filter: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            msgTypeStat = api_sw->strategyPrefilter.isMsgTypeFilterActive();
            std::cout << "Strategy Prefilter: MsgType filter: " << ((msgTypeStat) ? "enabled" : "disabled") << std::endl;

            break;
        }
        case 2: {
            std::cout << "Running setup for FeedType filter..." << std::endl;
            Magmio::TError error;
            bool feedTypeStat;

            feedTypeStat = api_sw->strategyPrefilter.isFeedTypePassed(Magmio::StrategyPrefilter::MARKET_TYPE);
            std::cout << "Strategy Prefilter: FeedType MARKET is " << ((feedTypeStat) ? "passed" : "dropped") << std::endl;

            if ((error = api_sw->strategyPrefilter.dropFeedType(Magmio::StrategyPrefilter::MARKET_TYPE)) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to drop FeedType MARKET: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            feedTypeStat = api_sw->strategyPrefilter.isFeedTypePassed(Magmio::StrategyPrefilter::MARKET_TYPE);
            std::cout << "Strategy Prefilter: FeedType MARKET is " << ((feedTypeStat) ? "passed" : "dropped") << std::endl;

            if ((error = api_sw->strategyPrefilter.enableFeedTypeFilter()) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to enable FeedType filter: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            break;
        }
        case 3: {
            std::cout << "Running setup for SW Sync filter..." << std::endl;
            Magmio::TError error;

            if ((error = api_sw->strategyPrefilter.enableSwSyncFilter()) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to enable SW Sync filter: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            break;
        }
        case 4: {
            std::cout << "Running setup for Book filter..." << std::endl;
            Magmio::TError error;

            if ((error = api_sw->strategyPrefilter.useAllLevels(false)) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to use only Top level: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            if ((error = api_sw->strategyPrefilter.usePrice(true)) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to use price for change: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            if ((error = api_sw->strategyPrefilter.useSize(true)) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to use size for change: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            if ((error = api_sw->strategyPrefilter.enableBookFilter()) != Magmio::SUCCESS) {
                std::cerr << "ERROR: Strategy Prefilter: Failed to enable Book filter: " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;
            }

            break;
        }
        default: {
            std::cout << "Unknown strategy prefilter setup (" << setupIndex << "). Strategy prefilter unit stays in default mode." << std::endl;
            break;
        }
    }
    return Magmio::SUCCESS;
}
