import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing
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



# Input file
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        "file:RAW2DIGI_electronRun.root"
        #"file:RAW2DIGI_pedestalRun.root"
        #"file:RAW2DIGI_110723_1.root"
    )
)


process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('test_repacked_eRun___.root')
    #fileName = cms.untracked.string('testRawDataBuffer_repacked_eRun_full_manual_passThrough.root')
)

process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string("Digis_data_eRun_full_GdChannels_debug.root")
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
    

)

process.RawDataBuffer = cms.EDProducer("rawDataProducer_repacked",
   
    hgcalDigis = cms.untracked.InputTag("hgcalDigis","","RAW2DIGI"),
    #hgcalDigis = cms.untracked.InputTag("hgcalDigis","","RAW2DQM"),
    #econds = cms.untracked.InputTag("hgcalDigis","","RAW2DQM")
    #metaData = cms.InputTag("hgcalTrigTimeProducer","","RAW2DQM")
)

process.p = cms.Path(process.RawDataBuffer)
process.endPath = cms.EndPath(process.out)

