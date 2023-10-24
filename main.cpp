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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <memory>
#include <magmio/magmio.h>
#include <magmio/settings.h>
/* Uncomment for CME only - required by order_example.cpp */
//#include <magmio/msgs/cme_ilink/generic_mapping.h>
#include "model.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "strategies.h"


MagmioModel *model = NULL;
Magmio::MagmioApi *api_sw = NULL;
bool stop = false;

TDatasetStreams *tbFileStreams = NULL;

// Define the function to be called when ctrl-c (SIGINT) signal is sent
void signal_callback_handler(int signum) {
    if (signum == SIGINT && !stop) {
        stop = true;
        cout << "Stopping Magmio model." << endl;
        if (model!=NULL) model->stop();
    }
    api_sw->signal_handler(signum);
}


void freeMem() {
   if (api_sw  != NULL)
      delete api_sw ;

   if (model != NULL)
      delete model;

   if (tbFileStreams != NULL)
      delete tbFileStreams;
}

#include "strategy_prefilter_example.cpp"

#include "order_example.cpp"


int main(int argc, char *argv[]) {

   //handle CTRL+C (stop the application)
   signal(SIGINT, signal_callback_handler);

   //more dynamic arguments are read from CLI
   int c;
   Magmio::Log::tVerbosityLevel verbosity = Magmio::Log::ERROR;
   char *filePcap = NULL;
   char *fileMapping = NULL;
   char *fileGlobalParams = NULL;
   char *filePersymbolroParams = NULL;
   char *filePerGroupParams = NULL;
   char *filePersymbolrwParams = NULL;
   char *fileConfig = NULL;
   char *filePrefilterConfig = NULL;
   char *genNewMapping = NULL;
   bool verifyDecoderIn = false;
   bool verifyDecoder = false;
   bool verifyEtob = false;
   bool verifyStrategy = false;
   bool enableSender = false;
   bool enableSenderPipe = false;
   bool realMulticast = false;
   unsigned char pcap_port = 0;
   int enableOsOrders = 0;
   bool book_bypass = false;
   bool recovery_inhibit = false;
   bool decode_packets = false;
   bool genTestbenchData = false;
   Decoder::DummyEopMode dummy_mode = Decoder::DummyEopMode::DISABLED;
   unsigned int strategyPrefilterSetup = 0;
   bool enableStrategyPrefilter = false;
   bool verifyBookChannel = false;
 
     std::thread event_thread, sw_upd_thread;
   // Filenames of generated testbench data files
   const std::string fileTBDirectory{"."};
   const std::string fileTBDecodedMsgs{fileTBDirectory + "/decoded_msgs.dat"};
   const std::string fileTBInfo{fileTBDirectory + "/info.dat"};
   const std::string fileTBCurrentTobs{fileTBDirectory + "/curr_tobs.dat"};
   const std::string fileTBPreviousTobs{fileTBDirectory + "/prev_tobs.dat"};
   const std::string fileTBPersymbolroParams{fileTBDirectory + "/persymbolro_params.dat"};
   const std::string fileTBPersymbolrwParams{fileTBDirectory + "/persymbolrw_params.dat"};
   const std::string fileTBGlobalParams{fileTBDirectory + "/global_params.dat"};
   const std::string fileTBPerGroupParams{fileTBDirectory + "/pergroup_params.dat"};
   const std::string fileTBOrderOutputs{fileTBDirectory + "/order_outputs.dat"};

   while ((c = getopt (argc, argv, "joOhAdDesBIfp:m:c:g:l:u:r:t:M:v:P:E:a:x:zw")) != -1) {
      switch(c) {
         case 'h':
            std::cout << "\t-h\t\tDisplay this help and exit" << std::endl;
            std::cout << "\t-p <file>\tLoad PCAP trace file." << std::endl;
            std::cout << "\t-P <port 0-7>\tTo which port replay the pcap (default 0)." << std::endl;
            std::cout << "\t-m <file>\tLoad mapping file" << std::endl;
            std::cout << "\t-c <file>\tLoad Magmio API configuration from file" << std::endl;
            std::cout << "\t-g <file>\tLoad global parameters from file" << std::endl;
            std::cout << "\t-l <file>\tLoad persymbolro parameters from file" << std::endl;
            std::cout << "\t-u <file>\tLoad pergroup parameters from file" << std::endl;
            std::cout << "\t-r <file>\tLoad persymbolrw parameters from file" << std::endl;
            std::cout << "\t-t <file>\tLoad strategy prefilter configuration from file" << std::endl;
            std::cout << "\t-M <file>\tGenerate mapping file to \"<file>.txt\" and info file to \"<file>.info\" Default <file> name is \"./mapping\"" << std::endl;
            std::cout << "\t-f\t\tGenerate dataset files for hls strategy testbench (.dat)" << std::endl;
            std::cout << "\t-A\t\tPrint Decoder input" << std::endl;
            std::cout << "\t-d\t\tPrint Decoder output" << std::endl;
            std::cout << "\t-e\t\tPrint Enhanced Top of Book output" << std::endl;
            std::cout << "\t-s\t\tPrint Strategy Engine output" << std::endl;
            std::cout << "\t-v <num>\tSet verbosity level 0 - 4 (QUIET, ERROR, WARNING, INFO, DEBUG)" << std::endl;
            std::cout << "\t-o\t\tEnable order sender" << std::endl;
            std::cout << "\t-O\t\tEnable order pipe (command thread, run with -o)" << std::endl;
            std::cout << "\t-E <num>\tHow many order can be generated upon start of model." << std::endl;
            std::cout << "\t-j\t\tUse real multicast data" << std::endl;
            std::cout << "\t-B\t\tEnable book bypass" << std::endl;
            std::cout << "\t-I\t\tInhibit Feed Recovery function" << std::endl;
            std::cout << "\t-D\t\tDecode packets and print fields from template" << std::endl;
            std::cout << "\t-a\t\tSet Dummy EOP Mode to one of [(0) 1 2] meaning [(Disabled) OnExport Always]" << std::endl;
            std::cout << "\t-x <num>\tUse a strategy prefilter setup" << std::endl;
            std::cout << "\t-z\t\tEnable strategy prefilter" << std::endl;
            std::cout << "\t-w\t\tPrint book channel output" << std::endl;

            return 0;
            break;
         case 'p':
            filePcap = optarg;
            break;
         case 'P':
            pcap_port = atoi(optarg);
            break;
         case 'E':
            enableOsOrders = atoi(optarg);
            break;
         case 'm':
            fileMapping = optarg;
            break;
         case 'g':
            fileGlobalParams = optarg;
            break;
         case 'l':
            filePersymbolroParams = optarg;
            break;
         case 'u':
            filePerGroupParams = optarg;
            break;
         case 'r':
            filePersymbolrwParams = optarg;
            break;
         case 't':
            filePrefilterConfig = optarg;
            break;
         case 'M':
            genNewMapping = optarg;
            break;
         case 'f':
            genTestbenchData = true;
            break;
         case 'A':
            verifyDecoderIn = true;
            break;
         case 'd':
            verifyDecoder = true;
            break;
         case 'e':
            verifyEtob = true;
            break;
         case 's':
            verifyStrategy = true;
            break;
         case 'c':
            fileConfig = optarg;
            break;
         case 'v':
            verbosity = Magmio::Log::tVerbosityLevel(atoi(optarg));
            break;
         case 'o':
            enableSender = true;
            break;
         case 'O':
            enableSenderPipe = true;
            break;
         case 'j':
            realMulticast = true;
            break;
         case 'B':
            book_bypass = true;
            break;
         case 'I':
            recovery_inhibit = true;
            break;
         case 'D':
            decode_packets = true;
            break;
         case 'a':
            dummy_mode = (Decoder::DummyEopMode) atoi(optarg);
            break;
         case 'x':
            strategyPrefilterSetup = atoi(optarg);
            break;
         case 'z':
            enableStrategyPrefilter = true;
            break;
         case 'w':
            verifyBookChannel = true;
            break;
         default:
            cerr << "ERROR: You have error(s) in your argument list!" << endl;
            break;
      }
   }


    if (fileMapping==NULL) {
        std::cerr << "ERROR: missing mapping file" << std::endl;
        return -1;
    }

    if (fileConfig==NULL) {
        std::cerr << "ERROR: missing Magmio API configuration file" << std::endl;
        return -1;
    }

    if ((filePcap==NULL && !realMulticast)  && !enableSender) {
        std::cerr << "ERROR: missing PCAP trace file" << std::endl;
        return -1;
    }


    // create Magmio API in DEBUG (software simulation) mode
    api_sw = new Magmio::MagmioApi(0, fileConfig, Magmio::Settings::TApiExtras(Magmio::Settings::MODEL | Magmio::Settings::NO_CONNECTION));

    //set verbosity level of API, both screen and file
    api_sw->log.setPrintVerbosity(verbosity);
    api_sw->log.createFile(verbosity, "model_log.txt");

    std::cout << "Initializing model in first iteration..." << std::endl;
   
    // create instance of simulation model, pass UNITIALIZED API!
    MagmioModel::TModelConf conf;
    conf.statsEnabled = false;
    conf.strategyOutputs = ORDER_SESSIONS;
    conf.apiMapping = fileMapping;
    conf.apiGlobalParams = GLOBAL_PARAMS_COUNT;
    conf.apiPersymbolroParams = PERSYMBOLRO_PARAMS_COUNT;
    conf.apiPerGroupParams = PERGROUP_PARAMS_COUNT;
    conf.apiPersymbolrwParams = PERSYMBOLRW_PARAMS_COUNT;
    conf.apiPretradeParams = PRETRADE_PARAMS_COUNT;
    conf.apiCurrTobLevels = BOOK_LEVELS;
    conf.apiPrevTobLevels = BOOK_LEVELS;
    conf.bypass_enabled = book_bypass;
    conf.pltLevels = 24;
    conf.recovery_inhibit = recovery_inhibit;
    conf.decodePackets = decode_packets;
    conf.dummyMode = dummy_mode;
    conf.stratPrefilterMsgTypeWidth = 12;
    conf.stratPrefilterEnabled = enableStrategyPrefilter;
    conf.etobChannelEnabled = verifyEtob;
    conf.bookChannelEnabled = verifyBookChannel;
    conf.bookChannelPrevExportedLevels = BOOK_LEVELS;
    conf.bookChannelCurrExportedLevels = BOOK_LEVELS;
    conf.uncrossPrevBook = false;
    conf.uncrossCurrBook = false;

    model = new MagmioModel(api_sw);

    // initialize the model and SW API
    if (model->init(conf)!= 0) {
        cerr << "ERROR: Error initializing model!" << endl;
        freeMem();
        return -1;
    }

    // load global params
    if (fileGlobalParams!=NULL) {
        if (api_sw->params.global.loadFromFile(fileGlobalParams)!=0) std::cerr << "ERROR: while loading global parameters" << std::endl;
    }

    // load persymbolro params
    if (filePersymbolroParams!=NULL) {
        if (api_sw->params.persymbolro.loadFromFile(filePersymbolroParams)!=0) std::cerr << "ERROR: while loading persymbolro parameters" << std::endl;
    }

    // load pergroup params
    if (filePerGroupParams!=NULL) {
        if (api_sw->params.pergroup.loadFromFile(filePerGroupParams)!=0) std::cerr << "ERROR: while loading pergroup parameters" << std::endl;
    }

    // load persymbolrw params
    if (filePersymbolrwParams!=NULL) {
        if (api_sw->params.persymbolrw.loadFromFile(filePersymbolrwParams)!=0) std::cerr << "ERROR: while loading persymbolrw parameters" << std::endl;
    }

    ///////////////////////////////////////
        

    
    //////////////////////////
    // api_sw->initTupleMemory("/storage/personal/dohnal/tuples.txt");
    // enable testbench data generation
    if (genTestbenchData) {
        tbFileStreams = new TDatasetStreams;
        bool tb_files_open = false;

        // Check if TB dir exist, if not create new one
        if ((mkdir(fileTBDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) && (errno != EEXIST)){
             std::cerr << "ERROR: Could not create testbench directory " << fileTBDirectory << std::endl;
        } else {
            // Open files and remove their previous content
            tbFileStreams->previousTobs.open(fileTBPreviousTobs, std::ofstream::out | std::ofstream::trunc);
            tbFileStreams->currentTobs.open(fileTBCurrentTobs, std::ofstream::out | std::ofstream::trunc);
            tbFileStreams->decodedMsgs.open(fileTBDecodedMsgs, std::ofstream::out | std::ofstream::trunc);
            tbFileStreams->info.open(fileTBInfo, std::ofstream::out | std::ofstream::trunc);
            tbFileStreams->globalParams.open(fileTBGlobalParams, std::ofstream::out | std::ofstream::trunc);
            tbFileStreams->pergroupParams.open(fileTBPerGroupParams, std::ofstream::out | std::ofstream::trunc);
            tbFileStreams->persymbolroParams.open(fileTBPersymbolroParams, std::ofstream::out | std::ofstream::trunc);
            tbFileStreams->persymbolrwParams.open(fileTBPersymbolrwParams, std::ofstream::out | std::ofstream::trunc);
            tbFileStreams->orderOutputs.open(fileTBOrderOutputs, std::ofstream::out | std::ofstream::trunc);

            // Only if all files were successfuly opened, data will be generated (all data for a specific sample must be synchronized)
            tb_files_open = tbFileStreams->previousTobs.is_open() && \
                            tbFileStreams->currentTobs.is_open() && \
                            tbFileStreams->decodedMsgs.is_open() && \
                            tbFileStreams->info.is_open() && \
                            tbFileStreams->globalParams.is_open() && \
                            tbFileStreams->pergroupParams.is_open() && \
                            tbFileStreams->persymbolroParams.is_open() && \
                            tbFileStreams->persymbolrwParams.is_open() && \
                            tbFileStreams->orderOutputs.is_open();
        }

        if (tb_files_open) {
            model->recordStrategyInouts(tbFileStreams);
            std::cout << "INFO: Strategy input/output recording enabled." << std::endl;
        } else {
            std::cerr << "ERROR: Strategy input/output recording failed." << std::endl;
        }
    }

    // subscribe data stream from Arbiter.
    if (verifyDecoderIn) {
        api_sw->stream.decoder_input.subscribe();
    }

    // subscribe data stream from Decoder.
    if (verifyDecoder or genNewMapping != NULL) {
        api_sw->stream.decoder.subscribe();
    }

    // subscribe data stream from Enhanced Top of Book
    else if (verifyEtob) {
        api_sw->stream.etob.subscribe();
    }

    // subscribe data stream from book channel
    else if (verifyBookChannel) {
        api_sw->stream.book.subscribe();
    }

    // subscribe data stream from Strategy engine
    else if (verifyStrategy) {
        api_sw->stream.strategy.subscribe();
    }

    // Load strategy prefilter configuration from file
    if (filePrefilterConfig) {
        if (api_sw->strategyPrefilter.loadFromFile(filePrefilterConfig) != Magmio::SUCCESS) {
            std::cerr << "ERROR: Strategy Prefilter: loading configuration from file failed!" << std::endl;
            return -1;
        }
    }

    // Apply chosen strategy prefilter setup
    if (strategyPrefilterSetup) {
        if (setStrategyPrefilter(strategyPrefilterSetup) != Magmio::SUCCESS) {
            goto stopExit;
        } else {
            std::cout << "INFO: Strategy prefilter setup has finished." << std::endl;
        }
    }

    /**********************************************************************/
    cout << "Running Magmio model..." << endl;
       uint64_t swUpdateTestSID = 406930;
          std::cout << " Setting sid from main to triggerstretegy..."<< swUpdateTestSID<< std::endl;
          std::vector<uint8_t> dec_msg = {99,255,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40};
          Magmio::TError error;
         if ((error = api_sw->triggerStrategy(swUpdateTestSID, dec_msg)) != Magmio::SUCCESS) 
         {std::cerr << "ERROR (Sw Trigger): Trigger strategy failed - " << Magmio::Log::getErrorMessage(error) << std::endl;
                return error;}
               
          
            else
            {
              std::cout<<"It WORKS"<<std::endl;
            }

   
  

    if (enableSender) {
       //Connect order sender to market using function from orders sender example file
       if (!startSender(enableSenderPipe))
          goto stopExit;
       if (enableOsOrders)
         api_sw->order.enable(enableOsOrders);
    }

    if (realMulticast) {
       if (!model->startMulticast())
         goto stopExit;
    }
    else if (filePcap) {
       // This operation should be performed after subscibing streams.
       // If stream is not subscribed, output data are dropped.
       model->replayPcap(filePcap, pcap_port);
    }

    /*********************************************************************/

    if (verifyDecoder or genNewMapping != NULL) {

        // containers for generating new mapping file
        std::set<uint64_t> sid_set; // only to search if already present
        std::vector<uint64_t> sid_vect; // to keep unordered sids as they come in packets
        std::vector<uint64_t> pack_vect; // to keep packet number from which we got the SID

        //read messages from Decoder stream in infinite loop
        while(!stop) {
            if (model->eof()) {
                if (api_sw->stream.decoder.getFifoSize() == 0)
                  break; // end of file and no messages in fifo
            }

            if (api_sw->stream.decoder.getFifoSize() == 0)
              continue;

            //read message
            Magmio::Decoder::Message msg;
            Magmio::TError err;
            if ((err = api_sw->stream.decoder.getNextMessage(msg)) != 0) {
                if (!stop) std::cerr << "ERROR: " << Magmio::Log::getErrorMessage(err) << std::endl;
                break;
            }

            if (verifyDecoder){
              //print message
              std::cout << msg << std::endl;
            }

            if (genNewMapping != NULL){
              if (sid_set.end() == sid_set.find(msg.decoder.sid)){ // if not present
                sid_set.insert(msg.decoder.sid);
                sid_vect.push_back(msg.decoder.sid);
                pack_vect.push_back(msg.info.sendingTime);
              }
            }
        }
        // stream should be always unsubscribed at the end
          
       
        api_sw->stream.decoder.unsubscribe();

        if (genNewMapping != NULL){
          std::string mapTxt   = genNewMapping;
          mapTxt += ".txt";
          std::string mapInfo  = genNewMapping;
          mapInfo += ".info";

          const char *CmapTxt  = mapTxt.c_str();
          const char *CmapInfo = mapInfo.c_str();
          ofstream FmapTxt (CmapTxt);
          ofstream FmapInfo (CmapInfo);

          if (FmapTxt.is_open() and FmapInfo.is_open()){
            int size = sid_set.size();
            for (int i = 0; i < size; ++i){

              FmapTxt << sid_vect[i] << std::endl;
              FmapInfo << "# " << pack_vect[i] << "\n" << sid_vect[i] << std::endl;
            }
            FmapTxt.close();
            FmapInfo.close();
            }else{
              std::cerr << "ERROR: Unable to open one or both files to output new mapping!" << std::endl;
              return -1;
            }
        }
    }

    else if (verifyEtob) {

        //read messages from Enhanced top of book stream in infinite loop
        while(!stop) {
            if (model->eof()) {
              if (api_sw->stream.etob.getFifoSize() == 0)
                break; // end of file and no messages in fifo
            }

            if (api_sw->stream.etob.getFifoSize() == 0)
              continue;

            //read message
            Magmio::Etob::Message msg;
            Magmio::TError err;
            if ((err = api_sw->stream.etob.getNextMessage(msg)) != 0) {
                if (!stop) std::cerr << "ERROR: " << Magmio::Log::getErrorMessage(err) << std::endl;
                break;
            }

            //print message
            std::cout << msg << std::endl;
        }
        // stream should be always unsubscribed at the end
        api_sw->stream.etob.unsubscribe();
    }

    else if (verifyBookChannel) {
        //read messages from Book stream in infinite loop
        while(!stop) {
            if (model->eof()) {
              if (api_sw->stream.book.getFifoSize() == 0)
                break; // end of file and no messages in fifo
            }

            if (api_sw->stream.book.getFifoSize() == 0)
              continue;

            //read message
            Magmio::BookChannel::ExportMessage msg;
            Magmio::TError err;
            if ((err = api_sw->stream.book.getNextMessage(msg)) != 0) {
                if (!stop) std::cerr << "ERROR: " << Magmio::Log::getErrorMessage(err) << std::endl;
                break;
            }

            //print message
            std::cout << msg << std::endl;
        }
        // stream should be always unsubscribed at the end
        api_sw->stream.book.unsubscribe();
    }

    else if (verifyStrategy) {
        //read messages from Strategy stream in infinite loop
        while(!stop) {
            if (model->eof()) {
              if (api_sw->stream.strategy.getFifoSize() == 0)
                break; // end of file and no messages in fifo
            }

            if (api_sw->stream.strategy.getFifoSize() == 0)
              continue;

            //read message
            Magmio::Strategy::Message msg;
            Magmio::TError err;
            if ((err = api_sw->stream.strategy.getNextMessage(msg)) != 0) {
                if (!stop) std::cerr << "ERROR: " << Magmio::Log::getErrorMessage(err) << std::endl;
                break;
            }
            //print message
            std::cout << msg << std::endl;
        }
        // stream should be always unsubscribed at the end
        api_sw->stream.strategy.unsubscribe();
    } else if (verifyDecoderIn) {
        //read messages from Arbiter stream in infinite loop
        while(!stop) {
            if (model->eof()) {
              if (api_sw->stream.decoder_input.getFifoSize() == 0)
                break; // end of file and no messages in fifo
            }

            if (api_sw->stream.decoder_input.getFifoSize() == 0)
              continue;

            //read message
            Magmio::DecoderIn::Message msg;
            Magmio::TError err;
            if ((err = api_sw->stream.decoder_input.getNextMessage(msg)) != 0) {
                if (!stop) std::cerr << "ERROR: " << Magmio::Log::getErrorMessage(err) << std::endl;
                break;
            }
            //print message
            std::cout << msg << std::endl;
        }
        // stream should be always unsubscribed at the end
        api_sw->stream.decoder_input.unsubscribe();
    }


    while (!stop) {
      if (model->eof())
        break;
    }

    //following is used to pause model until CTR+C is pressed when order sender is enabled
    if (enableSender) {
      while (!stop) {
         sleep(1);
      }
     
      std::cout << "Stopping model..." << std::endl;
      
    }

stopExit:
    if (enableSender) {
      //call stop from order example file
      stopSender();
    }

    //disable real multicast if any
    model->stopMulticast();

    model->verifyStatistics();

    //close testbench file streams
    if (tbFileStreams) {
        tbFileStreams->previousTobs.close();
        tbFileStreams->currentTobs.close();
        tbFileStreams->decodedMsgs.close();
        tbFileStreams->info.close();
        tbFileStreams->globalParams.close();
        tbFileStreams->pergroupParams.close();
        tbFileStreams->persymbolroParams.close();
        tbFileStreams->persymbolrwParams.close();
        tbFileStreams->orderOutputs.close();
    }

    //dont forget do deallocate objects in proper order!
   
   
     
    freeMem();

    return 0;
}
