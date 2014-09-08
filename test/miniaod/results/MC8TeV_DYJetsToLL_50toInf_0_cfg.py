import FWCore.ParameterSet.Config as cms

#from UserCode.llvv_fwk.mvaConfig_cfi import ewkzp2jFullNoQG as mySignalMVA
from UserCode.llvv_fwk.mvaConfig_cfi import ewkzp2jFull as mySignalMVA
#from UserCode.llvv_fwk.mvaConfig_cfi import ewkzp2jBase as mySignalMVA

datapileup_70300_2012abcd=cms.vdouble(8.09479e-06,5.64253e-05,0.000234438,0.000708129,0.0017105,0.00349358,0.00625553,0.0100708,0.0148505,0.020349,0.0262113,0.0320441,0.0374847,0.0422506,0.0461603,0.0491294,0.0511502,0.0522664,0.0525496,0.0520821,0.0509474,0.0492268,0.0469996,0.0443453,0.0413455,0.0380853,0.0346522,0.0311346,0.0276184,0.0241833,0.0208995,0.0178247,0.0150024,0.0124611,0.0102148,0.00826464,0.00660065,0.00520452,0.00405205,0.00311564,0.00236635,0.00177563,0.00131662,0.000964921,0.000699098,0.000500836,0.000354862,0.00024873,0.000172505,0.00011841,8.04614e-05,5.41399e-05,3.60823e-05,2.38255e-05,1.55917e-05,1.01156e-05,6.50871e-06,4.15502e-06,2.63279e-06,1.65666e-06,1.03577e-06,6.43829e-07,3.98156e-07,2.45158e-07,1.50427e-07,9.20683e-08,5.62672e-08,3.43759e-08,2.10198e-08,1.28799e-08,7.91828e-09,4.88962e-09,3.03584e-09,1.89664e-09,1.19296e-09,7.55627e-10,4.81954e-10,3.09438e-10,1.99876e-10,1.2979e-10,8.46506e-11,5.54024e-11,3.63523e-11,2.38919e-11,1.57154e-11,1.0338e-11,6.79661e-12,4.46326e-12,2.9262e-12,1.91457e-12,1.24969e-12,8.13541e-13,5.2808e-13,3.41728e-13,2.20422e-13,1.417e-13,9.07784e-14,5.79506e-14,3.68613e-14,2.33614e-14)

datapileup_70300_2012abcd_singleMu=cms.vdouble( 1.74887e-05, 8.41971e-05, 0.000287341, 0.000803268, 0.00187968, 0.00377078, 0.0066576, 0.0105832, 0.0154249, 0.0209121, 0.0266813, 0.0323481, 0.0375723, 0.0421003, 0.0457791, 0.0485477, 0.0504145, 0.0514311, 0.0516703, 0.0512103, 0.0501269, 0.0484915, 0.046373, 0.0438405, 0.0409668, 0.0378292, 0.0345099, 0.0310934, 0.0276634, 0.0242989, 0.0210703, 0.0180364, 0.015242, 0.0127175, 0.0104787, 0.00852824, 0.00685803, 0.00545122, 0.00428495, 0.00333277, 0.00256669, 0.00195893, 0.00148318, 0.00111545, 0.00083458, 0.000622389, 0.000463664, 0.000345947, 0.000259249, 0.000195714, 0.000149271, 0.000115306, 9.03708e-05, 7.19217e-05, 5.81115e-05, 4.76144e-05, 3.94894e-05, 3.30754e-05, 2.79112e-05, 2.36762e-05, 2.01478e-05, 1.71707e-05, 1.4635e-05, 1.24614e-05, 1.05913e-05, 8.97977e-06, 7.59119e-06, 6.39635e-06, 5.37056e-06, 4.49252e-06, 3.74351e-06, 3.10699e-06, 2.56822e-06, 2.1141e-06, 1.73297e-06, 1.4145e-06, 1.14958e-06, 9.30199e-07, 7.49367e-07, 6.01001e-07, 4.79843e-07, 3.81374e-07, 3.01728e-07, 2.37618e-07, 1.86267e-07, 1.45337e-07, 1.12875e-07, 8.72566e-08, 6.71403e-08, 5.1423e-08, 3.9204e-08, 2.97519e-08, 2.24765e-08, 1.69039e-08, 1.26565e-08, 9.43474e-09, 7.00267e-09, 5.17537e-09, 3.80882e-09, 2.79153e-09)
 

runProcess = cms.PSet(
    input = cms.vstring("root://eoscms//eos/cms/store/mc/Spring14miniaod/DYJetsToLL_M-50_13TeV-madgraph-pythia8/MINIAODSIM/PU20bx25_POSTLS170_V5-v1/00000/0A097C5B-9607-E411-A2A0-0026181D291E.root","root://eoscms//eos/cms/store/mc/Spring14miniaod/DYJetsToLL_M-50_13TeV-madgraph-pythia8/MINIAODSIM/PU20bx25_POSTLS170_V5-v1/00000/0C4273AF-A107-E411-9289-D4AE52AAF583.root","root://eoscms//eos/cms/store/mc/Spring14miniaod/DYJetsToLL_M-50_13TeV-madgraph-pythia8/MINIAODSIM/PU20bx25_POSTLS170_V5-v1/00000/0CD3EB1A-A007-E411-B601-D4AE52AAF583.root","root://eoscms//eos/cms/store/mc/Spring14miniaod/DYJetsToLL_M-50_13TeV-madgraph-pythia8/MINIAODSIM/PU20bx25_POSTLS170_V5-v1/00000/1AF5235F-8F07-E411-9342-90B11C04FE0C.root","root://eoscms//eos/cms/store/mc/Spring14miniaod/DYJetsToLL_M-50_13TeV-madgraph-pythia8/MINIAODSIM/PU20bx25_POSTLS170_V5-v1/00000/1AFA578D-A007-E411-BA93-90B11C06954E.root","root://eoscms//eos/cms/store/mc/Spring14miniaod/DYJetsToLL_M-50_13TeV-madgraph-pythia8/MINIAODSIM/PU20bx25_POSTLS170_V5-v1/00000/1E2A638F-A007-E411-BA19-001E675A6AB3.root","root://eoscms//eos/cms/store/mc/Spring14miniaod/DYJetsToLL_M-50_13TeV-madgraph-pythia8/MINIAODSIM/PU20bx25_POSTLS170_V5-v1/00000/2A0F865F-8F07-E411-BB92-0026181D28BB.root",),
    outdir = cms.string("/afs/cern.ch/work/q/querten/public/14_09_05_2l2vfwk_MiniAOD/CMSSW_7_0_8/src/UserCode/llvv_fwk/test/miniaod/results"),
    isMC = cms.bool(True),
    xsec = cms.double(3503.71),
    suffix = cms.string(""), 
    cprime = cms.double(-1),	
    brnew = cms.double(-1),	
    mctruthmode = cms.int32(0),
    jacknife = cms.vint32(0,0),
    saveSummaryTree = cms.bool(True),
    runSystematics = cms.bool(True),
    weightsFile = cms.vstring(),
    dirName = cms.string("dataAnalyzer"),
    useMVA = cms.bool(True),
    tmvaInput = mySignalMVA,
    jecDir = cms.string('${CMSSW_BASE}/src/UserCode/llvv_fwk/data/jec'),
    datapileup = datapileup_70300_2012abcd,
    datapileupSingleLep = datapileup_70300_2012abcd_singleMu,   	
#    datapileupSingleLep = datapileup_70300_2012abcd,   	
    debug = cms.bool(True)	
)
