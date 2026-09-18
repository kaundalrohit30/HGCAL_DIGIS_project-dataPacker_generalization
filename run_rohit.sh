#runno=117138; #pedetal run ADC-1 ZS
runno=118388; #electron run ADC > P+3N ZS;
datadir=/eos/cms/store/group/dpg_hgcal/tb_hgcal/2026/B27/OrbitData/run${runno};
cmsRun -j FrameworkJobReport_RAW2DIGI.xml $CMSSW_BASE/src/HGCalCommissioning/Configuration/test/step_RAW2DIGI.py \
       run=${runno} lumi=1 era=TB2026/v4 daqSourceMode=DTH \
       sourceIdentifier=source  buBaseDirsNumStreams=2 buBaseDirsStreamIDs=1600,1601 \
       overrideRangeLS=1,1  files=${datadir} \
       inputTrigFiles=None  yamls="{}" \
       #output=NANO_DIGI_electronRun_118388.root maxEvents=-1 secondaryOutput=RAW2DIGI_electronRun_118388.root  enableTPGunpacker=False
       output=RAW2DIGI_electronRun_118388.root maxEvents=-1 secondaryOutput=RAW2DIGI_electronRun_118388.root  enableTPGunpacker=False
