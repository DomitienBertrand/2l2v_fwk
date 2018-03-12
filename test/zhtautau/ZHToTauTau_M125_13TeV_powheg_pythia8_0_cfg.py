import FWCore.ParameterSet.Config as cms

process = cms.Process("AnalysisProc")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 10000

import PhysicsTools.PythonAnalysis.LumiList as LumiList
LumiList.LumiList().getVLuminosityBlockRange()

process.source = cms.Source("PoolSource", fileNames =  cms.untracked.vstring('') )
from RecoJets.JetProducers.PileupJetIDParams_cfi import cutbased as pu_jetid


###### Electron VID
from RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Spring15_25ns_V1_cff import *

if hasattr(cutBasedElectronID_Spring15_25ns_V1_standalone_loose,'isPOGApproved'):
    del cutBasedElectronID_Spring15_25ns_V1_standalone_loose.isPOGApproved
if hasattr(cutBasedElectronID_Spring15_25ns_V1_standalone_medium,'isPOGApproved'):
    del cutBasedElectronID_Spring15_25ns_V1_standalone_medium.isPOGApproved
if hasattr(cutBasedElectronID_Spring15_25ns_V1_standalone_tight,'isPOGApproved'):
    del cutBasedElectronID_Spring15_25ns_V1_standalone_tight.isPOGApproved

myVidElectronId = cms.PSet(
    loose = cutBasedElectronID_Spring15_25ns_V1_standalone_loose,
    medium = cutBasedElectronID_Spring15_25ns_V1_standalone_medium,
    tight = cutBasedElectronID_Spring15_25ns_V1_standalone_tight
)
#######

#from UserCode.llvv_fwk.mvaConfig_cfi import ewkzp2jFullNoQG as mySignalMVA
from UserCode.llvv_fwk.mvaConfig_cfi import ewkzp2jFull as mySignalMVA
#from UserCode.llvv_fwk.mvaConfig_cfi import ewkzp2jBase as mySignalMVA

datapileup_70300_2012abcd=cms.vdouble(8.09479e-06,5.64253e-05,0.000234438,0.000708129,0.0017105,0.00349358,0.00625553,0.0100708,0.0148505,0.020349,0.0262113,0.0320441,0.0374847,0.0422506,0.0461603,0.0491294,0.0511502,0.0522664,0.0525496,0.0520821,0.0509474,0.0492268,0.0469996,0.0443453,0.0413455,0.0380853,0.0346522,0.0311346,0.0276184,0.0241833,0.0208995,0.0178247,0.0150024,0.0124611,0.0102148,0.00826464,0.00660065,0.00520452,0.00405205,0.00311564,0.00236635,0.00177563,0.00131662,0.000964921,0.000699098,0.000500836,0.000354862,0.00024873,0.000172505,0.00011841,8.04614e-05,5.41399e-05,3.60823e-05,2.38255e-05,1.55917e-05,1.01156e-05,6.50871e-06,4.15502e-06,2.63279e-06,1.65666e-06,1.03577e-06,6.43829e-07,3.98156e-07,2.45158e-07,1.50427e-07,9.20683e-08,5.62672e-08,3.43759e-08,2.10198e-08,1.28799e-08,7.91828e-09,4.88962e-09,3.03584e-09,1.89664e-09,1.19296e-09,7.55627e-10,4.81954e-10,3.09438e-10,1.99876e-10,1.2979e-10,8.46506e-11,5.54024e-11,3.63523e-11,2.38919e-11,1.57154e-11,1.0338e-11,6.79661e-12,4.46326e-12,2.9262e-12,1.91457e-12,1.24969e-12,8.13541e-13,5.2808e-13,3.41728e-13,2.20422e-13,1.417e-13,9.07784e-14,5.79506e-14,3.68613e-14,2.33614e-14)

datapileup_70300_2012abcd_singleMu=cms.vdouble( 1.74887e-05, 8.41971e-05, 0.000287341, 0.000803268, 0.00187968, 0.00377078, 0.0066576, 0.0105832, 0.0154249, 0.0209121, 0.0266813, 0.0323481, 0.0375723, 0.0421003, 0.0457791, 0.0485477, 0.0504145, 0.0514311, 0.0516703, 0.0512103, 0.0501269, 0.0484915, 0.046373, 0.0438405, 0.0409668, 0.0378292, 0.0345099, 0.0310934, 0.0276634, 0.0242989, 0.0210703, 0.0180364, 0.015242, 0.0127175, 0.0104787, 0.00852824, 0.00685803, 0.00545122, 0.00428495, 0.00333277, 0.00256669, 0.00195893, 0.00148318, 0.00111545, 0.00083458, 0.000622389, 0.000463664, 0.000345947, 0.000259249, 0.000195714, 0.000149271, 0.000115306, 9.03708e-05, 7.19217e-05, 5.81115e-05, 4.76144e-05, 3.94894e-05, 3.30754e-05, 2.79112e-05, 2.36762e-05, 2.01478e-05, 1.71707e-05, 1.4635e-05, 1.24614e-05, 1.05913e-05, 8.97977e-06, 7.59119e-06, 6.39635e-06, 5.37056e-06, 4.49252e-06, 3.74351e-06, 3.10699e-06, 2.56822e-06, 2.1141e-06, 1.73297e-06, 1.4145e-06, 1.14958e-06, 9.30199e-07, 7.49367e-07, 6.01001e-07, 4.79843e-07, 3.81374e-07, 3.01728e-07, 2.37618e-07, 1.86267e-07, 1.45337e-07, 1.12875e-07, 8.72566e-08, 6.71403e-08, 5.1423e-08, 3.9204e-08, 2.97519e-08, 2.24765e-08, 1.69039e-08, 1.26565e-08, 9.43474e-09, 7.00267e-09, 5.17537e-09, 3.80882e-09, 2.79153e-09)


datapileup_200pb_2015bcd_ll_Preliminary = cms.vdouble(0, 169, 538, 1562, 3650, 7094, 11435, 16000, 19857, 21999, 22816, 22006, 19766, 17567, 14740, 12202, 10001, 8014, 6423, 4908, 3735, 2803, 2061, 1484, 1037, 688, 435, 293, 174, 117, 82, 39, 25, 14, 11, 5, 2, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  )


datapileup_40pb_2015b_singlemu_PietroPreliminary = cms.vdouble(0, 0.000810074, 0.000608754, 0.00108809, 0.00249253, 0.00551713, 0.0105118, 0.0185071, 0.0286402, 0.041285, 0.0545865, 0.066728, 0.0771487, 0.0843004, 0.0874832, 0.0865053, 0.0815586, 0.0744261, 0.0637849, 0.054203, 0.0435427, 0.0336157, 0.0249877, 0.0184448, 0.0129276, 0.00930386, 0.00608754, 0.00412227, 0.00258361, 0.00155783, 0.00104015, 0.000613547, 0.00038826, 0.000282807, 0.000139007, 8.14867e-05, 3.35533e-05, 3.35533e-05, 1.91733e-05, 4.79334e-06, 4.79334e-06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

datapileup_42pb_2015b_singlemu_cleanNvtx_PietroPreliminary = cms.vdouble(0, 0.000520232, 0.000399503, 0.000812177, 0.00193605, 0.00449551, 0.00890321, 0.0160218, 0.0253465, 0.036965, 0.0495867, 0.0622983, 0.0730827, 0.082482, 0.0871882, 0.0879104, 0.0847583, 0.0780194, 0.0678628, 0.0582638, 0.0469855, 0.0370682, 0.0278972, 0.0199334, 0.0141692, 0.00980538, 0.00633717, 0.00422771, 0.00259896, 0.00165289, 0.000994367, 0.000557548, 0.000357797, 0.000237068, 0.000158045, 7.24374e-05, 3.73162e-05, 3.0731e-05, 1.53655e-05, 6.58521e-06, 4.39014e-06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

datapileup_42pb_2015C_singlemu_cleanNvtx_PietroPreliminary = cms.vdouble( 3.9990225e-05, 0.00064873031, 0.001959521, 0.0054431139, 0.011774899, 0.022367866, 0.038852725, 0.058638999, 0.079758281, 0.094741285, 0.10511208, 0.10750261, 0.10176179, 0.090711159, 0.07544378, 0.060140854, 0.045606629, 0.033391838, 0.023540912, 0.015938326, 0.010077537, 0.0066961409, 0.0040212393, 0.0024971674, 0.0015818356, 0.00084423808, 0.00040878896, 0.00025327142, 0.00013774411, 3.5546866e-05, 4.4433583e-05, 4.4433583e-06, 1.3330075e-05, 0, 0, 4.4433583e-06, 0, 4.4433583e-06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 )

datapileup_official = cms.vdouble( 141899.9,    622421.5,    818662.3,    1253138,    2114164,    4937264,    1.627961e+07,    6.475337e+07,    1.817159e+08,    3.220406e+08,    4.16444e+08,    4.291912e+08,    3.663307e+08,    2.557795e+08,    1.447021e+08,    6.727821e+07,    2.736262e+07,    1.139197e+07,    5668287,    3025054,    1401679,    511811,    146735.4,    35294.85,    8269.747,    2235.301,    721.3224,    258.8397,    97.26937,    36.8714,    13.72746,    4.931709,    1.692407,    0.5518936,    0.1706013,    0.0499358,    0.01383391,    0.003626617,    0.0008996063,    0.0002111484,    4.689276e-05,    9.85392e-06,    1.959294e-06,    3.686198e-07,    6.562482e-08,    1.105342e-08,    1.762478e-09,    2.614969e-10,    4.768003e-11)

datapileup_lumi6p2_ichep2016 = cms.vdouble(1811.43, 64856.2, 341725, 687474, 1.08201e+06, 1.52343e+06, 2.13894e+06, 8.85766e+06, 3.004e+07, 6.28157e+07, 1.1405e+08, 1.6771e+08, 2.21769e+08, 2.92337e+08, 3.77241e+08, 4.57831e+08, 5.16658e+08, 5.43844e+08, 5.39866e+08, 5.11582e+08, 4.63676e+08, 4.00447e+08, 3.27676e+08, 2.51639e+08, 1.79746e+08, 1.19225e+08, 7.38221e+07, 4.29701e+07, 2.36718e+07, 1.24401e+07, 6.29422e+06, 3.08961e+06, 1.47617e+06, 685902, 309435, 136033, 59392, 27072.9, 14076.7, 9048.31, 7126.81, 6361.48, 6015.27, 5819.76, 5670.29, 5529.32, 5378.21, 5206.23, 5008.46)

datapileup_2016 = cms.vdouble(2090.04, 159957, 614019, 1.24536e+06, 1.94245e+06, 2.63418e+06, 3.54009e+06, 8.46173e+06, 2.92479e+07, 6.98063e+07, 1.36518e+08, 2.29982e+08, 3.52225e+08, 4.83416e+08, 6.04336e+08, 7.18921e+08, 8.22821e+08, 9.02958e+08, 9.5287e+08, 9.72127e+08, 9.62216e+08, 9.24428e+08, 8.6315e+08, 7.85157e+08, 6.95304e+08, 5.96246e+08, 4.91754e+08, 3.88446e+08, 2.93778e+08, 2.12971e+08, 1.47977e+08, 9.8322e+07, 6.22909e+07, 3.75695e+07, 2.15786e+07, 1.18166e+07, 6.17703e+06, 3.08751e+06, 1.47976e+06, 683088, 305855, 134437, 59359.5, 27542.8, 14455.7, 9219.82, 7180.4, 6406.26, 6116.86)

datapileup_latest = cms.vdouble(0, 238797, 837543, 2.30843e+06, 3.12475e+06, 4.47619e+06, 5.99591e+06, 7.0009e+06, 1.28917e+07, 3.52617e+07, 7.87012e+07, 1.76946e+08, 3.6009e+08, 6.02766e+08, 8.76519e+08, 1.17447e+09, 1.48906e+09, 1.75935e+09, 1.94393e+09, 2.04917e+09, 2.10158e+09, 2.13279e+09, 2.1491e+09, 2.12899e+09, 2.06265e+09, 1.96288e+09, 1.84187e+09, 1.70414e+09, 1.55452e+09, 1.39949e+09, 1.24353e+09, 1.08882e+09, 9.37305e+08, 7.92044e+08, 6.56718e+08, 5.34467e+08, 4.27127e+08, 3.35106e+08, 2.57725e+08, 1.93751e+08, 1.41831e+08, 1.00671e+08, 6.90139e+07, 4.55401e+07, 2.88475e+07, 1.75063e+07, 1.01626e+07, 5.63778e+06, 2.98728e+06, 1.512e+06, 731845, 339822, 152545, 67404.8, 30489.7, 15152.1, 8975.91, 6496.15, 5434.81, 4889.96, 4521.72, 4208.46, 3909.76, 3614.27, 3320.72, 3031.1, 2748.24, 2474.98, 2213.82, 1966.82, 1735.55, 1521.11, 1324.15, 1144.9, 983.22, 838.668, 710.534, 597.91, 499.739, 414.866, 342.082, 280.162, 227.901, 184.137, 147.773, 117.789, 93.2547, 73.3322, 57.2764, 44.4338, 34.2379, 26.2034, 19.9188, 15.0392, 11.2782, 8.40063, 6.21495, 4.56686, 3.33312, 2.41623)

from os import path as path

theLumiMask = path.expandvars("")

process.syncNtupler = cms.EDAnalyzer('ZHTauTauAnalyzer',
#runProcess = cms.PSet(
    dtag  = cms.string("ZHToTauTau_M125_13TeV_powheg_pythia8"),
    input = cms.untracked.vstring("file:/storage/data/cms/store/mc/RunIISummer16MiniAODv2/ZHToTauTau_M125_13TeV_powheg_pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/20245004-44C7-E611-A5AF-A0369F3016EC.root",
#"file:/storage/data/cms/store/mc/RunIISummer16MiniAODv2/ZHToTauTau_M125_13TeV_powheg_pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/220CB8B6-53C7-E611-BC88-0CC47AD99052.root",
#"file:/storage/data/cms/store/mc/RunIISummer16MiniAODv2/ZHToTauTau_M125_13TeV_powheg_pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/2411BE9B-74C8-E611-84FA-02163E019D73.root",
#"file:/storage/data/cms/store/mc/RunIISummer16MiniAODv2/ZHToTauTau_M125_13TeV_powheg_pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/2A9B92A1-89C6-E611-A50D-002590E2D9FE.root",
#"file:/storage/data/cms/store/mc/RunIISummer16MiniAODv2/ZHToTauTau_M125_13TeV_powheg_pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/4C20BEC8-D9C7-E611-B74B-FA163E2D1FE1.root",
#"file:/storage/data/cms/store/mc/RunIISummer16MiniAODv2/ZHToTauTau_M125_13TeV_powheg_pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/505A0021-F7C7-E611-9687-D067E5F91E51.root",
#"file:/storage/data/cms/store/mc/RunIISummer16MiniAODv2/ZHToTauTau_M125_13TeV_powheg_pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/600D3FD8-48C7-E611-B341-90B11C0BD210.root",
#"file:/storage/data/cms/store/mc/RunIISummer16MiniAODv2/ZHToTauTau_M125_13TeV_powheg_pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/88867835-A7C7-E611-B26A-008CFA11113C.root",
),
    outfile = cms.string("file:ZHToTauTau_M125_13TeV_powheg_pythia8_0.root"),
    isMC = cms.bool(True),
    triggerstudy = cms.bool(False),
    xsec = cms.double(0.05785),
    suffix = cms.string(""),
    cprime = cms.double(-1),
    brnew = cms.double(-1),
    mctruthmode = cms.int32(0),
    jacknife = cms.vint32(0,0),
    saveSummaryTree = cms.bool(True),
    runSystematics = cms.bool(False),
    runSVfit = cms.bool(False),
    resonance = cms.double(1),
    weightsFile = cms.vstring(""),
    puWeightsFile = cms.vstring(""),
    dirName = cms.string("dataAnalyzer"),
    useMVA = cms.bool(True),
    tmvaInput = mySignalMVA,
    muscleDir =  cms.string('${CMSSW_BASE}/src/UserCode/llvv_fwk/data/jec/'),
    jecDir = cms.string('${CMSSW_BASE}/src/UserCode/llvv_fwk/data/jec/25ns/'),
    datapileup = datapileup_latest,
    datapileupSingleLep = datapileup_latest,
    debug = cms.bool(False),
    lumisToProcess = LumiList.LumiList(filename = theLumiMask).getVLuminosityBlockRange(),
    pujetidparas = cms.PSet(pu_jetid),
    electronidparas = cms.PSet(myVidElectronId),
    maxevents = cms.int32(-1), # set to -1 when running on grid.
    vtxSrc                = cms.InputTag("offlineSlimmedPrimaryVertices"),
    rho                   = cms.InputTag("fixedGridRhoFastjetAll"),
    muonSrc               = cms.InputTag("slimmedMuons"),
    electronSrc           = cms.InputTag("slimmedElectrons"),
    recHitCollectionEBSrc = cms.InputTag("reducedEgamma","reducedEBRecHits"),
    recHitCollectionEESrc = cms.InputTag("reducedEgamma","reducedEERecHits"),
    tauSrc                = cms.InputTag("slimmedTaus"),
    packCandSrc           = cms.InputTag("packedPFCandidates"),
    jetSrc                = cms.InputTag("slimmedJets"),
    pfMETSrc              = cms.InputTag("slimmedMETs"),
    triggerResultSrc      = cms.InputTag("TriggerResults","","HLT"),
    metFilterResultSrc    = cms.InputTag("TriggerResults","","PAT"),
    genParticleSrc        = cms.InputTag("prunedGenParticles"),
    genEventInfoProduct   = cms.InputTag("generator"),
    PUInfo                = cms.InputTag("slimmedAddPileupInfo"),
    lheEventProducts      = cms.InputTag("externalLHEProducer"),
)

#from Configuration.AlCa.GlobalTag import GlobalTag
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag

runOnData = not process.syncNtupler.isMC

# Latest JEC
if runOnData:
  process.GlobalTag.globaltag = '80X_dataRun2_2016SeptRepro_v7'
  #process.source.lumisToProcess = LumiList.LumiList(filename = '../json/Cert_13TeV_16Dec2015ReReco_Collisions15_25ns_JSON.txt').getVLuminosityBlockRange()
else:
  process.GlobalTag.globaltag = '80X_mcRun2_asymptotic_2016_TrancheIV_v8'


process.source = cms.Source ("PoolSource", fileNames = process.syncNtupler.input)

process.TFileService = cms.Service("TFileService", fileName = process.syncNtupler.outfile )

process.p = cms.Path(process.syncNtupler)
