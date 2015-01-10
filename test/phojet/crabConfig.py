from WMCore.Configuration import Configuration
config = Configuration()

config.section_("General")
#config.General.requestName = 'GJets_HT-100to200'
config.General.requestName = 'GJet_Pt-15to3000'
config.General.workArea = 'crab_phojet'

config.section_("JobType")
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'phojet_cfg.py'
config.JobType.scriptExe = 'submit.sh'
config.JobType.inputFiles = ['FrameworkJobReport.xml', 'x509_proxy',
                             'runPhoJetAnalysis', 'phojet_cfg.py']
config.JobType.outputFiles = ['output.root']

config.section_("Data")
#config.Data.inputDataset = '/GJets_HT-100to200_Tune4C_13TeV-madgraph-tauola/Phys14DR-PU20bx25_PHYS14_25_V1-v1/MINIAODSIM'
config.Data.inputDataset = "/GJet_Pt-15to3000_Tune4C_13TeV_pythia8/Spring14miniaod-PU20bx25_POSTLS170_V5-v1/MINIAODSIM"
config.Data.inputDBS = 'global'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 10
#config.Data.publication = True
#config.Data.publishDBS = 'phys03'
#config.Data.publishDataName = 'phojet_analysis'

config.section_("Site")
config.Site.storageSite = 'T2_US_Purdue'
