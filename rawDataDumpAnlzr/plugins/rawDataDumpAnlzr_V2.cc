// -*- C++ -*-
// use cmsRun rawDataDump_V2_cfg.py fedId=1601 maxEvents=-1 to run
// Package:    rawDataProducer_repacked_gen/rawDataDumpAnlzr
// Class:      rawDataDumpAnlzr
//
/**\class rawDataDumpAnlzr rawDataDumpAnlzr.cc rawDataProducer_repacked_gen/rawDataDumpAnlzr/plugins/rawDataDumpAnlzr.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Rohit Kaundal
//         Created:  Tue, 15 Sep 2026 05:32:04 GMT
//
//

// system include files
#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>

// user include files

#include "DataFormats/FEDRawData/interface/RawDataBuffer.h"
#include "DataFormats/FEDRawData/interface/FEDHeader.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDTrailer.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1D.h"
#include "TH2D.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

//using reco::TrackCollection;

using namespace edm;

namespace {
  template <typename T>
  std::set<T> make_set(std::vector<T> const& v) {
    std::set<T> s;
    for (auto const& e : v)
      s.insert(e);
    return s;
  }

  template <typename T>
  std::set<T> make_set(std::vector<T>&& v) {
    std::set<T> s;
    for (auto& e : v)
      s.insert(std::move(e));
    return s;
  }
}

class rawDataDumpAnlzr_V2 : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit rawDataDumpAnlzr_V2(const edm::ParameterSet&);
  ~rawDataDumpAnlzr_V2() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // ----------member data ---------------------------

const std::set<int> feds_;
//const edm::EDGetTokenT<FEDRawDataCollection> phase1_token_;
const edm::EDGetTokenT<RawDataBuffer> phase2_token_;
const edm::EDGetTokenT<RawDataBuffer> phase3_token_;
const bool dumpPayload_, usePhase2_;  

TH1D *CRC_diff;
TH1D *EvtNum;
TH1D *ECOND_id;
TH2D *ECOND_v_CRC_diff;
TH1D *h_payloadLength_org;
TH1D *h_payloadLength_rpc;

size_t evtCtr = 0;
int evtctr = 0;

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  edm::ESGetToken<SetupData, SetupRecord> setupToken_;
#endif
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
rawDataDumpAnlzr_V2::rawDataDumpAnlzr_V2(const edm::ParameterSet& iConfig)
    : feds_(make_set(iConfig.getUntrackedParameter<std::vector<int>>("feds"))),
      //phase1_token_(consumes<FEDRawDataCollection>(iConfig.getUntrackedParameter<edm::InputTag>("label"))),
      phase2_token_(consumes<RawDataBuffer>(iConfig.getUntrackedParameter<edm::InputTag>("label1"))),
      phase3_token_(consumes<RawDataBuffer>(iConfig.getUntrackedParameter<edm::InputTag>("label2"))),
      dumpPayload_(iConfig.getUntrackedParameter<bool>("dumpPayload")),
      usePhase2_(iConfig.getUntrackedParameter<bool>("usePhase2")) {

  edm::Service<TFileService> fs;
  CRC_diff = fs->make<TH1D>("CRC_diff", "CRC difference;original_CRC - repacked_CRC;Counts", 2, 0, 2);
  EvtNum = fs->make<TH1D>("EvtNum", "Events having different CRC;Event number;Counts", 10000, 0, 10000);
  ECOND_id = fs->make<TH1D>("ECOND_ID", "ECOND ID having different CRC;ECOND ID;Counts", 12, 0, 12);
  ECOND_v_CRC_diff = fs->make<TH2D>("ECOND_vs_CRCdiff", "ECOND ID vs CRC difference;ECOND ID;CRC_diff", 12, 0, 12, 2, 0, 2);
  h_payloadLength_org = fs->make<TH1D>("PayloadLength_Org", "PayloadLength Org;Payload length;Counts", 250, 0, 250);
  h_payloadLength_rpc = fs->make<TH1D>("PayloadLength_Rpc", "PayloadLength Rpc;Payload length;Counts", 250, 0, 250);

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  setupDataToken_ = esConsumes<SetupData, SetupRecord>();
#endif
  //now do what ever initialization is needed
}

rawDataDumpAnlzr_V2::~rawDataDumpAnlzr_V2() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called for each event  ------------
void rawDataDumpAnlzr_V2::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  using namespace std;

  int eventNum = iEvent.id().event();

  Handle<RawDataBuffer> rawdata2;
  Handle<RawDataBuffer> rawdata3;
  iEvent.getByToken(phase2_token_, rawdata2);
  iEvent.getByToken(phase3_token_, rawdata3);

  uint64_t econdHeader, econdHeaderPos, econdHeaderMask, econdPayloadPos, econdPayloadMask, econdHeaderMarker, nextEcondHeaderMarker, econdPayloadLength, econdTrailer, eRxHeaderStat, eRxHeaderStatMask;
  std::vector<uint64_t> originalCRC, repackedCRC;
  std::vector<uint64_t> originalDataWord, repackedDataWord;
  std::vector<uint32_t> badPayloadLength_re, badPayloadLength_org; 
  originalCRC.clear();
  repackedCRC.clear();
  originalDataWord.clear();
  repackedDataWord.clear();
  badPayloadLength_re.clear();
  badPayloadLength_org.clear();

  //oooooooooooOOOOOOOOOOOOOOOOOOOOOO for original raw data OOOOOOOOOOOOOooooooooooooooooooooooooo

  //cout << "Original Raw data" << endl;

  for (const auto& it : rawdata2->map()) {
    auto fedid = it.first;
    if (!feds_.empty() && feds_.find(fedid) == feds_.end())
      continue;

    //auto offset = it.second.first;
    auto size = it.second.second;
    //cout << "FED# " << std::setw(4) << fedid << " " << std::setw(8) << size << " bytes  offset=" << std::setw(8)
    //    << offset;

    const auto& data = rawdata2->fragmentData(fedid);
    const auto* start_fed_data = &(data.data().front());
    FEDHeader header(start_fed_data);
    FEDTrailer trailer(start_fed_data + size - FEDTrailer::length);

    //cout << " L1Id: " << std::setw(8) << header.lvl1ID();
    //cout << " BXId: " << std::setw(4) << header.bxID();
    //cout << '\n';

    if (dumpPayload_) {
      const uint64_t* payload = (uint64_t*)(start_fed_data);
      cout << std::hex << std::setfill('0');
      size_t step1 = 1;
      //for (unsigned int i = 0; i < size / sizeof(uint64_t); i+=step1) {
      //cout << "===============oooooooOOOOOOOOOOOOOOOOO Event number: " << eventNum << " OOOOOOOOOOOOOOOOOoooooooooooooooo===================" << endl;
      for (size_t i = 0; i < size / sizeof(uint64_t); i+=step1) {
        //cout << std::setw(4) << i << "  " << std::setw(8) << payload[i] << " WordSize: " << sizeof(payload[i]) << '\n';
        econdHeader = 0x154; //340;
        econdHeaderPos = 55;
        econdHeaderMask = 0x1ff;
        econdPayloadPos = 46;
        econdPayloadMask = 0x1ff;
        
        econdHeaderMarker = ((payload[i] >> econdHeaderPos) & econdHeaderMask);
        econdPayloadLength = ((payload[i] >> econdPayloadPos) & econdPayloadMask);
        //eRxHeaderStat = ((payload[i+1] >> 58) & 0x3F);

        //nextEcondHeaderMarker = ();

        if(econdHeaderMarker == econdHeader){//} and eRxHeaderStat == 0x38){
          //cout << econdHeaderMarker << endl;
          h_payloadLength_org->Fill(econdPayloadLength);
          //if(econdPayloadLength != 127 and econdPayloadLength != 196){
          //if(eventNum == 1696 or eventNum == 8925){
          //  cout << endl;
          //  cout << "===============oooooooOOOOOOOOOOOOOOOOO Event number: " << std::dec<< eventNum << " OOOOOOOOOOOOOOOOOoooooooooooooooo===================" << endl;
          //  cout << "Original Data Word: " << std::hex << std::setw(16) << payload[i] << "  EcondHeader marker: " << econdHeaderMarker << "  PayloadLength: " << std::dec << econdPayloadLength << endl;
          //}
          //cout << std::dec << econdPayloadLength << endl;
          //if(econdPayloadLength != 127 and econdPayloadLength != 196){
          //  //cout << "PayloadLength: " << std::dec << econdPayloadLength << " EventNum: " << eventNum << endl;
          //  badPayloadLength_org.push_back(econdPayloadLength);
          //}
          if(econdPayloadLength % 2 == 0){
            econdTrailer = payload[i+(econdPayloadLength/2)];
            step1 = econdPayloadLength/2+1;
          }
          else{  
            econdTrailer = payload[i+(econdPayloadLength/2) + 1];
            step1 = (econdPayloadLength/2 + 2);
          }
          originalCRC.push_back(econdTrailer);
            //cout <<  std::hex <<  std::setw(8) << econdTrailer << endl;//" " << std::dec << econdPayloadLength << endl;
          //step1 += econdPayloadLength;
        }
        else step1 = 1;
      }
      cout << std::dec << std::setfill(' ');
    
    const uint64_t* payload1 = (uint64_t*)(start_fed_data);
    for(unsigned int i = 0; i < size / sizeof(uint64_t); i++) {
      originalDataWord.push_back(payload1[i]);
    }
    }

    //if (not trailer.check()) {
    //  cout << "    FED trailer check failed\n";
    //}
    //if (trailer.fragmentLength() * 8 != size) {
    //  cout << "    FED fragment size mismatch: " << trailer.fragmentLength() << " (fragment length) vs " << size / 8
    //      << " (data size) words\n";
    //}
  }


  //oooooooooooOOOOOOOOOOOOOOOOOOOOOO for repacked raw data OOOOOOOOOOOOOooooooooooooooooooooooooo
//cout << "Repacked Raw data" << endl;  

for (const auto& it : rawdata3->map()) {
    auto fedid = it.first;
    if (!feds_.empty() && feds_.find(fedid) == feds_.end())
      continue;

    //auto offset = it.second.first;
    auto size = it.second.second;
    //cout << "FED# " << std::setw(4) << fedid << " " << std::setw(8) << size << " bytes  offset=" << std::setw(8)
    //    << offset;

    const auto& data = rawdata3->fragmentData(fedid);
    const auto* start_fed_data = &(data.data().front());
    FEDHeader header(start_fed_data);
    FEDTrailer trailer(start_fed_data + size - FEDTrailer::length);

    //cout << " L1Id: " << std::setw(8) << header.lvl1ID();
    //cout << " BXId: " << std::setw(4) << header.bxID();
    //cout << '\n';

    if (dumpPayload_) {
      const uint64_t* payload = (uint64_t*)(start_fed_data);
      cout << std::hex << std::setfill('0');
      size_t step2 = 1;
      //for (unsigned int i = 0; i < size / sizeof(uint32_t); i+=step2) {
      for (size_t i = 0; i < size / sizeof(uint64_t); i+=step2) {
        //cout << std::setw(4) << i << "  " << std::setw(8) << payload[i] << " WordSize: " << sizeof(payload[i]) << '\n';
        econdHeader = 0x154;//340;
        econdHeaderPos = 55;
        econdHeaderMask = 0x1ff;
        econdPayloadPos = 46;
        econdPayloadMask = 0x1ff;
        
        econdHeaderMarker = ((payload[i] >> econdHeaderPos) & econdHeaderMask);
        econdPayloadLength = ((payload[i] >> econdPayloadPos) & econdPayloadMask);
        //eRxHeaderStat = ((payload[i+1] >> 58) & 0x3F);

        if(econdHeaderMarker == econdHeader){//} and eRxHeaderStat == 0x38){
          //cout << econdHeaderMarker << endl;
          
          h_payloadLength_rpc->Fill(econdPayloadLength);

          //if(econdPayloadLength != 127 and econdPayloadLength != 196){
          //if(eventNum == 1696 or eventNum == 8925){
          //  cout << "Repacked Data Word: " << std::hex << std::setw(16) << payload[i] << "  EcondHeader marker: " << econdHeaderMarker << "  PayloadLength: " << std::dec << econdPayloadLength << endl;
          //}
          //if(econdPayloadLength != 127 and econdPayloadLength != 196){
          //  //cout << "PayloadLength: " << std::dec << econdPayloadLength << " EventNum: " << eventNum << endl;
          //  badPayloadLength_re.push_back(econdPayloadLength);
          //}
          //cout << std::dec << econdPayloadLength << endl;
          if(econdPayloadLength % 2 == 0){
            econdTrailer = payload[i+(econdPayloadLength/2)];
            step2 = econdPayloadLength/2 + 1;
          }  
          else{
            econdTrailer = payload[i+(econdPayloadLength/2) + 1];
            step2 = (econdPayloadLength/2 + 2);
          }
          repackedCRC.push_back(econdTrailer);
          //cout <<  std::hex <<  std::setw(8) << econdTrailer << endl;//" " << std::dec << econdPayloadLength << endl;
          //step2 += econdPayloadLength;
          
        }
        else step2 = 1;
        //cout << endl;
        //cout << "Step: " << std::dec << step << endl;
      
      }
      cout << std::dec << std::setfill(' ');

    const uint64_t* payload1 = (uint64_t*)(start_fed_data);
    for(unsigned int i = 0; i < size / sizeof(uint64_t); i++) {
      repackedDataWord.push_back(payload1[i]);
    }
    }

    //if (not trailer.check()) {
    //  cout << "    FED trailer check failed\n";
    //}
    //if (trailer.fragmentLength() * 8 != size) {
    //  cout << "    FED fragment size mismatch: " << trailer.fragmentLength() << " (fragment length) vs " << size / 8
    //      << " (data size) words\n";
    //}
  }

  //int badEvents[38] = {347, 926, 1021, 1217, 1231, 1424, 1551, 1664, 1732, 1991, 2095, 2234, 2717, 2971, 3381, 4020, 4420, 4770, 5211, 5848, 5948, 5996, 6008, 6021, 6041, 6117, 6511, 6646, 7607, 7737, 7816, 8264, 8558, 9132, 9329};
  //cout << endl;
  int cntr = 0;
  //if(eventNum == badEvents[evtctr]){
  //if(eventNum == 1696 or eventNum == 8925){
  //  cout << endl;
  //  cout << "===============oooooooOOOOOOOOOOOOOOOOO Event number: " << eventNum << " OOOOOOOOOOOOOOOOOoooooooooooooooo===================" << endl;
  //  for(size_t i = 0; i < originalDataWord.size(); i++){
  //    cout << std::hex << "Orginal: " << std::setw(16) <<  originalDataWord[i] << "   Rapacked: " << repackedDataWord[i] << "   Difference: " << std::setw(16) << originalDataWord[i] - repackedDataWord[i] << endl;
  //    
  //  }
  //  //evtctr++;
  //}
  for(size_t i = 0; i < originalCRC.size(); i++){
    
    //if(originalCRC[i] == repackedCRC[i]){
    //  CRC_diff->Fill(0);
    //  ECOND_v_CRC_diff->Fill(i, 0);
    //}  
    //else{
    //  CRC_diff->Fill(1);
    //  //EvtNum->Fill(eventNum);
    //  ECOND_id->Fill(i);
    //  ECOND_v_CRC_diff->Fill(i, 1);
    //  cntr++;
    //} 

    if(originalCRC[i] - repackedCRC[i] != 0){
      //cout << " OriginalCRC: " << std::hex << std::setw(8) << originalCRC[i] << " RepackedCRC: " << std::setw(8) <<  repackedCRC[i] << endl;
      CRC_diff->Fill(1);
      //EvtNum->Fill(eventNum);
      ECOND_id->Fill(i);
      ECOND_v_CRC_diff->Fill(i, 1);
      cntr++;
    }
    else{
      CRC_diff->Fill(0);
      ECOND_v_CRC_diff->Fill(i, 0);
    }

  
  //cout << std::dec << i  << " OriginalCRC: " << std::hex <<  std::setw(8) << originalCRC[i] << " RepackedCRC: " << std::setw(8) << repackedCRC[i] << " difference: " << std::setw(8) << originalCRC[i] - repackedCRC[i] << endl;
  
  }
  //cout << endl;
  if(cntr != 0){
    EvtNum->Fill(eventNum);
    //cout << eventNum << ", ";
    //cout << " EventNum: " << eventNum << "," //"  OriginalCRC_vec_Size:" << originalCRC.size() << "  RepackedCRC_vec_Size:" << repackedCRC.size() << endl;
  }
  
  
  //std::vector<int> badEvts{926, 1021, 1205, 1217, 1231, 1551, 1664, 1732, 1991, 2095, 2234, 2971, 3381, 3832, 4020, 4244, 4420, 4770, 5211, 5848, 5948, 5996, 6008, 6021, 6041, 6117, 6511, 6646, 7607, 7737, 7816, 8222, 8264, 8558, 8884, 9132, 9262, 9329};
  //if(eventNum == badEvts[evtCtr] and evtCtr < badEvts.size()){
  //  cout << "=========================== " << eventNum << " =================================" << endl;
  //  cout << "originalDataWord_vec_Size: " << originalDataWord.size() << " repackedDataWord_vec_Size: " << repackedDataWord.size() << endl;
  //  for(size_t i = 0; i < originalDataWord.size(); i++){
  //    cout << std::hex << "originalWord: " << std::setw(16) << originalDataWord[i] << " repackedWord: " << std::setw(16) << repackedDataWord[i] << "  Difference: " << originalDataWord[i] - repackedDataWord[i] << endl;
  //    if(originalDataWord[i] != repackedDataWord[i]){
  //      //cout << std::hex << "originalWord: " << std::setw(16) << originalDataWord[i] << " repackedWord: " << std::setw(16) << repackedDataWord[i] << endl;
  //    }
  //  }
  //evtCtr++;
  //}

  //for(size_t i = 0; i < badPayloadLength_org.size(); i++){
  //  cout << badPayloadLength_org[i] - badPayloadLength_re[i] << endl;
  //}

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  // if the SetupData is always needed
  auto setup = iSetup.getData(setupToken_);
  // if need the ESHandle to check if the SetupData was there or not
  auto pSetup = iSetup.getHandle(setupToken_);
#endif
}

// ------------ method called once each job just before starting event loop  ------------
void rawDataDumpAnlzr_V2::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void rawDataDumpAnlzr_V2::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void rawDataDumpAnlzr_V2::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
      desc.addUntracked<std::vector<int>>("feds")->setComment("List of FED IDs of interest");
      desc.addUntracked<edm::InputTag>("label1")->setComment("Label for the raw data collection");
      desc.addUntracked<edm::InputTag>("label2")->setComment("Label for the raw data collection");
      desc.addUntracked<bool>("dumpPayload")->setComment("Enable payload dump");
      desc.addUntracked<bool>("usePhase2")
          ->setComment("Use Phase 2 RawDataBuffer instead of Phase 1's FEDRawDataCollection");
      descriptions.add("dumpFEDdata_V2", desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(rawDataDumpAnlzr_V2);
