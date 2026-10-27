// -*- C++ -*-
//
// Package:    rawDataProducer_repacked_gen/rawDataProducer_repacked
// Class:      rawDataProducer_repacked
//
/**\class rawDataProducer_repacked rawDataProducer_repacked.cc rawDataProducer_repacked_gen/rawDataProducer_repacked/plugins/rawDataProducer_repacked.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Rohit Kaundal
//         Created:  Wed, 05 Aug 2026 06:31:40 GMT
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

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
#include "CondFormats/DataRecord/interface/HGCalModuleConfigurationRcd.h"
#include "CondFormats/HGCalObjects/interface/HGCalCalibrationParameterHost.h"
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
#include <algorithm>
#include <boost/crc.hpp>

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

std::vector<bool> packingParm(std::vector<uint8_t> &tctp, std::vector<uint16_t> &adcm1, std::vector<uint16_t> &adc, std::vector<uint16_t> &adcm1Th, std::vector<uint16_t> &adcTh){
  std::vector<bool> passZSm1, passZS, packerPar;
  bool passZSm1Parm, passZSParm;
  //std::cout << "chNum: " << tctp.size() << std::endl;
  for(size_t i = 0; i < tctp.size(); i++){
    //std::cout << "chId: " << i << " tctp: " << static_cast<unsigned int>(tctp[i]) << " ADC: " << adc[i] << "  ADCm1: " << adcm1[i] <<  "  Threshold: " << adcTh[i] << std::endl;
    if(tctp[i] == 0){
      //std::cout << "chId: " << i << " ADC: " << adc[i] << "  ADCm1: " << adcm1[i] <<  "  Threshold: " << adcTh[i] << std::endl;
      //std::cout << "adcm1: " << adcm1[i] << std::endl;
      if(adc[i] > adcTh[i] and adcm1[i] <= adcm1Th[i]){
        passZSm1.push_back(false);
        passZS.push_back(true);
      }
      else if(adc[i] > adcTh[i] and adcm1[i] > adcm1Th[i]){
        passZSm1.push_back(true);
        passZS.push_back(true);
      }   
      else if(adc[i] <= adcTh[i]){
        passZS.push_back(false);
      }
      else continue;
    }
    else continue;
  }
  bool checkPassZSm1 = std::any_of(passZSm1.begin(),
                                 passZSm1.end(),
                                 [](bool enabled) { return enabled; }); 
  if(!checkPassZSm1)
    passZSm1Parm = false;
  else passZSm1Parm = true;

  //for(size_t i = 0; i < passZS.size(); i++){
  //  std::cout << "passZS ch: " << passZS[i] << std::endl;
  //}

  bool checkPassZS = std::any_of(passZS.begin(),
                                 passZS.end(),
                                 [](bool enabled) { return enabled; });
  if(!checkPassZS)
    passZSParm = false;
  else passZSParm = true;
  
  packerPar.push_back(passZSParm);
  packerPar.push_back(passZSm1Parm);
  return packerPar;    
}

/*std::vector<bool> packingParm(std::vector<uint16_t> adcm1, std::vector<uint16_t> adc, std::vector<uint16_t> toa, std::vector<uint16_t> adcm1Th, std::vector<uint16_t> adcTh){//}, std::vector<uint16_t> toaTh){
  //uint16_t adcTh, adcm1Th, toaTh;
  uint16_t toaTh = 0;
  std::vector<bool> passZSm1, hasToa;
  std::vector<uint8_t> tctp;
  for(size_t i = 0; i < adc.size(); i++){
    if(adcm1[i] != 0 or adc[i] != 0 or toa[i] != 0){
      if(adc[i] > adcTh[i] and adcm1[i] > adcm1Th[i] and toa[i] <= toaTh){
        tctp.push_back(0);
        hasToa.push_back(false);
        passZSm1.push_back(true);
      }
      else if(adc[i] > adcTh[i] and adcm1[i] < adcm1Th[i] and toa[i] <= toaTh){
        tctp.push_back(0);
        hasToa.push_back(false);
        passZSm1.push_back(false);
      }
      else if(adc[i] > adcTh[i] and adcm1[i] > adcm1Th[i] and toa[i] == 0){
        tctp.push_back(1);
        //hasToA.push_back(false);
        //passZSm1.push_back(true);
      } 
      else if(adc[i] > adcTh[i] and adcm1[i] < adcm1Th[i] and toa[i] > toaTh){
        tctp.push_back(0);
        hasToa.push_back(true);
        passZSm1.push_back(false);
      }
      else if(adc[i] > adcTh[i] and adcm1[i] > adcm1Th[i] and toa[i] > toaTh){
        tctp.push_back(0);
        hasToa.push_back(true);
        passZSm1.push_back(true);
      }
      else if(adc[i] == 0 and adcm1[i] > adcm1Th[i] and toa[i] > toaTh){
        tctp.push_back(3);
        //hasToa.push_back(true);
        //passZSm1.push_back(true);
      }
      else continue;
      
    } else tctp.push_back(0);

  }
  bool passZSm1Param, hasToaParam;
  std::vector<bool> packerPrm;
  //for(size_t i = 0; i < passZSm1.size(); i++){
  //  std::cout << "passZSm1 ch: " << passZSm1[i] << std::endl;
  //}
  bool checkPassZSm1 = std::any_of(passZSm1.begin(),
                                 passZSm1.end(),
                                 [](bool enabled) { return enabled; }); 
  //std::cout << "checkPassZSm1: " << checkPassZSm1 << std::endl;
  if(!checkPassZSm1)
    passZSm1Param = false;
  else passZSm1Param = true;

  bool checkHasToa = std::any_of(hasToa.begin(),
                                 hasToa.end(),
                                 [](bool enabled) { return enabled; });
  if(!checkHasToa)
    hasToaParam = false;
  else hasToaParam = true;

  packerPrm.push_back(passZSm1Param);
  packerPrm.push_back(hasToaParam);

  return packerPrm;
} */





uint16_t compressToT(uint16_t totraw) {
    if (totraw > 0x1ff)
      return (0x200 | (totraw >> 3));
    return (totraw & 0x1ff);
  }

class rawDataProducer_repacked : public edm::stream::EDProducer<> {
public:
  explicit rawDataProducer_repacked(const edm::ParameterSet&);
  ~rawDataProducer_repacked() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginStream(edm::StreamID) override;
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endStream() override;

  //void beginRun(edm::Run const&, edm::EventSetup const&) override;
  //void endRun(edm::Run const&, edm::EventSetup const&) override;
  //void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  //void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

  // ----------member data ---------------------------
  edm::EDGetTokenT<hgcaldigi::HGCalDigiHost> digisToken_;
  const edm::ESGetToken<hgcalrechit::HGCalCalibParamHost, HGCalModuleConfigurationRcd> calibToken_;
  edm::EDGetTokenT<hgcaldigi::HGCalECONDPacketInfoHost> econdInfoTkn_;
  edm::EDGetTokenT<hgcaldigi::HGCalFEDPacketInfoHost> fedInfoTkn_;

  //edm::EDGetTokenT<HGCalTestSystemTrigTimeCollection> trigtimeToken_;
  //edm::EDGetTokenT<HGCalTestSystemMCP> mcpToken_;
  //edm::EDGetTokenT<HGCalTestSystemTimingIn> timeinToken_;

  edm::ESGetToken<hgcal::HGCalDenseIndexInfoHost, HGCalDenseIndexInfoRcd> denseIndexInfoTkn_;
  edm::ESGetToken<hgcal::HGCalMappingCellParamHost, HGCalElectronicsMappingRcd> cellTkn_;
  edm::ESGetToken<hgcal::HGCalMappingModuleParamHost, HGCalElectronicsMappingRcd> moduleTkn_;
  edm::ESGetToken<HGCalMappingModuleIndexer, HGCalElectronicsMappingRcd> moduleIndexTkn_;
  edm::EDPutTokenT<RawDataBuffer> rawDataBufferPutToken_;

  int eventNum;
  std::vector<uint16_t> payloadLength, BX, L1A, fedBX, cm0, cm1;
  //std::vector<std::vector<uint16_t>> adc_Threshold, adcm1_Threshold, toa_Threshold;
  struct packingThresholds {
    std::vector<uint16_t> adc_Threshold, adcm1_Threshold, toa_Threshold;
  };
  std::vector<uint8_t> Orbit;
  std::vector<uint32_t> cbBX, cbOrbit, fedObt;
  std::vector<uint64_t> fedL1A;

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
rawDataProducer_repacked::rawDataProducer_repacked(const edm::ParameterSet& iConfig)
: digisToken_(consumes<hgcaldigi::HGCalDigiHost>(iConfig.getUntrackedParameter<edm::InputTag>("hgcalDigis"))),
    calibToken_{esConsumes(iConfig.getParameter<edm::ESInputTag>("calibSource"))},
    econdInfoTkn_(consumes<hgcaldigi::HGCalECONDPacketInfoHost>(iConfig.getUntrackedParameter<edm::InputTag>("hgcalDigis"))),
    fedInfoTkn_(consumes<hgcaldigi::HGCalFEDPacketInfoHost>(iConfig.getUntrackedParameter<edm::InputTag>("hgcalDigis"))),
    //trigtimeToken_(consumes<HGCalTestSystemTrigTimeCollection>(iConfig.getParameter<edm::InputTag>("metaData"))),
    //mcpToken_(consumes<HGCalTestSystemMCP>(iConfig.getParameter<edm::InputTag>("metaData"))),
    //timeinToken_(consumes<HGCalTestSystemTimingIn>(iConfig.getParameter<edm::InputTag>("metaData"))),
    denseIndexInfoTkn_(esConsumes<hgcal::HGCalDenseIndexInfoHost, HGCalDenseIndexInfoRcd>()),
    cellTkn_(esConsumes()),
    moduleTkn_(esConsumes()),
    moduleIndexTkn_(esConsumes<HGCalMappingModuleIndexer, HGCalElectronicsMappingRcd>()),
    rawDataBufferPutToken_(produces())

{

}

rawDataProducer_repacked::~rawDataProducer_repacked() {
  
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void rawDataProducer_repacked::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  using namespace std;
  
  eventNum = iEvent.id().event();
  payloadLength.clear();
  BX.clear();
  L1A.clear();
  Orbit.clear();
  cm0.clear();
  cm1.clear();
  cbBX.clear();
  cbOrbit.clear();
  fedBX.clear();
  fedL1A.clear();
  fedObt.clear();

  const auto& digis = iEvent.getHandle(digisToken_);
  auto const& digis_view = digis->const_view();
  //int32_t ndigis = digis->const_view().metadata().size();

  auto const& hostCalibParamProvider = iSetup.getData(calibToken_);

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

  int32_t nGoodDigis = 0;
  if(digis.isValid()){
  int32_t ndigis = digis->const_view().metadata().size();
  //assert(ndigis == ndenseIndices);
  
  }
  
  cout << "EventNo:  " << eventNum << endl;
  
  uint32_t nmodules = moduleIndex.maxModulesCount();
  std::vector<uint32_t> cbIdx(0), ECONDidx(0);
  for(size_t i = 0; i < nmodules; i++){
    //auto econdID = moduleInfo_view[i].econdidx();
    //auto cbID = moduleInfo_view[i].captureblockidx();
    cbIdx.push_back(moduleInfo_view[i].captureblockidx());
    ECONDidx.push_back(moduleInfo_view[i].econdidx());
    //kcout << "ECOND_ID: " << econdID << " CB ID: " << cbID << endl;
  }

  std::vector<uint32_t> fed(nmodules), seq(nmodules), nErx(nmodules);
  uint32_t idx(0);
  for(auto it : moduleIndex.typecodeMap() ) {

      fed[idx] = it.second.first;
      seq[idx] = it.second.second;
      nErx[idx] = moduleIndex.getNumERxs(fed[idx], seq[idx]);
      //std::cout << "nErx: " << moduleIndex.getNumERxs(fed[idx], seq[idx]) << std::endl;
      idx++;

      //
    }


    //if(eventNum == 1){//} or eventNum == 366 or eventNum == 520 or eventNum == 739){
    /*cout << std::left
     << std::setw(10) << "tctp"
     << std::setw(10) << "adc"
     << std::setw(10) << "adcm1"
     << std::setw(10) << "tot"
     << std::setw(10) << "toa"
     << std::setw(10) << "cm"
     << std::setw(10) << "flags"
     << std::setw(10) << "channel"
     << std::setw(10) << "fedId"
     << std::setw(15) << "fedReadoutSeq"
     << std::setw(20) << "chI1"
     << std::setw(20) << "chI2"
     << std::setw(15) << "modI1"
     << std::setw(15) << "modI2"
     << std::setw(15) << "chType"
     << std::setw(15) << "isSiPM"
     << std::setw(15) << "iscalib"
     << '\n';*/
    //}



  int maxCBidx = *std::max_element(cbIdx.begin(), cbIdx.end());

  int eRxNum = 0;
  int chIdx = 0;
  int TotalERx =  std::accumulate(nErx.begin(), nErx.end(), uint32_t{0});
  
  std::vector<hgcal::econd::ERxData> allERxData(TotalERx);
  std::vector<hgcal::econd::ERxChannelEnable> enableMaps(TotalERx, hgcal::econd::ERxChannelEnable(37,false));

  std::vector<packingThresholds> Thresholds(TotalERx);

  const std::vector<uint8_t> econd_status(12, hgcal::backend::ECONDPacketStatus::Normal); //for CB

  for (int32_t i = 0; i < ndenseIndices && digis.isValid(); i++) {
    //uint16_t TOT = digis_view.tot()[i];

  /*uint32_t cellInfoIdx(denseIndexInfo_view.cellInfoIdx()[i]);
  uint32_t modInfoIdx(denseIndexInfo_view.modInfoIdx()[i]);
  if(digis_view.flags()[i] != hgcal::DIGI_FLAG::NotAvailable){//} and eventNum == 1){//} or eventNum == 366 or eventNum == 520 or eventNum == 739){
    cout << std::left
     << std::setw(10) << static_cast<unsigned int>(digis_view.tctp()[i])
     << std::setw(10) << digis_view.adc()[i]
     << std::setw(10) << digis_view.adcm1()[i]
     << std::setw(10) << compressToT(digis_view.tot()[i])
     << std::setw(10) << digis_view.toa()[i]
     << std::setw(10) << digis_view.cm()[i]
     << std::setw(10) << digis_view.flags()[i]
     << std::setw(10) << denseIndexInfo_view.chNumber()[i]
     << std::setw(10) << denseIndexInfo_view.fedId()[i]
     << std::setw(15) << denseIndexInfo_view.fedReadoutSeq()[i]
     << std::setw(20) << cellInfo_view.i1()[cellInfoIdx]
     << std::setw(20) << cellInfo_view.i2()[cellInfoIdx]
     << std::setw(15) << moduleInfo_view.i1()[modInfoIdx]
     << std::setw(15) << moduleInfo_view.i2()[modInfoIdx]
     << std::setw(15) << cellInfo_view.t()[cellInfoIdx]
     << std::setw(15) << moduleInfo_view.isSiPM()[modInfoIdx]
     << std::setw(15) << cellInfo_view.iscalib()[cellInfoIdx]
     << '\n';
  } */ 
    eRxNum = i / 37;
    chIdx  = denseIndexInfo_view.chNumber()[i] % 37;

    //cout << "eRxNum: " << eRxNum << " chIdx: " << chIdx << " ADC_ped: " << hostCalibParamProvider.view()[i].ADC_ped() << " Noise: " << hostCalibParamProvider.view()[i].Noise() << endl;
    uint16_t k = 3; //k-sigma threshold
    //adc_Threshold.push_back(hostCalibParamProvider.view()[i].ADC_ped() + k*hostCalibParamProvider.view()[i].Noise());
    //adc_Threshold[eRxNum].push_back(hostCalibParamProvider.view()[i].ADC_ped() + k*hostCalibParamProvider.view()[i].Noise());

    uint16_t adc_Thr = hostCalibParamProvider.view()[i].ADC_ped() + k*hostCalibParamProvider.view()[i].Noise();
    uint16_t adcm1_Thr = hostCalibParamProvider.view()[i].ADC_ped();// + k*hostCalibParamProvider.view()[i].Noise();
    Thresholds.at(eRxNum).adc_Threshold.push_back(adc_Thr);

    if(digis_view.flags()[i] == hgcal::DIGI_FLAG::NotAvailable){
      enableMaps.at(eRxNum).at(chIdx) = false;
      allERxData.at(eRxNum).tctp.push_back(uint8_t(0));
      allERxData.at(eRxNum).adc.push_back(uint16_t(0));
      allERxData.at(eRxNum).adcm.push_back(uint16_t(0));
      allERxData.at(eRxNum).toa.push_back(uint16_t(0));
      allERxData.at(eRxNum).tot.push_back(uint16_t(0));
      allERxData.at(eRxNum).passZS.push_back(false);
      allERxData.at(eRxNum).passZSm1.push_back(false);
      //allERxData.at(eRxNum).meta.push_back(moduleInfo_view.captureblockidx()[modInfoIdx]);
    }
    else{
      enableMaps.at(eRxNum).at(chIdx) = true;
      allERxData.at(eRxNum).tctp.push_back(digis_view.tctp()[i]);
      allERxData.at(eRxNum).adc.push_back(digis_view.adc()[i]);
      allERxData.at(eRxNum).adcm.push_back(digis_view.adcm1()[i]);
      allERxData.at(eRxNum).toa.push_back(digis_view.toa()[i]);
      allERxData.at(eRxNum).tot.push_back(compressToT(digis_view.tot()[i]));
      //if(digis_view.adc()[i] > adc_Thr){
        allERxData.at(eRxNum).passZS.push_back((digis_view.adc()[i] > adc_Thr ? true : false));
        //cout << "erx: " << eRxNum << " chIdx: " << chIdx << " adc: " << digis_view.adc()[i] << " adcm1: " << digis_view.adcm1()[i] << " Threshold: " << adc_Thr << " passZS = true" << endl;
      //}
      //else{
      //  allERxData.at(eRxNum).passZS.push_back(false);
      //  cout << "erx: " << eRxNum << " chIdx: " << chIdx << " adc: " << digis_view.adc()[i] << " adcm1: " << digis_view.adcm1()[i] << " Threshold: " << adc_Thr << " passZS = false" << endl;
      //} 

      //if(digis_view.adcm1()[i] > adc_Thr){
        allERxData.at(eRxNum).passZSm1.push_back((digis_view.adcm1()[i] > adc_Thr ? true : false));
        //cout << "erx: " << eRxNum << " chIdx: " << chIdx << " adc: " << digis_view.adc()[i] << " adcm1: " << digis_view.adcm1()[i] << " Threshold: " << adc_Thr << " passZSm1 = true" << endl;
      //}
      //else{
      //  allERxData.at(eRxNum).passZSm1.push_back(false);
      //  cout << "erx: " << eRxNum << " chIdx: " << chIdx << " adc: " << digis_view.adc()[i] << " adcm1: " << digis_view.adcm1()[i] << " Threshold: " << adc_Thr << " passZSm1 = false" << endl;
      //} 
      //allERxData.at(eRxNum).meta.push_back(moduleInfo_view.captureblockidx()[modInfoIdx]);
       nGoodDigis++;
    }

    
  }
  int32_t necons = 0;
  if(econdInfo.isValid()){
  necons = econdInfo->const_view().metadata().size();
  //assert(ndigis == ndenseIndices);

  for(int imod=0; imod<necons; imod++){
    const auto econd = econdInfo_view[imod];
    //payloads[imod] = econd.payloadLength();
    payloadLength.push_back(econd.payloadLength());
    BX.push_back(econd.BX());
    L1A.push_back(econd.L1A());
    Orbit.push_back(econd.Orbit());
    //if(imod == 0){
    cbBX.push_back(econd.CBBX());
    cbOrbit.push_back(econd.CBOrbit());
    for(size_t ierx=0; ierx<nErx[imod]; ierx++){
      
      cm0.push_back(econd.cm().coeff(ierx,0));
      cm1.push_back(econd.cm().coeff(ierx,1));
      
    }

  }
}

  ///////////oooooooooooOOOOOOOOOOOOO FED Info OOOOOOOOOOOOOOOoooooooooooooooooo////////////////////
  int32_t nfed = 0;
  if(fedInfo.isValid()){
    nfed = fedInfo->const_view().metadata().size();
    //assert(ndigis == ndenseIndices);
  }

  for(int i = 0; i < nfed && fedInfo.isValid(); i++){
    const auto fed = fedInfo_view[i];
    if(fed.FEDPayload() != 0){
      fedBX.push_back(fed.FEDBX());
      fedL1A.push_back(fed.FEDL1A());
      fedObt.push_back(fed.FEDOrbit());
    }
  }


  //oooooooooooooOOOOOOOOOOOOOOO For getting the right sequence of DIGIS oooooooooooOOOOOOOOOOOOOOOOOOOO

    struct ECONDGroup {
      uint32_t econdID;
      uint32_t cbID;
      uint32_t eRxNum;
      uint16_t pLoad;
      size_t startIndex;
      size_t chIndex;
    };

    std::vector<ECONDGroup> groups;

    size_t startIndex = 0;
    size_t chIndex = 0;

    for (size_t i = 0; i < nErx.size(); ++i) {

        ECONDGroup group;

        group.econdID   = ECONDidx[i];
        group.cbID      = cbIdx[i];
        group.eRxNum    = nErx[i];
        group.pLoad     = payloadLength[i];
        group.startIndex = startIndex;
        group.chIndex = chIndex;

        groups.push_back(group);

        startIndex += nErx[i];

        chIndex += nErx[i]*37; //  37 channels per eRx
    }

    std::sort(
        groups.begin(),
        groups.end(),
        [](const ECONDGroup& a, const ECONDGroup& b) {
            return a.econdID < b.econdID;
        }
    );

    std::vector<hgcal::econd::ERxData> oldAllErxData = allERxData;
    std::vector<hgcal::econd::ERxChannelEnable> oldEnableChannel = enableMaps;
    std::vector<uint16_t> old_cMode0 = cm0;
    std::vector<uint16_t> old_cMode1 = cm1;
    std::vector<packingThresholds> old_Th = Thresholds;
    //std::vector<uint16_t> old_ploadLength = payloadLength;

    std::vector<uint32_t> newERxNum;
    std::vector<uint32_t> newECONDid;
    std::vector<uint32_t> newCaptureBlockID;

    std::vector<hgcal::econd::ERxData> newAllErxData;
    std::vector<hgcal::econd::ERxChannelEnable> newEnableChMaps;
    std::vector<uint16_t> new_cMode0;
    std::vector<uint16_t> new_cMode1;
    std::vector<packingThresholds> new_Th;
    std::vector<uint16_t> new_payloadLength;

    newERxNum.reserve(nErx.size());
    newECONDid.reserve(ECONDidx.size());
    newCaptureBlockID.reserve(cbIdx.size());
    newAllErxData.reserve(allERxData.size());
    newEnableChMaps.reserve(enableMaps.size());
    new_cMode0.reserve(cm0.size());
    new_cMode1.reserve(cm1.size());
    new_Th.reserve(Thresholds.size());
    new_payloadLength.reserve(payloadLength.size());

    for (const auto& group : groups) {

        // Add group information
        newERxNum.push_back(group.eRxNum);
        newECONDid.push_back(group.econdID);
        newCaptureBlockID.push_back(group.cbID);
        new_payloadLength.push_back(group.pLoad);

        // Copy this group's eRx entries
        for (size_t j = 0; j < group.eRxNum; ++j) {

            newAllErxData.push_back(
                oldAllErxData[group.startIndex + j]
            );
            newEnableChMaps.push_back(oldEnableChannel[group.startIndex + j]);

            new_cMode0.push_back(old_cMode0[group.startIndex + j]);
            new_cMode1.push_back(old_cMode1[group.startIndex + j]);
            new_Th.push_back(old_Th[group.startIndex + j]);
        }

        //for(size_t j = 0; j < group.eRxNum*37; ++j){
        //  new_adcTh.push_back(old_adcTh[group.chIndex + j]);
        //}
    }

    nErx = std::move(newERxNum);
    ECONDidx = std::move(newECONDid);
    cbIdx = std::move(newCaptureBlockID);
    allERxData = std::move(newAllErxData);
    enableMaps = std::move(newEnableChMaps);
    cm0 = std::move(new_cMode0);
    cm1 = std::move(new_cMode1);
    Thresholds = std::move(new_Th);
    payloadLength = std::move(new_payloadLength);

  //oooooooooOOOOOOOOOOOOOOOOOOOo==================oooooooooooOOOOOOOOOOOOOOOOOOOOO

  //cout << "Size: " << adc_Threshold.size() << endl;
  //for(size_t i = 0; i < Thresholds.size(); i++){
  //  for(size_t j =0 ; j<Thresholds[i].adc_Threshold.size(); j++){
  //    cout <<  "erx: " << i << " ch: " << j << " Old_adc_Th: " << old_Th[i].adc_Threshold[j] <<"  Nw_adc_Th: " << Thresholds[i].adc_Threshold[j] << " difference: " << old_Th[i].adc_Threshold[j] - Thresholds[i].adc_Threshold[j] << endl;
  //  }
  //  
  //}

  /////////////// eRx payload + header generation   (ECOND packet inside CB) ///////////////

  std::vector<uint32_t> econdPacket{0};  
  std::vector<uint32_t> CRC_calc{0};
  std::vector<uint32_t> crcvec;
  econdPacket.clear();
  int econdIdx = 0;
  uint16_t header =  340;//170;
  bool passThrough;
  //bool passThrough1 = false;
  std::vector<bool> passThrough1{false, false, false,false,false,false,false,false,false,true,false,false,}; //for configuration 11LD, 1 Scint module
  //std::vector<bool> passThrough1{false, false, false,false,false,false,false,false,true,false,false,}; //for configuration 10LD, 1 Scint module
  uint8_t ht = 0;
  uint8_t ebo = 0;
  uint8_t ehHam = 9;
  uint8_t rr = 0;
  uint8_t stat = 7;
  uint8_t Ham = 0;
  bool bitE = true;
  bool passZS = true;
  bool passZSm1 = false;
  bool hasToA = false;
  bool charMode = false;
  uint8_t econCrc = 244;
  int ECOND_counter = 0;
  int CB_idx = 0;
  uint32_t padding = 0;
  int moduleNum = 0;


  std::map<uint32_t, uint32_t> count;
  std::vector<uint32_t> enabledECOND;

    for (int x : cbIdx) {
        count[x]++;
    }

    for (auto [number, frequency] : count) {
        enabledECOND.push_back(frequency);
        //if(eventNum == 290 or eventNum == 366 or eventNum == 520 or eventNum == 739){
        //cout << frequency << endl;
        //}
    }


//===============ooooooooooooOOOOOOOOOOOOOOOOOOOOOoooooooooooooooooooOOOOOOOOOOOOOOOOOOOOoooooooo==================


  std::vector<uint16_t> CalculatedPayloadLength;
  CalculatedPayloadLength.clear();
  std::vector<uint32_t> dataWords;
  size_t moduleNum1 = 0;
  size_t moduleCountr = 0;
  std::vector<bool> packingParameter;
  for(size_t erx = 0; erx < allERxData.size(); ++erx){

    //=======================ooooooooooOOOOOOOOOOOOOOOOOOO To emulate packing parameter passZSm1 OOOOOOOOOOOOOOOoooooooooooo========================
    //if(erx > 53 and erx < 59){
    //cout << "\vEcond_Id: " << moduleCountr << " erx: " << erx << endl;
    packingParameter = packingParm(allERxData[erx].tctp, allERxData[erx].adcm, allERxData[erx].adc, Thresholds[erx].adc_Threshold, Thresholds[erx].adc_Threshold);
    //bool passZSm1 = packingParameter[1];
    //cout << "erx: " << erx << " passZS: " << packingParameter[0] << " passZsm1: " << packingParameter[1] << endl;
    //}//cout << endl;
    
    //passZSm1Parm = packingParm(allERxData[erx].adcm, allERxData[erx].adc, allERxData[erx].toa, Thresholds[erx].adc_Threshold, Thresholds[erx].adc_Threshold);
    //cout << "erx: " << erx << "  passZSm1: " << passZSm1Parm[0] << "  hasToa: " << passZSm1Parm[1] << endl;
  

    uint32_t eRxSum = std::accumulate(nErx.begin(), nErx.begin() + moduleNum1, uint32_t{0});
    passThrough = passThrough1[moduleCountr];
    
    if(erx == 0 or erx == eRxSum){
      //cout << "erx: " << erx << " ErxSum: " << eRxSum << endl;
      if(erx != 0){      
        CalculatedPayloadLength.push_back(dataWords.size()); 
      }
      dataWords.clear();
      moduleNum1++;
    }
    bool eRxPresent = std::any_of(
      enableMaps[erx].begin(),
      enableMaps[erx].end(),
      [](bool enabled) { return enabled; }
      );

      if (!eRxPresent) {
          bitE = false;
          //passThrough = false;
          const auto eRxheader = hgcal::econd::eRxSubPacketHeader(stat, Ham, bitE, cm0[erx], cm1[erx], enableMaps[erx]);  //eRx Header for each eRX
          dataWords.push_back(eRxheader[0]);  

      } else{
        //bitE = false;
        //passThrough = false;

          const auto eRxheader = hgcal::econd::eRxSubPacketHeader(stat, Ham, bitE, cm0[erx], cm1[erx], enableMaps[erx]);  //eRx Header for each eRX
          dataWords.push_back(eRxheader[0]);      
          dataWords.push_back(eRxheader[1]);

          const auto erx_chan_data = hgcal::econd::produceERxData(enableMaps[erx], allERxData[erx], /*passZS, passZSm1, hasToA,*/ charMode, passThrough);  //eRx data for each eRX
          for(size_t ch = 0; ch < erx_chan_data.size(); ++ch){
            dataWords.push_back(erx_chan_data[ch]);
          }
        }

      if(erx == eRxSum-1){
        moduleCountr++;
      }
      if(erx==allERxData.size()-1){
        CalculatedPayloadLength.push_back(dataWords.size());
      }
      //if(eventNum == 290 or eventNum == 366 or eventNum == 520 or eventNum == 739){
      //  cout << "erx: " << std::dec << erx << " dataWords.size(): " << std::dec << dataWords.size() << endl;
      //  for(size_t k = 0; k < dataWords.size(); k++){
      //    cout << std::dec <<  k << "   Word: " << std::hex << dataWords[k] << endl;
      //  }
      //}
  }
//if(eventNum == 290 or eventNum == 366 or eventNum == 520 or eventNum == 739){  
  //for(size_t i = 0; i < CalculatedPayloadLength.size(); ++i){
  //  cout << "Calculated Payload Length: " << CalculatedPayloadLength[i] << " actualPayloadLength: " << payloadLength[i] << endl;
  //}
//}

//ooooooooooOOOOOOOOOOOOOOOOOOOOOOOOO==========================+OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOooooooooooooooooooooooooooo

  for(size_t erx = 0; erx < allERxData.size(); ++erx){   //looping over all eRx

    //packingParameter = packingParm(allERxData[erx].tctp, allERxData[erx].adcm, allERxData[erx].adc, Thresholds[erx].adc_Threshold, Thresholds[erx].adc_Threshold);
    //bool passZSm1 = packingParameter[1];

    uint32_t eRxSum = std::accumulate(nErx.begin(), nErx.begin() + moduleNum, uint32_t{0});
    if(erx == 0 or erx == eRxSum){  
      CRC_calc.clear();

    ///////////////ooooooooooooOOOOOOOOOOOOOOOOO CB Header OOOOOOOOOOOOOOoooooooooooooooooooo////////////////////
      //cout << HGCalMappingModuleIndexer::maxECONDperCB_ << endl;
      if((erx == 0) or (cbIdx[ECOND_counter] != cbIdx[ECOND_counter-1])){
          std::vector<uint8_t> econd_status(12, hgcal::backend::ECONDPacketStatus::InactiveECOND);
          for(size_t enableEcon = 0; enableEcon < enabledECOND[CB_idx]; enableEcon++){
            econd_status[enableEcon]     = hgcal::backend::ECONDPacketStatus::Normal;
            //econd_status[1]     = hgcal::backend::ECONDPacketStatus::Normal;
          }
          
          //econd_status[2 * CB_idx]     = hgcal::backend::ECONDPacketStatus::Normal;
          //econd_status[2 * CB_idx + 1] = hgcal::backend::ECONDPacketStatus::Normal;

          uint32_t evt = eventNum;
          auto cbHeader =
          hgcal::backend::buildCaptureBlockHeader(
              cbBX[CB_idx*2],
              evt,
              cbOrbit[CB_idx*2],
              econd_status);

          uint32_t CAPTUREBLOCK_RESERVED_POS = 25;
          uint32_t CAPTUREBLOCK_RESERVED_MASK = 0x7f;
          uint32_t value = 0x7f;

          cbHeader[0] &= ~(CAPTUREBLOCK_RESERVED_MASK << CAPTUREBLOCK_RESERVED_POS);

          cbHeader[0] |= ((value & CAPTUREBLOCK_RESERVED_MASK) << CAPTUREBLOCK_RESERVED_POS);
          econdPacket.push_back(cbHeader[0]);
          econdPacket.push_back(cbHeader[1]);
          //ECOND_counter = 0;
          CB_idx++;
        }

      passThrough = passThrough1[econdIdx];
      //passThrough = passThrough1;

      auto econdHeader = hgcal::econd::eventPacketHeader(header,
                                                        CalculatedPayloadLength[econdIdx]+1,//payloadLength[econdIdx], 
                                                        passThrough,
                                                        true,
                                                        ht,
                                                        ebo,
                                                        true,
                                                        false,
                                                        ehHam,
                                                        BX[econdIdx],
                                                        L1A[econdIdx],
                                                        Orbit[econdIdx],
                                                        false,
                                                        rr);
        
      econdHeader[1] |= (econCrc & hgcal::ECOND_FRAME::EHCRC_MASK) << hgcal::ECOND_FRAME::EHCRC_POS;

      econdPacket.push_back(econdHeader[0]);  //filling EcondHeaders
      econdPacket.push_back(econdHeader[1]);
      econdIdx++;
      ECOND_counter++;
      moduleNum++;
      }

      bool eRxPresent = std::any_of(
      enableMaps[erx].begin(),
      enableMaps[erx].end(),
      [](bool enabled) { return enabled; }
      );

      if (!eRxPresent) {
          bitE = false;
          //passThrough = false;
          const auto eRxheader = hgcal::econd::eRxSubPacketHeader(stat, Ham, bitE, cm0[erx], cm1[erx], enableMaps[erx]);  //eRx Header for each eRX
          econdPacket.push_back(eRxheader[0]);   //filling ERxHeaders 
          CRC_calc.push_back(eRxheader[0]);  

      } else{
        //bitE = false;
        //passThrough = false;

          const auto eRxheader = hgcal::econd::eRxSubPacketHeader(stat, Ham, bitE, cm0[erx], cm1[erx], enableMaps[erx]);  //eRx Header for each eRX
          econdPacket.push_back(eRxheader[0]);     //filling ERxHeaders 
          econdPacket.push_back(eRxheader[1]);

          CRC_calc.push_back(eRxheader[0]);      
          CRC_calc.push_back(eRxheader[1]);

          const auto erx_chan_data = hgcal::econd::produceERxData(enableMaps[erx], allERxData[erx], /*passZS, passZSm1, hasToA,*/ charMode, passThrough);  //eRx data for each eRX
          for(size_t ch = 0; ch < erx_chan_data.size(); ++ch){
            econdPacket.push_back(erx_chan_data[ch]);   //filling eRx payload(channel data)
            CRC_calc.push_back(erx_chan_data[ch]);
          }
        }

  ////// ooooooooooOOOOOOOOOOOOOOOOO CRC Computation (ECOND Tailer) OOOOOOOOOOOOOOoooooooooooooo /////////////////////////

      if(erx+1 == eRxSum){
        crcvec.assign(CRC_calc.begin(), CRC_calc.end());
        std::transform(crcvec.begin(), crcvec.end(), crcvec.begin(), [](uint32_t w) {
        return ((w << 24) & 0xFF000000) | ((w << 8) & 0x00FF0000) | ((w >> 8) & 0x0000FF00) |
              ((w >> 24) & 0x000000FF);  //swapping endianness
        });

        auto array = &(crcvec[0]);
        auto bytes = reinterpret_cast<const unsigned char *>(array);
        auto crc32 = boost::crc<32,
                             hgcal::ECOND_FRAME::CRC_POL,
                             hgcal::ECOND_FRAME::CRC_INITREM,
                             hgcal::ECOND_FRAME::CRC_FINALXOR,
                             false,
                             //false>(bytes, (payloadLength[econdIdx-1]-1) * 4);
                             false>(bytes, (CalculatedPayloadLength[econdIdx-1]) * 4);  //need to be checked !!!!!


        if((CRC_calc.size() + 1) % 2 != 0){
          econdPacket.push_back(crc32);
          econdPacket.push_back(padding);  //padding word

          //econdPayloadCheck.push_back(crc32);
        }else{
          econdPacket.push_back(crc32);
          //econdPayloadCheck.push_back(crc32);
        }
      }

    }


  
  /*for (size_t i = 0; i + 1 < econdPacket.size(); i += 2) {
  //for (size_t i = 0; i < econdPacket.size(); i++) {

    //if(sizeof(econdPacket[i]) > 4 or sizeof(econdPacket[i]) < 4)
    //  cout << "Word " << i << " Size: " << sizeof(econdPacket[i]) << endl;

    //cout << i+1 << " 32-bit word: "
    //     << std::hex << econdPacket[i]
    //     << "  "
    //     << std::bitset<32>(econdPacket[i])
    //     << std::dec << endl;

    uint64_t word64 =
        static_cast<uint64_t>(econdPacket[i+1]) |
        (static_cast<uint64_t>(econdPacket[i]) << 32);

    cout << "64-bit word: "
         << std::hex << word64
         << "  "
         << std::bitset<64>(word64)
         << std::dec << endl;
  }
  }*/




  /////////////oooooooooooOOOOOOOOOOOO SLink Header OOOOOOOOOOOOOOOOoooooooooooooooo////////////////

size_t payloadBytes = econdPacket.size() * sizeof(uint32_t);

size_t remainder = payloadBytes % 16;
if (remainder != 0) {
    size_t paddingBytes = 16 - remainder;
    // Since each vector element is 4 bytes
    size_t paddingWords = paddingBytes / sizeof(uint32_t);

    for (size_t i = 0; i < paddingWords; ++i) {
        econdPacket.push_back(0);
    }
}

for (size_t i = 0; i + 1 < econdPacket.size(); i += 2) {
    std::swap(econdPacket[i], econdPacket[i + 1]);       // To get the correct sequence of the data wwords in the payload as in original raw data
  }

size_t payloadBytes_afterPadding = econdPacket.size() * sizeof(uint32_t);
size_t totalSize = sizeof(SLinkRocketHeader_v3) + econdPacket.size() * sizeof(uint32_t) + sizeof(SLinkRocketTrailer_v3);
uint32_t sid = 1601;
uint8_t emu_status = 0;
uint16_t l1a_types = 132;
uint8_t l1a_phys = 0;
uint64_t global_event_id = fedL1A[0];
uint16_t status = 4;
uint16_t crc = 57005;
uint16_t daqcrc = 57005;

//unsigned char srcData[totalSize];

constexpr size_t hdrsize = sizeof(SLinkRocketHeader_v3);
constexpr size_t trsize  = sizeof(SLinkRocketTrailer_v3);

//cout << "ECOND packetSize: " << econdPacket.size() << "  payloadBytes: "  << econdPacket.size() * sizeof(uint32_t) << "  totalSize: " << totalSize << endl;

unsigned char slinkPacket[totalSize];
std::memset(slinkPacket, 0, totalSize);
 
auto sh0 = new ((void*)slinkPacket) SLinkRocketHeader_v3(sid, l1a_types, l1a_phys, emu_status, global_event_id);

//std::memcpy(
//    slinkPacket.data() + hdrsize,
//    slinkPayload.data(),
//    payloadBytes);
std::memcpy(
    slinkPacket + hdrsize,
    econdPacket.data(),
    payloadBytes);

auto st0 = new ((void*)(slinkPacket+hdrsize+payloadBytes_afterPadding)) 
    SLinkRocketTrailer_v3(status, crc, fedObt[0], fedBX[0], totalSize >> SLR_WORD_NUM_BYTES_SHIFT, daqcrc);


auto rawDataBuffer = std::make_unique<RawDataBuffer>(totalSize);

rawDataBuffer->addSource(sid, slinkPacket, totalSize);

iEvent.put(rawDataBufferPutToken_, std::move(rawDataBuffer));


}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void rawDataProducer_repacked::beginStream(edm::StreamID) {
  // please remove this method if not needed
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void rawDataProducer_repacked::endStream() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void rawDataProducer_repacked::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.addUntracked<edm::InputTag>(
      "hgcalDigis",
      edm::InputTag("hgcalDigis"));
  desc.add("calibSource", edm::ESInputTag{})->setComment("Label for calibration parameters");

  descriptions.add("rawDataBufferProducer", desc);
}


//define this as a plug-in
DEFINE_FWK_MODULE(rawDataProducer_repacked);
