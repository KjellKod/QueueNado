/*
 * File:   KrakenBattle.h
 * Author: Kjell Hedstrom
 *
 * Created on November 24, 2015
 */

 #include "Kraken.h"
#include <vector>
#include <string>


#pragma once

namespace KrakenBattle {
   /**
   * Collection of static functions for forwarding data from Kraken to Harpoon receiver
   * (the www-UI)
   * Below the following format is used
   * uuid:       734a83c7-9435-4605-b1f9-4724c81faf21  (random uuid)
   * empty_uuid: 00000000-0000-0000-0000-000000000000
   * <DATA>, <END>, <ERROR>: Just like it seems, strings that look exactly like that
   * data: pcap chunks, raw bytes
   * error: error message, a string
   *
   * The data format is as follows
   * uuid<DATA>data: continously sending pcap chunks
   * uuid<ERROR>error: stop sending for one UUID, reason for error is given
   * uuid<DONE>: done with sending for one UUID, completed without error
   * empty_uuid<END>: done with sending for ALL UUIDs.
   */
   enum class SendType {Data, Done, Error, End};
   enum class ProgressType{Continue, Stop};

   std::vector<uint8_t>  MergeData(const std::string& uuid, const KrakenBattle::SendType& type, const Kraken::Chunks& optional_data, const std::string& optional_error_msg);
   KrakenBattle::ProgressType  SendChunks(Kraken* kraken, const std::string& uuid, const Kraken::Chunks& chunk, const KrakenBattle::SendType& type, const std::string& error);
   KrakenBattle::ProgressType  ForwardPCapChunksToClient(Kraken* kraken, const std::string& uuid,const Kraken::Chunks& chunk, const KrakenBattle::SendType& sendState, const std::string& error);
   std::string SendTypeToString(const KrakenBattle::SendType& type);
} // KrakenBattle