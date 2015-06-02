#include <iostream>
#include <boost/shared_ptr.hpp>
#include "Math/GenVector/Boost.h"

#include "UserCode/llvv_fwk/interface/tdrstyle.h"
#include "UserCode/llvv_fwk/interface/JSONWrapper.h"
#include "UserCode/llvv_fwk/interface/MacroUtils.h"
#include "UserCode/llvv_fwk/interface/RootUtils.h"

#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TROOT.h"
#include "TString.h"
#include "TList.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TObjArray.h"
#include "THStack.h"
#include "TGraphErrors.h"

#include<iostream>
#include<fstream>
#include<map>
#include<algorithm>
#include<vector>
#include<set>

using namespace std;

TString outUrl("./");
TString suffixUrl(""); 
TString inFileUrl("");
TString systFileUrl("");
TString jsonFileUrl("");
TString histo("finalevtflow");
TString signalTag("");
std::set<TString> systVars;
std::vector<int> binsToProject;
std::vector<std::string> channels;
float lumiUnc(0.027);
float selEffUnc(0.0374); // STILL USED FOR THE TABLE, set to the quadratical sum of the muonid, electronid, trigeff
float muonidUnc(0.01);
float electronidUnc(0.02);
float emuTrigEffUnc(0.03);
float eeTrigEffUnc(0.03);
float mumuTrigEffUnc(0.03);
float iEcm(8);
bool doPowers = true;
bool statBinByBin = false;

//wrapper for a projected shape for a given set of cuts
struct Shape_t
{
  TH1F* data, *totalBckg, *totalSplusB, *signal;
  std::vector<TH1F*> bckg;
  //  std::vector<TH1F*> signalMassPoints; 
  
  //the first key corresponds to the proc name
  //the second key is the name of the variation: e.g. jesup, jesdown, etc.
  std::map<TString,std::map<TString, TH1F*> > bckgVars;
  std::set<TString> dataDrivenBckg;
  std::map<TString, TH1F*> signalVars;
  
  //cross section and uncertainties
  std::map<TString,std::pair<float,float> > crossSections;

  //signal rate uncertainties
  std::map<TString,std::pair<float,float> > rateUncs;
};


//
void printHelp();
Shape_t getShapeFromFile(TFile* inF, TString ch,JSONWrapper::Object &Root,TFile *systF=0);
void getYieldsFromShapes(const map<TString, Shape_t> &allShapes);
void convertShapesToDataCards(const map<TString, Shape_t> &allShapes);
void saveShapeForMeasurement(TH1F *h, TDirectory *oDir,TString syst="");
TString convertNameForDataCard(TString title);
TString convertNameForFileName(TString histoName);
TString convertMassPointNameForFileName(TString title);
float getIntegratedSystematics(TH1F *h,const std::map<TString, TH1F*> &hSysts, std::map<TString,float> &rateSysts);
std::map<TString,float> getDYUncertainties(TString ch);

//
std::map<TString,float> getDYUncertainties(TString ch)
{
  //assign the bin to use
  int dybin=-1;
  if(ch=="ee")   dybin=0;
  if(ch=="mumu") dybin=1;
  if(ch=="emu")  dybin=2;

  //build the uncertainty map (for an inclusive channel I'm averaging the unc - it doesn't really matter as it will be combined in the end)
  std::map<TString,float> dysfUncs; 
  float stat[]        = {0.064,      0.038,     0.079};     dysfUncs["stat"]         = (dybin==-1 ? (stat[0]+stat[1]+stat[2])/3             : stat[dybin]);
  float jes[]         = {0.021,      0.019,     0.041};     dysfUncs["jes"]          = (dybin==-1 ? (jes[0]+jes[1]+jes[2])/3                : jes[dybin]);
  float jer[]         = {0.014,      0.014,     0.013 };    dysfUncs["jer"]          = (dybin==-1 ? (jer[0]+jer[1]+jer[2])/3                : jer[dybin]);
  float pu[]          = {0.02,       0.01,      0.008 };    dysfUncs["pu"]           = (dybin==-1 ? (pu[0]+pu[1]+pu[2])/3                   : pu[dybin]);
  float mcsignal[]    = {0.087,      0.010,     0.053 };    dysfUncs["mcsignal"]     = (dybin==-1 ? (mcsignal[0]+mcsignal[1]+mcsignal[2])/3 : mcsignal[dybin]);
  float q2[]          = {0.014,      0.014,     0.001 };    dysfUncs["q2"]           = (dybin==-1 ? (q2[0]+q2[1]+q2[2])/3                   : q2[dybin]);
  float meps[]        = {0.011,      0.011,     0.001 };    dysfUncs["meps"]         = (dybin==-1 ? (meps[0]+meps[1]+meps[2])/3             : meps[dybin]);
  float dy_template[] = {0.08,       0.06,      0.01 };  
  if(dybin==-1)
    {
      dysfUncs["dy_ee_template"]   = dy_template[0];
      dysfUncs["dy_mumu_template"] = dy_template[1];
      dysfUncs["dy_emu_template"]  = dy_template[2];
    }
  else
    dysfUncs["dy_"+ch+"_template"] = dy_template[dybin];

  return dysfUncs;
}


//
TString convertNameForDataCard(TString title)
{
  if(title=="VV")                                             return "vv";
  if(title=="QCD")                                            return "qcd";
  if(title=="W#rightarrow l#nu")                              return "w"; 
  if(title=="W,multijets")                                    return "wjets";
  if(title=="other t#bar{t}")                                 return "otherttbar";
  if(title=="Z#rightarrow ll")                                return "dy"; 
  if(title=="Single top")                                     return "st";
  if(title=="t#bar{t}V")                                      return "ttv";
  if(title=="t#bar{t}")                                       return "ttbar";
  if(title=="t#bar{t}+b#bar{b}")                              return "ttbb";
  if(title=="t#bar{t} dileptons")                             return "ttbar";
  if(title=="#splitline{H^{+}#rightarrow tb}{[180 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow#tau#nu}{[180 GeV]}") {signalTag = "TBH" ; return "TBH";}   
  if(title=="#splitline{H^{+}#rightarrow tb}{[200 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow#tau#nu}{[200 GeV]}") {signalTag = "TBH" ; return "TBH";}   
  if(title=="#splitline{H^{+}#rightarrow tb}{[220 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow#tau#nu}{[220 GeV]}") {signalTag = "TBH" ; return "TBH";}   
  if(title=="#splitline{H^{+}#rightarrow tb}{[240 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow tb}{[250 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow#tau#nu}{[250 GeV]}") {signalTag = "TBH" ; return "TBH";}   
  if(title=="#splitline{H^{+}$#rightarrow tb}{[260 GeV]}")    {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow tb}{[280 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow tb}{[300 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow#tau#nu}{[300 GeV]}") {signalTag = "TBH" ; return "TBH";}   
  if(title=="#splitline{H^{+}#rightarrow tb}{[350 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow tb}{[400 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow#tau#nu}{[400 GeV]}") {signalTag = "TBH" ; return "TBH";}   
  if(title=="#splitline{H^{+}#rightarrow tb}{[500 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow#tau#nu}{[500 GeV]}") {signalTag = "TBH" ; return "TBH";}   
  if(title=="#splitline{H^{+}#rightarrow tb}{[600 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="#splitline{H^{+}#rightarrow#tau#nu}{[600 GeV]}") {signalTag = "TBH" ; return "TBH";}   
  if(title=="#splitline{H^{+}#rightarrow tb}{[700 GeV]}")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [180 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [200 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [220 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [240 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [250 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [260 GeV]")    {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [280 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [300 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [350 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [400 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [500 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [600 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 5 [700 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [180 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [200 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [220 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [240 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [250 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [260 GeV]")    {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [280 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [300 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [350 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [400 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [500 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [600 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  if(title=="tanb 30 [700 GeV]")     {signalTag = "HTB" ; return "HTB";}   
  return title;
}
// 
TString convertNameForFileName(TString histoName)
{
  if(histoName=="finalevtflow0") return "_0btag";
  if(histoName=="finalevtflow1") return "_1btag";
  if(histoName=="finalevtflow2") return "_2btags";
  if(histoName=="finalevtflow3") return "_3btags";
  if(histoName=="finalevtflow4") return "_4btags";
  if(histoName=="finalevtflow5") return "_geq5btags";
  return ""; 
}

// 
TString convertMassPointNameForFileName(TString title)
{

  if(title=="#splitline{H^{+}#rightarrow tb}{[180 GeV]}")     return "_m180_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[200 GeV]}")     return "_m200_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[220 GeV]}")     return "_m220_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[240 GeV]}")     return "_m240_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[250 GeV]}")     return "_m250_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[260 GeV]}")     return "_m260_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[280 GeV]}")     return "_m280_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[300 GeV]}")     return "_m300_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[350 GeV]}")     return "_m350_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[400 GeV]}")     return "_m400_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[500 GeV]}")     return "_m500_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[600 GeV]}")     return "_m600_";
  if(title=="#splitline{H^{+}#rightarrow tb}{[700 GeV]}")     return "_m700_";



  return ""; 
}


//
float getIntegratedSystematics(TH1F *h,const std::map<TString, TH1F*> &hSysts, std::map<TString,float> &rateSysts, int bin)
{
  if(h==0) return -1;
  float rate=h->GetBinContent(bin);
  float varUp(0),varDown(0);
  for(std::map<TString, TH1F*>::const_iterator it=hSysts.begin(); it!=hSysts.end(); it++)
    {
      float var=it->second->GetBinContent(bin)/rate;
      if(isnan(var) || isinf(var)) continue;
      if(var>1) varUp += pow(var-1,2);
      else      varDown += pow(1-var,2);
    } 
  for(std::map<TString, float>::iterator it=rateSysts.begin(); it!=rateSysts.end(); it++)
    {
      varUp += pow(it->second,2);
      varDown += pow(it->second,2);
    }
  varUp=sqrt(varUp);
  varDown=sqrt(varDown);
  float var=0.5*(varUp+varDown);
  return var*rate;
}


//
void printHelp()
{
  printf("Options\n");
  printf("--out          --> output director\n");
  printf("--suffix       --> suffix to append to datacard filenames\n");
  printf("--in           --> input file from plotter\n");
  printf("--syst         --> input file with syst shapes\n");
  printf("--json         --> json file with the sample descriptor\n");
  printf("--histo        --> name of histogram to be used\n");
  printf("--noPowers     --> Do not use powers of 10 for numbers in tables\n");
  printf("--bins         --> list of bins to be used (they must be comma separated without space)\n");
  printf("--ch           --> list of channels to be used (they must be comma separated without space)\n");
  printf("--statBinByBin --> statistical uncertainty shape is bin by bin (uncorrelated bins) ");
}

//
Shape_t getShapeFromFile(TFile* inF, TString ch, JSONWrapper::Object &Root, TFile *systF)
{
  Shape_t shape; 
  shape.totalBckg=NULL;
  shape.signal=NULL;
  shape.data=NULL;

  std::vector<JSONWrapper::Object> Process = Root["proc"].daughters();
  for(unsigned int i=0;i<Process.size();i++)
    {
      TString procCtr(""); procCtr+=i;
      TString proc=(Process[i])["tag"].toString();
      cout << "--------------------" << endl;
      cout << "Tag: " << proc << endl;  

      TDirectory *pdir = (TDirectory *)inF->Get(proc);         
      if(pdir==0) continue;
      
      bool isData(Process[i]["isdata"].toBool());
      bool isSignal(Process[i]["issignal"].toBool());
      bool isInvisible(Process[i]["isinvisible"].toBool());
      int color(1);       if(Process[i].isTag("color" ) ) color  = (int)Process[i]["color" ].toInt();
      int lcolor(color);  if(Process[i].isTag("lcolor") ) lcolor = (int)Process[i]["lcolor"].toInt();
      int mcolor(color);  if(Process[i].isTag("mcolor") ) mcolor = (int)Process[i]["mcolor"].toInt();
      int fcolor(color);  if(Process[i].isTag("fcolor") ) fcolor = (int)Process[i]["fcolor"].toInt();
      int lwidth(1);      if(Process[i].isTag("lwidth") ) lwidth = (int)Process[i]["lwidth"].toInt();
      int lstyle(1);      if(Process[i].isTag("lstyle") ) lstyle = (int)Process[i]["lstyle"].toInt();
      int fill(1001);     if(Process[i].isTag("fill"  ) ) fill   = (int)Process[i]["fill"  ].toInt();
      int marker(20);     if(Process[i].isTag("marker") ) marker = (int)Process[i]["marker"].toInt();
  
      TH1F* syst = (TH1F*) pdir->Get("optim_systs");
      for(int ivar = 1; ivar<= (syst==0? 1 : syst->GetNbinsX());ivar++)
	{
	  TString varName = (syst==0? "" : syst->GetXaxis()->GetBinLabel(ivar));
	  if(!varName.IsNull()) systVars.insert(varName);
	  
	  cout << "Process: " << proc << ", for syst " << varName << ", or " << syst->GetXaxis()->GetBinLabel(ivar) << endl;
	  
	  TString histoName = ch;  if(!ch.IsNull()) histoName += "_"; histoName += histo+varName ;
	  TH1F* hshape = (TH1F*) pdir->Get( histoName );
	  if(hshape==0) continue;
	  
	  //project out required bins (set the others to 0)
	  if(binsToProject.size()) {
	    for(int ibin=1; ibin<=hshape->GetXaxis()->GetNbins(); ibin++) { 
	      if(find(binsToProject.begin(),binsToProject.end(),ibin) != binsToProject.end()) continue;
	      hshape->SetBinContent(ibin,0);
	      hshape->SetBinError(ibin,0);
	    }
	  }
	  else for(int ibin=1; ibin<=hshape->GetXaxis()->GetNbins(); ibin++) binsToProject.push_back(ibin); 

	  //format shape
	  utils::root::fixExtremities(hshape,true,true);
	  hshape->SetDirectory(0);  
	  hshape->SetTitle(proc);
	  hshape->SetFillColor(color); 
	  hshape->SetLineColor(lcolor); 
	  hshape->SetMarkerColor(mcolor);
	  hshape->SetFillStyle(fill);  
	  hshape->SetFillColor(fcolor);
	  hshape->SetLineWidth(lwidth); 
	  hshape->SetMarkerStyle(marker); 
	  hshape->SetLineStyle(lstyle);
	
	 //save in structure
	  if(isData){
	    if(varName=="")  shape.data=hshape;
	    else continue;
	  }else if(isSignal){
	    if(varName==""){
	      float thxsec = Process[i]["data"].daughters()[0]["xsec"].toDouble();
	      float thxsecunc=0;
	      if(Process[i]["data"].daughters()[0].isTag("xsecunc") )  thxsecunc = Process[i]["data"].daughters()[0]["xsecunc"].toDouble();
	      shape.crossSections[proc]=std::pair<float,float>(thxsec,thxsecunc);
	      shape.signal=hshape;
	      //	      shape.signalMassPoints.push_back(hshape);

	      //get alternative shapes for signal from systematics file
	      if(systF)
		{
		  TString signalVars[]={"q2up","q2down","mepsup","mepsdown"};
                  TString ttbarSplit[]={"other t#bar{t}","t#bar{t}+b#bar{b}","t#bar{t} dileptons"};
		  for(size_t isigvar=0; isigvar<sizeof(signalVars)/sizeof(TString); isigvar++)
		    {
                      for(size_t ittbarSplit=0; ittbarSplit<sizeof(ttbarSplit)/sizeof(TString); ++ittbarSplit){
                        TH1F *hmcsig=(TH1F *)systF->Get(ttbarSplit[ittbarSplit]+"syst"+signalVars[isigvar]+"/"+histoName);
                        // TH1F *normHisto=hshape; 
                        //if(signalVars[isigvar].Contains("meps") || signalVars[isigvar].Contains("q2"))
                        //normHisto=(TH1F *)systF->Get("t#bar{t}systspincorrel/"+histoName);
                        if(hmcsig==0) { cout << "Skipping null variation: " << signalVars[isigvar] << endl;  continue; }
                        for(int ibin=1; ibin<=hshape->GetXaxis()->GetNbins(); ibin++) { 
                          if(find(binsToProject.begin(),binsToProject.end(),ibin) != binsToProject.end()) continue;
                          hmcsig->SetBinContent(ibin,0);
                          hmcsig->SetBinError(ibin,0);
                        }
                        hmcsig->SetDirectory(0); 
                        //Double_t sf=hshape->Integral()/hmcsig->Integral();
                        //hmcsig->Scale(hshape->Integral()/hmcsig->Integral()); 
                        hmcsig->SetName(hmcsig->GetName()+TString("mcsignalup"));
                        hmcsig->SetTitle(proc);
                        
                        //check this rates from theortical point of view. in prep the variation is very high...
                        // if(signalVars[isigvar]=="q2up")           shape.rateUncs["q2_rate"]=std::pair<float,float>(sf,sf);
                        // else if(signalVars[isigvar]=="q2down")    shape.rateUncs["q2_rate"].second=sf;
                        // if(signalVars[isigvar]=="mepsup")         shape.rateUncs["meps_rate"]=std::pair<float,float>(sf,sf);
                        // else if(signalVars[isigvar]=="mepsdown")  shape.rateUncs["meps_rate"].second=sf;
                        
                        //if variation corresponds already to a signed variation save it directly
                        //otherwise create an artificial symmetric variation to build the +/- envelope of the nominal shape
                        if(signalVars[isigvar].EndsWith("up") || signalVars[isigvar].EndsWith("down"))  
                          {
                            shape.signalVars[signalVars[isigvar]]=hmcsig;
                          }
                        else
                          {
                            shape.signalVars[signalVars[isigvar]+"up"]   = hmcsig;
                            TH1F *hmcsigdown=(TH1F *) hmcsig->Clone(hmcsig->GetName()+TString("mcsignaldown"));
                            for(int ibin=1; ibin<=hmcsigdown->GetXaxis()->GetNbins();ibin++)
                              {
                                float var=hmcsig->GetBinContent(ibin)-hshape->GetBinContent(ibin);
                                float newVal(hshape->GetBinContent(ibin)-var);
                                hmcsigdown->SetBinContent(ibin,newVal);
                              }
                            shape.signalVars[signalVars[isigvar]+"down"] = hmcsigdown;
                          }
                      }
                    }
		}
            }
	    else{
	      shape.signalVars[varName]=hshape;
            }
	  }//else if(proc.Contains("Z#rightarrow ll")){
	  // if(varName==""){
	  //   shape.dataDrivenBckg.insert(proc);
	  //
	  //   //get the parametrized dy uncertainties
	  //   std::map<TString,float> dyUnc=getDYUncertainties(ch);
	  //   
	  //   //set the estimated stat uncertainty
	  //   for(int ibin=1; ibin<=hshape->GetXaxis()->GetNbins(); ibin++) hshape->SetBinError(ibin,hshape->GetBinContent(ibin)*dyUnc["stat"]); 
	  //   shape.bckg.push_back(hshape);
	  //
	  //   //set the other uncertainties with variations of the shape
	  //   for(std::map<TString,float>::iterator dyUncIt=dyUnc.begin(); dyUncIt!=dyUnc.end(); dyUncIt++)
	  //	{
	  //	  if(dyUncIt->first=="stat") continue; //this is set separately
	  //	  if(dyUncIt->second<=0)     continue;
	  //	  systVars.insert(dyUncIt->first+"up");  systVars.insert(dyUncIt->first+"down");
	  //
	  //	  TString newName(hshape->GetName()); newName+=dyUncIt->first;
	  //	  TH1F *dyUncUp   = (TH1F *) hshape->Clone(newName+"up");   dyUncUp->SetDirectory(0);
	  //	  TH1F *dyUncDown = (TH1F *) hshape->Clone(newName+"down"); dyUncDown->SetDirectory(0);
	  //	  for(int ibin=1; ibin<=hshape->GetXaxis()->GetNbins(); ibin++) 
	  //	    {
	  //	      dyUncUp  ->SetBinContent(ibin,std::min(2*hshape->GetBinContent(ibin), std::max(0.01*hshape->GetBinContent(ibin), hshape->GetBinContent(ibin)*(1+dyUncIt->second))));
	  //	      dyUncDown->SetBinContent(ibin,std::min(2*hshape->GetBinContent(ibin), std::max(0.01*hshape->GetBinContent(ibin), hshape->GetBinContent(ibin)*(1-dyUncIt->second))));
	  //	    }
	  //	  shape.bckgVars[proc][dyUncIt->first+"up"]=dyUncUp;
	  //	  shape.bckgVars[proc][dyUncIt->first+"down"]=dyUncDown;
	  //	}
	  // }
	  //
	  else{
            if(!isInvisible){
              if(varName==""){
                shape.bckg.push_back(hshape);
                float thxsec = Process[i]["data"].daughters()[0]["xsec"].toDouble();
                float thxsecunc=0;
                if(Process[i]["data"].daughters()[0].isTag("xsecunc") )  thxsecunc = Process[i]["data"].daughters()[0]["xsecunc"].toDouble();
                
                // converter
                if(proc.Contains("Z#rightarrow ll") || proc.Contains("dy"         )   ) thxsecunc = 0.04*thxsec;
                if(proc.Contains("VV")              || proc.Contains("vv"	        )   ) thxsecunc = 0.04*thxsec;
                if(proc.Contains("W,multijets")     || proc.Contains("wjets"      )   ) thxsecunc = 0.04*thxsec;
                if(proc.Contains("other t#bar{t}")  || proc.Contains("otherttbar" )   ) thxsecunc = 0.;//0.06*thxsec;
                if(proc.Contains("Single top")      || proc.Contains("st"	        )   ) thxsecunc = 0.0686*thxsec;
                if(proc.Contains("t#bar{t}")        || proc.Contains("ttbar"      )   ) thxsecunc = 0.;//06*thxsec;
                
                
                shape.crossSections[proc]=std::pair<float,float>(thxsec,thxsecunc);
	     
                //get alternative shapes for ttbar background from systematics file
                if(proc.Contains("t#bar{t}") || proc.Contains("ttbar") )///////systF)
                  {
                    TString signalVars[]={"q2up","q2down","mepsup","mepsdown"};
                    TString ttbarSplit[]={"other t#bar{t}","t#bar{t}+b#bar{b}","t#bar{t} dileptons"};
                    for(size_t isigvar=0; isigvar<sizeof(signalVars)/sizeof(TString); isigvar++)
                      {
                        for(size_t ittbarSplit=0; ittbarSplit<sizeof(ttbarSplit)/sizeof(TString); ++ittbarSplit){
                          //TH1F *hmcsig=(TH1F *)systF->Get(ttbarSplit[ittbarSplit]+"syst"+signalVars[isigvar]+"/"+histoName);
                          TH1F *hmcsig=(TH1F *)inF->Get(ttbarSplit[ittbarSplit]+"syst"+signalVars[isigvar]+"/"+histoName);
                          
                          // TH1F *normHisto=hshape; 
                          //if(signalVars[isigvar].Contains("meps") || signalVars[isigvar].Contains("q2"))
                          //normHisto=(TH1F *)systF->Get("t#bar{t}systspincorrel/"+histoName);
                          if(hmcsig==0) { cout << "Skipping null variation: " << signalVars[isigvar] << endl;                           cout << "File " << inF->GetName() << ",  try to access: " << ttbarSplit[ittbarSplit]+"syst"+signalVars[isigvar]+"/"+histoName << endl; continue; }

                          if( ttbarSplit[ittbarSplit] != hshape->GetTitle() ) continue;

                          cout << "Variation "  << signalVars[isigvar] << ": base " << hshape->GetTitle() << " " << hshape->Integral() << ", varied " << hmcsig->GetTitle() << " " << hmcsig->Integral() << "from directory " << ttbarSplit[ittbarSplit]+"syst"+signalVars[isigvar]+"/"+histoName << endl;
                          for(int ibin=1; ibin<=hshape->GetXaxis()->GetNbins(); ibin++) { 
                            if(find(binsToProject.begin(),binsToProject.end(),ibin) != binsToProject.end()) continue;
                            hmcsig->SetBinContent(ibin,0);
                            hmcsig->SetBinError(ibin,0);
                          }
                          hmcsig->SetDirectory(0); 
                          //Double_t sf=hshape->Integral()/hmcsig->Integral();
                          /////////////vvvvvvvvvvhmcsig->Scale(hshape->Integral()/hmcsig->Integral()); 
                          hmcsig->SetName(proc+signalVars[isigvar]);
                          hmcsig->SetTitle(proc);
                          
                          //check this rates from theortical point of view. in prep the variation is very high...
                          // if(signalVars[isigvar]=="q2up")           shape.rateUncs["q2_rate"]=std::pair<float,float>(sf,sf);
                          // else if(signalVars[isigvar]=="q2down")    shape.rateUncs["q2_rate"].second=sf;
                          // if(signalVars[isigvar]=="mepsup")         shape.rateUncs["meps_rate"]=std::pair<float,float>(sf,sf);
                          // else if(signalVars[isigvar]=="mepsdown")  shape.rateUncs["meps_rate"].second=sf;
                          
                          //if variation corresponds already to a signed variation save it directly
                          //otherwise create an artificial symmetric variation to build the +/- envelope of the nominal shape
                          if(signalVars[isigvar].EndsWith("up") || signalVars[isigvar].EndsWith("down"))  
                            {
                              //shape.signalVars[signalVars[isigvar]]=hmcsig;
                              shape.bckgVars[proc][signalVars[isigvar]]=hmcsig;
                              //shape.bckg.push_back(hmcsig);
                            }
                          else
                            {
                              //shape.signalVars[signalVars[isigvar]+"up"]   = hmcsig;
                              shape.bckgVars[proc][signalVars[isigvar]+"up"]   = hmcsig;
                              TH1F *hmcsigdown=(TH1F *) hmcsig->Clone(hmcsig->GetName()+TString("mcsignaldown"));
                              for(int ibin=1; ibin<=hmcsigdown->GetXaxis()->GetNbins();ibin++)
                                {
                                  float var=hmcsig->GetBinContent(ibin)-hshape->GetBinContent(ibin);
                                  float newVal(hshape->GetBinContent(ibin)-var);
                                  hmcsigdown->SetBinContent(ibin,newVal);
                                }
                              //shape.signalVars[signalVars[isigvar]+"down"] = hmcsigdown;
                              shape.bckgVars[proc][signalVars[isigvar]+"down"] = hmcsigdown;
                              //shape.bckg.push_back(hmcsigdown);
                            }
                        }
                      }
                  }
              }
              else{
                shape.bckgVars[proc][varName]=hshape;
              }
	    }
	  }
	}
    }

  //compute the total background
  for(size_t i=0; i<shape.bckg.size(); i++)
    {
      if(shape.totalBckg==0) { shape.totalBckg = (TH1F *)shape.bckg[i]->Clone(ch+"_"+histo+"_total"); shape.totalBckg->SetDirectory(0); shape.totalBckg->SetTitle("total bckg"); }
      else                   { shape.totalBckg->Add(shape.bckg[i]); }
    }

  //total prediction
  if(shape.totalBckg && shape.signal)
    {
      shape.totalSplusB = (TH1F *) shape.totalBckg->Clone(ch+"_"+histo+"_totalsplusb"); 
      shape.totalSplusB->SetTitle("total");
      shape.totalSplusB->SetDirectory(0);
      shape.totalSplusB->Add(shape.signal);
    }
  
  //all done
  return shape;
}


//
void getYieldsFromShapes(const map<TString, Shape_t> &allShapes)
{
  FILE* pFile = fopen(outUrl+"CrossSectionYields"+convertNameForFileName(histo)+"_"+suffixUrl+".tex","w");

  TH1F *dataTempl=allShapes.begin()->second.data;
  const std::vector<TH1F *> &bckgTempl=allShapes.begin()->second.bckg;
  for(std::vector<int>::iterator bIt = binsToProject.begin(); bIt != binsToProject.end(); bIt++)
    {

//      cout << "pointer " << dataTempl << endl;
//      cout << "histo" << dataTempl->GetTitle();
//      cout << ", with " << dataTempl->GetNbinsX();
//      cout << " bins, choosing bin " << (*bIt) << endl;
 	
      TString cat=dataTempl->GetXaxis()->GetBinLabel(*bIt);
        
      //table header
      fprintf(pFile,"\\begin{center}\n\\caption{Event yields expected for background and signal processes and observed in data for the %s category. The uncertainty associated to the limited statistics in the MC is quoted separately from the other systematic uncertainties.}\n\\label{tab:table}\n",cat.Data());
      TString Ccol   = "\\begin{tabular}{|c|";
      TString Cval   = "Channel ";
      for(std::map<TString,Shape_t>::const_iterator cIt=allShapes.begin(); cIt!=allShapes.end(); cIt++) {
	TString ch(cIt->first); if(ch.IsNull()) ch="inclusive"; 
	Ccol += "l";
	TString icol(ch/*+"-"+cat*/); utils::TLatexToTex(icol);
	Cval += " & "+icol+" ";      
      }
      Ccol += "}\\hline\\hline\n"; fprintf(pFile,"%s",Ccol.Data());
      Cval += "\\\\\\hline\n";      fprintf(pFile,"%s",Cval.Data());

      //event yields
      std::map<TString, TString > CByields;
      TString CSyields,CSpByields,CDyields;
      int ich(0);
      for(std::map<TString,Shape_t>::const_iterator cIt=allShapes.begin(); cIt!= allShapes.end(); cIt++,ich++) 
	{
	  const Shape_t &shape=cIt->second;
	  
	  //data
	  if(ich==0) CDyields = "data ";
	  CDyields += " & "; CDyields += (int) shape.data->GetBinContent(*bIt);
	  
	  float totalSyst(0);
	  
	  //signal
	  TString sigProc=shape.signal->GetTitle();
	  if(ich==0) { CSyields = sigProc; utils::TLatexToTex(CSyields); }
	  std::map<TString,float> sigRateSysts;
	  if(shape.crossSections.find(sigProc) != shape.crossSections.end())
	    {
	      std::pair<float,float> xsec=shape.crossSections.find(sigProc)->second;
	      sigRateSysts["xsec"]=xsec.second/xsec.first;
	  }
	  for(std::map<TString,std::pair<float,float> >::const_iterator rIt = shape.rateUncs.begin(); rIt!=shape.rateUncs.end(); rIt++)
	      sigRateSysts[rIt->first]=0.5*(fabs(rIt->second.first-1)+fabs(rIt->second.second-1));
	  sigRateSysts["lumi"]=lumiUnc;
	  sigRateSysts["seleff"]=selEffUnc;
	  float sSyst=getIntegratedSystematics(shape.signal,shape.signalVars,sigRateSysts,*bIt);
	  totalSyst += pow(sSyst,2); 
	  CSyields += " & ";
	  CSyields += utils::toLatexRounded( shape.signal->GetBinContent(*bIt), shape.signal->GetBinError(*bIt), sSyst, doPowers);
	  
	  //background
	  for(std::vector<TH1F *>::const_iterator bckgIt=bckgTempl.begin(); bckgIt!=bckgTempl.end(); bckgIt++)
	    {
	      TString proc=(*bckgIt)->GetTitle();
	      bool procFound(false);
	      for(std::vector<TH1F *>::const_iterator bckgItt=shape.bckg.begin(); bckgItt!=shape.bckg.end(); bckgItt++)
		{
		  if(proc!=TString((*bckgItt)->GetTitle())) continue;
		  float bSyst(-1);
		  if(true)
		    {
		      std::map<TString,float> rateSysts;
		      if(shape.dataDrivenBckg.find(proc)==shape.dataDrivenBckg.end())
			{
			  if(shape.crossSections.find(proc) != shape.crossSections.end())
			    {
			      std::pair<float,float> xsec=shape.crossSections.find(proc)->second;
			      rateSysts["xsec"]=xsec.second/xsec.first;
			    }
			  rateSysts["lumi"]=lumiUnc;
			}
		      std::map<TString, TH1F*> bckgVars;
		      if(shape.bckgVars.find(proc)!=shape.bckgVars.end()) bckgVars =shape.bckgVars.find(proc)->second;
		      bSyst=getIntegratedSystematics(*bckgItt,bckgVars,rateSysts,*bIt);
		      totalSyst += pow(bSyst,2); 
		    }
		  
		  if(ich==0) { CByields[proc]=proc; utils::TLatexToTex(CByields[proc]); }
		  CByields[proc] += " & "; CByields[proc] += utils::toLatexRounded( (*bckgItt)->GetBinContent(*bIt), (*bckgItt)->GetBinError(*bIt), bSyst, doPowers); 
		  procFound=true;
		  break;
		}
	      if(procFound) continue;
	      CByields[proc] += " & ";
	    }	  
	  
	  //signal + background
	  totalSyst=sqrt(totalSyst);
	  if(ich==0) { CSpByields = shape.totalSplusB->GetTitle(); utils::TLatexToTex(CSpByields); }
	  CSpByields += " & "; CSpByields += utils::toLatexRounded( shape.totalSplusB->GetBinContent(*bIt), shape.totalSplusB->GetBinError(*bIt),totalSyst, doPowers);
	}
      for(std::map<TString,TString>::iterator cbyIt=CByields.begin(); cbyIt!=CByields.end(); cbyIt++)
	{ 
	  cbyIt->second += "\\\\\n";  
	  fprintf(pFile,"%s",cbyIt->second.Data()); 
	}
      CSyields += "\\\\\\hline\n";       fprintf(pFile,"%s",CSyields.Data()); 
      CSpByields += "\\\\\\hline\n";     fprintf(pFile,"%s",CSpByields.Data());
      CDyields += "\\\\\\hline\n";       fprintf(pFile,"%s",CDyields.Data());      

      //close table
      fprintf(pFile,"\\hline\\end{tabular}\n\\end{center}\n");
      fprintf(pFile,"\n\n\n\n");
    }
  fclose(pFile);
}

//
void saveShapeForMeasurement(TH1F *h, TDirectory *oDir,TString syst)
{
  if(h==0 || oDir==0) return;
  oDir->cd();
  TString proc=convertNameForDataCard(h->GetTitle());
  if(syst.IsNull())
    {
      if(proc=="data") h->Write("data_obs");
      else {
	h->Write(proc);
	
	//build also the statistical uncertainty shapes
	//for each bin set content as val +/- statErr (beware however of negative and extremely large values)
	TString statSystName(proc+"_"); 
	//	if(proc=="signal") statSystName=signalTag+"_";//"ttbar_";
	statSystName+=oDir->GetTitle(); 
	statSystName+="_stat";
	TH1* statup   = (TH1 *)h->Clone(statSystName+"Up");
	TH1* statdown = (TH1 *)h->Clone(statSystName+"Down");
	for(int ibin=1; ibin<=statup->GetXaxis()->GetNbins(); ibin++){
	  statup  ->SetBinContent(ibin,std::min(2*h->GetBinContent(ibin), std::max(0.01*h->GetBinContent(ibin), statup  ->GetBinContent(ibin) + statup  ->GetBinError(ibin))));
	  statdown->SetBinContent(ibin,std::min(2*h->GetBinContent(ibin), std::max(0.01*h->GetBinContent(ibin), statdown->GetBinContent(ibin) - statdown->GetBinError(ibin))));
	}
//	statup  ->Write(proc+"_"+statSystName+"Up");
//	statdown->Write(proc+"_"+statSystName+"Down");

	/////////////////////////////////////////////////////////
	TH1* hc = (TH1*) h->Clone("TMPFORSTAT");
	
	//bin by bin stat uncertainty
	int BIN=0;
	for(int ibin=1; ibin<=hc->GetXaxis()->GetNbins(); ibin++){           
	  if(hc->GetBinContent(ibin)<=0 || hc->GetBinContent(ibin)/hc->Integral()<0.0000001)continue;
	  //           if(h->GetBinContent(ibin)<=0)continue;
	  char ibintxt[255]; sprintf(ibintxt, "_b%i", BIN);BIN++;
	  TH1* statU=(TH1 *)hc->Clone(TString(hc->GetName())+"StatU"+ibintxt);//  statU->Reset();
	  TH1* statD=(TH1 *)hc->Clone(TString(hc->GetName())+"StatD"+ibintxt);//  statD->Reset();           
	  statU->SetBinContent(ibin,std::min(2*hc->GetBinContent(ibin), std::max(0.01*hc->GetBinContent(ibin), hc->GetBinContent(ibin) + hc->GetBinError(ibin))));   statU->SetBinError(ibin, 0);
	  statD->SetBinContent(ibin,std::min(2*hc->GetBinContent(ibin), std::max(0.01*hc->GetBinContent(ibin), hc->GetBinContent(ibin) - hc->GetBinError(ibin))));   statD->SetBinError(ibin, 0);
	  //           statU->SetBinContent(ibin,std::min(2*h->GetBinContent(ibin), std::max(0.0, h->GetBinContent(ibin) + h->GetBinError(ibin))));   statU->SetBinError(ibin, 0);
	  //           statD->SetBinContent(ibin,std::min(2*h->GetBinContent(ibin), std::max(0.0, h->GetBinContent(ibin) - h->GetBinError(ibin))));   statD->SetBinError(ibin, 0);
	  ///	    uncShape[prefix+"stat"+suffix+ibintxt+suffix2+"Up"  ] = statU;
	  ///	    uncShape[prefix+"stat"+suffix+ibintxt+suffix2+"Down"] = statD;
	  /*h->SetBinContent(ibin, 0);*/  hc->SetBinError(ibin, 0);  //remove this bin from shape variation for the other ones
	  //printf("%s --> %f - %f - %f\n", (prefix+"stat"+suffix+ibintxt+suffix2+"Up").c_str(), statD->Integral(), h->GetBinContent(ibin), statU->Integral() );
	  statU->Write(proc+"_"+proc+ibintxt+"_"+statSystName+"Up");
	  statD->Write(proc+"_"+proc+ibintxt+"_"+statSystName+"Down");
	}
	
	////////////////////////////////////////////////////////
	


        // TEMP FOR ADDITIONAL JET BY JET
	TString addSystSystName(proc+"_"); 
	//	if(proc=="signal") statSystName=signalTag+"_";//"ttbar_";
	addSystSystName+=oDir->GetTitle(); 
	addSystSystName+="_additional";
        
	hc = (TH1*) h->Clone("TMPFORADDITIONALSYST");
	
	//bin by bin stat uncertainty
        BIN=0;
	for(int ibin=1; ibin<=hc->GetXaxis()->GetNbins(); ibin++){           
	  if(hc->GetBinContent(ibin)<=0 || hc->GetBinContent(ibin)/hc->Integral()<0.0000001)continue;
	  //           if(h->GetBinContent(ibin)<=0)continue;
	  char ibintxt[255]; sprintf(ibintxt, "_b%i", BIN);BIN++;
	  TH1* statU=(TH1 *)hc->Clone(TString(hc->GetName())+"AdditionalU"+ibintxt);//  statU->Reset();
	  TH1* statD=(TH1 *)hc->Clone(TString(hc->GetName())+"AdditionalD"+ibintxt);//  statD->Reset();           
	  statU->SetBinContent(ibin,1.44*hc->GetBinContent(ibin) );   statU->SetBinError(ibin, 0);
	  statD->SetBinContent(ibin,0.56*hc->GetBinContent(ibin) );   statD->SetBinError(ibin, 0);
	  //           statU->SetBinContent(ibin,std::min(2*h->GetBinContent(ibin), std::max(0.0, h->GetBinContent(ibin) + h->GetBinError(ibin))));   statU->SetBinError(ibin, 0);
	  //           statD->SetBinContent(ibin,std::min(2*h->GetBinContent(ibin), std::max(0.0, h->GetBinContent(ibin) - h->GetBinError(ibin))));   statD->SetBinError(ibin, 0);
	  ///	    uncShape[prefix+"stat"+suffix+ibintxt+suffix2+"Up"  ] = statU;
	  ///	    uncShape[prefix+"stat"+suffix+ibintxt+suffix2+"Down"] = statD;
	  /*h->SetBinContent(ibin, 0);*/  hc->SetBinError(ibin, 0);  //remove this bin from shape variation for the other ones
	  //printf("%s --> %f - %f - %f\n", (prefix+"stat"+suffix+ibintxt+suffix2+"Up").c_str(), statD->Integral(), h->GetBinContent(ibin), statU->Integral() );
          if(proc=="ttbb"){
            statU->Write(proc+"_"+proc+ibintxt+"_"+addSystSystName+"Up");
            statD->Write(proc+"_"+proc+ibintxt+"_"+addSystSystName+"Down");
          }
	}
	
	////////////////////////////////////////////////////////
	


	
	
	
      }
    }
  else
    {
      TString systName(proc+"_");
      //systName+=oDir->GetTitle();
      //systName+="_";
      systName+=syst;
      systName.ReplaceAll("down","Down");
      systName.ReplaceAll("up","Up");
     
      ///      systName.ReplaceAll("topptunc", TString("topptunc")+oDir->GetTitle());
      //cout << "FUCKING SHAPE: " << h->GetName() << endl;
      h->Write(systName);
    }
}


//
void convertShapesToDataCards(const map<TString, Shape_t> &allShapes)
{
  
  //  for(size_t currentPoint=0; currentPoint<allShapes.signalMassPoints.size(); ++currentPoint){

    TFile *fOut = TFile::Open(outUrl+"CrossSectionShapes"+convertNameForFileName(histo)+"_"+suffixUrl+".root","RECREATE");
    for(std::map<TString, Shape_t>::const_iterator it=allShapes.begin(); it!=allShapes.end(); it++)
      {
	TString ch(it->first); if(ch.IsNull()) ch="inclusive";
	TDirectory *oDir=fOut->mkdir(ch);
	
	TString shapesFile("DataCard_"+ch+convertNameForFileName(histo)+"_"+suffixUrl+".txt");
	const Shape_t &shape=it->second;
	
	FILE* pFile = fopen(outUrl+shapesFile,"w");
	
	fprintf(pFile, "imax 1\n");
	fprintf(pFile, "jmax *\n");
	fprintf(pFile, "kmax *\n");
	fprintf(pFile, "-------------------------------\n");
	TString shapesFileName("CrossSectionShapes"+convertNameForFileName(histo)+"_"+suffixUrl+".root");
	fprintf(pFile, "shapes * * %s %s/$PROCESS %s/$PROCESS_$SYSTEMATIC\n",shapesFileName.Data(), ch.Data(), ch.Data());
	fprintf(pFile, "-------------------------------\n");
	
	//observations
	fprintf(pFile, "bin a\n");
	fprintf(pFile, "observation %f\n",shape.data->Integral());
	fprintf(pFile, "-------------------------------\n");
	saveShapeForMeasurement(shape.data,oDir);
	
	//process rows
	fprintf(pFile,"%30s ", "bin");
	fprintf(pFile,"%6s ","a");
	for(size_t j=0; j<shape.bckg.size(); j++) fprintf(pFile,"%6s ", "a");
	fprintf(pFile,"\n");
	
	fprintf(pFile,"%30s ", "process");
	fprintf(pFile,"%6s ", convertNameForDataCard(shape.signal->GetTitle()).Data());
	for(size_t j=0; j<shape.bckg.size(); j++) fprintf(pFile,"%6s ", convertNameForDataCard(shape.bckg[j]->GetTitle()).Data());
	fprintf(pFile,"\n");
	
	fprintf(pFile,"%30s ", "process");
	fprintf(pFile,"%6s ","0");
	for(size_t j=0; j<shape.bckg.size(); j++) fprintf(pFile,"%6d ", int(j+1));
	fprintf(pFile,"\n");
	
	fprintf(pFile,"%30s ", "rate");
	fprintf(pFile,"%6.3f ",shape.signal->Integral());
	saveShapeForMeasurement(shape.signal,oDir);
	for(size_t j=0; j<shape.bckg.size(); j++) { fprintf(pFile,"%6.3f ", shape.bckg[j]->Integral()); saveShapeForMeasurement(shape.bckg[j],oDir); }
	fprintf(pFile,"\n");
	fprintf(pFile, "-------------------------------\n");
	
	//systematics
	
	//lumi
	fprintf(pFile,"%30s_%dTeV %10s","lumi",int(iEcm),"lnN");
	fprintf(pFile,"%6.3f ",1+lumiUnc);
	for(size_t j=0; j<shape.bckg.size(); j++) {
	  if(shape.dataDrivenBckg.find(shape.bckg[j]->GetTitle()) != shape.dataDrivenBckg.end()) fprintf(pFile,"%6s ","-");
	  else                                                                                   fprintf(pFile,"%6.3f ",1+lumiUnc);
	}
	fprintf(pFile,"\n");
	
	// dy 30 percent conservative uncertainty (from looking at the first bin of nbtags) 
	if(ch == "ee" || ch == "mumu" ){
	  fprintf(pFile,"%30s_%dTeV %10s","dy_additional",int(iEcm),"lnN");
	  fprintf(pFile,"%6s ","-");
	  for(size_t j=0; j<shape.bckg.size(); ++j) {
	    if(convertNameForDataCard(shape.bckg[j]->GetTitle()) == "dy" ) fprintf(pFile,"%6.3f ", 1.30);
	    else                                                           fprintf(pFile,"%6s ","-");
	  }
	  fprintf(pFile,"\n");
	}
	//muon id
	fprintf(pFile,"%30s_%dTeV %10s","muonid",int(iEcm),"lnN");
	fprintf(pFile,"%6.3f ",1+muonidUnc);
	for(size_t j=0; j<shape.bckg.size(); j++) {
	  if(shape.dataDrivenBckg.find(shape.bckg[j]->GetTitle()) != shape.dataDrivenBckg.end()) fprintf(pFile,"%6s ","-");
	  else                                                                                   fprintf(pFile,"%6.3f ",1+muonidUnc);
	}
	fprintf(pFile,"\n");
	//electron id
	fprintf(pFile,"%30s_%dTeV %10s","electronid",int(iEcm),"lnN");
	fprintf(pFile,"%6.3f ",1+electronidUnc);
	for(size_t j=0; j<shape.bckg.size(); j++) {
	  if(shape.dataDrivenBckg.find(shape.bckg[j]->GetTitle()) != shape.dataDrivenBckg.end()) fprintf(pFile,"%6s ","-");
	  else                                                                                   fprintf(pFile,"%6.3f ",1+electronidUnc);
	}
	fprintf(pFile,"\n");
	//trigger efficiency
        float trigEffUnc(0.);
        if(ch=="emu")  trigEffUnc=emuTrigEffUnc;
        if(ch=="mumu") trigEffUnc=mumuTrigEffUnc;
        if(ch=="ee")   trigEffUnc=eeTrigEffUnc;
        TString trigefflabel("trigeff_"+ch);
	fprintf(pFile,"%30s_%dTeV %10s",trigefflabel.Data(),int(iEcm),"lnN");
	fprintf(pFile,"%6.3f ",1+trigEffUnc);
	for(size_t j=0; j<shape.bckg.size(); j++) {
	  if(shape.dataDrivenBckg.find(shape.bckg[j]->GetTitle()) != shape.dataDrivenBckg.end()) fprintf(pFile,"%6s ","-");
	  else                                                                                   fprintf(pFile,"%6.3f ",1+trigEffUnc);
	}
	fprintf(pFile,"\n");
	
	////diepton BR
	//fprintf(pFile,"%35s %10s","br","lnN");
	//fprintf(pFile,"%6.3f ",1.017);
	//for(size_t j=0; j<shape.bckg.size(); j++) {
	//  if(convertNameForDataCard(shape.bckg[j]->GetTitle())!="ttbar") fprintf(pFile,"%6s ","-");
	//  else                                                           fprintf(pFile,"%6.3f ",1.017);
	//}
	//fprintf(pFile,"\n");

	//q2
	///// now shape ///// fprintf(pFile,"%35s %10s","q2scale","lnN");
	///// now shape ///// fprintf(pFile,"%6.3f ",1.00);
	///// now shape ///// for(size_t j=0; j<shape.bckg.size(); j++) {
	///// now shape /////   if(convertNameForDataCard(shape.bckg[j]->GetTitle())!="ttbar") fprintf(pFile,"%6s ","-");
	///// now shape /////   else                                                           fprintf(pFile,"%6.3f ",1.03);
	///// now shape ///// }
	///// now shape ///// fprintf(pFile,"\n");
        ///// now shape ///// 
	///// now shape ///// //matching
	///// now shape ///// fprintf(pFile,"%35s %10s","matching","lnN");
	///// now shape ///// fprintf(pFile,"%6.3f ",1.00);
	///// now shape ///// for(size_t j=0; j<shape.bckg.size(); j++) {
	///// now shape /////   if(convertNameForDataCard(shape.bckg[j]->GetTitle())!="ttbar") fprintf(pFile,"%6s ","-");
	///// now shape /////   else                                                           fprintf(pFile,"%6.3f ",1.01);
	///// now shape ///// }
	///// now shape ///// fprintf(pFile,"\n");


/// 	//matching
/// 	fprintf(pFile,"%35s %10s","ttbttjj","lnN");
/// 	fprintf(pFile,"%6.3f ",1.00);
/// 	for(size_t j=0; j<shape.bckg.size(); j++) {
/// 	  if(convertNameForDataCard(shape.bckg[j]->GetTitle())!="ttbb") fprintf(pFile,"%6s ","-");
/// 	  else                                                           fprintf(pFile,"%6.3f ",1.44); // 23./16.
/// 	}
/// 	fprintf(pFile,"\n");


	////diepton BR
	//fprintf(pFile,"%35s %10s","br","lnN");
	//fprintf(pFile,"%6.3f ",1.017);
	//for(size_t j=0; j<shape.bckg.size(); j++) {
	//  if(convertNameForDataCard(shape.bckg[j]->GetTitle())!="ttbar") fprintf(pFile,"%6s ","-");
	//  else                                                           fprintf(pFile,"%6.3f ",1.017);
	//}
	//fprintf(pFile,"\n");
	
	//rate systematics
	for(std::map<TString,std::pair<float,float> >::const_iterator rIt = shape.rateUncs.begin(); rIt!=shape.rateUncs.end(); rIt++)
	  {
	    fprintf(pFile,"%35s %10s",rIt->first.Data(),"lnN");
	    fprintf(pFile,"%6.3f ",1+0.5*(fabs(rIt->second.first-1)+fabs(rIt->second.second-1)));
	    for(size_t j=0; j<shape.bckg.size(); j++) {
	      fprintf(pFile,"%6s ","-");
	    }
	  }
	
	//th.xsec
	// fprintf(pFile,"%35s %10s ", "theoryUncXS_ttbar", "lnN");
	//       std::pair<float,float> ttbarXsec=shape.crossSections.find(shape.signal->GetTitle())->second;
	//       fprintf(pFile,"%6.3f ",1.0+ttbarXsec.second/ttbarXsec.first);
	//       for(size_t j=0; j<shape.bckg.size(); j++) {
	// 	if(convertNameForDataCard(shape.bckg[j]->GetTitle())!="ttbar") fprintf(pFile,"%6s ","-");
	// 	else                                                           fprintf(pFile,"%6.3f ",1.0+ttbarXsec.second/ttbarXsec.first);
	//       }
	//       fprintf(pFile,"\n");
	
        int processedTTlines(0);
	for(size_t j=0; j<shape.bckg.size(); j++)
	  {
	    TString proc(convertNameForDataCard(shape.bckg[j]->GetTitle()));
            if(proc.Contains("ttbar") || proc.Contains("ttbb")){
              if(processedTTlines>0)continue;
              processedTTlines++;
              TString uncName("xsect_tt_8TeV_scale" );
              fprintf(pFile,"%35s %10s ", uncName.Data(), "lnN");
              fprintf(pFile,"%6s ","-");
              for(size_t k=0; k<shape.bckg.size(); k++) {
                TString intProc(convertNameForDataCard(shape.bckg[j]->GetTitle()));
                if(k!=j && !(intProc.Contains("ttbar") || proc.Contains("ttbb"))  ) fprintf(pFile,"%6s ","-");
                else if(shape.dataDrivenBckg.find(shape.bckg[k]->GetTitle()) != shape.dataDrivenBckg.end()) fprintf(pFile,"%6s ","-");
                else                                                                                        fprintf(pFile,"%6s ", "0.9659/1.0253"  );
              }
              fprintf(pFile,"\n");
              uncName= "xsect_tt_8TeV_pdf_alphaS";
              fprintf(pFile,"%35s %10s ", uncName.Data(), "lnN");
              fprintf(pFile,"%6s ","-");
              for(size_t k=0; k<shape.bckg.size(); k++) {
                TString intProc(convertNameForDataCard(shape.bckg[j]->GetTitle()));
                if(k!=j && !(intProc.Contains("ttbar") || proc.Contains("ttbb")) ) fprintf(pFile,"%6s ","-");
                else if(shape.dataDrivenBckg.find(shape.bckg[k]->GetTitle()) != shape.dataDrivenBckg.end()) fprintf(pFile,"%6s ","-");
                else                                                                                        fprintf(pFile,"%6s ", "1.0463"  );
              }
              fprintf(pFile,"\n");
              
            }
            else
              {
                std::pair<float,float> procXsec=shape.crossSections.find(shape.bckg[j]->GetTitle())->second;
                if(procXsec.second<=0) continue;
                if(shape.dataDrivenBckg.find(shape.bckg[j]->GetTitle()) != shape.dataDrivenBckg.end()) continue;
                
                TString uncName("theoryUncXS_"+proc);
                fprintf(pFile,"%35s %10s ", uncName.Data(), "lnN");
                fprintf(pFile,"%6s ","-");
                for(size_t k=0; k<shape.bckg.size(); k++) {
                  if(k!=j) fprintf(pFile,"%6s ","-");
                  else if(shape.dataDrivenBckg.find(shape.bckg[k]->GetTitle()) != shape.dataDrivenBckg.end()) fprintf(pFile,"%6s ","-");
                  else                                                                                        fprintf(pFile,"%6.3f ",1.0+procXsec.second/procXsec.first);
                }
                fprintf(pFile,"\n");
              }
	  }
	
/// meh /// 	//fakes
/// meh /// 	fprintf(pFile,"%35s %10s ", "fakes", "lnN");
/// meh /// 	fprintf(pFile,"%6s ","-");
/// meh /// 	for(size_t j=0; j<shape.bckg.size(); j++) {
/// meh /// 	  TString name=convertNameForDataCard(shape.bckg[j]->GetTitle());
/// meh /// 	  if(name!="qcd" && name!="ttbar" && name !="w")  fprintf(pFile,"%6s ","-");
/// meh /// 	  else                                            fprintf(pFile,"%6s ","2.0");
/// meh /// 	}
/// meh /// 	fprintf(pFile,"\n");
	
	//systematics described by shapes

        systVars.insert("dileq2up");
        systVars.insert("dilemepsup");

        systVars.insert("ttbbq2up");
        systVars.insert("ttbbmepsup");

        systVars.insert("otheq2up");
        systVars.insert("othemepsup");

	for(std::set<TString>::iterator it=systVars.begin(); it!=systVars.end(); it++)
	  {
	    if(it->EndsWith("down")) continue;
	    if(it->Contains("leff")) continue;
	    TString systName(*it);
            //cout << "1) " << systName;
	    if(systName.EndsWith("up")) systName.Remove(systName.Length()-2,2);
	    bool systIsValid(false);
	    TString systLine("");
	    char systBuf[500];
	    TString systLineName(systName);
	    if(systName=="topptunc") systLineName.Append(oDir->GetName());
	    sprintf(systBuf,"%35s %10s ", systLineName.Data(), "shape");   systLine+=systBuf; memset( systBuf, 0, sizeof(systBuf) );
	    if(shape.signalVars.find(*it)==shape.signalVars.end())     { sprintf(systBuf,"%6s ","-"); systLine+=systBuf; memset( systBuf, 0, sizeof(systBuf) ); }
	    else if(shape.signalVars.find(*it)->second->Integral()==0) { sprintf(systBuf,"%6s ","-"); systLine+=systBuf; memset( systBuf, 0, sizeof(systBuf) ); }
            else 
	      { 
                if(systName=="pdf"){ sprintf(systBuf,"%6s ","-"); systLine+=systBuf; memset( systBuf, 0, sizeof(systBuf) );}
                else{

                  systIsValid=true;
                  sprintf(systBuf,"%6s ","1"); 
                  systLine+=systBuf; memset( systBuf, 0, sizeof(systBuf) );
                  saveShapeForMeasurement(shape.signalVars.find(systName+"up")->second,oDir,systName+"up"); 
                  saveShapeForMeasurement(shape.signalVars.find(systName+"down")->second,oDir,systName+"down"); 
                }
	      }

            TString check=systName;
            for(size_t j=0; j<shape.bckg.size(); j++) {
              //cout << ", 2) " << systName << ", " << check << endl;
              if(systName.BeginsWith("ttbb") || systName.BeginsWith("dile") || systName.BeginsWith("othe")) systName.Remove(0,4);               
              //cout << "CHECK IT " << check << ", " << systName << ", " << shape.bckg[j]->GetTitle() << ", " << shape.bckg[j]->GetName() << endl;
              
	      if(shape.bckgVars.find(shape.bckg[j]->GetTitle())==shape.bckgVars.end())                                                                { sprintf(systBuf,"%6s ","-"); systLine+=systBuf; memset( systBuf, 0, sizeof(systBuf) ); }
	      //else if(shape.dataDrivenBckg.find(shape.bckg[j]->GetTitle()) != shape.dataDrivenBckg.end())                                           { sprintf(systBuf,"%6s ","-"); systLine+=systBuf; memset( systBuf, 0, sizeof(systBuf) ); }
	      else if(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")==shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.end()) { sprintf(systBuf,"%6s ","-"); systLine+=systBuf; memset( systBuf, 0, sizeof(systBuf) ); 
                //cout << "meh2 title " << shape.bckg[j]->GetTitle() << ", "  << *it  << ", " << systName  << endl;
              }
	      else if(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second->Integral()==0)                                        { sprintf(systBuf,"%6s ","-"); systLine+=systBuf; memset( systBuf, 0, sizeof(systBuf) ); }
	      else                                                                                                                                  
		{ 
                  
                  // Check for filling
                  if( 
                     (check.BeginsWith("dile") && !(TString(shape.bckg[j]->GetTitle()).Contains("dileptons"))) ||
                     (check.BeginsWith("ttbb") && !(TString(shape.bckg[j]->GetTitle()).Contains("b#bar{b}")) ) ||
                     (check.BeginsWith("othe") && !(TString(shape.bckg[j]->GetTitle()).Contains("other")    ))
                      ) sprintf(systBuf,"%6s ","-");
                  else{
                    systIsValid=true;
                    sprintf(systBuf,"%6s ","1"); 
                  }
                  systLine+=systBuf; memset( systBuf, 0, sizeof(systBuf) );

		  if(systName=="pdf")
		    {
		      shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second->Scale( shape.bckg[j]->Integral() / shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second->Integral() );
		      shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"down")->second->Scale( shape.bckg[j]->Integral() / shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"down")->second->Integral() );
		    }

		  if(systName=="topptunc")
		    {
		      saveShapeForMeasurement(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second,oDir,systName+TString(oDir->GetName())+"up"); 
		      saveShapeForMeasurement(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"down")->second,oDir,systName+TString(oDir->GetName())+"down"); 
		    }
                  else if(check.BeginsWith("dile") && TString(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second->GetTitle()).Contains("dileptons"))
                    {
                      //cout << "Dilepton save" << endl;
		      saveShapeForMeasurement(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second,oDir,"dile"+systName+"up"); 
		      saveShapeForMeasurement(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"down")->second,oDir,"dile"+systName+"down"); 

                    }
                  else if(check.BeginsWith("ttbb") && TString(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second->GetTitle()).Contains("b#bar{b}"))
                    {
                      //cout << "ttbb save" << endl;
		      saveShapeForMeasurement(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second,oDir,"ttbb"+systName+"up"); 
		      saveShapeForMeasurement(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"down")->second,oDir,"ttbb"+systName+"down"); 

                    }
                  else if(check.BeginsWith("othe") && TString(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second->GetTitle()).Contains("other"))
                    {
                      //cout << "other save" << endl;
		      saveShapeForMeasurement(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second,oDir,"othe"+systName+"up"); 
		      saveShapeForMeasurement(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"down")->second,oDir,"othe"+systName+"down"); 

                    }
		  else
		    {
                      //cout << "ELSE: " << check << ", " << shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second->GetTitle() << endl;
                      if(
                         (check.BeginsWith("dile") && !(TString(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second->GetTitle()).Contains("dileptons")) ) ||
                         (check.BeginsWith("ttbb") && !(TString(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second->GetTitle()).Contains("b#bar{b}") ) ) ||
                         (check.BeginsWith("othe") && !(TString(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second->GetTitle()).Contains("other")    ) )
                         )
                        cout << "do not save for those " << endl;
                      else
                        {
                          saveShapeForMeasurement(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"up")->second,oDir,systName+"up"); 
                          saveShapeForMeasurement(shape.bckgVars.find(shape.bckg[j]->GetTitle())->second.find(systName+"down")->second,oDir,systName+"down"); 
                        }
		    }
		}
	    }
	    //cout << "I arrive here" << endl;
	    if(systIsValid) fprintf(pFile,"%s \n",systLine.Data());
	  }


	//MC statistics (is also systematic but written separately, it is saved at the same time as the nominal shape)
	TString myName=convertNameForDataCard(shape.signal->GetTitle()).Data();
	int BIN=0;
	for(int ibin=1; ibin<=shape.signal->GetXaxis()->GetNbins(); ibin++){           
	  if(shape.signal->GetBinContent(ibin)<=0 || shape.signal->GetBinContent(ibin)/shape.signal->Integral()<0.0000001)continue;	  
	  char ibintxt[255]; sprintf(ibintxt, "_b%i", BIN);BIN++;
	  fprintf(pFile,"%35s %10s ", (myName/*signalTag*/+ibintxt+"_"+myName+"_"+ch+"_stat").Data(), "shape");
	  fprintf(pFile,"%6s ","1");
	  for(size_t j=0; j<shape.bckg.size(); j++) {
	    //	if(convertNameForDataCard(shape.bckg[j]->GetTitle())!="ttbar") fprintf(pFile,"%6s ","-");
	    //	else                                                           fprintf(pFile,"%6s ","1");
	    fprintf(pFile,"%6s","-");
	  }
	  fprintf(pFile,"\n");
	}
	for(size_t j=0; j<shape.bckg.size(); j++)
	  {
	    TString proc(convertNameForDataCard(shape.bckg[j]->GetTitle()));
	    //	  if(proc=="ttbar") continue;
	    //if(shape.dataDrivenBckg.find(shape.bckg[j]->GetTitle()) != shape.dataDrivenBckg.end()) continue;
	    BIN=0;
	    for(int ibin=1; ibin<=shape.bckg[j]->GetXaxis()->GetNbins(); ibin++){
	      if(shape.bckg[j]->GetBinContent(ibin)<=0 || shape.bckg[j]->GetBinContent(ibin)/shape.bckg[j]->Integral()<0.0000001)continue;	  
	      char ibintxt[255]; sprintf(ibintxt, "_b%i", BIN); BIN++;
	      fprintf(pFile,"%35s %10s ", (proc+ibintxt+"_"+proc+"_"+ch+"_stat").Data(), "shape");
	      fprintf(pFile,"%6s ","-");
	      for(size_t k=0; k<shape.bckg.size(); k++) {
		if(k!=j) fprintf(pFile,"%6s ","-");
		//else if(shape.dataDrivenBckg.find(shape.bckg[k]->GetTitle()) != shape.dataDrivenBckg.end()) fprintf(pFile,"%6s ","-");
		else     fprintf(pFile,"%6s ","1");
	      }
              fprintf(pFile,"\n");
              
              if(proc=="ttbb"){
                fprintf(pFile,"%35s %10s ", (proc+ibintxt+"_"+proc+"_"+ch+"_additional").Data(), "shape");
                fprintf(pFile,"%6s ","-");
                for(size_t k=0; k<shape.bckg.size(); k++) {
                  if(k!=j) fprintf(pFile,"%6s ","-");
                  //else if(shape.dataDrivenBckg.find(shape.bckg[k]->GetTitle()) != shape.dataDrivenBckg.end()) fprintf(pFile,"%6s ","-");
                  else     fprintf(pFile,"%6s ","1");
                }
                fprintf(pFile,"\n");              
              }



	    }
	  }
	
	//all done
	fprintf(pFile,"\n");
	fclose(pFile);
      }      
    
    fOut->Close();
    
    // }
}      

///void showShape(std::vector<TString>& selCh , TString histoName, TString SaveName)
///{
///  int NLegEntry = 0;
///  std::map<string, THStack*          > map_stack;
///  std::map<string, TGraphErrors*     > map_unc;
///  std::map<string, TH1*              > map_data;
///  std::map<string, std::vector<TH1*> > map_signals;
///  std::map<string, int               > map_legend;
///  //           TLegend* legA  = new TLegend(0.6,0.5,0.99,0.85, "NDC");
///  //           TLegend* legA  = new TLegend(0.03,0.00,0.97,0.70, "NDC");
///  TLegend* legA  = new TLegend(0.03,0.99,0.97,0.89, "NDC");
///  
///  //order the proc first
///  sortProc();
///  
///  //loop on sorted proc
///  for(unsigned int p=0;p<sorted_procs.size();p++){
///    string procName = sorted_procs[p];
///    std::map<string, ProcessInfo_t>::iterator it=procs.find(procName);
///    if(it==procs.end())continue;
///    for(std::map<string, ChannelInfo_t>::iterator ch = it->second.channels.begin(); ch!=it->second.channels.end(); ch++){
///      if(std::find(selCh.begin(), selCh.end(), ch->second.channel)==selCh.end())continue;
///      if(ch->second.shapes.find(histoName.Data())==(ch->second.shapes).end())continue;
///      TH1* h = ch->second.shapes[histoName.Data()].histo();
///      
///      if(it->first=="total"){
///        double Uncertainty = ch->second.shapes[histoName.Data()].getScaleUncertainty() / h->Integral();
///        double Maximum = 0;
///        TGraphErrors* errors = new TGraphErrors(h->GetXaxis()->GetNbins());
///        errors->SetFillStyle(3427);
///        errors->SetFillColor(kGray+1);
///        errors->SetLineStyle(1);
///        errors->SetLineColor(2);
///        int icutg=0;
///        for(int ibin=1; ibin<=h->GetXaxis()->GetNbins(); ibin++){
///          if(h->GetBinContent(ibin)>0)
///            errors->SetPoint(icutg,h->GetXaxis()->GetBinCenter(ibin), h->GetBinContent(ibin));
///          errors->SetPointError(icutg,h->GetXaxis()->GetBinWidth(ibin)/2.0, sqrt(pow(h->GetBinContent(ibin)*Uncertainty,2) + pow(h->GetBinError(ibin),2) ) );
///          //                        errors->SetPointError(icutg,h->GetXaxis()->GetBinWidth(ibin)/2.0, 0 );
///          Maximum =  std::max(Maximum , h->GetBinContent(ibin) + errors->GetErrorYhigh(icutg));
///          icutg++;
///        }errors->Set(icutg);
///        errors->SetMaximum(Maximum);
///        map_unc[ch->first] = errors;
///        continue;//otherwise it will fill the legend
///      }else if(it->second.isBckg){                 
///        if(map_stack.find(ch->first)==map_stack.end()){
///          map_stack[ch->first] = new THStack((ch->first+"stack").c_str(),(ch->first+"stack").c_str());                    
///        }
///                    map_stack   [ch->first]->Add(h,"HIST");
///                    
///      }else if(it->second.isSign){                    
///        map_signals [ch->first].push_back(h);
///        
///      }else if(it->first=="data"){
///        map_data[ch->first] = h;
///      }
///      
///      if(map_legend.find(it->first)==map_legend.end()){
///        map_legend[it->first]=1;
///        legA->AddEntry(h,it->first.c_str(),it->first=="data"?"P":it->second.isSign?"L":"F");
///        NLegEntry++;
///      }
///    }
///  }
///  
///  int NBins = map_data.size()/selCh.size();
///  TCanvas* c1 = new TCanvas("c1","c1",300*NBins,300*selCh.size());
///  c1->SetTopMargin(0.00); c1->SetRightMargin(0.00); c1->SetBottomMargin(0.00);  c1->SetLeftMargin(0.00);
///  TPad* t2 = new TPad("t2","t2", 0.03, 0.90, 1.00, 1.00, -1, 1);  t2->Draw();  c1->cd();
///  t2->SetTopMargin(0.00); t2->SetRightMargin(0.00); t2->SetBottomMargin(0.00);  t2->SetLeftMargin(0.00);
///  TPad* t1 = new TPad("t1","t1", 0.03, 0.03, 1.00, 0.90, 4, 1);  t1->Draw();  t1->cd();
///  t1->SetTopMargin(0.00); t1->SetRightMargin(0.00); t1->SetBottomMargin(0.00);  t1->SetLeftMargin(0.00);
///  t1->Divide(NBins, selCh.size(), 0, 0);
///  
///  int I=1;
///  for(std::map<string, THStack*>::iterator p = map_stack.begin(); p!=map_stack.end(); p++){
///    //init tab
///    TVirtualPad* pad = t1->cd(I);
///    pad->SetTopMargin(0.06); pad->SetRightMargin(0.03); pad->SetBottomMargin(0.07);  pad->SetLeftMargin(0.06);
///    pad->SetLogy(true); 
///    
///    //print histograms
///    TH1* axis = (TH1*)map_data[p->first]->Clone("axis");
///    axis->Reset();      
///    axis->GetXaxis()->SetRangeUser(0, axis->GetXaxis()->GetXmax());
///    axis->SetMinimum(1E-2);
///    double signalHeight=0; for(unsigned int s=0;s<map_signals[p->first].size();s++){signalHeight = std::max(signalHeight, map_signals[p->first][s]->GetMaximum());}
///    axis->SetMaximum(1.5*std::max(signalHeight , std::max( map_unc[p->first]->GetMaximum(), map_data[p->first]->GetMaximum())));
///    if((I-1)%NBins!=0)axis->GetYaxis()->SetTitle("");
///    axis->Draw();
///    p->second->Draw("same");
///    map_unc [p->first]->Draw("2 same");
///    map_data[p->first]->Draw("same");
///    for(unsigned int i=0;i<map_signals[p->first].size();i++){
///      map_signals[p->first][i]->Draw("HIST same");
///    }
///    
///    
///    
///    //print tab channel header
///    TPaveText* Label = new TPaveText(0.1,0.81,0.94,0.89, "NDC");
///    Label->SetFillColor(0);  Label->SetFillStyle(0);  Label->SetLineColor(0); Label->SetBorderSize(0);  Label->SetTextAlign(31);
///    TString LabelText = procs["data"].channels[p->first].channel+"  -  "+procs["data"].channels[p->first].bin;
///    LabelText.ReplaceAll("mumu","#mu#mu"); LabelText.ReplaceAll("geq2jets","#geq2jets"); LabelText.ReplaceAll("eq0jets","0jet");  LabelText.ReplaceAll("eq1jets","1jet");
///    Label->AddText(LabelText);  Label->Draw();
///    
///    I++;
///  }
///  //print legend
///  c1->cd(0);
///  legA->SetFillColor(0); legA->SetFillStyle(0); legA->SetLineColor(0);  legA->SetBorderSize(0); legA->SetHeader("");
///  legA->SetNColumns((NLegEntry/2) + 1);
///  legA->Draw("same");    legA->SetTextFont(42);
///  
///  //print canvas header
///  t2->cd(0);
///  //           TPaveText* T = new TPaveText(0.1,0.995,0.84,0.95, "NDC");
///  TPaveText* T = new TPaveText(0.1,0.7,0.9,1.0, "NDC");
///  T->SetFillColor(0);  T->SetFillStyle(0);  T->SetLineColor(0); T->SetBorderSize(0);  T->SetTextAlign(22);
///  if(systpostfix.Contains('8')){ T->AddText("CMS preliminary, #sqrt{s}=8.0 TeV");
///  }else{                         T->AddText("CMS preliminary, #sqrt{s}=7.0 TeV");
///  }T->Draw();
///  
///  //save canvas
///  c1->SaveAs(SaveName+"_Shape.png");
///  c1->SaveAs(SaveName+"_Shape.pdf");
///  c1->SaveAs(SaveName+"_Shape.C");
///  delete c1;
///}



//
int main(int argc, char* argv[])
{
  setTDRStyle();

  //get input arguments
  for(int i=1;i<argc;i++){
    string arg(argv[i]);
    if(arg.find("--help")        !=string::npos)              { printHelp(); return -1;} 
    else if(arg.find("--out")     !=string::npos && i+1<argc) { outUrl = argv[i+1]; outUrl+="/";  i++;  printf("out = %s\n", outUrl.Data());  }
    else if(arg.find("--suffix") !=string::npos && i+1<argc) { suffixUrl = argv[i+1];  i++;  printf("suffix = %s\n", suffixUrl.Data());  }
    else if(arg.find("--in")     !=string::npos && i+1<argc)  { inFileUrl = argv[i+1];  i++;  printf("in = %s\n", inFileUrl.Data());  }
    else if(arg.find("--syst")   !=string::npos && i+1<argc)  { systFileUrl = argv[i+1];  i++;  printf("syst = %s\n", systFileUrl.Data());  }
    else if(arg.find("--json")   !=string::npos && i+1<argc)  { jsonFileUrl  = argv[i+1];  i++;  printf("json = %s\n", jsonFileUrl.Data()); }
    else if(arg.find("--histo")  !=string::npos && i+1<argc)  { histo     = argv[i+1];  i++;  printf("histo = %s\n", histo.Data()); }
    else if(arg.find("--noPowers" )!=string::npos){ doPowers=false;    }
    else if(arg.find("--bins")   !=string::npos && i+1<argc)  { char* pch = strtok(argv[i+1],",");printf("bins to use are : ");while (pch!=NULL){int b; sscanf(pch,"%d",&b); printf(" %d ",b); binsToProject.push_back(b);  pch = strtok(NULL,",");}printf("\n"); i++; }
    else if(arg.find("--ch")     !=string::npos && i+1<argc)  { char* pch = strtok(argv[i+1],",");printf("ch to use are : ");  while (pch!=NULL){printf(" %s ",pch); channels.push_back(pch);  pch = strtok(NULL,",");}printf("\n"); i++; }
    else if(arg.find("--statBinByBin")  !=string::npos){ statBinByBin=true;}
  }
  if(jsonFileUrl.IsNull() || inFileUrl.IsNull() || histo.IsNull()) { printHelp(); return -1; }
  if(channels.size()==0) { channels.push_back("ee"); channels.push_back("mumu"); channels.push_back("emu"); channels.push_back(""); }

 
  //get the shapes
  TFile *inF = TFile::Open(inFileUrl);
  JSONWrapper::Object jsonF(jsonFileUrl.Data(), true);
  TFile *systF=0;
  if(!systFileUrl.IsNull()) systF=TFile::Open(systFileUrl);
  std::map<TString, Shape_t> shapes;
  for(std::vector<string>::iterator cIt = channels.begin(); cIt != channels.end(); cIt++) shapes[*cIt]=getShapeFromFile(inF, *cIt, jsonF,systF);
  inF->Close();
  //  if(!systF) systF->Close();

  //print the tables/datacards
  getYieldsFromShapes(shapes);
  
  //convert shapes to datacards
  convertShapesToDataCards(shapes);

  //  //combine the datacards and run profile likelihood analysis
  //  TString combCardCmd("combineCards.py "),plrAnalysisCmd("runPLRanalysis --in "); 
  //  int icard(1);
  //  for(std::map<TString, Shape_t>::const_iterator it=shapes.begin(); it!=shapes.end(); it++)
  //    {
  //      TString ch(it->first); if(ch.IsNull() || ch=="inclusive") continue;
  //      combCardCmd += "Name"; combCardCmd += icard; combCardCmd +="="+outUrl+"DataCard_"+ch+".dat ";
  //      plrAnalysisCmd += outUrl+"DataCard_"+ch+".dat,";
  //      icard++;
  //    }
  //  combCardCmd += " > "+outUrl+"DataCard_combined.dat";
  //  plrAnalysisCmd += outUrl+"DataCard_combined.dat";
  //  gSystem->Exec("mv DataCard* " + outUrl);
  //  gSystem->Exec(combCardCmd.Data());
  //  gSystem->Exec(plrAnalysisCmd.Data());
  //  gSystem->Exec("mv PLR* " + outUrl);
}

