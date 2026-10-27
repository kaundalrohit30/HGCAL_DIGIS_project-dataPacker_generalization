import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing

configdir = "/eos/home-r/rkaundal/Rohit/HGCAL_git_nw/HGCAL_DIGIS_project/HGCal_Digis_generalization/Cleaned_code/CMSSW_20_1_0_pre2/src/HGCalCommissioning/Calibrations"

options = VarParsing('python')
options.register('verbosity',0,mytype=VarParsing.varType.int,
                 info='Tester verbosity: 0 = Base+FED prints / 1 = +Module prints / 2 = +Cell prints')
options.register('modules','HGCalCommissioning/Calibrations/TB2026/maps/modulelocator_v4.txt',mytype=VarParsing.varType.string,
                 info="Path to module mapper. Absolute, or relative to CMSSW src directory")

#options.register('modules','HGCalCommissioning/Calibrations/TB2025/maps/modulelocator_v2.txt',mytype=VarParsing.varType.string,
#                 info="Path to module mapper. Absolute, or relative to CMSSW src directory")


#options.register('sicells','Geometry/HGCalMapping/data/CellMaps/WaferCellMapTraces.txt',mytype=VarParsing.varType.string,
options.register('sicells','HGCalCommissioning/Calibrations/P5/maps/WaferCellMapTraces.txt',mytype=VarParsing.varType.string,
                 info="Path to Si cell mapper. Absolute, or relative to CMSSW src directory")
#options.register('sicells','Geometry/HGCalMapping/data/CellMaps/WaferCellMapTraces.txt',mytype=VarParsing.varType.string,
#                 info="Path to Si cell mapper. Absolute, or relative to CMSSW src directory")

options.register('sipmcells','HGCalCommissioning/Calibrations/TB2026/maps/channels_sipmontile_tb.hgcal.txt',mytype=VarParsing.varType.string,
                 info="Path to SiPM-on-tile cell mapper. Absolute, or relative to CMSSW src directory")
#options.register('sipmcells','Geometry/HGCalMapping/data/CellMaps/channels_sipmontile.hgcal.txt',mytype=VarParsing.varType.string,
#                 info="Path to SiPM-on-tile cell mapper. Absolute, or relative to CMSSW src directory")

options.register('offsetfile','Geometry/HGCalMapping/data/CellMaps/calibration_to_surrounding_offsetMap.txt',mytype=VarParsing.varType.string,
                 info="Path to calibration-to-surrounding cell offset file. Absolute, or relative to CMSSW src directory")

options.register('params',
                 #f"{configdir}/level0_calib_Relay1727210224.json",
                 f"{configdir}/TB2026/calib/level0_calib_118361.json",
                 mytype=VarParsing.varType.string,
                 info="Path to calibration parameters (JSON format)")
options.register('energyloss',
                 f"{configdir}/EnergyLoss/hgcal_energyloss_v16.json",
                 mytype=VarParsing.varType.string,
                 info="Path to calibration parameters (JSON format)")
options.parseArguments()

process = cms.Process("HGC")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.EventContent.EventContent_cff')
process.load("FWCore.MessageService.MessageLogger_cfi")

process.load("Configuration.Geometry.GeometryExtendedRun4D104Reco_cff")
process.load("Configuration.Geometry.GeometryExtendedRun4D104_cff")
from Geometry.HGCalMapping.hgcalmapping_cff import customise_hgcalmapper
process = customise_hgcalmapper(process)

kwargs = { k: getattr(options,k) for k in ['modules','sicells','sipmcells','offsetfile'] if getattr(options,k)!='' }
process = customise_hgcalmapper(process, **kwargs)

#from Configuration.Eras.Era_Phase2C17I13M9_cff import Phase2C17I13M9 as Era_Phase2
#process = cms.Process('hgcalrechit::HGCalCalibrationESProducer@alpaka',Era_Phase2)

process.hgcalCalibParamESProducer = cms.ESProducer( # ESProducer to load calibration parameters from JSON file, like pedestals
  'hgcalrechit::HGCalCalibrationESProducer@alpaka',
  filename=cms.FileInPath(options.params),
  filenameEnergyLoss=cms.FileInPath(options.energyloss),
  #indexSource=cms.ESInputTag('hgCalMappingESProducer',''),
  #mapSource=cms.ESInputTag('hgCalMappingModuleESProducer','')
)


# Input file
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        #"file:/eos/user/r/rkaundal/Rohit/HGCAL_git_nw/HGCAL_DIGIS_project/HGCal_Digis_generalization/CMSSW_20_1_0_pre2/src/rawDataProducer_repacked_gen/rawDataProducer_repacked/python/RAW2DIGI_electronRun.root"
        #"file:RAW2DIGI_pedestalRun.root"
        "file:/eos/user/r/rkaundal/Rohit/HGCAL_git_nw/HGCAL_DIGIS_project/HGCal_Digis_generalization/Cleaned_code/CMSSW_20_1_0_pre2/src/RAW2DIGI_electronRun_118388.root"
        #"file:/eos/user/r/rkaundal/Rohit/HGCAL_git_nw/HGCAL_DIGIS_project/HGCal_Digis_generalization/Cleaned_code/CMSSW_20_1_0_pre2/src/RAW2DIGI_fixedADCRun_117489.root"
        #"file:/eos/user/r/rkaundal/Rohit/HGCAL_git_nw/HGCAL_DIGIS_project/HGCal_Digis_generalization/Cleaned_code/CMSSW_20_1_0_pre2/src/RAW2DIGI_electronRun_118181.root"
    )
)


process.out = cms.OutputModule("PoolOutputModule",
    #fileName = cms.untracked.string('test_RawDataBuffer_eRun_cbH_mod.root')
    fileName = cms.untracked.string('debug_test_RawDataBuffer_eRun_118388_packingParm.root')
    #fileName = cms.untracked.string('testRawDataBuffer_repacked_eRun_118181_v4_CalcPl_e_250GeV.root')
)

#process.TFileService = cms.Service(
#    "TFileService",
#    fileName = cms.string("test_out.root")
#)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(5)
    

)

process.RawDataBuffer = cms.EDProducer("rawDataProducer_repacked_packingParm",
   
    hgcalDigis = cms.untracked.InputTag("hgcalDigis","","RAW2DIGI"),
    #hgcalDigis = cms.untracked.InputTag("hgcalDigis","","RAW2DQM"),
    #econds = cms.untracked.InputTag("hgcalDigis","","RAW2DQM")
    #metaData = cms.InputTag("hgcalTrigTimeProducer","","RAW2DQM")
    #calibSource=cms.ESInputTag('hgcalCalibParamESProducer', ''),
)

process.p = cms.Path(process.RawDataBuffer)
process.endPath = cms.EndPath(process.out)

