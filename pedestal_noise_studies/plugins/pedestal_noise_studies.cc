// -*- C++ -*-
//
// Package:    rawDataProducer_repacked_gen/pedestal_noise_studies
// Class:      pedestal_noise_studies
//
/**\class pedestal_noise_studies pedestal_noise_studies.cc rawDataProducer_repacked_gen/pedestal_noise_studies/plugins/pedestal_noise_studies.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Rohit Kaundal
//         Created:  Wed, 23 Sep 2026 09:55:39 GMT
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "FWCore/Utilities/interface/ESGetToken.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

#include "HGCalCommissioning/SystemTestEventFilters/interface/HGCalTestSystemMetaData.h"
//DetId
#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
//Digi information
#include "DataFormats/HGCalDigi/interface/HGCalDigiHost.h"
#include "DataFormats/HGCalDigi/interface/HGCalRawDataDefinitions.h"
//mapping information
#include "CondFormats/DataRecord/interface/HGCalElectronicsMappingRcd.h"
#include "CondFormats/DataRecord/interface/HGCalDenseIndexInfoRcd.h"
#include "CondFormats/HGCalObjects/interface/HGCalMappingParameterHost.h"
#include "CondFormats/HGCalObjects/interface/HGCalMappingModuleIndexer.h"
#include "DataFormats/HGCalDigi/interface/HGCalECONDPacketInfoSoA.h"
#include "DataFormats/HGCalDigi/interface/HGCalECONDPacketInfoHost.h"
#include "DataFormats/HGCalDigi/interface/HGCalFEDPacketInfoSoA.h"
#include "DataFormats/HGCalDigi/interface/HGCalFEDPacketInfoHost.h"
#include "Geometry/HGCalMapping/interface/HGCalMappingTools.h"
#include "FWCore/Utilities/interface/Exception.h"
//Data packing
#include "DataFormats/HGCalDigi/interface/HGCROCChannelDataFrame.h"
#include "SimCalorimetry/HGCalSimAlgos/interface/HGCalRawDataPackingTools.h"
#include "DataFormats/HGCalDigi/interface/HGCalRawDataDefinitions.h"
#include "DataFormats/FEDRawData/interface/SLinkRocketHeaders.h"

#include "DataFormats/FEDRawData/interface/RawDataBuffer.h"
#include "FWCore/Utilities/interface/EDPutToken.h"

#include <iostream>
//#include <boost/crc.hpp>

#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include <vector>
#include <memory>
//#include "TFile.h"
#include <unordered_map>
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

class pedestal_noise_studies : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit pedestal_noise_studies(const edm::ParameterSet&);
  ~pedestal_noise_studies() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // ----------member data ---------------------------
  edm::EDGetTokenT<TrackCollection> tracksToken_;  //used to select what tracks to read from configuration file

  edm::EDGetTokenT<hgcaldigi::HGCalDigiHost> digisToken_;
  edm::EDGetTokenT<hgcaldigi::HGCalECONDPacketInfoHost> econdInfoTkn_;
  edm::EDGetTokenT<hgcaldigi::HGCalFEDPacketInfoHost> fedInfoTkn_;

  edm::ESGetToken<hgcal::HGCalDenseIndexInfoHost, HGCalDenseIndexInfoRcd> denseIndexInfoTkn_;
  edm::ESGetToken<hgcal::HGCalMappingCellParamHost, HGCalElectronicsMappingRcd> cellTkn_;
  edm::ESGetToken<hgcal::HGCalMappingModuleParamHost, HGCalElectronicsMappingRcd> moduleTkn_;
  edm::ESGetToken<HGCalMappingModuleIndexer, HGCalElectronicsMappingRcd> moduleIndexTkn_;

  std::vector<uint16_t> adc;
  std::vector<TH1D*> h_ADC_channel;

  int moduleNum, chNum;

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
pedestal_noise_studies::pedestal_noise_studies(const edm::ParameterSet& iConfig)
    : digisToken_(consumes<hgcaldigi::HGCalDigiHost>(iConfig.getUntrackedParameter<edm::InputTag>("hgcalDigis"))),
      econdInfoTkn_(consumes<hgcaldigi::HGCalECONDPacketInfoHost>(iConfig.getUntrackedParameter<edm::InputTag>("hgcalDigis"))),
      fedInfoTkn_(consumes<hgcaldigi::HGCalFEDPacketInfoHost>(iConfig.getUntrackedParameter<edm::InputTag>("hgcalDigis"))),
  
      denseIndexInfoTkn_(esConsumes<hgcal::HGCalDenseIndexInfoHost, HGCalDenseIndexInfoRcd>()),
      cellTkn_(esConsumes()),
      moduleTkn_(esConsumes()),
      moduleIndexTkn_(esConsumes<HGCalMappingModuleIndexer, HGCalElectronicsMappingRcd>()),
      rawDataBufferPutToken_(produces()) {


  edm::Service<TFileService> fs;
//
    for (int mod = 0; mod < 12; ++mod) {
      for(int ch = 0; ch < 222; ++ch){
        h_ADC_channel.push_back(
            fs->make<TH1D>(
                Form("ADC_mod%d_ch%d", mod, ch),
                Form("ADC Distribution module %d channel %d;ADC;Counts", mod, ch),
                1024, 0, 1024
            )
        );
      }
    }

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  setupDataToken_ = esConsumes<SetupData, SetupRecord>();
#endif
  //now do what ever initialization is needed
}

pedestal_noise_studies::~pedestal_noise_studies() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called for each event  ------------
void pedestal_noise_studies::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  const auto& digis = iEvent.getHandle(digisToken_);
  auto const& digis_view = digis->const_view();
  //int32_t ndigis = digis->const_view().metadata().size();

  auto const& denseIndexInfo = iSetup.getData(denseIndexInfoTkn_);
  auto const& denseIndexInfo_view = denseIndexInfo.const_view();
  int32_t ndenseIndices = denseIndexInfo_view.metadata().size();

  const auto& econdInfo = iEvent.getHandle(econdInfoTkn_);
  auto const& econdInfo_view = econdInfo->const_view();

  const auto& fedInfo = iEvent.getHandle(fedInfoTkn_);
  auto const& fedInfo_view = fedInfo->const_view();

  auto const& cellInfo = iSetup.getData(cellTkn_);
  auto const& cellInfo_view = cellInfo.const_view();
  auto const& moduleInfo = iSetup.getData(moduleTkn_);
  auto const& moduleIndex = iSetup.getData(moduleIndexTkn_);
  auto const& moduleInfo_view = moduleInfo.const_view();

  int32_t ndigis = 0;
  int32_t nGoodDigis = 0;
  if(digis.isValid()){
  ndigis = digis->const_view().metadata().size();
  //assert(ndigis == ndenseIndices);
  
  }
  cout << "EventNo:  " << eventNum << endl;


  for (int32_t i = 0; i < ndenseIndices && digis.isValid(); i++){
    uint32_t modInfoIdx(denseIndexInfo_view.modInfoIdx()[i]);
    moduleNum = moduleInfo_view.econdidx()[modInfoIdx];
    chNum = denseIndexInfo_view.chNumber()[i];

  }
  

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  // if the SetupData is always needed
  auto setup = iSetup.getData(setupToken_);
  // if need the ESHandle to check if the SetupData was there or not
  auto pSetup = iSetup.getHandle(setupToken_);
#endif
}

// ------------ method called once each job just before starting event loop  ------------
void pedestal_noise_studies::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void pedestal_noise_studies::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void pedestal_noise_studies::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //edm::ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks", edm::InputTag("ctfWithMaterialTracks"));
  //descriptions.addWithDefaultLabel(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(pedestal_noise_studies);
