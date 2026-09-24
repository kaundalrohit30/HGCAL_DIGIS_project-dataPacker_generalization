#include "SimCalorimetry/HGCalSimAlgos/interface/HGCalRawDataPackingTools.h"
#include "DataFormats/HGCalDigi/interface/HGCalRawDataDefinitions.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"
#include <iostream>
#include <bitset>

std::vector<uint32_t> hgcal::econd::produceERxData(const ERxChannelEnable& channel_enable,
                                                   const ERxData& erx,
                                                   bool passZS,
                                                   bool passZSm1,
                                                   bool hasToA,
                                                   bool char_mode,
                                                   bool passThrough) {
  auto format_word = [&erx, &passZS, &passZSm1, &hasToA, &char_mode, &passThrough](size_t i) -> std::pair<uint32_t, uint8_t> {
    //std::cout << "format_word:  i: " << i << "  erx.tctp.size(): " << erx.tctp.size() << std::endl;
    if (i >= erx.tctp.size())
      throw cms::Exception("HGCalEmulator")
          << "Not enough channels in eRx data payload: " << i << " >= " << erx.tctp.size() << ".";
    if (char_mode)  // characterisation mode is easy
      return std::make_pair(((uint8_t)erx.tctp.at(i) & 0x3) << 30 | (erx.adc.at(i) & 0x3ff) << 20 |
                                (erx.tot.at(i) & 0x3ff) << 10 | (erx.toa.at(i) & 0x3ff),
                            32);
    else if(passThrough){
      if(erx.tctp.at(i) == 0x3 or erx.tctp.at(i) == 0x2){
        return std::make_pair(((uint8_t)erx.tctp.at(i) & 0x3) << 30 | (erx.adcm.at(i) & 0x3ff) << 20 |
                                (erx.tot.at(i) & 0x3ff) << 10 | (erx.toa.at(i) & 0x3ff),
                                32);
      }
      else{
        return std::make_pair(((uint8_t)erx.tctp.at(i) & 0x3) << 30 | (erx.adcm.at(i) & 0x3ff) << 20 |
                                (erx.adc.at(i) & 0x3ff) << 10 | (erx.toa.at(i) & 0x3ff),
                                32);
      }
    }
    else{
      switch (erx.tctp.at(i)) {
        case ToTStatus::ZeroSuppressed: {
          //std::cout << "oooooooooOOOOOOOOOOOOoooooooooooo ToTStatus::ZeroSuppressed: oooooooooOOOOOOOOOOOOoooooooooooo" << std::endl; 

          if (!passZS)
            throw cms::Exception("HGCalEmulator") << "ToT status is ZeroSuppressed, but event frame does not pass ZS.";
          if (passZSm1) { 
            if(erx.toa.at(i)!=0)
              return std::make_pair((0x1 << 30) | ((erx.adcm.at(i) & 0x3ff) << 20) | ((erx.adc.at(i) & 0x3ff) << 10) |
                                        (erx.toa.at(i) & 0x3ff),
                                    32);
            else if (hasToA)
              return std::make_pair((0x1 << 30) | ((erx.adcm.at(i) & 0x3ff) << 20) | ((erx.adc.at(i) & 0x3ff) << 10) |
                                        (erx.toa.at(i) & 0x3ff),
                                    32);
            else
              return std::make_pair(((erx.adcm.at(i) & 0x3ff) << 10) | (erx.adc.at(i) & 0x3ff), 24);
              //return std::make_pair((0x0 << 20) | ((erx.adcm.at(i) & 0x3ff) << 10) | (erx.adc.at(i) & 0x3ff), 24);

          }
          // at this point, does not have any BX-1 ZS info
          if (hasToA)
            return std::make_pair((0x3 << 20) | ((erx.adc.at(i) & 0x3ff) << 10) | (erx.toa.at(i) & 0x3ff), 24);
          //return std::make_pair((0x1 << 10) | (erx.adc.at(i) & 0x3ff), 16);
          return std::make_pair((0x1 << 12) | ((erx.adc.at(i) & 0x3ff) << 2), 16);
        }
        case ToTStatus::noZeroSuppressed_TOASuppressed:
          //std::cout << "oooooooooOOOOOOOOOOOOoooooooooooo ToTStatus::noZeroSuppressed_TOASuppressed: oooooooooOOOOOOOOOOOOoooooooooooo" << std::endl;
          return std::make_pair((0x2 << 20) | ((erx.adcm.at(i) & 0x3ff) << 10) | (erx.adc.at(i) & 0x3ff), 24);
        case ToTStatus::invalid:
          //std::cout << "oooooooooOOOOOOOOOOOOoooooooooooo ToTStatus::invalid: oooooooooOOOOOOOOOOOOoooooooooooo" << std::endl;
          return std::make_pair(
              (0x2 << 30) | ((erx.adcm.at(i) & 0x3ff) << 20) | ((erx.adc.at(i) & 0x3ff) << 10) | (erx.toa.at(i) & 0x3ff),
              32);
        case ToTStatus::AutomaticFull:
          //std::cout << "oooooooooOOOOOOOOOOOOoooooooooooo ToTStatus::AutomaticFull: oooooooooOOOOOOOOOOOOoooooooooooo" << std::endl;
          return std::make_pair(
              (0x3 << 30) | ((erx.adcm.at(i) & 0x3ff) << 20) | ((erx.tot.at(i) & 0x3ff) << 10) | (erx.toa.at(i) & 0x3ff),
              32);
        default:
          throw cms::Exception("HGCalEmulator")
              << "Invalid ToT status retrieved for channel " << i << ": " << (int)erx.tctp.at(i) << ".";
      }
    }
  };

  std::vector<uint32_t> data{0};
  std::vector<uint32_t>::iterator it_data = data.begin();

  uint8_t msb = 0;
  for (size_t i = 0; i < channel_enable.size(); ++i) {
    if (!channel_enable.at(i))
      continue;
    const auto [word, nbits] = format_word(i);  // retrieve the channel word in its proper formatting

    //std::cout << "msb: " << static_cast<uint32_t>(msb) << std::endl;

    if (msb == 32)//>= 32)  // if we are already at the next word
      it_data = data.insert(data.end(), 0);
    msb %= 32;  // 0 <= msb < 32

    /*if (msb > 0)  // do we have some room for additional information?
      *it_data &= ((1 << msb) - 1);
    if (msb + nbits > 32) {  // spilling onto the next word
      uint8_t nbits_word1 = 32 - msb;
      *it_data |= (word & ((1 << nbits_word1) - 1)) << msb;
      //std::cout << "Original word: " << std::hex << word << "   " << std::bitset<32>(word) << std::dec<< std::endl;
      //std::cout << "joined with previous word: " << std::hex << *it_data << "   " << std::bitset<32>(*it_data) << std::dec<< std::endl;
      it_data = data.insert(data.end(), word >> nbits_word1);
      //std::cout << "spilling to next word: " << std::hex << (word >> nbits_word1) << "   " << std::bitset<32>(word >> nbits_word1) << std::dec<< std::endl;
    }else  // everything fits into one word
      *it_data |= word << msb;*/
    
    if (msb > 0){// and (msb + nbits >= 32)){  // do we have some room for additional information?
      *it_data &= ((1 << 32) - 1);
      //*it_data &= ((1 << msb) - 1);
      //*it_data <<= (32-msb); 
    }//else
     //   *it_data &= ((1 << msb) - 1);

    if (msb + nbits > 32) {  // spilling onto the next word
      uint8_t nbits_word1 = 32 - msb;
      //std::cout << "previous word: " << std::hex << *it_data << "   " << std::bitset<32>(*it_data) << std::dec<< std::endl;
      *it_data |= (word & (((1 << nbits_word1) - 1) << (nbits - nbits_word1))) >> (nbits - nbits_word1);
      //std::cout << "Original word: " << std::hex << word << "   " << std::bitset<32>(word) << "  shifted word: " << ((word & (((1 << nbits_word1) - 1) << (nbits - nbits_word1))) >> (nbits - nbits_word1))  << std::dec<< std::endl;
      //std::cout << "joined with previous word: " << std::hex << *it_data << "   " << std::bitset<32>(*it_data) << std::dec<< std::endl;
      it_data = data.insert(data.end(), ((word << (32 - nbits + nbits_word1))));//>> (32 - nbits + nbits_word1)));
      //std::cout << "spilling to next word: " << std::hex << ((word << (32 - nbits + nbits_word1))) << "   " << std::bitset<32>(((word << (32 - nbits + nbits_word1)))) << std::dec<< std::endl;
    } // everything fits into one word
    else if(msb + nbits == 32){
      *it_data |= word;
      //std::cout << "direct word1: " << std::hex << *it_data << "   " << std::bitset<32>(*it_data) << std::dec<< std::endl;
    }else{
      *it_data |= word << (32-nbits);//msb;
      //std::cout << "direct word2: " << std::hex << *it_data << "   " << std::bitset<32>(*it_data) << std::dec<< std::endl;
    }
      
      msb += nbits;
  }
  return data;
}

//
std::vector<uint32_t> hgcal::econd::eRxSubPacketHeader(uint8_t stat,
                                                       uint8_t hamming,
                                                       bool bitE,
                                                       uint16_t common_mode0,
                                                       uint16_t common_mode1,
                                                       const ERxChannelEnable& channel_enable) {
  uint64_t channels_map64b(0);
  size_t i = 0;
  //std::cout << "channelEnable original: " << channel_enable << std::endl;
  for (const auto& ch : channel_enable){
    channels_map64b |= (uint64_t(ch) << i++);
    //std::cout << ch;
  }
  //std::cout << std::endl;
  //std::cout << std::hex <<  std::bitset<64>(channels_map64b) << "  channelEnable original: " <<  channels_map64b  << std::dec << std::endl;
  return hgcal::econd::eRxSubPacketHeader(stat, hamming, bitE, common_mode0, common_mode1, channels_map64b);
}


//
std::vector<uint32_t> hgcal::econd::eRxSubPacketHeader(
    uint8_t stat, uint8_t hamming, bool bitE, uint16_t common_mode0, uint16_t common_mode1, uint64_t channels_map) {
  std::vector<uint32_t> header = {
      (stat & hgcal::ECOND_FRAME::ERXSTAT_MASK) << hgcal::ECOND_FRAME::ERXSTAT_POS |
      (hamming & hgcal::ECOND_FRAME::ERXHAM_MASK) << hgcal::ECOND_FRAME::ERXHAM_POS |
      (common_mode0 & hgcal::ECOND_FRAME::COMMONMODE0_MASK) << hgcal::ECOND_FRAME::COMMONMODE0_POS |
      (common_mode1 & hgcal::ECOND_FRAME::COMMONMODE1_MASK) << hgcal::ECOND_FRAME::COMMONMODE1_POS};

  //summarize the channel status map
  const uint32_t chmapw0(channels_map & hgcal::ECOND_FRAME::CHMAP0_MASK),
      chmapw1((channels_map >> 32) & hgcal::ECOND_FRAME::CHMAP32_MASK);
    //std::cout << std::hex << "channelMap: " << channels_map << "   chanmap1: " << chmapw1 << std::dec << std::endl;
  //add the channel map
  if (chmapw0 == 0 && chmapw1 == 0) {  // empty channels map (empty eRx)
    header[0] |= (bitE << hgcal::ECOND_FRAME::ERX_E_POS);
    //header[0] |= (bitE << hgcal::ECOND_FRAME::ERXFORMAT_POS);
    header[0] |= (1 << hgcal::ECOND_FRAME::ERXFORMAT_POS);  //raise the F bit (empty eRX)
    //std::cout << "empty eRx header0 " << std::hex << header[0] << std::dec << std::endl;
  } else {
    header[0] |= (chmapw1 & hgcal::ECOND_FRAME::CHMAP32_MASK) << hgcal::ECOND_FRAME::CHMAP32_POS;
    header.push_back((chmapw0 & hgcal::ECOND_FRAME::CHMAP0_MASK) << hgcal::ECOND_FRAME::CHMAP0_POS);
    //std::cout << "eRx header0 " << std::hex << header[0] << std::dec << std::endl;
  }

  return header;
}

static uint8_t computeCRC8bit(const std::vector<uint32_t>& event_header) {
  uint8_t crc = 0x42;  //TODO: implement 8-bit Bluetooth CRC
  return crc;
}

//
std::vector<uint32_t> hgcal::econd::eventPacketHeader(uint16_t header,
                                                      uint16_t payload,
                                                      bool bitP,
                                                      bool bitE,
                                                      uint8_t ht,
                                                      uint8_t ebo,
                                                      bool bitM,
                                                      bool bitT,
                                                      uint8_t ehHam,
                                                      uint16_t bx,
                                                      uint16_t l1a,
                                                      uint8_t orb,
                                                      bool bitS,
                                                      uint8_t rr) {
  std::vector<uint32_t> words(2, 0);

  words[0] = (header & hgcal::ECOND_FRAME::HEADER_MASK) << hgcal::ECOND_FRAME::HEADER_POS |
             (payload & hgcal::ECOND_FRAME::PAYLOAD_MASK) << hgcal::ECOND_FRAME::PAYLOAD_POS |
             bitP << hgcal::ECOND_FRAME::BITP_POS | bitE << hgcal::ECOND_FRAME::BITE_POS |
             (ht & hgcal::ECOND_FRAME::HT_MASK) << hgcal::ECOND_FRAME::HT_POS |
             (ebo & hgcal::ECOND_FRAME::EBO_MASK) << hgcal::ECOND_FRAME::EBO_POS |
             bitM << hgcal::ECOND_FRAME::BITM_POS | bitT << hgcal::ECOND_FRAME::BITT_POS |
             (ehHam & hgcal::ECOND_FRAME::EHHAM_MASK) << hgcal::ECOND_FRAME::EHHAM_POS;

  words[1] = (bx & hgcal::ECOND_FRAME::BX_MASK) << hgcal::ECOND_FRAME::BX_POS |
             (l1a & hgcal::ECOND_FRAME::L1A_MASK) << hgcal::ECOND_FRAME::L1A_POS |
             (orb & hgcal::ECOND_FRAME::ORBIT_MASK) << hgcal::ECOND_FRAME::ORBIT_POS |
             bitS << hgcal::ECOND_FRAME::BITS_POS | (rr & hgcal::ECOND_FRAME::RR_MASK) << hgcal::ECOND_FRAME::RR_POS;

  const auto crc = computeCRC8bit(words);
  words[1] |= (crc & hgcal::ECOND_FRAME::EHCRC_MASK) << hgcal::ECOND_FRAME::EHCRC_POS;

  return words;
}

uint32_t hgcal::econd::buildIdleWord(uint8_t bufStat, uint8_t err, uint8_t rr, uint32_t progPattern) {
  return (progPattern & 0xffffff) << 8 | (rr & 0x3) << 6 | (err & 0x7) << 3 | (bufStat & 0x7) << 0;
}

//
std::vector<uint32_t> hgcal::backend::buildCaptureBlockHeader(uint32_t bunch_crossing,
                                                              uint32_t event_counter,
                                                              uint32_t orbit_counter,
                                                              const std::vector<uint8_t>& econd_statuses) {
  if (econd_statuses.size() > 12)
    throw cms::Exception("HGCalEmulator") << "Invalid size for ECON-D statuses: " << econd_statuses.size() << ".";
  std::vector<uint32_t> header(2, 0);
  header[0] =
      (bunch_crossing & hgcal::BACKEND_FRAME::CAPTUREBLOCK_BC_MASK) << hgcal::BACKEND_FRAME::CAPTUREBLOCK_BC_POS |
      (event_counter & hgcal::BACKEND_FRAME::CAPTUREBLOCK_EC_MASK) << hgcal::BACKEND_FRAME::CAPTUREBLOCK_EC_POS |
      (orbit_counter & hgcal::BACKEND_FRAME::CAPTUREBLOCK_OC_MASK) << hgcal::BACKEND_FRAME::CAPTUREBLOCK_OC_POS |
      (econd_statuses[11] & 0x7) << 1 | ((econd_statuses[10] >> 2) & 0x1);
  for (size_t i = 0; i < 11; i++)
    header[1] |= (econd_statuses[i] & 0x7) << i * 3;
  return header;
}

//
std::vector<uint32_t> hgcal::backend::buildSlinkHeader(
    uint8_t boe, uint8_t v, uint64_t global_event_id, uint32_t content_id, uint32_t fed_id) {
  std::vector<uint32_t> header(4, 0);
  header[0] = (boe & hgcal::BACKEND_FRAME::SLINK_BOE_MASK) << hgcal::BACKEND_FRAME::SLINK_BOE_POS |
              (v & hgcal::BACKEND_FRAME::SLINK_V_MASK) << hgcal::BACKEND_FRAME::SLINK_V_POS |
              ((global_event_id >> 41) & hgcal::BACKEND_FRAME::SLINK_GLOBAL_EVENTID_MSB_MASK)
                  << hgcal::BACKEND_FRAME::SLINK_GLOBAL_EVENTID_MSB_POS;
  header[1] = (global_event_id & hgcal::BACKEND_FRAME::SLINK_GLOBAL_EVENTID_LSB_MASK);
  header[2] = (content_id & hgcal::BACKEND_FRAME::SLINK_CONTENTID_MASK) << hgcal::BACKEND_FRAME::SLINK_CONTENTID_POS;
  header[3] = (fed_id & hgcal::BACKEND_FRAME::SLINK_SOURCEID_MASK) << hgcal::BACKEND_FRAME::SLINK_SOURCEID_POS;

  return header;
}

//
std::vector<uint32_t> hgcal::backend::buildSlinkTrailer(uint8_t eoe,
                                                        uint16_t daqcrc,
                                                        uint32_t event_length,
                                                        uint16_t bxid,
                                                        uint32_t orbit_id,
                                                        uint16_t crc,
                                                        uint16_t status) {
  std::vector<uint32_t> trailer(4, 0);

  trailer[0] = (eoe & hgcal::BACKEND_FRAME::SLINK_EOE_MASK) << hgcal::BACKEND_FRAME::SLINK_EOE_POS |
               (daqcrc & hgcal::BACKEND_FRAME::SLINK_DAQCRC_MASK) << hgcal::BACKEND_FRAME::SLINK_DAQCRC_POS;
  trailer[1] = (event_length & hgcal::BACKEND_FRAME::SLINK_EVLENGTH_MASK) << hgcal::BACKEND_FRAME::SLINK_EVLENGTH_POS |
               (bxid & hgcal::BACKEND_FRAME::SLINK_BXID_MASK) << hgcal::BACKEND_FRAME::SLINK_BXID_POS;
  trailer[2] = (orbit_id & hgcal::BACKEND_FRAME::SLINK_ORBID_MASK) << hgcal::BACKEND_FRAME::SLINK_ORBID_POS;
  trailer[3] = (crc & hgcal::BACKEND_FRAME::SLINK_CRC_MASK) << hgcal::BACKEND_FRAME::SLINK_CRC_POS |
               (status & hgcal::BACKEND_FRAME::SLINK_STATUS_MASK) << hgcal::BACKEND_FRAME::SLINK_STATUS_POS;

  return trailer;
}

//
uint32_t hgcal::backend::buildSlinkContentId(SlinkEmulationFlag e, uint8_t l1a_subtype, uint16_t l1a_fragment_cnt) {
  return 0x0 | (l1a_fragment_cnt & 0xffff) | (l1a_subtype & 0xff) << 16 | (e & 0x3) << 24;
}

uint16_t hgcal::backend::buildSlinkRocketStatus(
    bool fed_crc_err, bool slinkrocket_crc_err, bool source_id_err, bool sync_lost, bool fragment_trunc) {
  return 0x0 | (fed_crc_err & 0x1) << 0 | (slinkrocket_crc_err & 0x1) << 1 | (source_id_err & 0x1) << 2 |
         (sync_lost & 0x1) << 3 | (fragment_trunc & 0x1) << 4;
}
