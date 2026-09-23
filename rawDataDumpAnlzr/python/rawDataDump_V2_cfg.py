import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

process = cms.Process("TEST2")

options = VarParsing('standard')
options.register( 'fedId', [2001], VarParsing.multiplicity.list, VarParsing.varType.int, "FED IDs")
options.register( 'skip', 0, VarParsing.multiplicity.singleton, VarParsing.varType.int, "skip this number of events")
options.parseArguments()

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))
#process.maxEvents = cms.untracked.PSet(
#    input = cms.untracked.int32(1)
#)

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 10000

process.source = cms.Source("PoolSource",
                            fileNames = cms.untracked.vstring(
                                        #"file:/eos/user/r/rkaundal/Rohit/HGCAL_git_nw/HGCAL_DIGIS_project/HGCal_Digis_generalization/CMSSW_16_1_0/src/rawDataProducer_repacked_gen/rawDataProducer_repacked/python/testRawDataBuffer_repacked_pedestalRun_full.root"#RAW2DIGI_110723_1.root"
                                        #"file:/eos/user/r/rkaundal/Rohit/HGCAL_git_nw/HGCAL_DIGIS_project/HGCal_Digis_generalization/CMSSW_16_1_0/src/rawDataProducer_repacked_gen/rawDataProducer_repacked/python/test_repacked_eRun___afterTOT_compression.root"
                                        "file:/eos/user/r/rkaundal/Rohit/HGCAL_git_nw/HGCAL_DIGIS_project/HGCal_Digis_generalization/CMSSW_16_1_0/src/rawDataProducer_repacked_gen/rawDataProducer_repacked/python/test_RawDataBuffer_repacked_fixedADC_run_2026.root"
                                        )#cms.untracked.vstring(*options.files),
                            #skipEvents = cms.untracked.uint32(options.skip)
                            )

process.TFileService = cms.Service(
    "TFileService",
    #fileName = cms.string("CRC_diff_Cal_pedestalRun.root")
    fileName = cms.string("CRC_diff_fixedADC_Run_2026.root")
)

process.dump = cms.EDAnalyzer("rawDataDumpAnlzr_V2",
                              label1=cms.untracked.InputTag('rawDataCollector'),
                              label2=cms.untracked.InputTag('RawDataBuffer'),
                              feds=cms.untracked.vint32(*options.fedId),
                              dumpPayload=cms.untracked.bool(True),
                              usePhase2=cms.untracked.bool(True)
                              )
process.p = cms.Path(process.dump)
