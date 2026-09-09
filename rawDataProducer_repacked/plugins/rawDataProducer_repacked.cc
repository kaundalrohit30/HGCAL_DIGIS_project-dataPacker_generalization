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

  //std::string tableName_,typeCodeTableName_;

  TTree* tree;

  int eventNum;

  std::vector<uint16_t> tctp ,adc, adcm1 ,tot ,toa ,cm ,flags ,channel ,fedId ,fedReadoutSeq, payloadLength, BX, L1A, fedBX;

  std::vector<int> chI1  ,chI2  ,modI1  ,modI2  ,chType, nDigis, nDenseIndices;

  std::vector<uint8_t> isSiPM, iscalib, Orbit;

  std::vector<uint16_t> cm0, cm1;//, econd_status;

  std::vector<uint32_t> cbBX, cbOrbit, fedObt;

  std::vector<uint64_t> fedL1A;

  std::vector<TH1D*> h_ADC_channel;
  //std::unordered_map<int, TH1F*> pedestalADC_;
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

  
    edm::Service<TFileService> fs;

    for (int mod = 0; mod < 12; ++mod) {
        h_ADC_channel.push_back(
            fs->make<TH1D>(
                Form("ADC_mod%d", mod),
                Form("ADC Distribution module %d;ADC;Counts", mod),
                1024, 0, 1024
            )
        );
    }


  
    tree = new TTree("DIGI_info","HGCal_Digi_Info");

    tree->Branch("eventNum",&eventNum);
    tree->Branch("nDigis",&nDigis);
    tree->Branch("nDenseIndices",&nDenseIndices);
    //tree->Branch("energy",&hit_energy);
    tree->Branch("tctp",&tctp);
    tree->Branch("adc",&adc);
    tree->Branch("adcm1",&adcm1);
    //tree->Branch("detID",&detId);

    tree->Branch("tot",&tot);
    tree->Branch("toa",&toa);
    tree->Branch("cm",&cm);
    tree->Branch("flags",&flags);

    tree->Branch("channel",&channel);
    tree->Branch("fedId",&fedId);
    tree->Branch("fedReadoutSeq",&fedReadoutSeq);
    tree->Branch("payloadLength",&payloadLength);

    tree->Branch("chI1",&chI1);
    tree->Branch("chI2",&chI2);
    tree->Branch("modI1",&modI1);
    tree->Branch("modI2",&modI2);

    tree->Branch("isSiPM",&isSiPM);
    tree->Branch("iscalib",&iscalib);
    tree->Branch("BX",&BX);
    tree->Branch("L1A",&L1A);
    tree->Branch("Orbit",&Orbit);

    tree->Branch("cm0",&cm0);
    tree->Branch("cm1",&cm1);

  //register your products
  /* Examples
  produces<ExampleData2>();

  //if do put with a label
  produces<ExampleData2>("label");
 
  //if you want to put into the Run
  produces<ExampleData2,InRun>();
  */
  //now do what ever other initialization is needed
}

rawDataProducer_repacked::~rawDataProducer_repacked() {
  // do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void rawDataProducer_repacked::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  using namespace std;
  
  eventNum = iEvent.id().event();
  nDigis.clear();
  nDenseIndices.clear();
  tctp.clear();
  adc.clear();
  adcm1.clear();
  tot.clear();
  toa.clear();
  cm.clear();
  flags.clear();
  channel.clear();
  fedId.clear();
  fedReadoutSeq.clear();
  payloadLength.clear();
  chI1.clear();
  chI2.clear();
  modI1.clear();
  modI2.clear();
  chType.clear();
  isSiPM.clear();
  iscalib.clear();
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
  //econd_status.clear();
  //allDataWords.clear();

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
    cout << "ndigis: " << ndigis << "  ndenseIndices: " << ndenseIndices << endl;

    
    nDenseIndices.push_back(ndenseIndices);
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


  uint32_t nmodules = moduleIndex.maxModulesCount();
  std::vector<uint32_t> fed(nmodules), seq(nmodules), nErx(nmodules);
  //typedef std::pair<std::string, std::vector<uint32_t> > TypeCode2Idx_t;
  //std::vector<TypeCode2Idx_t> typeCodeTreeIndices;
  uint32_t idx(0);
  for(auto it : moduleIndex.typecodeMap() ) {

      //std::string typecode = it.first;
      //std::replace( typecode.begin(), typecode.end(), '-', '_');
      //typeCodeTreeIndices.push_back( TypeCode2Idx_t(typecode, {idx} ) );

      fed[idx] = it.second.first;
      seq[idx] = it.second.second;
      nErx[idx] = moduleIndex.getNumERxs(fed[idx], seq[idx]);
      //std::cout << "nErx: " << moduleIndex.getNumERxs(fed[idx], seq[idx]) << std::endl;
      idx++;

      //
    }

  std::vector<hgcal::econd::ERxData> allERxData;//(72);
  std::vector<hgcal::econd::ERxChannelEnable> enableMaps;//(72, hgcal::econd::ERxChannelEnable(37,false));
  //hgcal::econd::ERxChannelEnable enableMap(37,false);
  //enableMaps.push_back(enableMap);

  const std::vector<uint8_t> econd_status(12, hgcal::backend::ECONDPacketStatus::Normal); //for CB
  
  int eRxNum = 0;
  int TotalERx =  std::accumulate(nErx.begin(), nErx.end(), uint32_t{0});
  int chIdx = 0;
  int previousERx = -1;
  //std::vector<uint16_t> moduleIdx;
  //int moduleIdxCounter = 0;
  
  uint16_t modulNum[12] = {0,1,2,3,4,5,6,7,8,9,10,11};

  for (int32_t i = 0; i < ndenseIndices && digis.isValid(); i++) {

    //if((i == 0) or (i/37 != (i-1)/37)){
    //  moduleIdx.push_back(denseIndexInfo_view.fedReadoutSeq()[i]);
    //  //cout << " idx: " << moduleIdx.size()-1 << " moduleIdx: " << denseIndexInfo_view.fedReadoutSeq()[i] << endl;
    //}
    if ((digis_view.flags()[i] == hgcal::DIGI_FLAG::NotAvailable)) {
        continue;
    }

    tctp.push_back(digis_view.tctp()[i]);
    adc.push_back(digis_view.adc()[i]);
    adcm1.push_back(digis_view.adcm1()[i]);
    tot.push_back(digis_view.tot()[i]);
    toa.push_back(digis_view.toa()[i]);
    cm.push_back(digis_view.cm()[i]);
    flags.push_back(digis_view.flags()[i]);
    channel.push_back(denseIndexInfo_view.chNumber()[i]);
    fedId.push_back(denseIndexInfo_view.fedId()[i]);
    fedReadoutSeq.push_back(denseIndexInfo_view.fedReadoutSeq()[i]);
    uint32_t cellInfoIdx(denseIndexInfo_view.cellInfoIdx()[i]);
    chType.push_back(cellInfo_view.t()[cellInfoIdx]);
    chI1.push_back(cellInfo_view.i1()[cellInfoIdx]);
    chI2.push_back(cellInfo_view.i2()[cellInfoIdx]);   
    uint32_t modInfoIdx(denseIndexInfo_view.modInfoIdx()[i]);
    modI1.push_back(moduleInfo_view.i1()[modInfoIdx]);
    modI2.push_back(moduleInfo_view.i2()[modInfoIdx]); 
    isSiPM.push_back((uint8_t) moduleInfo_view.isSiPM()[modInfoIdx]);
    iscalib.push_back(cellInfo_view.iscalib()[cellInfoIdx]);
     
    nGoodDigis++;
  
    eRxNum = i / 37;
    chIdx  = denseIndexInfo_view.chNumber()[i] % 37;
    int channel = denseIndexInfo_view.chNumber()[i];

    if(denseIndexInfo_view.fedReadoutSeq()[i] == modulNum[denseIndexInfo_view.fedReadoutSeq()[i]]){

    //if (channel >= 0 && channel < static_cast<int>(h_ADC_channel.size())) {
    //    h_ADC_channel[channel]->Fill(digis_view.adc()[i]);
    //}
      h_ADC_channel[denseIndexInfo_view.fedReadoutSeq()[i]]->Fill(digis_view.adc()[i]);
    }

    while (allERxData.size() <= static_cast<size_t>(eRxNum)) {
          //cout << "vecIdx: " << allERxData.size() << " moduleIdx: " << denseIndexInfo_view.fedReadoutSeq()[i] << endl;
        hgcal::econd::ERxData erxData;
        erxData.tctp.resize(37, 0);
        erxData.adc.resize(37, 0);
        erxData.adcm.resize(37, 0);
        erxData.toa.resize(37, 0);
        erxData.tot.resize(37, 0);
        allERxData.push_back(std::move(erxData));
        
        //allERxData.emplace_back();
        enableMaps.emplace_back(37, false);
        //moduleIdxCounter++;
    }

    //if (eRxNum != previousERx) {
    //    allERxData.emplace_back();
    //    enableMaps.emplace_back(37, false);
//
    //    previousERx = eRxNum;
    //}

    // Current eRx is the last element
    //size_t idx = allERxData.size() - 1;

    //cout << i << "  eRxNum: " << i << "/" << 37 << " = " <<  eRxNum << "  chId " << chIdx << "  FilledIdx: " << idx << endl;
    //cout << i << "  eRxNum: " << eRxNum << " eRxDataSize: " << allERxData.size()  << "  chId " << chIdx << endl;
    
    //cout << "vecIdx: " << eRxNum << " moduleIdx: " << denseIndexInfo_view.fedReadoutSeq()[i] << endl;
    enableMaps.at(eRxNum).at(chIdx) = true;
    allERxData.at(eRxNum).tctp.at(chIdx) = digis_view.tctp()[i];
    allERxData.at(eRxNum).adc.at(chIdx) = digis_view.adc()[i];
    allERxData.at(eRxNum).adcm.at(chIdx) = digis_view.adcm1()[i];
    allERxData.at(eRxNum).toa.at(chIdx) = digis_view.toa()[i];
    allERxData.at(eRxNum).tot.at(chIdx) = digis_view.tot()[i];
    //moduleIdxCounter++;
    //allERxData[eRxNum].tctp.push_back(digis_view.tctp()[i]);
    //allERxData[eRxNum].adc.push_back(digis_view.adc()[i]);
    //allERxData[eRxNum].adcm.push_back(digis_view.adcm1()[i]);
    //allERxData[eRxNum].toa.push_back(digis_view.toa()[i]);
    //allERxData[eRxNum].tot.push_back(digis_view.tot()[i]);
      
    /*cout << std::left
     << std::setw(10) << static_cast<unsigned int>(digis_view.tctp()[i])
     << std::setw(10) << digis_view.adc()[i]
     << std::setw(10) << digis_view.adcm1()[i]
     << std::setw(10) << digis_view.tot()[i]
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
     << '\n';*/
    }
    //}

    if(allERxData.size() != static_cast<size_t>(TotalERx)){
      while (allERxData.size() <= static_cast<size_t>(TotalERx)){
          //cout << "vecIdx: " << allERxData.size() << " moduleIdx: " << denseIndexInfo_view.fedReadoutSeq()[i] << endl;
        hgcal::econd::ERxData erxData;
        erxData.tctp.resize(37, 0);
        erxData.adc.resize(37, 0);
        erxData.adcm.resize(37, 0);
        erxData.toa.resize(37, 0);
        erxData.tot.resize(37, 0);
        allERxData.push_back(std::move(erxData));
        
        //allERxData.emplace_back();
        enableMaps.emplace_back(37, false);
        //moduleIdxCounter++;
    }
    }
  
    cout << "nGoodDigis: " << nGoodDigis << endl;
    cout << endl;

    nDigis.push_back(nGoodDigis);

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
      //cout << "module: " << imod << "  CBBX: " << econd.CBBX() << endl;
    //}
    //econd_status.push_back(econd.cbFlag());


    //cout << "nEcond = " << imod << "  Payload Length: " << econd.payloadLength() <<  "  BX = " << econd.BX() << "  L1A = " << static_cast<unsigned int>(econd.L1A()) << "  Orbit = " << static_cast<unsigned int>(econd.Orbit()) << endl;

    for(size_t ierx=0; ierx<6; ierx++){
      //cmsums[ierx][imod] = econd.cm().coeff(ierx,0) + econd.cm().coeff(ierx,1);
      //cout << "eRx" << ierx << "  ECOND flag: " << static_cast<unsigned int>(econd.econdFlag()) << "   CM: "  << econd.cm().coeff(ierx,0) << "  " << econd.cm().coeff(ierx,1) << endl;
      //if(econd.cm().coeff(ierx,0) <= 1024 and econd.cm().coeff(ierx,1) <= 1024){
        cm0.push_back(econd.cm().coeff(ierx,0));
        cm1.push_back(econd.cm().coeff(ierx,1));
      //}

      //cout << "ierx: " << ierx << "  cm0: " << econd.cm().coeff(ierx,0) << "  cm1: " << econd.cm().coeff(ierx,1) << "  cmSum: " << econd.cm().coeff(ierx,0) + econd.cm().coeff(ierx,1) <<  endl;
    }

  }
  }


    //for (size_t erx = 0; erx < allERxData.size(); ++erx) {
//
    //  cout << "\neRx = " << erx << " eRXDataSize: " << enableMaps[erx].size() << endl;
//
    //  for (size_t ch = 0; ch < enableMaps[erx].size(); ++ch) {
//
    //    cout << "channel status: " << ch << "  enabled: " << enableMaps[erx][ch] << endl;
    //      //cout << "channel " << ch  
    //      //     << " ADC = " << allERxData[erx].adc[ch]
    //      //     << " ADCm = " << allERxData[erx].adcm[ch]
    //      //     << " TOA = " << allERxData[erx].toa[ch]
    //      //     << " TOT = " << allERxData[erx].tot[ch]
    //      //     << " TCTP = " << static_cast<unsigned int>(allERxData[erx].tctp[ch])
    //      //     << endl;
    //  }
    //}

  tree->Fill();
  ///////////oooooooooooOOOOOOOOOOOOO FED Info OOOOOOOOOOOOOOOoooooooooooooooooo////////////////////
  int32_t nfed = 0;
  if(fedInfo.isValid()){
    nfed = fedInfo->const_view().metadata().size();
    //assert(ndigis == ndenseIndices);
  }

  //cout << "nfed: " << nfed << endl;
  for(int i = 0; i < nfed && fedInfo.isValid(); i++){
    const auto fed = fedInfo_view[i];
    if(fed.FEDPayload() != 0){
      fedBX.push_back(fed.FEDBX());
      fedL1A.push_back(fed.FEDL1A());
      fedObt.push_back(fed.FEDOrbit());
      //cout << "FED Payload: " << fed.FEDPayload() << "  FED BX: " << fed.FEDBX() << "  FED L1A: " << fed.FEDL1A() << "  FED Orbit: " << fed.FEDOrbit() << endl;
    }
  }






  /////////////// eRx payload + header generation   (ECOND packet inside CB) ///////////////

  std::vector<uint32_t> econdPacket{0};  
  std::vector<uint32_t> CRC_calc{0};
  std::vector<uint32_t> crcvec;
  econdPacket.clear();
  int econdIdx = 0;
  uint16_t header =  340;//170;
  bool passThrough = false;
  uint8_t ht = 0;
  uint8_t ebo = 0;
  uint8_t ehHam = 9;
  uint8_t rr = 0;
  uint8_t stat = 7;
  uint8_t Ham = 0;
  bool bitE = true;
  bool passZS = true;
  bool passZSm1 = true; 
  bool hasToA = false;
  bool charMode = false;
  uint8_t econCrc = 244;
  int ECOND_counter = 0;
  int CB_idx = 0;
  uint32_t padding = 0;
  int moduleNum = 0;

  for(size_t erx = 0; erx < allERxData.size(); ++erx){   //looping over all eRx

    bool alleRxPresent = std::all_of(enableMaps[erx].begin(),
                enableMaps[erx].end(),
                [](bool enabled) { return enabled; });
    if(alleRxPresent){
      passThrough = true;
    }
    else 
      passThrough = false;

    uint32_t eRxSum = std::accumulate(nErx.begin(), nErx.begin() + moduleNum, uint32_t{0});
    
    if(erx == 0 or erx == eRxSum){  
      CRC_calc.clear();

  ///////////////ooooooooooooOOOOOOOOOOOOOOOOO CB Header OOOOOOOOOOOOOOoooooooooooooooooooo////////////////////

    if(ECOND_counter % 2 == 0){ // 2 ECOND per CB
        std::vector<uint8_t> econd_status(12, hgcal::backend::ECONDPacketStatus::InactiveECOND);

        econd_status[2 * CB_idx]     = hgcal::backend::ECONDPacketStatus::Normal;
        econd_status[2 * CB_idx + 1] = hgcal::backend::ECONDPacketStatus::Normal;

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



      auto econdHeader = hgcal::econd::eventPacketHeader(header,
                                                              payloadLength[econdIdx], 
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

      //cout << "ECONDCRC: " << std::hex << (econCrc & hgcal::ECOND_FRAME::EHCRC_MASK) << hgcal::ECOND_FRAME::EHCRC_POS << std::dec << endl;
      //cout << "ECOND header 1: " << std::hex << econdHeader[1] << std::dec << endl;

      econdPacket.push_back(econdHeader[0]);  //filling EcondHeaders
      econdPacket.push_back(econdHeader[1]);
      econdIdx++;
      ECOND_counter++;
      //eRxSum += nErx[moduleNum];
      //cout << "eRxSum: " << eRxSum << endl;
      moduleNum++;

      //CRC_calc.push_back(econdHeader[0]);
      //CRC_calc.push_back(econdHeader[1]);

    }

    bool eRxPresent = std::any_of(
    enableMaps[erx].begin(),
    enableMaps[erx].end(),
    [](bool enabled) { return enabled; }
    );

    // All 37 channels are true
    
    if(passThrough == true){
      //bitE = false;
      //passThrough = true;
      const auto eRxheader = hgcal::econd::eRxSubPacketHeader(stat, Ham, bitE, cm0[erx], cm1[erx], enableMaps[erx]);  //eRx Header for each eRX
      //cout << "cm0: " << cm0[erx] << "  cm1: " << cm1[erx] << endl;
      econdPacket.push_back(eRxheader[0]);     //filling ERxHeaders 
      econdPacket.push_back(eRxheader[1]);

      CRC_calc.push_back(eRxheader[0]);      
      CRC_calc.push_back(eRxheader[1]);

      const auto erx_chan_data = hgcal::econd::produceERxData(enableMaps[erx], allERxData[erx], passZS, passZSm1, hasToA, charMode, passThrough);  //eRx data for each eRX
      ///int i = 0;
      //const auto [word, nbits]
      for(size_t ch = 0; ch < erx_chan_data.size(); ++ch){
        econdPacket.push_back(erx_chan_data[ch]);   //filling eRx payload(channel data)
        CRC_calc.push_back(erx_chan_data[ch]);
//
      }
    }
    else if (!eRxPresent) {
        bitE = false;
        //passThrough = false;
        const auto eRxheader = hgcal::econd::eRxSubPacketHeader(stat, Ham, bitE, cm0[erx], cm1[erx], enableMaps[erx]);  //eRx Header for each eRX
        econdPacket.push_back(eRxheader[0]);   //filling ERxHeaders 
        CRC_calc.push_back(eRxheader[0]);  
    } else{
      //bitE = false;
      //passThrough = false;
      
        const auto eRxheader = hgcal::econd::eRxSubPacketHeader(stat, Ham, bitE, cm0[erx], cm1[erx], enableMaps[erx]);  //eRx Header for each eRX
      //cout << "cm0: " << cm0[erx] << "  cm1: " << cm1[erx] << endl;
      econdPacket.push_back(eRxheader[0]);     //filling ERxHeaders 
      econdPacket.push_back(eRxheader[1]);

      CRC_calc.push_back(eRxheader[0]);      
      CRC_calc.push_back(eRxheader[1]);

      const auto erx_chan_data = hgcal::econd::produceERxData(enableMaps[erx], allERxData[erx], passZS, passZSm1, hasToA, charMode, passThrough);  //eRx data for each eRX
      ///int i = 0;
      //const auto [word, nbits]
      for(size_t ch = 0; ch < erx_chan_data.size(); ++ch){
        econdPacket.push_back(erx_chan_data[ch]);   //filling eRx payload(channel data)
        CRC_calc.push_back(erx_chan_data[ch]);
//
      }
    }

//////// ooooooooooOOOOOOOOOOOOOOOOO CRC Computation (ECOND Tailer) OOOOOOOOOOOOOOoooooooooooooo /////////////////////////

    if(erx+1 == eRxSum){
      //cout << nextModuleIdx << "  " << currentModuleIdx << "  " << payloadLength[econdIdx-1] << endl;//"   " << std::hex << crc32 << std::dec << endl;
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
                           false>(bytes, (payloadLength[econdIdx-1] - 1) * 4);  //need to be checked !!!!!

      
      if((CRC_calc.size() + 1) % 2 != 0){
        econdPacket.push_back(crc32);
        econdPacket.push_back(padding);  //padding word
      }else
        econdPacket.push_back(crc32);

    }
    
  }



  ////int counterECON = 0;
  //for (size_t i = 0; i + 1 < econdPacket.size(); i += 2) {
  ////for (size_t i = 0; i < econdPacket.size(); i++) {
//
  //  //if(sizeof(econdPacket[i]) > 4 or sizeof(econdPacket[i]) < 4)
  //  //  cout << "Word " << i << " Size: " << sizeof(econdPacket[i]) << endl;
//
  //  //cout << i+1 << " 32-bit word: "
  //  //     << std::hex << econdPacket[i]
  //  //     << "  "
  //  //     << std::bitset<32>(econdPacket[i])
  //  //     << std::dec << endl;
//
  //  uint64_t word64 =
  //      static_cast<uint64_t>(econdPacket[i+1]) |
  //      (static_cast<uint64_t>(econdPacket[i]) << 32);
//
  //  cout << "64-bit word: "
  //       << std::hex << word64
  //       << "  "
  //       << std::bitset<64>(word64)
  //       << std::dec << endl;
  //}

  //cout << "ECOND packetSize before: " << econdPacket.size() << endl;


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
uint64_t global_event_id = fedL1A[0];//eventNum;
uint16_t status = 4;
uint16_t crc = 57005;
uint16_t daqcrc = 57005;

//unsigned char srcData[totalSize];

constexpr size_t hdrsize = sizeof(SLinkRocketHeader_v3);
constexpr size_t trsize  = sizeof(SLinkRocketTrailer_v3);

cout << "ECOND packetSize: " << econdPacket.size() << "  payloadBytes: "  << econdPacket.size() * sizeof(uint32_t) << "  totalSize: " << totalSize << endl;

unsigned char slinkPacket[totalSize];
std::memset(slinkPacket, 0, totalSize);
//
//auto sh0 = new ((void*)slinkPacket.data()) SLinkRocketHeader_v3(sid, l1a_types, l1a_phys, emu_status, global_event_id);  
auto sh0 = new ((void*)slinkPacket) SLinkRocketHeader_v3(sid, l1a_types, l1a_phys, emu_status, global_event_id);

//std::memcpy(
//    slinkPacket.data() + hdrsize,
//    slinkPayload.data(),
//    payloadBytes);
std::memcpy(
    slinkPacket + hdrsize,
    econdPacket.data(),
    payloadBytes);

//auto st0 = new ((void*)(slinkPacket.data()+hdrsize+payloadBytes)) 
//    SLinkRocketTrailer_v3(status, crc, fedObt[global_event_id-1], fedBX[global_event_id-1], totalSize >> SLR_WORD_NUM_BYTES_SHIFT, daqcrc);
auto st0 = new ((void*)(slinkPacket+hdrsize+payloadBytes_afterPadding)) 
    SLinkRocketTrailer_v3(status, crc, fedObt[0], fedBX[0], totalSize >> SLR_WORD_NUM_BYTES_SHIFT, daqcrc);



//const uint32_t* words =
//    reinterpret_cast<const uint32_t*>(slinkPacket);
//
//for (size_t i = 0; i < totalSize/4; ++i) {
//    std::cout << "Word " << i << " = 0x"
//              << std::hex << std::setw(8) << std::setfill('0')
//              << words[i] << std::dec << '\n';
//}

auto rawDataBuffer = std::make_unique<RawDataBuffer>(totalSize);

rawDataBuffer->addSource(sid, slinkPacket, totalSize);

//auto const& fragData0 = rawDataBuffer->fragmentData(sid);
//cout << "fragment size = " << fragData0.size() << endl;
//assert(fragData0.size());
//auto hdrView0 = makeSLinkRocketHeaderView(fragData0.dataHeader(hdrsize));
//auto trlView0 = makeSLinkRocketTrailerView(fragData0.dataTrailer(trsize), hdrView0->version());

iEvent.put(rawDataBufferPutToken_, std::move(rawDataBuffer));


/*
  //////// ooooooooooOOOOOOOOOOOOOOOOO CRC Computation (ECOND Tailer) OOOOOOOOOOOOOOoooooooooooooo /////////////////////////
  int payloadCounter = 0;
  int payloadLength = 236; //with econdheaders, no trailer
  std::vector<uint32_t> CRCEcondPacket;
  std::vector<uint32_t> CRC;
  std::vector<uint32_t> crcvec;
  crcvec.clear();
  for(int i = 0; i < 12; i++){
    CRCEcondPacket.clear();
    for(int j = payloadCounter; j < payloadCounter+payloadLength; j++){
      CRCEcondPacket.push_back(econdPacket[j]);
      //cout << std::hex << econdPacket[i] << std::dec << endl;
    }
    //cout << endl;
  //uint32_t crc =
  //  econd_crc32(
  //      reinterpret_cast<const uint8_t*>(CRCEcondPacket.data() + 2),
  //      (CRCEcondPacket.size() - 2) * sizeof(uint32_t)
  //);


  crcvec.assign(CRCEcondPacket.begin() + 2, CRCEcondPacket.end());
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
                           false>(bytes, (235 - 1) * 4);

  CRC.push_back(crc32);
  //cout << std::hex << crc << std::dec << endl;
  payloadCounter += payloadLength;
      
    }


std::vector<uint32_t> econdPacketsWithTrailer;
constexpr size_t econdPacketLength = 236;   // words, excluding trailer
size_t packetStart = 0;
uint32_t paddingWord = 0;

for (size_t i = 0; i < 12; ++i) {

    // Copy one ECON-D packet
    econdPacketsWithTrailer.insert(
        econdPacketsWithTrailer.end(),
        econdPacket.begin() + packetStart,
        econdPacket.begin() + packetStart + econdPacketLength);

    // Append its trailer
    econdPacketsWithTrailer.push_back(CRC[i]);
    //econdPacketsWithTrailer.push_back(crcvec[i]);
    econdPacketsWithTrailer.push_back(paddingWord);

    packetStart += econdPacketLength;

    //cout << "CRC" << i << ": " << std::hex << CRC[i] << std::dec << endl;
}



//int count = 0;
//for(size_t i = 0; i < econdPacketsWithTrailer.size(); i++){
//    
//    if(i%238 == 0){
//      cout <<  endl;
//      cout << "Econd " << count << endl;//"   " <<  i << "  " << i%234 << endl;
//      count++;
//    }  
//    cout << "idx: " << i%238 << "  Word: " << std::hex << econdPacketsWithTrailer[i] << std::dec << endl;
//}

  


  //std::vector<std::vector<uint32_t>> erxPayloads(72);
//
  //for(int erx = 0; erx < 72; ++erx)
  //{
  //    int start = erx * 37;
//
  //    erxPayloads[erx].assign(
  //        allDataWords.begin() + start,
  //        allDataWords.begin() + start + 37
  //    );
  //}


  ///////////////ooooooooooooOOOOOOOOOOOOOOOOO CB Header & SLink PAyload OOOOOOOOOOOOOOoooooooooooooooooooo////////////////////

  //uint32_t bx = BX[0];
  //uint32_t obt = Orbit[0];
  std::vector<uint32_t> slinkPayload{0};
  slinkPayload.clear();
  int counter = 0;
  int twoEcondlnth = 476;//474;  // data words in 2 econds  (including 2 ECOND header + 235 ECOND payload + 1 Padding word)*2)

  for (unsigned icb = 0; icb < 6; ++icb) {

  std::vector<uint8_t> econd_status(12, hgcal::backend::ECONDPacketStatus::InactiveECOND);

  econd_status[2 * icb]     = hgcal::backend::ECONDPacketStatus::Normal;
  econd_status[2 * icb + 1] = hgcal::backend::ECONDPacketStatus::Normal;

  uint32_t evt = eventNum;

  auto cbHeader =
      hgcal::backend::buildCaptureBlockHeader(
          cbBX[evt-1],
          evt,
          cbOrbit[evt-1],
          econd_status);
          
  uint32_t CAPTUREBLOCK_RESERVED_POS = 25;
  uint32_t CAPTUREBLOCK_RESERVED_MASK = 0x7f;
  uint32_t value = 0x7f;

  cbHeader[0] &= ~(CAPTUREBLOCK_RESERVED_MASK << CAPTUREBLOCK_RESERVED_POS);

  cbHeader[0] |= ((value & CAPTUREBLOCK_RESERVED_MASK) << CAPTUREBLOCK_RESERVED_POS);

  slinkPayload.push_back(cbHeader[0]);
  slinkPayload.push_back(cbHeader[1]);
  
  slinkPayload.insert(
    slinkPayload.end(),
    econdPacketsWithTrailer.begin() + counter,
    econdPacketsWithTrailer.begin() + counter + twoEcondlnth
  );

  counter += twoEcondlnth;
 //cout << "cbIdx: " << icb << "  cbHeader0: " << std::hex << cbHeader[0] << "   cbHeader1: " << cbHeader[1] << std::dec << "  cbBX: " << cbBX[0] << 
  //"  event: " << evt << "  cbOrbit: " << cbOrbit[0] << endl;

  //cout << "cbHeader0: " << std::hex << cbHeader[0]
  //     << "  " << std::bitset<32>(cbHeader[0]) << ",  cbHeader1: " << cbHeader[1]
  //     << "  " << std::bitset<32>(cbHeader[1]) << std::dec << endl;
  //  "  cbBX: " << cbBX[0] << 
  //    "  event: " << evt << "  cbOrbit: " << cbOrbit[0] <<  endl;

}

//for(size_t i = 0; i < slinkPayload.size(); i++){
//    cout << "idx: " << i << "  Word: " << std::hex << slinkPayload[i] << std::dec << std::endl;
//}


for (size_t i = 0; i + 1 < slinkPayload.size(); i += 2) {
    std::swap(slinkPayload[i], slinkPayload[i + 1]);       // To get the correct sequence of the data wwords in the payload as in original raw data
}


/////////////oooooooooooOOOOOOOOOOOO SLink Header OOOOOOOOOOOOOOOOoooooooooooooooo////////////////

size_t payloadBytes = slinkPayload.size() * sizeof(uint32_t);
size_t totalSize = sizeof(SLinkRocketHeader_v3) + payloadBytes + sizeof(SLinkRocketTrailer_v3);
uint32_t sid = 1601;
uint8_t emu_status = 0;
uint16_t l1a_types = 132;
uint8_t l1a_phys = 0;
uint64_t global_event_id = fedL1A[0];//eventNum;
uint16_t status = 4;
uint16_t crc = 57005;
uint16_t daqcrc = 57005;

//unsigned char srcData[totalSize];

constexpr size_t hdrsize = sizeof(SLinkRocketHeader_v3);
constexpr size_t trsize  = sizeof(SLinkRocketTrailer_v3);


unsigned char slinkPacket[totalSize];
std::memset(slinkPacket, 0, totalSize);

//auto sh0 = new ((void*)slinkPacket.data()) SLinkRocketHeader_v3(sid, l1a_types, l1a_phys, emu_status, global_event_id);  
auto sh0 = new ((void*)slinkPacket) SLinkRocketHeader_v3(sid, l1a_types, l1a_phys, emu_status, global_event_id);  

//uint64_t Slink_vPos = 52;
//uint64_t Slink_vMask = 0xf;
//uint64_t value = 2;
////
//auto* words = reinterpret_cast<uint64_t*>(sh0);

//cout << "1 SlinkHeader1: " << std::hex << words[1] << "   " << std::bitset<64>(words[1]) << "  SlinkHeader0: " << words[0] << "   " << std::bitset<64>(words[0]) << std::dec << endl;

//cout << "Size: " << sizeof(*words) << "  " << sizeof(*sh0) << endl;
//cout << words << "   " << sh0 << endl;
//words[1] &= ~(Slink_vMask << Slink_vPos);
//cout << "2 SlinkHeader1: " << std::hex << words[1] << "   " << std::bitset<64>(words[1]) << std::dec << endl;
//words[1] |= (value << Slink_vPos);
//
//cout << "3 SlinkHeader1: " << std::hex << words[1] << "   " << std::bitset<64>(words[1]) << std::dec << endl;

//std::memcpy(
//    slinkPacket.data() + hdrsize,
//    slinkPayload.data(),
//    payloadBytes);
std::memcpy(
    slinkPacket + hdrsize,
    slinkPayload.data(),
    payloadBytes);

//auto st0 = new ((void*)(slinkPacket.data()+hdrsize+payloadBytes)) 
//    SLinkRocketTrailer_v3(status, crc, fedObt[global_event_id-1], fedBX[global_event_id-1], totalSize >> SLR_WORD_NUM_BYTES_SHIFT, daqcrc);
auto st0 = new ((void*)(slinkPacket+hdrsize+payloadBytes)) 
    SLinkRocketTrailer_v3(status, crc, fedObt[0], fedBX[0], totalSize >> SLR_WORD_NUM_BYTES_SHIFT, daqcrc);



//const uint32_t* words =
//    reinterpret_cast<const uint32_t*>(slinkPacket);
//
//for (size_t i = 0; i < totalSize/4; ++i) {
//    std::cout << "Word " << i << " = 0x"
//              << std::hex << std::setw(8) << std::setfill('0')
//              << words[i] << std::dec << '\n';
//}

auto rawDataBuffer = std::make_unique<RawDataBuffer>(totalSize);

rawDataBuffer->addSource(sid, slinkPacket, totalSize);

//auto const& fragData0 = rawDataBuffer->fragmentData(sid);
//cout << "fragment size = " << fragData0.size() << endl;
//assert(fragData0.size());
//auto hdrView0 = makeSLinkRocketHeaderView(fragData0.dataHeader(hdrsize));
//auto trlView0 = makeSLinkRocketTrailerView(fragData0.dataTrailer(trsize), hdrView0->version());

iEvent.put(rawDataBufferPutToken_, std::move(rawDataBuffer));*/


}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void rawDataProducer_repacked::beginStream(edm::StreamID) {
  // please remove this method if not needed
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void rawDataProducer_repacked::endStream() {
  // please remove this method if not needed
}

// ------------ method called when starting to processes a run  ------------
/*
void
rawDataBufferProducer::beginRun(edm::Run const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when ending the processing of a run  ------------
/*
void
rawDataBufferProducer::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when starting to processes a luminosity block  ------------
/*
void
rawDataBufferProducer::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
rawDataBufferProducer::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void rawDataProducer_repacked::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.addUntracked<edm::InputTag>(
      "hgcalDigis",
      edm::InputTag("hgcalDigis"));

  descriptions.add("rawDataBufferProducer", desc);
}


//define this as a plug-in
DEFINE_FWK_MODULE(rawDataProducer_repacked);
