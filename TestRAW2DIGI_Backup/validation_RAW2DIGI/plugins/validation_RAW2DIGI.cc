// -*- C++ -*-
//
// Package:    TestRAW2DIGI/validation_RAW2DIGI
// Class:      validation_RAW2DIGI
//
/**\class validation_RAW2DIGI validation_RAW2DIGI.cc TestRAW2DIGI/validation_RAW2DIGI/plugins/validation_RAW2DIGI.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Rohit Kaundal
//         Created:  Thu, 17 Sep 2026 06:22:46 GMT
//
//

// system include files
#include <memory>
#include <algorithm> // for std::min
#include <string> // for std::string, std::to_string()

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESWatcher.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/FEDRawData/interface/RawDataBuffer.h"
#include "DataFormats/HGCalDigi/interface/HGCalDigiHost.h"
#include "DataFormats/HGCalDigi/interface/HGCalECONDPacketInfoHost.h"
#include "DataFormats/HGCalDigi/interface/HGCalFEDPacketInfoHost.h"
#include "DataFormats/HGCalDigi/interface/HGCalRawDataDefinitions.h"

#include "CondFormats/DataRecord/interface/HGCalElectronicsMappingRcd.h"
#include "CondFormats/HGCalObjects/interface/HGCalMappingModuleIndexer.h"
#include "CondFormats/HGCalObjects/interface/HGCalMappingCellIndexer.h"
#include "CondFormats/DataRecord/interface/HGCalModuleConfigurationRcd.h"
#include "CondFormats/HGCalObjects/interface/HGCalConfiguration.h"

#include "EventFilter/HGCalRawToDigi/interface/HGCalUnpacker.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1D.h"
#include "TH2D.h"
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

uint16_t compressToT(uint16_t totraw) {
    if (totraw > 0x1ff)
      return (0x200 | (totraw >> 3));
    return (totraw & 0x1ff);
  }

class validation_RAW2DIGI : public edm::one::EDAnalyzer<
                                  edm::one::SharedResources,
                                  edm::one::WatchRuns>  {
public:
  explicit validation_RAW2DIGI(const edm::ParameterSet&);
  ~validation_RAW2DIGI() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  //void beginJob() override;
  void beginRun(edm::Run const&, edm::EventSetup const&) override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  
  void endRun(edm::Run const&, edm::EventSetup const&) override;
  //void endJob() override;

  // ----------member data ---------------------------
  const edm::EDGetTokenT<RawDataBuffer> fedRawTokenOrg_;
  const edm::EDGetTokenT<RawDataBuffer> fedRawTokenRpc_;

  // output tokens
  //const edm::EDPutTokenT<hgcaldigi::HGCalDigiHost> digisToken_;
  //const edm::EDPutTokenT<hgcaldigi::HGCalECONDPacketInfoHost> econdPacketInfoToken_;
  //const edm::EDPutTokenT<hgcaldigi::HGCalFEDPacketInfoHost> fedPacketInfoToken_;
  
  // config tokens and objects
  edm::ESWatcher<HGCalElectronicsMappingRcd> mapWatcher_;
  edm::ESGetToken<HGCalMappingCellIndexer, HGCalElectronicsMappingRcd> cellIndexToken_Org;
  edm::ESGetToken<HGCalMappingModuleIndexer, HGCalElectronicsMappingRcd> moduleIndexToken_Org;
  edm::ESGetToken<HGCalConfiguration, HGCalModuleConfigurationRcd> configToken_Org; 
  HGCalMappingCellIndexer cellIndexer_Org;
  HGCalMappingModuleIndexer moduleIndexer_Org;
  HGCalConfiguration config_Org;
  HGCalUnpacker unpacker_Org;

  edm::ESGetToken<HGCalMappingCellIndexer, HGCalElectronicsMappingRcd> cellIndexToken_Rpc;
  edm::ESGetToken<HGCalMappingModuleIndexer, HGCalElectronicsMappingRcd> moduleIndexToken_Rpc;
  edm::ESGetToken<HGCalConfiguration, HGCalModuleConfigurationRcd> configToken_Rpc; 
  HGCalMappingCellIndexer cellIndexer_Rpc;
  HGCalMappingModuleIndexer moduleIndexer_Rpc;
  HGCalConfiguration config_Rpc;
  HGCalUnpacker unpacker_Rpc;

  std::map<uint32_t, std::vector<uint32_t> > aveadc_map_;
  
  //TH1D* h_adc_diff, h_adcm1_diff, h_tot_diff, h_toa_diff, h_tctp_diff, h_cm_diff, h_event;
  TH1D* h_adc_diff;
  TH1D* h_adcm1_diff;
  TH1D* h_tot_diff;
  TH1D* h_toa_diff;
  TH1D* h_tctp_diff;
  TH1D* h_cm_diff;
  TH1D* h_event;
  TH1D* h_Econd;
  TH1D* h_erx;
  TH1D* h_channel;

  TH2D* h2_TOT;
  TH2D* h2_adc;
  TH2D* h2_adcm1;
  TH2D* h2_toa;

  //TH1D* h_TOT_Rpc;
  //TH1D* h_adc_Rpc;
  //TH1D* h_adcm1_Rpc;
  //TH1D* h_toa_Rpc;

  //TH1D* h_TOT_Org;
  //TH1D* h_adc_Org;
  //TH1D* h_adcm1_Org;
  //TH1D* h_toa_Org;


  TH1D* h_TOT_Org_comp;
  TH1D* h_TOT_Rpc_comp;

  TH1D* h_TOT_Org_9_ECONDid;
  TH1D* h_TOT_Rpc_9_ECONDid;


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
validation_RAW2DIGI::validation_RAW2DIGI(const edm::ParameterSet& iConfig)
    : fedRawTokenOrg_(consumes<RawDataBuffer>(iConfig.getParameter<edm::InputTag>("src1"))),
      fedRawTokenRpc_(consumes<RawDataBuffer>(iConfig.getParameter<edm::InputTag>("src2"))),
      //digisToken_(produces<hgcaldigi::HGCalDigiHost>()),
      //econdPacketInfoToken_(produces<hgcaldigi::HGCalECONDPacketInfoHost>()),
      //fedPacketInfoToken_(produces<hgcaldigi::HGCalFEDPacketInfoHost>()),
      cellIndexToken_Org(esConsumes<edm::Transition::BeginRun>()),
      moduleIndexToken_Org(esConsumes<edm::Transition::BeginRun>()),
      configToken_Org(esConsumes<edm::Transition::BeginRun>()),
      cellIndexToken_Rpc(esConsumes<edm::Transition::BeginRun>()),
      moduleIndexToken_Rpc(esConsumes<edm::Transition::BeginRun>()),
      configToken_Rpc(esConsumes<edm::Transition::BeginRun>()) {

  edm::Service<TFileService> fs;
  h_adc_diff = fs->make<TH1D>("ADC", "ADC difference;ADC_diff;Counts", 1024, 0, 1024);
  h_adcm1_diff = fs->make<TH1D>("ADCm1", "ADCm1 difference;ADCm1_diff:Counts", 1024, 0, 1024);
  h_tot_diff = fs->make<TH1D>("TOT", "TOT difference;TOT_diff;Counts", 1024, 0, 1024);
  h_toa_diff = fs->make<TH1D>("TOA", "TOA difference;TOA_diff;Counts", 5000, 0, 5000);
  h_tctp_diff = fs->make<TH1D>("TcTp", "TcTp difference;TcTp_diff;Counts", 5, 0, 5);
  h_cm_diff = fs->make<TH1D>("CM", "CM difference;CM_diff;Counts", 1024, 0, 1024);
  h_event = fs->make<TH1D>("Event", "Events having discrepancy;EventNum;Counts", 10000, 0, 10000);
  h_Econd = fs->make<TH1D>("ECOND", "ECONDs having discrepancy;ECOND ID;Counts", 12, 0, 12);
  h_erx = fs->make<TH1D>("Erx", "Erxs having discrepancy;Erx ID;Counts", 6, 0, 6);
  h_channel = fs->make<TH1D>("Channel", "Channels having discrepancy;Channel ID;Counts", 37, 0, 37);
  //h_TOT_Org = fs->make<TH1D>("TOT_Org", "TOT Org ;TOT;Counts", 5000, 0, 5000);
  //h_TOT_Rpc = fs->make<TH1D>("TOT_Rpc", "TOT Rpc ;TOT;Counts", 5000, 0, 5000);
  h_TOT_Org_comp = fs->make<TH1D>("TOT_Org_Comp", "TOT Org Compressed;TOT;Counts", 5000, 0, 5000);
  h_TOT_Rpc_comp = fs->make<TH1D>("TOT_Rpc_Comp", "TOT Rpc Compressed ;TOT;Counts", 5000, 0, 5000);
  h_TOT_Org_9_ECONDid = fs->make<TH1D>("TOT_Org_9", "TOT Org distribution (9th ECOND id);TOT;Counts", 5000, 0, 5000); 
  h_TOT_Rpc_9_ECONDid = fs->make<TH1D>("TOT_Rpc_9", "TOT Rpc distribution (9th ECOND id) ;TOT;Counts", 5000, 0, 5000);

  h2_TOT = fs->make<TH2D>("TOT_2d", "TOT Org vs TOT Rpc;TOT_Org;TOT_Rpc", 5000, 0, 5000, 5000, 0, 5000);;
  h2_adc = fs->make<TH2D>("ADC_2d", "ADC Org vs ADC Rpc;ADC_Org;ADC_Rpc", 1024, 0, 1024, 1024, 0, 1024);
  h2_adcm1 = fs->make<TH2D>("ADCm1_2d", "ADCm1 Org vs ADCm1 Rpc;ADCm1_Org;ADCm1_Rpc", 1024, 0, 1024, 1024, 0, 1024);
  h2_toa = fs->make<TH2D>("TOA_2d", "TOA Org vs TOA Rpc;TOA_Org;TOA_Rpc", 5000, 0, 5000, 5000, 0, 5000);

  //h_adc_Rpc = fs->make<TH1D>("ADC_Rpc", "ADC_Rpc;ADC;Counts", 1024, 0, 1024);;
  //h_adcm1_Rpc = fs->make<TH1D>("ADCm1_Rpc", "ADCm1 Rpc ;ADCm1;Counts", 1024, 0, 1024);;
  //h_toa_Rpc = fs->make<TH1D>("TOA_Rpc", "TOA Rpc ;TOA;Counts", 5000, 0, 5000);;

  //h_adc_Org = fs->make<TH1D>("ADC_Org", "ADC_Org;ADC;Counts", 1024, 0, 1024);
  //h_adcm1_Org = fs->make<TH1D>("ADCm1_Org", "ADCm1 Org ;ADCm1;Counts", 1024, 0, 1024);
  //h_toa_Org = fs->make<TH1D>("TOA_Org", "TOA Org ;TOA;Counts", 5000, 0, 5000);

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  setupDataToken_ = esConsumes<SetupData, SetupRecord>();
#endif
  //now do what ever initialization is needed
}

validation_RAW2DIGI::~validation_RAW2DIGI() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

void validation_RAW2DIGI::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {
  if (mapWatcher_.check(iSetup)) {
  moduleIndexer_Org= iSetup.getData(moduleIndexToken_Org);
  cellIndexer_Org = iSetup.getData(cellIndexToken_Org);
  config_Org = iSetup.getData(configToken_Org);

  moduleIndexer_Rpc= iSetup.getData(moduleIndexToken_Rpc);
  cellIndexer_Rpc = iSetup.getData(cellIndexToken_Rpc);
  config_Rpc = iSetup.getData(configToken_Rpc);
  }
}

void validation_RAW2DIGI::endRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {

}

// ------------ method called for each event  ------------
void validation_RAW2DIGI::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

 std::cout << ">>> TestHGCalRawToDigi: Event " << iEvent.id()
            << " ========================================================================================" << std::endl;

  int eventNum = iEvent.id().event();


  hgcaldigi::HGCalDigiHost digis_Org(cms::alpakatools::host(), moduleIndexer_Org.maxDataSize());
  hgcaldigi::HGCalECONDPacketInfoHost econdPacketInfo_Org(cms::alpakatools::host(), moduleIndexer_Org.maxModulesCount());
  hgcaldigi::HGCalFEDPacketInfoHost fedPacketInfo_Org(cms::alpakatools::host(), moduleIndexer_Org.fedCount());

  hgcaldigi::HGCalDigiHost digis_Rpc(cms::alpakatools::host(), moduleIndexer_Rpc.maxDataSize());
  hgcaldigi::HGCalECONDPacketInfoHost econdPacketInfo_Rpc(cms::alpakatools::host(), moduleIndexer_Rpc.maxModulesCount());
  hgcaldigi::HGCalFEDPacketInfoHost fedPacketInfo_Rpc(cms::alpakatools::host(), moduleIndexer_Rpc.fedCount());
  
  auto view1 = digis_Org.view();
  for (size_t i = 0; i < moduleIndexer_Org.maxDataSize(); ++i) {
    view1.tctp()[i]  = 0;
    view1.adcm1()[i] = 0;
    view1.adc()[i]   = 0;
    view1.tot()[i]   = 0;
    view1.toa()[i]   = 0;
    view1.cm()[i]    = 0;
    //view1.flags()[i] = 0;
}

auto view2 = digis_Rpc.view();
  for (size_t i = 0; i < moduleIndexer_Rpc.maxDataSize(); ++i) {
    view2.tctp()[i]  = 0;
    view2.adcm1()[i] = 0;
    view2.adc()[i]   = 0;
    view2.tot()[i]   = 0;
    view2.toa()[i]   = 0;
    view2.cm()[i]    = 0;
    //view2.flags()[i] = 0;
}

  // CREATE DIGIs
  // std::cout << "Created DIGIs SOA with " << digis.view().metadata().size() << " entries" << std::endl;
  const auto& fedBufferOrg = iEvent.get(fedRawTokenOrg_);
  const auto& fedBufferRpc = iEvent.get(fedRawTokenRpc_);
  //for (unsigned fedId = 0; fedId < moduleIndexer_.fedCount(); ++fedId) {
    //std::cout << "fed loop index = " << fedId << std::endl;
    //std::cout << "moduleIndexer_.fedCount() = " << moduleIndexer_.fedCount() << std::endl;
    unsigned fedId = 1601;
    const auto& fed_data_Org = fedBufferOrg.fragmentData(fedId);
    const auto& fed_data_Rpc = fedBufferRpc.fragmentData(fedId);
    //

    //std::cout << "FedBufferSize: " <<  sizeof(fedBuffer) << std::endl;

    //if (fed_data.size() == 0)
    //  continue;
    //std::cout << "Calling parseFEDData()" << std::endl;
    //std::cout << "FED " << fedId
    //      << " fragment size = "
    //      << fed_data.size() << std::endl;
    unpacker_Org.parseFEDData(fedId, fed_data_Org, moduleIndexer_Org, config_Org, digis_Org, fedPacketInfo_Org, econdPacketInfo_Org, /*headerOnlyMode*/ false);
    unpacker_Rpc.parseFEDData(fedId, fed_data_Rpc, moduleIndexer_Rpc, config_Rpc, digis_Rpc, fedPacketInfo_Rpc, econdPacketInfo_Rpc, /*headerOnlyMode*/ false);
  //}
  
  // CHECK DIGIs
  //for (unsigned fedId = 0; fedId < moduleIndexer_.fedCount(); ++fedId) {
    //std::cout << " CHeck Check Check " << std::endl;

    std::vector<uint16_t> tctp_Org ,adc_Org, adcm1_Org ,tot_Org ,toa_Org ,cm_Org ,flags_Org ,channel_Org, Econd_Org, Erx_Org;
    std::vector<uint16_t> tctp_Rpc ,adc_Rpc, adcm1_Rpc ,tot_Rpc ,toa_Rpc ,cm_Rpc ,flags_Rpc ,channel_Rpc, Econd_Rpc, Erx_Rpc;
    tctp_Org.clear();
    adc_Org.clear();
    adcm1_Org.clear();
    tot_Org.clear();
    toa_Org.clear();
    cm_Org.clear();
    flags_Org.clear();
    channel_Org.clear();
    Econd_Org.clear();
    Erx_Org.clear();

    tctp_Rpc.clear();
    adc_Rpc.clear();
    adcm1_Rpc.clear();
    tot_Rpc.clear();
    toa_Rpc.clear();
    cm_Rpc.clear();
    flags_Rpc.clear();
    channel_Rpc.clear();
    Econd_Rpc.clear();
    Erx_Rpc.clear();
    //=============ooooooooooooOOOOOOOOOOOOOOOOOOOOO Original Raw Data OOOOOOOOOOooooooooooooooooooooo=====================


    //std::cout << "fed=" << fedId << std::endl;
    const auto econdMaxOrg = moduleIndexer_Org.getNumModules(fedId);
    for (uint32_t econdIdx = 0; econdIdx < econdMaxOrg; econdIdx++) {
      //std::cout << "fed=" << fedId << ", econdIdx=" << econdIdx << std::endl;
      const auto erxMax = moduleIndexer_Org.getNumERxs(fedId,econdIdx);
      //std::cout << "   fed econd   eRx  chan |  tctp adcm1   adc   tot   toa    cm  flags" << std::endl;
      for (uint32_t erxIdx = 0; erxIdx < erxMax; erxIdx++) {
        uint32_t eRxDenseIdx = moduleIndexer_Org.getIndexForModuleErx(fedId, econdIdx, erxIdx);
        //std::cout << "   erxIdx=" << erxIdx << ", eRxDenseIdx=" << eRxDenseIdx << std::endl;
        uint32_t aveadc = 0, nchans = 0; // averaged over channels
        for (uint32_t channelIdx = 0; channelIdx < HGCalMappingCellIndexer::maxChPerErx_; channelIdx++) {
          uint32_t denseIdx = moduleIndexer_Org.getIndexForModuleData(fedId, econdIdx, erxIdx, channelIdx);
          if(digis_Org.view()[denseIdx].flags()!=hgcal::DIGI_FLAG::NotAvailable) {
            aveadc += digis_Org.view()[denseIdx].adc();
            nchans++;
          }

          uint16_t TOT_Org = digis_Org.view()[denseIdx].tot();
          //if(digis_Org.view()[denseIdx].flags()  == 0x0000 or digis_Org.view()[denseIdx].flags() == 0x8000){//!=hgcal::DIGI_FLAG::NotAvailable){
          //if(digis_Org.view()[denseIdx].flags() !=hgcal::DIGI_FLAG::NotAvailable){
            tctp_Org.push_back(digis_Org.view()[denseIdx].tctp());
            adc_Org.push_back(digis_Org.view()[denseIdx].adc());
            adcm1_Org.push_back(digis_Org.view()[denseIdx].adcm1());
            tot_Org.push_back(digis_Org.view()[denseIdx].tot());
            toa_Org.push_back(digis_Org.view()[denseIdx].toa());
            cm_Org.push_back(digis_Org.view()[denseIdx].cm());
            flags_Org.push_back(digis_Org.view()[denseIdx].flags());
            channel_Org.push_back(channelIdx);
            Econd_Org.push_back(econdIdx);
            Erx_Org.push_back(erxIdx);

            //h_TOT_Org->Fill(TOT_Org);
            h_TOT_Org_comp->Fill(compressToT(TOT_Org));

            //h_adc_Org->Fill(digis_Org.view()[denseIdx].adc());
            //h_adcm1_Org->Fill(digis_Org.view()[denseIdx].adcm1());
            //h_toa_Org->Fill(digis_Org.view()[denseIdx].toa());

            if(econdIdx == 9){
              h_TOT_Org_9_ECONDid->Fill(TOT_Org);
            }
          //}

          //std::cout << ">>> HGCalUnpacker:    channelIdx= " << channelIdx << ", denseIdx = " << denseIdx
          //          << ", ADC=" <<  adc << std::endl;

          //h_adc->Fill(digis.view()[denseIdx].adc());
          //h_adcm1->Fill(digis.view()[denseIdx].adcm1());
          //h_tot->Fill(digis.view()[denseIdx].tot());
          //h_toa->Fill(digis.view()[denseIdx].toa());
          //h_tctp->Fill(digis.view()[denseIdx].tctp());
          //h_cm->Fill(digis.view()[denseIdx].cm());
          
          /*std::cout << std::dec << std::setfill(' ')
                    << std::setw(6) << fedId << std::setw(6) << econdIdx
                    << std::setw(6) << erxIdx << std::setw(6) << channelIdx << " |"
                    << std::setw(6) << (uint32_t) digis_Org.view()[denseIdx].tctp()
                    << std::setw(6) << digis_Org.view()[denseIdx].adcm1() << std::setw(6) << digis_Org.view()[denseIdx].adc()
                    << std::setw(6) << digis_Org.view()[denseIdx].tot()   << std::setw(6) << digis_Org.view()[denseIdx].toa() 
                    << std::setw(6) << digis_Org.view()[denseIdx].cm()
                    << " 0x" << std::hex << std::setfill('0') << std::setw(4) << digis_Org.view()[denseIdx].flags()
                    << std::dec << std::setfill(' ') << std::endl;*/
        }
        aveadc *= 1./nchans; //HGCalMappingCellIndexer::maxChPerErx_;
        //if (aveadc_map_.find(eRxDenseIdx)!=aveadc_map_.end())
        //  aveadc_map_[eRxDenseIdx] = { };
        aveadc_map_[eRxDenseIdx].push_back(aveadc);
      } // close loop over eRx ROCs
    } // close loop over ECON-Ds
  //} // close loop over FEDs


  //===============ooooooooooOOOOOOOOOOOOOOOOOOOO repacked raw data OOOOOOOOOOOOOOoooooooooooooooooo================



  const auto econdMaxRpc = moduleIndexer_Rpc.getNumModules(fedId);
    for (uint32_t econdIdx = 0; econdIdx < econdMaxRpc; econdIdx++) {
      //std::cout << "fed=" << fedId << ", econdIdx=" << econdIdx << std::endl;
      const auto erxMax = moduleIndexer_Rpc.getNumERxs(fedId,econdIdx);
      //std::cout << "   fed econd   eRx  chan |  tctp adcm1   adc   tot   toa    cm  flags" << std::endl;
      for (uint32_t erxIdx = 0; erxIdx < erxMax; erxIdx++) {
        uint32_t eRxDenseIdx = moduleIndexer_Rpc.getIndexForModuleErx(fedId, econdIdx, erxIdx);
        //std::cout << "   erxIdx=" << erxIdx << ", eRxDenseIdx=" << eRxDenseIdx << std::endl;
        uint32_t aveadc = 0, nchans = 0; // averaged over channels
        for (uint32_t channelIdx = 0; channelIdx < HGCalMappingCellIndexer::maxChPerErx_; channelIdx++) {
          uint32_t denseIdx = moduleIndexer_Rpc.getIndexForModuleData(fedId, econdIdx, erxIdx, channelIdx);
          if(digis_Rpc.view()[denseIdx].flags()!=hgcal::DIGI_FLAG::NotAvailable) {
            aveadc += digis_Rpc.view()[denseIdx].adc();
            nchans++;
          }

          uint16_t TOT_Rpc = digis_Rpc.view()[denseIdx].tot();
          //if(digis_Rpc.view()[denseIdx].flags() == 0x0000 or digis_Rpc.view()[denseIdx].flags() == 0x8000){//!=hgcal::DIGI_FLAG::NotAvailable){
          //if(digis_Rpc.view()[denseIdx].flags() !=hgcal::DIGI_FLAG::NotAvailable){
            tctp_Rpc.push_back(digis_Rpc.view()[denseIdx].tctp());
            adc_Rpc.push_back(digis_Rpc.view()[denseIdx].adc());
            adcm1_Rpc.push_back(digis_Rpc.view()[denseIdx].adcm1());
            tot_Rpc.push_back(digis_Rpc.view()[denseIdx].tot());
            toa_Rpc.push_back(digis_Rpc.view()[denseIdx].toa());
            cm_Rpc.push_back(digis_Rpc.view()[denseIdx].cm());
            flags_Rpc.push_back(digis_Rpc.view()[denseIdx].flags());
            channel_Rpc.push_back(channelIdx);
            Econd_Rpc.push_back(econdIdx);
            Erx_Rpc.push_back(erxIdx);

            //h_adc_Rpc->Fill(digis_Rpc.view()[denseIdx].adc());
            //h_adcm1_Rpc->Fill(digis_Rpc.view()[denseIdx].adcm1());
            //h_toa_Rpc->Fill(digis_Rpc.view()[denseIdx].toa());
            //h_TOT_Rpc->Fill(digis_Rpc.view()[denseIdx].tot());
            //h_TOT_Rpc_comp->Fill(compressToT(TOT_Rpc));
            if(econdIdx == 9){
              h_TOT_Rpc_9_ECONDid->Fill(TOT_Rpc);
            }

          //}
          //std::cout << ">>> HGCalUnpacker:    channelIdx= " << channelIdx << ", denseIdx = " << denseIdx
          //          << ", ADC=" <<  adc << std::endl;

          //h_adc->Fill(digis.view()[denseIdx].adc());
          //h_adcm1->Fill(digis.view()[denseIdx].adcm1());
          //h_tot->Fill(digis.view()[denseIdx].tot());
          //h_toa->Fill(digis.view()[denseIdx].toa());
          //h_tctp->Fill(digis.view()[denseIdx].tctp());
          //h_cm->Fill(digis.view()[denseIdx].cm());
          
          /*std::cout << std::dec << std::setfill(' ')
                    << std::setw(6) << fedId << std::setw(6) << econdIdx
                    << std::setw(6) << erxIdx << std::setw(6) << channelIdx << " |"
                    << std::setw(6) << (uint32_t) digis_Rpc.view()[denseIdx].tctp()
                    << std::setw(6) << digis_Rpc.view()[denseIdx].adcm1() << std::setw(6) << digis_Rpc.view()[denseIdx].adc()
                    << std::setw(6) << digis_Rpc.view()[denseIdx].tot()   << std::setw(6) << digis_Rpc.view()[denseIdx].toa() 
                    << std::setw(6) << digis_Rpc.view()[denseIdx].cm()
                    << " 0x" << std::hex << std::setfill('0') << std::setw(4) << digis_Rpc.view()[denseIdx].flags()
                    << std::dec << std::setfill(' ') << std::endl;*/
        }
        aveadc *= 1./nchans; //HGCalMappingCellIndexer::maxChPerErx_;
        //if (aveadc_map_.find(eRxDenseIdx)!=aveadc_map_.end())
        //  aveadc_map_[eRxDenseIdx] = { };
        aveadc_map_[eRxDenseIdx].push_back(aveadc);
      } // close loop over eRx ROCs
    } // close loop over ECON-Ds


    //oooooooooOOOOOOOOOOOOOOOOOOOOoo======== Comparision =========OoooooOOOOOOOOOOOOOOOOoooooooo
    bool badEvents = false;
    //if(tctp_Org.size() != tctp_Rpc.size()){
    //  std::cout << "oOOOOOOOOOOOOOOOOOOOOOOoooo data size doesn't match: oooOOOOOOOOOOOOOOOOOOOo" << std::endl;
    //  //return;
    //}
    //else{
      for(size_t i = 0; i < tctp_Org.size(); i++){
        //if(flags_Org[i] == 0x0000 or flags_Org[i] == 0x8000){
          h_adc_diff->Fill(adc_Org[i] - adc_Rpc[i]);
          h_adcm1_diff->Fill(adcm1_Org[i] - adcm1_Rpc[i]);
          h_tot_diff->Fill(tot_Org[i] - tot_Rpc[i]);
          h_toa_diff->Fill(toa_Org[i] - toa_Rpc[i]);
          h_tctp_diff->Fill(tctp_Org[i] - tctp_Rpc[i]);
          h_cm_diff->Fill(cm_Org[i] - cm_Rpc[i]);

          h2_TOT->Fill(tot_Org[i], tot_Rpc[i]);
          h2_adc->Fill(adc_Org[i], adc_Rpc[i]);
          h2_adcm1->Fill(adcm1_Org[i], adcm1_Rpc[i]);
          h2_toa->Fill(toa_Org[i], toa_Rpc[i]);

          if((adc_Org[i] != adc_Rpc[i]) or (adcm1_Org[i] != adcm1_Rpc[i]) or (tot_Org[i] != tot_Rpc[i]) or (toa_Org[i] != toa_Rpc[i]) or (tctp_Org[i] != tctp_Rpc[i]) or (cm_Org[i] != cm_Rpc[i])){
            //h_event->Fill(eventNum);
            h_Econd->Fill(Econd_Org[i]);
            h_erx->Fill(Erx_Org[i]);
            h_channel->Fill(channel_Org[i]);
            badEvents = true;
          }
        //}
      }
      if(badEvents == true)
        h_event->Fill(eventNum);
    //}
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  // if the SetupData is always needed
  auto setup = iSetup.getData(setupToken_);
  // if need the ESHandle to check if the SetupData was there or not
  auto pSetup = iSetup.getHandle(setupToken_);
#endif
}

// ------------ method called once each job just before starting event loop  ------------
//void validation_RAW2DIGI::beginJob() {
//  // please remove this method if not needed
//}

// ------------ method called once each job just after ending the event loop  ------------
//void validation_RAW2DIGI::endJob() {
//  // please remove this method if not needed
//}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void validation_RAW2DIGI::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  //edm::ParameterSetDescription desc;
  //desc.setUnknown();
  //descriptions.addDefault(desc);

  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("src1", edm::InputTag("rawDataCollector"));
  desc.add<edm::InputTag>("src2", edm::InputTag("RawDataBuffer"));
  desc.add<std::vector<unsigned int> >("fedIds", {});
  descriptions.add("HGCalDigis_validation", desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(validation_RAW2DIGI);
