// <author>Cristóvão B da Cruz e Silva</author>
// <email>c.beirao@cern.ch</email>
// <date>2014-06-02</date>
// <summary>Script that runs an iterative cut optimization procedure. Cut values are scanned and the value with the maximum FOM is used.</summary>

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <cassert>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <fstream>

#include "TROOT.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TChain.h"
#include "TTree.h"
#include "TCut.h"
#include "TEventList.h"
#include "TInterpreter.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TGraph.h"
#include "TGraphErrors.h"

#include "UserCode/llvv_fwk/interface/JSONWrapper.h"
#include "UserCode/llvv_fwk/interface/llvvObjects.h"

class OptimizationRoundInfo;

typedef std::vector<std::pair<std::string,std::vector<std::pair<int,TChain*>>>> fileChains;

void printHelp();
fileChains getChainsFromJSON(JSONWrapper::Object& json, std::string RootDir, std::string type="BG", std::string treename="Events", std::string customExtension="_selected");
std::vector<OptimizationRoundInfo> getRoundsFromJSON(JSONWrapper::Object& json);
int getNumberOfPoints(fileChains files, std::string variable, TCut cut);
double applyCut(fileChains files,  TCut cut, bool isSig = false, bool correctFiles = true, bool normalize = false);
double getSignalScale(fileChains files,  TCut cut, bool correctFiles = true);

bool gDoneScale = false;
double gScale = 0;
bool gDoneSignalScale = false;
double gSignalScale = 0;

struct FOMInfo
{
  double FOM;
  double err;
  std::string var;
  double cutVal;
};

struct doubleUnc
{
  double value;
  double uncertainty;
};

std::ostream& operator << (std::ostream &o, doubleUnc& val)
{
  return o << val.value << " +- " << val.uncertainty;
}

struct SampleFiles
{
  std::string name;
  TChain* chain;
  int nFiles;
};

struct ProcessFiles
{
  std::string name;
  std::vector<SampleFiles> samples;
};

struct UserCutInfo
{
  std::string variable;
  std::string direction;
  double value;
};

struct CutInfo
{
  std::string variable;
  std::string direction;
  std::string value;
  std::map<std::string,std::map<std::string,std::map<std::string,doubleUnc>>> yield;
  double FOM;
  double FOMerr;
};

struct ReportInfo
{
  int round;
  std::vector<CutInfo> cuts;
};

class CutOptimizer
{
public:
  CutOptimizer(const std::string & jsonFile, const std::string & outDir, const std::vector<std::string> & plotExt);
  ~CutOptimizer();

  bool LoadJson();
  bool OptimizeAllRounds();
  bool OptimizeRound(size_t n);

  inline std::string getJson() const {return jsonFile_;};
  std::string& setJson(const std::string & jsonFile);
  inline std::string getOutDir() const {return outDir_;};
  std::string& setOutDir(const std::string & outDir);
  inline std::vector<std::string> getPlotExt() const {return plotExt_;};
  std::vector<std::string>& setPlotExt(const std::vector<std::string> & plotExt);
  inline size_t getNRounds() const {return roundInfo_.size();};
  inline void setVerbose() {verbose_ = true; return;};

private:
  std::string jsonFile_;
  std::string outDir_;
  std::vector<std::string> plotExt_;

  bool jsonLoaded;
  bool verbose_;

  bool isMultipointSignalSample_;
  int  nSignalPoints_;

  std::map<std::string,bool> FileExists_;
  std::vector<ReportInfo> report_;
//  JSONWrapper::Object* json;

  std::vector<OptimizationRoundInfo> roundInfo_;
  std::map<std::string,std::vector<ProcessFiles>> processes_;

  bool OptimizeRound_(size_t n);
  bool GetSamples(size_t n);
  int  GetSigPoints(size_t n);
  CutInfo GetBestCutAndMakePlots(size_t n, ReportInfo& report);
  std::map<std::string,std::map<std::string,std::map<std::string,doubleUnc>>> GetYields(ReportInfo& report, TCut signalSelection, TCut currentSelection, double integratedLuminosity);
  void SaveGraph(std::string& name, std::vector<double>& xVals, std::vector<double>& xValsUnc, std::string& xTitle, std::vector<double>& yVals, std::vector<double>& yValsUnc, std::string& yTitle);
  bool ClearSamples();

protected:
};

class OptimizationVariableInfo
{
public:
  OptimizationVariableInfo();
  ~OptimizationVariableInfo();

  // If using pointers the following lines should be uncommented and the respective functions defined
  // More info: http://www.cplusplus.com/articles/y8hv0pDG/
  //OptimizationVariableInfo(const OptimizationVariableInfo& other);
  //OptimizationVariableInfo& operator=(const OptimizationVariableInfo& rhs);

  inline std::string& name(){return _name;};
  inline std::string& expression(){return _expression;};
  inline double& minVal(){return _minVal;};
  inline double& maxVal(){return _maxVal;};
  inline double& step(){return _step;};
  inline std::string& cutDir(){return _cutDir;};
  inline std::string& label(){return _label;};

  friend std::vector<OptimizationRoundInfo> getRoundsFromJSON(JSONWrapper::Object& json);
  friend bool CutOptimizer::LoadJson();

private:
  std::string _name;
  std::string _expression;
  double _minVal, _maxVal, _step;
  std::string _cutDir;
  std::string _label;

protected:
};

class OptimizationRoundInfo
{
public:
  OptimizationRoundInfo();
  ~OptimizationRoundInfo();

  // If using pointers the following lines should be uncommented and the respective functions defined
  // More info: http://www.cplusplus.com/articles/y8hv0pDG/
  //OptimizationRoundInfo(const OptimizationRoundInfo& other);
  //OptimizationRoundInfo& operator=(const OptimizationRoundInfo& rhs);

  inline std::string& name(){return _name;};
  inline std::string& ttree(){return _ttree;};
  inline std::string& customExtension(){return _customExtension;};
  inline std::string& baseSelection(){return _baseSelection;};
  inline std::string& signalSelection(){return _signalSelection;};
  inline std::string& channel(){return _channel;};
  inline double& iLumi(){return _iLumi;};
  inline std::string& inDir(){return _inDir;};
  inline std::string& jsonFile(){return _jsonFile;};
  inline size_t nVars(){return _variables.size();};
  inline std::string pointVariable(){return _pointVariable;};
  inline bool isSelected(int pass){return (static_cast<size_t>(pass) < _UserCuts.size());};
  inline std::string getSelection(int pass){return getUserCut(pass);}; // Deprecated
  std::string getUserCut(int pass);
  inline std::string getUserCutVar(int pass){if(isSelected(pass)) return _UserCuts[pass].variable; else return "";};
  inline std::string getUserCutDir(int pass){if(isSelected(pass)) return _UserCuts[pass].direction; else return "";};
  inline double getUserCutValue(int pass){if(isSelected(pass)) return _UserCuts[pass].value; else return -9999;};
  inline UserCutInfo getUserCutInfo(int pass){if(isSelected(pass)) return _UserCuts[pass]; else return UserCutInfo{"","",-9999}; };
  inline std::string signalPoint(){return _signalPoint;};
  inline double sigCrossSection(){return _sigCrossSection;};
  inline int nInitEvents(){return _nInitEvents;};

  std::vector<std::string> getListOfVariables();
  std::unordered_map<std::string,std::string> getVariableExpressions();
  std::unordered_map<std::string,std::unordered_map<std::string,double>> getVariableParameterMap();
  std::unordered_map<std::string,std::string> getVariableLabels();

  friend std::vector<OptimizationRoundInfo> getRoundsFromJSON(JSONWrapper::Object& json);
  friend bool CutOptimizer::LoadJson();

private:
  static size_t _counter;
  std::string _name;
  std::string _ttree;
  std::string _customExtension;
  std::string _baseSelection;
  std::string _signalSelection;
  std::string _signalPoint;
  std::string _channel;
  std::vector<OptimizationVariableInfo> _variables;
  double      _iLumi;
  std::string _inDir;
  std::string _jsonFile;
  std::string _pointVariable;
  std::vector<std::string> _selected;
  std::vector<UserCutInfo> _UserCuts;
  double      _sigCrossSection;
  int         _nInitEvents;

  inline std::string selection(int pass){return _selected[pass];};

protected:
};
size_t OptimizationRoundInfo::_counter = 0;

std::unordered_map<std::string,bool> FileExists;


//
// Output codes:
// 0 - Everything has run fine
// 1 - Invalid arguments
// 2 - Problem parsing the arguments
//

int main(int argc, char** argv)
{
  AutoLibraryLoader::enable();

  std::string jsonFile;
  std::string outDir = "./OUT/";
  std::vector<std::string> plotExt;
  bool verbose = false;

//  gInterpreter->GenerateDictionary("llvvMet", "");
//  gROOT->ProcessLine("#include \"UserCode/llvv_fwk/interface/llvvObjects.h\"");

  // Parse the command line options
  for(int i = 1; i < argc; ++i)
  {
    std::string arg(argv[i]);

    // First see if help info is being requested, if so, print help and then exit
    if(arg.find("--help") != std::string::npos)
    {
      printHelp();
      return 0;
    }

    if(arg.find("--json") != std::string::npos)
    {
      jsonFile = argv[i+1];
      // TODO: Check if the file exists
      ++i;
    }

    if(arg.find("--outDir") != std::string::npos)
    {
      outDir = argv[i+1];
      ++i;
    }

    if(arg.find("--plotExt") != std::string::npos)
    {
      plotExt.push_back(argv[i+1]);
      ++i;
    }

    if(arg.find("--verbose") != std::string::npos)
    {
      verbose = true;
    }
  }
  if(plotExt.size() == 0)
    plotExt.push_back(".png");

  if(jsonFile == "")
  {
    std::cout << "You should define at least the following arguments: json" << std::endl;
    std::cout << "For more information, consult the help (\"runCutOptimizer --help\")" << std::endl;
    return 2;
  }

  if(verbose)
    std::cout << "Creating an object of the CutOptimizer class..." << std::endl;
  CutOptimizer myOptimizer(jsonFile, outDir, plotExt);
  if(verbose)
    myOptimizer.setVerbose();
  myOptimizer.LoadJson();
  myOptimizer.OptimizeAllRounds();
//  myOptimizer.OptimizeRound(2);
  return 0;
}

OptimizationRoundInfo::OptimizationRoundInfo()
{
  std::stringstream buf;
  buf << "Round_" << _counter;
  buf >> _name;
  _ttree           = "Events";
  _customExtension = "";
  _baseSelection   = "selected";
  _signalSelection = "";
  _channel         = "";
  _iLumi           = 1;
  _inDir           = "";
  _jsonFile        = "";
  ++_counter;
}

OptimizationRoundInfo::~OptimizationRoundInfo()
{
  _variables.clear();
}

std::string OptimizationRoundInfo::getUserCut(int pass)
{
  if(!isSelected(pass))
    return "";

  std::string retVal;
  bool found = false;

  for(auto var = _variables.begin(); var != _variables.end(); ++var)
  {
    if(var->name() == _UserCuts[pass].variable)
    {
      found = true;
      if(_UserCuts[pass].direction != "above" && _UserCuts[pass].direction != "below")
        break;
      std::stringstream buf;
      buf << _UserCuts[pass].value;
      buf >> retVal;
      if(_UserCuts[pass].direction == "above")
        retVal = ">" + retVal;
      else
        retVal = "<" + retVal;
      retVal = var->expression() + retVal;
      break;
    }
  }

  if(!found)
    return "";
  return retVal;
}

std::vector<std::string> OptimizationRoundInfo::getListOfVariables()
{
  std::vector<std::string> retVal;

  for(auto variable = _variables.begin(); variable != _variables.end(); ++variable)
    retVal.push_back(variable->name());

  return retVal;
}

std::unordered_map<std::string,std::string> OptimizationRoundInfo::getVariableLabels()
{
  std::unordered_map<std::string,std::string> retVal;

  for(auto variable = _variables.begin(); variable != _variables.end(); ++variable)
  {
    retVal[variable->name()] = variable->label();
    if(retVal[variable->name()] == "")
      retVal[variable->name()] = variable->name();
  }

  return retVal;
}

std::unordered_map<std::string,std::unordered_map<std::string,double>> OptimizationRoundInfo::getVariableParameterMap()
{
  std::unordered_map<std::string,std::unordered_map<std::string,double>> retVal;

  for(auto variable = _variables.begin(); variable != _variables.end(); ++variable)
  {
    std::unordered_map<std::string,double> tempVal;

    tempVal["minVal"] = variable->minVal();
    tempVal["maxVal"] = variable->maxVal();
    tempVal["step"]   = variable->step();
    tempVal["cutDir"] = (variable->cutDir()=="below")?-1:1;

    retVal[variable->name()] = tempVal;
  }

  return retVal;
}

std::unordered_map<std::string,std::string> OptimizationRoundInfo::getVariableExpressions()
{
  std::unordered_map<std::string,std::string> retVal;

  for(auto variable = _variables.begin(); variable != _variables.end(); ++variable)
  {
    retVal[variable->name()] = variable->expression();
  }

  return retVal;
}

OptimizationVariableInfo::OptimizationVariableInfo()
{
  _name = "";
  _expression = "";
  _minVal = 0;
  _maxVal = 0;
  _step = 0;
  _cutDir = "below";
}

OptimizationVariableInfo::~OptimizationVariableInfo()
{
}

int getNumberOfPoints(fileChains files, std::string variable, TCut cut)
{
  gROOT->cd();
  int retVal = 0;
  int maxVal = 501*1000+1;
  TH1D finalHist("finalHist", "finalHist", maxVal, 0, maxVal);

  for(auto process = files.begin(); process != files.end(); ++process)
  {
    for(auto sample = process->second.begin(); sample != process->second.end(); ++sample)
    {
      if(sample->first == 0)
        continue;

      TH1D* temp_hist = new TH1D("temp_hist", "temp_hist", maxVal, 0, maxVal);
      sample->second->Draw((variable+">>temp_hist").c_str(), cut*"weight", "goff");
      finalHist.Add(temp_hist);

      delete temp_hist;
    }
  }

  int nBins = finalHist.GetNbinsX();
  for(int i = 1; i <= nBins; ++i)
  {
    if(finalHist.GetBinContent(i) != 0)
      retVal++;
  }

  return retVal;
}

double getSignalScale(fileChains files,  TCut cut, bool correctFiles)
{
  double retVal = 0;

  if(!gDoneSignalScale)
  {
    for(auto process = files.begin(); process != files.end(); ++process)
    {
      for(auto sample = process->second.begin(); sample != process->second.end(); ++sample)
      {
        if(sample->first == 0)
          continue;
        TH1D temp_histo("temp_histo", "temp_histo", 1, 0, 20);
        sample->second->Draw("weight>>temp_histo", cut*"weight", "goff");

        double count = temp_histo.GetBinContent(0) + temp_histo.GetBinContent(1) + temp_histo.GetBinContent(2);

        if(correctFiles)
          count = count/sample->first;
        retVal += count;
      }
    }
  }
  else
    retVal = gSignalScale;

  gSignalScale = retVal;
  gDoneSignalScale = true;
  return retVal;
}

double applyCut(fileChains files,  TCut cut, bool isSig, bool correctFiles, bool normalize)
{
  gROOT->cd();
  double retVal = 0;

  if(!gDoneScale && isSig)
  {
    gScale = 0;
    for(auto process = files.begin(); process != files.end(); ++process)
    {
      for(auto sample = process->second.begin(); sample != process->second.end(); ++sample)
      {
        if(sample->first == 0)
          continue;
        TH1D temp_histo("temp_histo", "temp_histo", 1, 0, 20);
        sample->second->Draw("puWeight>>temp_histo", "puWeight", "goff");

        double count = temp_histo.GetBinContent(0) + temp_histo.GetBinContent(1) + temp_histo.GetBinContent(2);

        if(correctFiles)
          count = count/sample->first;
        gScale += count;
      }
    }

    gScale = 1/gScale;
    gDoneScale = true;
  }

  for(auto process = files.begin(); process != files.end(); ++process)
  {
    for(auto sample = process->second.begin(); sample != process->second.end(); ++sample)
    {
      if(sample->first == 0)
        continue;
//      TEventList selectedEvents("selectedList");
//      sample->second->Draw(">>selectedList", cut, "goff");
      TH1D temp_histo("temp_histo", "temp_histo", 1, 0, 20);
      if(isSig)
        sample->second->Draw("puWeight>>temp_histo", cut*"puWeight", "goff");
      else
        sample->second->Draw("weight>>temp_histo", cut*"weight", "goff");

      double count = temp_histo.GetBinContent(0) + temp_histo.GetBinContent(1) + temp_histo.GetBinContent(2);

      /*double weight = 0;
      double count = 0;
      sample->second->SetBranchAddress("weight", &weight);

      for(auto index = selectedEvents.GetN()-1; index >= 0; --index)
      {
        Long64_t entry = selectedEvents.GetEntry(index);
        sample->second->GetEntry(entry);
        count += weight;
      }
      sample->second->ResetBranchAddresses();// */

      if(correctFiles)
        count = count/sample->first;
      retVal += count;
    }
  }

  return retVal;
}

std::vector<OptimizationRoundInfo> getRoundsFromJSON(JSONWrapper::Object& json)
{
  std::vector<OptimizationRoundInfo> retVal;

  std::vector<JSONWrapper::Object> rounds = json["optim"].daughters();
  for(auto round = rounds.begin(); round != rounds.end(); ++round)
  {
    OptimizationRoundInfo roundInfo;
    //roundInfo._pointVariable = "stauMass*1000+neutralinoMass";

    std::string name = round->getString("name", "");
    if(name != "")
      roundInfo._name = name;
    roundInfo._iLumi = round->getDouble("iLumi", 0);
    if(roundInfo._iLumi <= 0)
    {
      std::cout << roundInfo._name << ": Integrated luminosity should be positive and non-zero. Continuing..." << std::endl;
      continue;
    }
    roundInfo._sigCrossSection = round->getDouble("sigCrossSection", 0);
    if(roundInfo._sigCrossSection <= 0)
    {
      std::cout << roundInfo._name << ": Signal cross section should be positive and non-zero. Continuing..." << std::endl;
      continue;
    }
    roundInfo._inDir = round->getString("inDir", "");
    if(roundInfo._inDir == "")
    {
      std::cout << roundInfo._name << ": Input directory should be defined in the json for the optimization round. Continuing..." << std::endl;
      continue;
    }
    roundInfo._jsonFile = round->getString("jsonFile", "");
    if(roundInfo._jsonFile == "")
    {
      std::cout << roundInfo._name << ": JSON file must be specified for optimization round in cut optimization JSON. Continuing..." << std::endl;
      continue;
    }

    roundInfo._ttree = round->getString("ttree", roundInfo._ttree);
    roundInfo._customExtension = round->getString("customExtension", roundInfo._customExtension);
    roundInfo._baseSelection = round->getString("baseSelection", roundInfo._baseSelection);
    roundInfo._signalSelection = round->getString("signalSelection", roundInfo._signalSelection);
    roundInfo._signalPoint = round->getString("signalPoint", roundInfo._signalPoint);
    roundInfo._channel = round->getString("channel", roundInfo._channel);
    roundInfo._pointVariable = round->getString("pointVariable", roundInfo._pointVariable);

    auto selected = (*round)["selected"].daughters();
    for(auto selection = selected.begin(); selection != selected.end(); ++selection)
    {
      std::string temp = selection->getString("selection", "");
      if(temp != "")
        roundInfo._selected.push_back(temp);
    }

    auto variables = (*round)["variables"].daughters();
    for(auto variable = variables.begin(); variable != variables.end(); ++variable)
    {
      OptimizationVariableInfo variableInfo;

      variableInfo._name = variable->getString("name", "");
      if(variableInfo._name == "")
      {
        std::cout << roundInfo._name << ": All variables must have names. Continuing..." << std::endl;
        continue;
      }
      variableInfo._expression = variable->getString("expression", "");
      if(variableInfo._expression == "")
      {
        std::cout << roundInfo._name << "::" << variableInfo._name << ": This variable must have an expression, it must be a valid root expression. Continuing..." << std::endl;
        continue;
      }
      variableInfo._minVal = variable->getDouble("minVal", 0);
      variableInfo._maxVal = variable->getDouble("maxVal", 0);
      if(variableInfo._maxVal - variableInfo._minVal <= 0)
      {
        std::cout << roundInfo._name << "::" << variableInfo._name << ": maxVal and minVal must be specified and define a valid range of values. Continuing..." << std::endl;
        continue;
      }
      variableInfo._step = variable->getDouble("step", 0);
      if(variableInfo._step <= 0 || variableInfo._step > variableInfo._maxVal - variableInfo._minVal)
      {
        std::cout << roundInfo._name << "::" << variableInfo._name << ": step must be a resonable and valid value. Continuing..." << std::endl;
        continue;
      }
      variableInfo._cutDir = variable->getString("cutDir", "below");
      std::transform(variableInfo._cutDir.begin(), variableInfo._cutDir.end(), variableInfo._cutDir.begin(), ::tolower);
      if(variableInfo._cutDir != "below" && variableInfo._cutDir != "above")
      {
        std::cout << roundInfo._name << "::" << variableInfo._name << ": the cut direction (cutDir) must be either below or above. Continuing..." << std::endl;
        continue;
      }
      variableInfo._label = variable->getString("label", "");

      roundInfo._variables.push_back(variableInfo);
    }

    retVal.push_back(roundInfo);
  }

  return retVal;
}

fileChains getChainsFromJSON(JSONWrapper::Object& json, std::string RootDir, std::string type, std::string treename, std::string customExtension)
{
  std::vector<std::pair<std::string,std::vector<std::pair<int,TChain*>>>> retVal;
  std::pair<std::string,std::vector<std::pair<int,TChain*>>> tempProcess;
  std::vector<std::pair<int,TChain*>> tempSamples;
  std::pair<int,TChain*> tempSample;

  if(type != "BG" && type != "SIG")
  {
    std::cout << "Unknown sample type requested." << std::endl;
    assert(type == "BG" || type == "SIG");
  }

  std::vector<JSONWrapper::Object> processes = json["proc"].daughters();
  for(auto process = processes.begin(); process != processes.end(); ++process)
  {
    bool isData = (*process)["isdata"].toBool();
    bool isSig  = !isData && (*process).isTag("spimpose") && (*process)["spimpose"].toBool();
    bool isMC   = !isData && !isSig;

    if(isData) // Here we are enforcing for the data samples to not even be present, might not make sense for a data-driven background estimation
      continue;

    if(type == "SIG" && isMC) // Here we make sure we are only processing the requested processes
      continue;
    if(type == "BG" && isSig)
      continue;

    tempProcess.first = (*process).getString("tag", "Sample");

    std::string filtExt;
    if((*process).isTag("mctruthmode"))
    {
      std::stringstream buf;
      buf << "_filt" << (*process)["mctruthmode"].toInt();
      buf >> filtExt;
    }

    std::vector<JSONWrapper::Object> samples = (*process)["data"].daughters();
    tempSamples.clear();
    for(auto sample = samples.begin(); sample != samples.end(); ++sample)
    {
      int nFiles = (*sample).getInt("split", 1);
      tempSample.first = 0;
      tempSample.second = new TChain(treename.c_str(), ((*sample).getString("dtag", "") + (*sample).getString("suffix", "")).c_str());
      for(int file = 0; file < nFiles; ++file)
      {
        std::string segmentExt;
        if(nFiles != 1)
        {
          std::stringstream buf;
          buf << "_" << file;
          buf >> segmentExt;
        }

        std::string fileName = RootDir + "/" + (*sample).getString("dtag", "") + (*sample).getString("suffix", "") + segmentExt + filtExt + customExtension + ".root";
        TFile* file = new TFile(fileName.c_str());
        bool& fileExists = FileExists[fileName];

        if(!file || file->IsZombie() || !file->IsOpen() || file->TestBit(TFile::kRecovered))
        {
          fileExists = false;
          file->Close();
          delete file;
          continue;
        }
        else
        {
          fileExists = true;
          file->Close();
          delete file;
        }

        // Chain the valid files together
        tempSample.second->Add(fileName.c_str());
        ++tempSample.first;
      }
      tempSamples.push_back(tempSample);
    }
    tempProcess.second = tempSamples;
    retVal.push_back(tempProcess);
  }

  return retVal;
}

CutOptimizer::CutOptimizer(const std::string & jsonFile, const std::string & outDir, const std::vector<std::string> & plotExt):
  jsonFile_(jsonFile), outDir_(outDir), plotExt_(plotExt), jsonLoaded(false), verbose_(false)
{
//  json = new JSONWrapper::Object(jsonFile_, true);
}

CutOptimizer::~CutOptimizer()
{
  std::cout << "The list of ignored files, either missing or corrupt, can be found below:" << std::endl;
  for(auto key = FileExists_.begin(); key != FileExists_.end(); ++key)
  {
    if(!key->second)
      std::cout << "  " << key->first << std::endl;
  }
}

bool CutOptimizer::LoadJson()
{
  if(jsonLoaded)
    return true;

  roundInfo_.clear();
  JSONWrapper::Object json(jsonFile_, true);

  std::vector<JSONWrapper::Object> rounds = json["optim"].daughters();
  for(auto round = rounds.begin(); round != rounds.end(); ++round)
  {
    OptimizationRoundInfo roundInfo;

    std::string name = round->getString("name", "");
    if(name != "")
      roundInfo._name = name;
    roundInfo._iLumi = round->getDouble("iLumi", 0);
    if(roundInfo._iLumi <= 0)
    {
      std::cout << roundInfo._name << ": Integrated luminosity should be positive and non-zero. Continuing..." << std::endl;
      continue;
    }
    roundInfo._sigCrossSection = round->getDouble("sigCrossSection", 0);
    if(roundInfo._sigCrossSection <= 0)
    {
      std::cout << roundInfo._name << ": Signal cross section should be positive and non-zero. Continuing..." << std::endl;
      continue;
    }
    roundInfo._inDir = round->getString("inDir", "");
    if(roundInfo._inDir == "")
    {
      std::cout << roundInfo._name << ": Input directory should be defined in the json for the optimization round. Continuing..." << std::endl;
      continue;
    }
    roundInfo._jsonFile = round->getString("jsonFile", "");
    if(roundInfo._jsonFile == "")
    {
      std::cout << roundInfo._name << ": JSON file must be specified for optimization round in cut optimization JSON. Continuing..." << std::endl;
      continue;
    }

    roundInfo._ttree = round->getString("ttree", roundInfo._ttree);
    roundInfo._customExtension = round->getString("customExtension", roundInfo._customExtension);
    roundInfo._baseSelection = round->getString("baseSelection", roundInfo._baseSelection);
    roundInfo._signalSelection = round->getString("signalSelection", roundInfo._signalSelection);
    roundInfo._signalPoint = round->getString("signalPoint", roundInfo._signalPoint);
    roundInfo._channel = round->getString("channel", roundInfo._channel);
    roundInfo._pointVariable = round->getString("pointVariable", roundInfo._pointVariable);
    roundInfo._nInitEvents = round->getInt("nInitEvents", roundInfo._nInitEvents);

    auto userCuts = (*round)["userCuts"].daughters();
    for(auto userCut = userCuts.begin(); userCut != userCuts.end(); ++userCut)
    {
      UserCutInfo temp;
      temp.variable = userCut->getString("variable", "");
      temp.direction = userCut->getString("direction", "");
      temp.value = userCut->getDouble("value", -9999);
      if(temp.variable != "" && (temp.direction == "below" || temp.direction == "above"))
        roundInfo._UserCuts.push_back(temp);
    }

    auto variables = (*round)["variables"].daughters();
    for(auto variable = variables.begin(); variable != variables.end(); ++variable)
    {
      OptimizationVariableInfo variableInfo;

      variableInfo._name = variable->getString("name", "");
      if(variableInfo._name == "")
      {
        std::cout << roundInfo._name << ": All variables must have names. Continuing..." << std::endl;
        continue;
      }
      variableInfo._expression = variable->getString("expression", "");
      if(variableInfo._expression == "")
      {
        std::cout << roundInfo._name << "::" << variableInfo._name << ": This variable must have an expression, it must be a valid root expression. Continuing..." << std::endl;
        continue;
      }
      variableInfo._minVal = variable->getDouble("minVal", 0);
      variableInfo._maxVal = variable->getDouble("maxVal", 0);
      if(variableInfo._maxVal - variableInfo._minVal <= 0)
      {
        std::cout << roundInfo._name << "::" << variableInfo._name << ": maxVal and minVal must be specified and define a valid range of values. Continuing..." << std::endl;
        continue;
      }
      variableInfo._step = variable->getDouble("step", 0);
      if(variableInfo._step <= 0 || variableInfo._step > variableInfo._maxVal - variableInfo._minVal)
      {
        std::cout << roundInfo._name << "::" << variableInfo._name << ": step must be a resonable and valid value. Continuing..." << std::endl;
        continue;
      }
      variableInfo._cutDir = variable->getString("cutDir", "below");
      std::transform(variableInfo._cutDir.begin(), variableInfo._cutDir.end(), variableInfo._cutDir.begin(), ::tolower);
      if(variableInfo._cutDir != "below" && variableInfo._cutDir != "above")
      {
        std::cout << roundInfo._name << "::" << variableInfo._name << ": the cut direction (cutDir) must be either below or above. Continuing..." << std::endl;
        continue;
      }
      variableInfo._label = variable->getString("label", "");

      roundInfo._variables.push_back(variableInfo);
    }

    roundInfo_.push_back(roundInfo);
  }

  if(roundInfo_.size() != 0)
    jsonLoaded = true;
  else
    std::cout << "Unable to load JSON, it might be empty/malformed or not present. Please check and try again." << std::endl;
  return jsonLoaded;
}

std::string& CutOptimizer::setJson(const std::string & jsonFile)
{
  if(jsonFile_ != jsonFile)
  {
    jsonLoaded = false;
    ClearSamples();
  }
  jsonFile_ = jsonFile;
  return jsonFile_;
}

std::string& CutOptimizer::setOutDir(const std::string & outDir)
{
  outDir_ = outDir;
  return outDir_;
}

std::vector<std::string>& CutOptimizer::setPlotExt(const std::vector<std::string> & plotExt)
{
  plotExt_ = plotExt;
  return plotExt_;
}

bool CutOptimizer::OptimizeAllRounds()
{
  bool retVal = true;

  if(!jsonLoaded)
  {
    std::cout << "The JSON file has not yet been loaded, attempting to load it" << std::endl;
    LoadJson();
    if(!jsonLoaded)
      return false;
  }

  for(size_t i = 0; i < roundInfo_.size(); ++i)
  {
    if(!OptimizeRound_(i))
      retVal = false;
  }

  return retVal;
}

bool CutOptimizer::OptimizeRound(size_t n)
{
  if(!jsonLoaded)
  {
    std::cout << "The JSON file has not yet been loaded, attempting to load it" << std::endl;
    LoadJson();
    if(!jsonLoaded)
      return false;
  }

  if(n > roundInfo_.size())
    return false;

  return OptimizeRound_(n);
}

bool CutOptimizer::OptimizeRound_(size_t n)
{
  bool retVal = false;
  if(roundInfo_[n].nVars() == 0)
  {
    std::cout << "There are no variables to optimize cuts on, skipping this round." << std::endl;
    return retVal;
  }

  system(("mkdir -p " + outDir_).c_str());
  std::stringstream stream;
  stream << "Round_" << n;
  std::string roundName;
  stream >> roundName;

  std::streambuf *coutbuf = std::cout.rdbuf();
  std::streambuf *cerrbuf = std::cerr.rdbuf();
  ofstream out(outDir_+"/"+roundName+".log", std::ios::out | std::ios::trunc);
  std::cout.rdbuf(out.rdbuf());
  std::cerr.rdbuf(out.rdbuf());

  std::cout << "Processing round: " << roundInfo_[n].name() << std::endl;
  std::cout << "\tReading from " << roundInfo_[n].jsonFile() << " and taking samples from " << roundInfo_[n].inDir() << " directory." << std::endl;
  std::cout << "\tUsing an integrated luminosity of " << roundInfo_[n].iLumi() << "." << std::endl;
  std::cout << "\tReading from ttree: " << roundInfo_[n].ttree();
  if(roundInfo_[n].baseSelection() != "")
    std::cout << ", with a base selection of \"" << roundInfo_[n].baseSelection() << "\"";
  if(roundInfo_[n].channel() != "")
    std::cout << " and performing cut optimization on the channel " << roundInfo_[n].channel();
  std::cout << "." << std::endl;
  std::cout << "\tThere are " << roundInfo_[n].nVars() << " variables to perform cut optimization on." << std::endl;

  GetSamples(n);

  std::cout << "\tFound " << processes_["BG"].size()  << " background processes:" << std::endl;
  for(auto process = processes_["BG"].begin(); process != processes_["BG"].end(); ++process)
  {
    std::cout << "\t  " << process->name << ":" << std::endl;
    for(auto sample = process->samples.begin(); sample != process->samples.end(); ++sample)
      std::cout << "\t    " << sample->chain->GetTitle() << " with " << sample->chain->GetEntries() << " entries in " << sample->nFiles << " files" << std::endl;
  }
  std::cout << "\tFound " << processes_["SIG"].size()  << " signal processes:" << std::endl;
  isMultipointSignalSample_ = false;
  for(auto process = processes_["SIG"].begin(); process != processes_["SIG"].end(); ++process)
  {
    std::cout << "\t  " << process->name << ":" << std::endl;
    if(process->name.find("TStauStau") != std::string::npos)
      isMultipointSignalSample_ = true;
    for(auto sample = process->samples.begin(); sample != process->samples.end(); ++sample)
      std::cout << "\t    " << sample->chain->GetTitle() << " with " << sample->chain->GetEntries() << " entries in " << sample->nFiles << " files" << std::endl;
  }

  if(processes_["BG"].size() != 0 && processes_["SIG"].size() != 0)
  {
    if(isMultipointSignalSample_)
    {
      nSignalPoints_ = GetSigPoints(n);
      std::cout << "Running on " << nSignalPoints_ << " signal points." << std::endl;
    }
    else
    {
      nSignalPoints_ = 1;
    }
    bool improve = true;

    ReportInfo myReport;
    myReport.round = n;

    CutInfo previousCut;
    previousCut.FOM = 0;
    previousCut.FOMerr = 0;

    while(improve && nSignalPoints_ != 0)
    {
      std::cout << roundInfo_[n].name() << ": Starting pass " << myReport.cuts.size() << std::endl;

      CutInfo thisCut = GetBestCutAndMakePlots(n, myReport);

      bool isSelected = roundInfo_[n].isSelected(myReport.cuts.size());

      if(thisCut.FOM == 0 && !isSelected)
      {
        improve = false;
      }
      else
      {
        if(thisCut.FOM - previousCut.FOM > previousCut.FOMerr || isSelected)
        {
          previousCut = thisCut;
          myReport.cuts.push_back(thisCut);
          std::cout << "Added cut on " << thisCut.variable << " with values " << thisCut.direction << " " << thisCut.value << ".";
          std::cout << "The cut has a Figure of Merit (FOM) of " << thisCut.FOM << " +- " << thisCut.FOMerr << "; with the following yields: To-Do"; //TODO
//          std::cout << "\tBG:" << thisCut.yield["BG"]
        }
        else
          improve = false;
      }
    }

    // TODO - Print the final selection info
    // TODO - Make the report (including the tex table with the yields)
  }
  else
  {
    if(processes_["BG"].size() == 0)
      std::cout << "\tIt doesn't make sense to perform cut optimization without any background samples. Skipping this round of optimization, please check the file " << roundInfo_[n].jsonFile() << "." << std::endl;
    if(processes_["SIG"].size() == 0)
      std::cout << "\tIt doesn't make sense to perform cut optimization without any signal samples. Skipping this round of optimization, please check the file " << roundInfo_[n].jsonFile() << "." << std::endl;
  }

  std::cout.rdbuf(coutbuf);
  std::cerr.rdbuf(cerrbuf);
  out.close();

  return retVal;
}

CutInfo CutOptimizer::GetBestCutAndMakePlots(size_t n, ReportInfo& report)
{
  CutInfo retVal;
  std::vector<std::string> variables = roundInfo_[n].getListOfVariables(); // TODO - buffer this stuff
  std::unordered_map<std::string,std::string> variableExpressions = roundInfo_[n].getVariableExpressions();
  std::unordered_map<std::string,std::unordered_map<std::string,double>> variableParameterMap = roundInfo_[n].getVariableParameterMap();
  std::unordered_map<std::string,std::string> variableLabels = roundInfo_[n].getVariableLabels();

  TCut baseSelection = roundInfo_[n].baseSelection().c_str();
  TCut signalSelection = roundInfo_[n].signalSelection().c_str();
  TCut cumulativeSelection = "";

  // Clean looping vector of variables already cut on and get the cumulative selection
  for(auto pass = report.cuts.begin(); pass != report.cuts.end(); ++pass)
  {
    auto newEnd = std::remove(variables.begin(), variables.end(), pass->variable);
    variables.erase(newEnd, variables.end());

    std::string tmpStr;
    std::stringstream buf;
    buf << pass->value;
    buf >> tmpStr;
    if(pass->direction == "below")
    {
      tmpStr = "<" + tmpStr;
    }
    else
    {
      if(pass->direction == "above")
        tmpStr = ">" + tmpStr;
      else
        assert(pass->direction == "below" || pass->direction == "above");
    }
    tmpStr = variableExpressions[pass->variable] + tmpStr;
    cumulativeSelection = cumulativeSelection && tmpStr.c_str();
  }

  bool isSelected = roundInfo_[n].isSelected(report.cuts.size());

  // Loop on variables
  for(auto variableName = variables.begin(); variableName != variables.end(); ++variableName)
  {
    double minVal = variableParameterMap[*variableName]["minVal"];
    double maxVal = variableParameterMap[*variableName]["maxVal"];
    double step   = variableParameterMap[*variableName]["step"];
    double cutDir = variableParameterMap[*variableName]["cutDir"]; // TODO - not being used, remove it, from everywhere
    std::cout << roundInfo_[n].name() << "::" << *variableName << " has started processing, with " << (maxVal-minVal)/step + 1 << " steps to be processed." << std::endl;

    std::vector<double> xVals, yVals, xValsUnc, yValsUnc;
    std::vector<double> signalYield, backgroundYield, signalYieldUnc, backgroundYieldUnc;

    for(double cutVal = minVal; cutVal <= maxVal; cutVal += step)
    {
      std::string thisCutStr;
      std::stringstream buf;
      buf << ">"; // TODO - scan for the opposite cut
      buf << cutVal;
      buf >> thisCutStr;
      thisCutStr = variableExpressions[*variableName] + thisCutStr;
      std::cout << "  The Cut: " << thisCutStr << std::endl;
      TCut thisCut = thisCutStr.c_str();
      TCut currentSelection = baseSelection && cumulativeSelection && thisCut;

      std::map<std::string,std::map<std::string,std::map<std::string,doubleUnc>>> yield = GetYields(report, signalSelection, currentSelection, roundInfo_[n].iLumi());

      double systUnc = 0.15;  // Hard coded systematic uncertainty /////////////////////////////////////////////////////////////////////////////////////
      // TODO - Add option to run with full systematic uncertainty (ie: calculate full analysis at each point) - is this feasible and reasonable?
      double nSIG = 0, nSIGUnc = 0;
      double nBG = 0, nBGUnc = 0;

      for(auto process = yield["SIG"].begin(); process != yield["SIG"].end(); ++process)
      {
        for(auto sample = process->second.begin(); sample != process->second.end(); ++sample)
        {
          nSIG += sample->second.value;
          nSIGUnc += sample->second.uncertainty*sample->second.uncertainty;
        }
      }
      nSIGUnc = std::sqrt(nSIGUnc);
      for(auto process = yield["BG"].begin(); process != yield["BG"].end(); ++process)
      {
        for(auto sample = process->second.begin(); sample != process->second.end(); ++sample)
        {
          nBG += sample->second.value;
          nBGUnc += sample->second.uncertainty*sample->second.uncertainty;
        }
      }
      nBGUnc = std::sqrt(nBGUnc);

      std::cout << "    n(Sig): " << nSIG << " +- " << nSIGUnc << std::endl;
      std::cout << "    n(Bkg): " << nBG  << " +- " << nBGUnc << std::endl;

      if(nBG == 0 || nSIG == 0)
        break;

      double dividend = std::sqrt(nBG + (systUnc*nBG)*(systUnc*nBG));
      double FOM = nSIG/dividend;
      double FOMUnc = 0;
      {
        double SIGContrib = nSIGUnc/dividend;
        double BGContrib = nBGUnc*nSIG*(1+2*systUnc*systUnc*nBG)/(2*dividend*dividend*dividend);
        FOMUnc = std::sqrt(SIGContrib*SIGContrib + BGContrib*BGContrib);
      }
      std::cout << "    FOM: " << FOM << " +- " << FOMUnc << std::endl;

      xVals.push_back(cutVal);
      xValsUnc.push_back(0);
      yVals.push_back(FOM);
      yValsUnc.push_back(FOMUnc);
      signalYield.push_back(nSIG);
      signalYieldUnc.push_back(nSIGUnc);
      backgroundYield.push_back(nBG);
      backgroundYieldUnc.push_back(nBGUnc);

      if(retVal.FOM < FOM)
      {
        retVal.FOM = FOM;
        retVal.FOMerr = FOMUnc;
        retVal.variable = *variableName;
        retVal.direction = "above";
        retVal.value = cutVal;
        retVal.yield = yield;
      }

      //if(nSIG < 0.05)  // Minimum number of signal events: TODO - implement option in json to specify this value
      //  break;
    }

    if(xVals.size() == 0)
    {
      std::cout << "  No points processed for \"above\"" << std::endl;
    }
    else
    {
      std::string xTitle = variableLabels[*variableName];
      std::string yTitle = "FOM";
      std::string name;
      std::stringstream buf;
      buf << outDir_ << "/";
      buf << roundInfo_[n].name() << "_Pass" << report.cuts.size() << "_" << *variableName;
      buf >> name;

      name = name + "_above";

      SaveGraph(name, xVals, xValsUnc, xTitle, yVals, yValsUnc, yTitle);
      yTitle = "SignalYield";
      std::string tmp = name+"_signalYield";
      SaveGraph(tmp, xVals, xValsUnc, xTitle, signalYield, signalYieldUnc, yTitle);
      yTitle = "BackgroundYield";
      tmp = name+"_backgroundYield";
      SaveGraph(tmp, xVals, xValsUnc, xTitle, backgroundYield, backgroundYieldUnc, yTitle);
    }

    xVals.clear();
    yVals.clear();
    xValsUnc.clear();
    xValsUnc.clear();
    signalYield.clear();
    backgroundYield.clear();
    signalYieldUnc.clear();
    backgroundYieldUnc.clear();

    for(double cutVal = maxVal; cutVal >= minVal; cutVal -= step)
    {
    }

    if(xVals.size() == 0)
    {
      std::cout << "  No points processed for \"below\"" << std::endl;
    }
    else
    {
      std::string xTitle = variableLabels[*variableName];
      std::string yTitle = "FOM";
      std::string name;
      std::stringstream buf;
      buf << outDir_ << "/";
      buf << roundInfo_[n].name() << "_Pass" << report.cuts.size() << "_" << *variableName;
      buf >> name;

      name = name + "_below";

      SaveGraph(name, xVals, xValsUnc, xTitle, yVals, yValsUnc, yTitle);
      yTitle = "SignalYield";
      std::string tmp = name+"_signalYield";
      SaveGraph(tmp, xVals, xValsUnc, xTitle, signalYield, signalYieldUnc, yTitle);
      yTitle = "BackgroundYield";
      tmp = name+"_backgroundYield";
      SaveGraph(tmp, xVals, xValsUnc, xTitle, backgroundYield, backgroundYieldUnc, yTitle);
    }
  }

  if(isSelected)
  {
    std::string thisCutStr = roundInfo_[n].getUserCut(report.cuts.size());
    std::cout << "  Found a user-defined cut for this pass: " << thisCutStr;

    TCut thisCut = thisCutStr.c_str();
    TCut currentSelection = baseSelection && cumulativeSelection && thisCut;

    std::map<std::string,std::map<std::string,std::map<std::string,doubleUnc>>> yield = GetYields(report, signalSelection, currentSelection, roundInfo_[n].iLumi());

    double systUnc = 0.15;  // Hard coded systematic uncertainty /////////////////////////////////////////////////////////////////////////////////////
    // TODO - Add option to run with full systematic uncertainty (ie: calculate full analysis at each point) - is this feasible and reasonable?
    double nSIG = 0, nSIGUnc = 0;
    double nBG = 0, nBGUnc = 0;

    for(auto process = yield["SIG"].begin(); process != yield["SIG"].end(); ++process)
    {
      for(auto sample = process->second.begin(); sample != process->second.end(); ++sample)
      {
        nSIG += sample->second.value;
        nSIGUnc += sample->second.uncertainty*sample->second.uncertainty;
      }
    }
    nSIGUnc = std::sqrt(nSIGUnc);
    for(auto process = yield["BG"].begin(); process != yield["BG"].end(); ++process)
    {
      for(auto sample = process->second.begin(); sample != process->second.end(); ++sample)
      {
        nBG += sample->second.value;
        nBGUnc += sample->second.uncertainty*sample->second.uncertainty;
      }
    }
    nBGUnc = std::sqrt(nBGUnc);

    std::cout << "    n(Sig): " << nSIG << " +- " << nSIGUnc << std::endl;
    std::cout << "    n(Bkg): " << nBG  << " +- " << nBGUnc << std::endl;

    double dividend = std::sqrt(nBG + (systUnc*nBG)*(systUnc*nBG));
    double FOM = 0;
    double FOMUnc = 0;
    if(nBG != 0)
    {
      FOM = nSIG/dividend;
      double SIGContrib = nSIGUnc/dividend;
      double BGContrib = nBGUnc*nSIG*(1+2*systUnc*systUnc*nBG)/(2*dividend*dividend*dividend);
      FOMUnc = std::sqrt(SIGContrib*SIGContrib + BGContrib*BGContrib);
    }
    std::cout << "    FOM: " << FOM << " +- " << FOMUnc << std::endl;

    retVal.FOM = FOM;
    retVal.FOMerr = FOMUnc;
    retVal.variable = roundInfo_[n].getUserCutVar(report.cuts.size());;
    retVal.direction = roundInfo_[n].getUserCutDir(report.cuts.size());;
    retVal.value = roundInfo_[n].getUserCutValue(report.cuts.size());;
    retVal.yield = yield;
  }

  return retVal;
}

/*struct CutInfo
{
  std::string variable;
  std::string direction;
  std::string value;
  std::map<std::string,std::map<std::string,std::map<std::string,doubleUnc>>> yield;
  double FOM;
  double FOMerr;
};

struct ReportInfo
{
  int round;
  std::vector<CutInfo> cuts;
}

struct SampleFiles
{
  std::string name;
  TChain* chain;
  int nFiles;
};

struct ProcessFiles
{
  std::string name;
  std::vector<SampleFiles> samples;
};// */

void CutOptimizer::SaveGraph(std::string& name, std::vector<double>& xVals, std::vector<double>& xValsUnc, std::string& xTitle, std::vector<double>& yVals, std::vector<double>& yValsUnc, std::string& yTitle)
{
  if(xVals.size() == 0 || yVals.size() == 0)
    return;

  gROOT->cd();
  TCanvas c1("c1", "c1", 800, 600);

  TGraphErrors Graph(xVals.size(), xVals.data(), yVals.data(), xValsUnc.data(), yValsUnc.data());
  // http://root.cern.ch/root/html/TAttMarker.html
  //FOMGraph.SetLineColor(2);
  //FOMGraph.SetLineWidth(2);
  //FOMGraph.SetMarkerColor(4);
  //FOMGraph.SetMarkerStyle(21);
  //http://root.cern.ch/root/html/TAttFill.html
  //http://root.cern.ch/root/html/TColor.html
  Graph.SetFillColor(kBlue-9);
  Graph.SetFillStyle(3354);

  if(yValsUnc.size() == 0)
  {
    Graph.SetMarkerStyle(21);
    Graph.Draw("ALP");
  }
  else
  {
    Graph.Draw("3A");
    Graph.Draw("same LP");
  }

  Graph.GetXaxis()->SetTitle(xTitle.c_str());
  Graph.GetYaxis()->SetTitle(yTitle.c_str());
  Graph.GetXaxis()->CenterTitle();

  for(auto ext = plotExt_.begin(); ext != plotExt_.end(); ++ext)
  {
    c1.SaveAs((name + *ext).c_str());
  }

  return;
}

std::map<std::string,std::map<std::string,std::map<std::string,doubleUnc>>> CutOptimizer::GetYields(ReportInfo& report, TCut signalSelection, TCut currentSelection, double integratedLuminosity)
{
  gROOT->cd();
  std::map<std::string,std::map<std::string,std::map<std::string,doubleUnc>>> retVal;

  for(auto index = processes_.begin(); index != processes_.end(); ++index)
  {
    std::map<std::string,std::map<std::string,doubleUnc>> typeYieldMap;
    for(auto process = index->second.begin(); process != index->second.end(); ++process)
    {
      std::map<std::string,doubleUnc> processYieldMap;
      for(auto sample = process->samples.begin(); sample != process->samples.end(); ++sample)
      {
        if(sample->nFiles == 0)
          continue;
        doubleUnc sampleYield;

        TH1D temp_histo("temp_histo", "temp_histo", 1, 0, 20);
        temp_histo.Sumw2();

        TCut cut = currentSelection;
        if(index->first == "SIG" && isMultipointSignalSample_)
          cut = signalSelection && currentSelection;

        if(nSignalPoints_ > 1 && index->first == "SIG")
          sample->chain->Draw("puWeight>>temp_histo", cut*"puWeight", "goff");
        else
          sample->chain->Draw("weight>>temp_histo", cut*"weight", "goff");

        sampleYield.value = temp_histo.GetBinContent(0) + temp_histo.GetBinContent(1) + temp_histo.GetBinContent(2);
        //double a = temp_histo.GetBinError(0);
        //double b = temp_histo.GetBinError(1);
        //double c = temp_histo.GetBinError(2);
        //double uncertainty = std::sqrt(a*a + b*b + c*c);
        TArrayD* w2Vec = temp_histo.GetSumw2();
        sampleYield.uncertainty = std::sqrt(w2Vec->fArray[0] + w2Vec->fArray[1] + w2Vec->fArray[2]);

        if(index->first != "Data")
        {
          if(!isMultipointSignalSample_ || index->first != "SIG")
          {
            sampleYield.value       = sampleYield.value       /sample->nFiles;
            sampleYield.uncertainty = sampleYield.uncertainty /sample->nFiles;
          }
          else
          {
            if(nSignalPoints_ > 1)
            {
              sampleYield.value       = (sampleYield.value       * roundInfo_[report.round].sigCrossSection())/(roundInfo_[report.round].nInitEvents() * nSignalPoints_);
              sampleYield.uncertainty = (sampleYield.uncertainty * roundInfo_[report.round].sigCrossSection())/(roundInfo_[report.round].nInitEvents() * nSignalPoints_);
            }
          }

          sampleYield.value       = sampleYield.value * integratedLuminosity;
          sampleYield.uncertainty = sampleYield.uncertainty * integratedLuminosity;
        }

        processYieldMap[sample->name] = sampleYield;
      }
      typeYieldMap[process->name] = processYieldMap;
    }
    retVal[index->first] = typeYieldMap;
  }

  return retVal;
}

int CutOptimizer::GetSigPoints(size_t n)
{
  gROOT->cd();
  int retVal = 0;
  int maxVal = 501*1000+1; //Change-me if you are having problems with multipart signal samples and it only finds some of them
  TH1D finalHist("finalHist", "finalHist", maxVal, 0, maxVal);

  std::string variable = roundInfo_[n].pointVariable();
  TCut baseSelection = roundInfo_[n].baseSelection().c_str();
  TCut signalSelection = roundInfo_[n].signalSelection().c_str();
  TCut cut = baseSelection && signalSelection;

  for(auto process = processes_["SIG"].begin(); process != processes_["SIG"].end(); ++process)
  {
    for(auto sample = process->samples.begin(); sample != process->samples.end(); ++sample)
    {
      if(sample->nFiles == 0)
        continue;

      TH1D* temp_hist = new TH1D("temp_hist", "temp_hist", maxVal, 0, maxVal);
      sample->chain->Draw((variable+">>temp_hist").c_str(), cut*"weight", "goff");
      finalHist.Add(temp_hist);
      delete temp_hist;
    }
  }

  int nBins = finalHist.GetNbinsX();
  for(int i = 1; i <= nBins; ++i)
    if(finalHist.GetBinContent(i) != 0)
      ++retVal;

  return retVal;
}

bool CutOptimizer::GetSamples(size_t n)
{
  ClearSamples();
  JSONWrapper::Object json(roundInfo_[n].jsonFile(), true);
  std::vector<JSONWrapper::Object> processes = json["proc"].daughters();

  for(auto process = processes.begin(); process != processes.end(); ++process)
  {
    bool isData = (*process)["isdata"].toBool();
    bool isSig  = !isData && (*process).isTag("spimpose") && (*process)["spimpose"].toBool();
    bool isMC   = !isData && !isSig;

    std::string type = "Data";
    if(isMC)
      type = "BG";
    if(isSig)
      type = "SIG";

    ProcessFiles tempProc;
    tempProc.name = (*process).getString("tag", "Sample");

    std::string filtExt;
    if((*process).isTag("mctruthmode"))
    {
      std::stringstream buf;
      buf << "_filt" << (*process)["mctruthmode"].toInt();
      buf >> filtExt;
    }

    std::vector<JSONWrapper::Object> samples = (*process)["data"].daughters();
    for(auto sample = samples.begin(); sample != samples.end(); ++sample)
    {
      SampleFiles tempSample;
      tempSample.nFiles = 0;
      tempSample.name = (*sample).getString("dtag", "");
      tempSample.chain = new TChain(roundInfo_[n].ttree().c_str(), ((*sample).getString("dtag", "") + (*sample).getString("suffix", "")).c_str());
      int nFiles = (*sample).getInt("split", 1);

      for(int file = 0; file < nFiles; ++file)
      {
        std::string segmentExt;
        if(nFiles != 1)
        {
          std::stringstream buf;
          buf << "_" << file;
          buf >> segmentExt;
        }

        std::string fileName = roundInfo_[n].inDir() + "/" + (*sample).getString("dtag", "") + (*sample).getString("suffix", "") + segmentExt + filtExt + roundInfo_[n].customExtension() + ".root";

        TFile* file = new TFile(fileName.c_str(), "READONLY");
        bool& fileExists = FileExists_[fileName];

        if(!file || file->IsZombie() || !file->IsOpen() || file->TestBit(TFile::kRecovered))
        {
          fileExists = false;
          file->Close();
          delete file;
          continue;
        }
        else
        {
          fileExists = true;
          file->Close();
          delete file;
        }

        tempSample.chain->Add(fileName.c_str());
        ++(tempSample.nFiles);
      }

      tempProc.samples.push_back(tempSample);
    }

    processes_[type].push_back(tempProc);
  }

  return true;
}

bool CutOptimizer::ClearSamples()
{
  for(auto iterator = processes_.begin(); iterator != processes_.end(); ++iterator)
  {
    for(auto process = iterator->second.begin(); process != iterator->second.end(); ++process)
    {
      for(auto sample = process->samples.begin(); sample != process->samples.end(); ++sample)
      {
        delete sample->chain;
      }
    }
  }
  processes_.clear();

  std::vector<ProcessFiles> temp;
  processes_["BG"] = temp;
  processes_["SIG"] = temp;
  processes_["Data"] = temp;

  return true;
}

void printHelp()
{
  std::cout << "runCutOptimizer help - There are the following options:" << std::endl << std::endl;

  std::cout << "--help    -->  Print this help message" << std::endl;
  std::cout << "--json    -->  Configuration file for cut optimization, should define which files to run on, where they are located, the integrated luminosity and the variables to optimize the cuts on" << std::endl;
  std::cout << "--outDir  -->  Path to the directory where to output plots and tables (will be created if it doesn't exist)" << std::endl;
  std::cout << "--plotExt -->  Extension format with which to save the plots, repeat this command if multiple formats are desired" << std::endl;
  std::cout << "--verbose -->  Set to verbose mode, the current step will be printed out to screen" << std::endl;

  std::cout << std::endl << "Example command:" << std::endl << "\trunCutOptimizer --json optimization_options.json --outDir ./OUT/" << std::endl;
  return;
}

