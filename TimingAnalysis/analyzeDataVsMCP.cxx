#include <TimingAnalysis.h>
#include <timingAlgorithm.h>

#include <TFileCollection.h>
#include <TGraph2DErrors.h>
#include <TTree.h>
#include <cstdio>
#include <fstream>

int main (int argc, char** argv)
{
//   std::string filename;
  std::string namesensor;
  std::string filename;
  std::string lp_string;
  std::string outputdir("./Results/");
  int firstchannel=3; //MCP
  int secondchannel=0;
  float cfd_threshold=0.4;
  float threshold_MCP=-0.01;
  float threshold=-0.01;
  float lowpass=0;
  float minTrackerX=0;
  float maxTrackerX=0;
  float minTrackerY=0;
  float maxTrackerY=0;
  int selectOnlyNewTracker = 0;

  std::string Run_config_in;
  int configuration = 0;



  // Additional parameters
  float hysteresis=0.3e-3;
  float min_amplitude_MCP=0;
  float max_amplitude_MCP=0.3;
  float min_amplitude_ch2=0;
  float max_amplitude_ch2=1;
  float baseline_p=0.2; //Use first 10% of the samples to compute baseline

  for (int i=1; i<argc; ++i)
  {
    if (argv[i][0] == '-') {
      // Option found
      std::string option(argv[i]);
      if ( option == "-h" || option == "--help" ) {
        std::cout << "List of options: " << std::endl;
        std::cout << "-h [ --help ]                     produce help message" << std::endl;
        std::cout << "-f [ --channel ] (=0)             channel to analyze" << std::endl;
        std::cout << "-k [ --configuration ]       Global congif to analyze" << std::endl;
        std::cout << "-c [ --cfd_threshold ] (=0.4)     CFD fraction" << std::endl;
        std::cout << "                                  a negative value will start a scan with" << std::endl;
        std::cout << "                                  a step equal to |cfd_threshold|" << std::endl;
        std::cout << "-t [ --threshold ] (=-0.1)        Threshold, negative for" << std::endl;
        std::cout << "                                  negative signals (V)" << std::endl;
        std::cout << "-p [ --lowpass ] (=0)             Lowpass filter frequency (Hz)" << std::endl;
        std::cout << "-o [ --outputdir ] (=./Results)   output directory" << std::endl;
        std::cout << "-i [ --Run_config_in ]            Run_config.txt input file" << std::endl;
        std::cout << "-s [ --saturation ] (=0.2)        saturation cut for DUT" << std::endl;
	std::cout << "-n [ --namesensor ]               name of the sensor and board" << std::endl;
	std::cout << "-y [ --newtracker ]               select if the run has nback && npix" << std::endl;
	return 0;
      }
      std::string value(argv[++i]);

      if ( option == "-f" || option == "--channel" )
        secondchannel = std::stoi(value);
      if ( option == "-i" || option == "--Run_cofig_in" )
        Run_config_in = value;
      if ( option == "-c" || option == "--cfd_threshold" )
        cfd_threshold = std::stof(value);
      if ( option == "-t" || option == "--threshold" )
        threshold = std::stof(value);
      if ( option == "-p" || option == "--lowpass" )
        lp_string = value;
      if ( option == "-o" || option == "--outputdir" )
        outputdir = value;
      if ( option == "-k" || option == "--configuration" )
        configuration = std::stoi(value);
     // if ( option == "-i" || option == "--filename" )
       // filename = value;
      if ( option == "-s" || option == "--saturation" )
        max_amplitude_ch2 = std::stof(value);
      if ( option == "--MCPsaturation" )
        max_amplitude_MCP = std::stof(value);
      if ( option == "--MCPthreshold" )
        threshold_MCP = std::stof(value);
      if ( option == "--MCPchannel" )
        firstchannel = std::stoi(value);
      if ( option == "-n" || option == "--namesensor" )
        namesensor = (value);
      if ( option == "-y" || option == "--newtracker" )
        selectOnlyNewTracker = std::stoi(value);
      if ( option == "--xmin")
	    minTrackerX = std::stof(value);
      if ( option == "--xmax")
	    maxTrackerX = std::stof(value);
      if ( option == "--ymin")
	    minTrackerY = std::stof(value);
      if ( option == "--ymax")
	    maxTrackerY = std::stof(value);
    }
  }

std::cout<<"Run_config_in: "<<Run_config_in<<std::endl;
  if (Run_config_in == "") {
    std::cout << "Input Run_ config.txt file required! For help use:" << std::endl;
    std::cout << argv[0] << " --help" << std::endl;
    return 0;
  }

     TChain* input_tree = new TChain("pulse");

  std::string newTracker_string = std::to_string(selectOnlyNewTracker);
  std::ifstream datafile (Run_config_in.c_str());
  std::string line;
  Int_t config,run;
  lowpass = std::stof(lp_string);



  if (datafile.is_open())
    {
      while ( getline (datafile,line) )
	{
	  if ((line.at(0)>='0' && line.at(0)<='9'))
	    {
	        std::stringstream iss(line);
	        Int_t run;
	        iss>>run>>config;

		if (config == configuration)
		{
		  TString path;
		  path.Form("root://cmsxrootd.fnal.gov//store/user/cmstestbeam/2019_04_April_CMSTiming/KeySightScope/RecoData/TimingDAQRECO/RecoWithTracks/v1/run_scope%i_converted.root/pulse",run);
	    std::cout<<"Searching File: "<<path<<std::endl;
		  TString path2 = path.Remove(path.Length()-6,6);
		  TFile *f_tmp = TFile::Open(path2);
      if (f_tmp != nullptr) f_tmp->ls();
      else std::cout<< "File not opened correctly!"<<std::endl;
      bool nbackfound = false;
		  if (f_tmp != nullptr) {
			  TChain chain_tmp("pulse");
			  chain_tmp.Add(path);
			  size_t n = chain_tmp.GetListOfBranches()->GetEntries();
			  for( size_t i = 0; i < n; ++ i ) {
			    TBranch *subbr = dynamic_cast<TBranch*>(chain_tmp.GetListOfBranches()->At(i));
          // std::cout<<subbr->GetName()<<std::endl;
			    if (subbr->GetName() == "nback")
            nbackfound = true;
			  }
			  delete f_tmp;
		  }
	     if (nbackfound == (selectOnlyNewTracker>0)) {
	       std::cout << "Adding file :: "<<path<<std::endl;
	       input_tree->Add(path);}
	}
    }
  }
}

  // Creating the analysis object from data TTree
  //TFile * input_file = new TFile(filename.c_str());
 // std::cout<<"ok"<<std::endl;
 // TTree* input_tree = nullptr;
  //input_tree = chain;
//   input_file->GetObject("pulse",input_tree);

  if (input_tree->GetEntries() < 1000) return 0;
  else std::cout<<"SELECTED: "<< namesensor << std::endl;

  TimingAnalysis example_analyzeData(input_tree, selectOnlyNewTracker, minTrackerX, maxTrackerX, minTrackerY, maxTrackerY);

  // Output file
  TString filenameTail(namesensor);
  filenameTail+="_config";
  filenameTail+=configuration;

  if(cfd_threshold >=0){
  float cfd_tmp =0 ;
  cfd_tmp = std::abs(cfd_threshold * 100);
  std::string cfd_string = std::to_string(cfd_tmp);
  cfd_string = cfd_string.erase(cfd_string.size()-7,cfd_string.size());
  filenameTail+="_CFD";
  filenameTail+=cfd_string;
  filenameTail+="_Ch";
  filenameTail+=firstchannel;
  filenameTail+="vsCh";
  filenameTail+=secondchannel;
  filenameTail+="_nback";
  filenameTail+=newTracker_string;
  filenameTail+="_filter";
   if(lowpass >= 0)   {
	  filenameTail+=lp_string;
   		}
   else if (lowpass < 0)  {
	   filenameTail+="_LPscan";
   		}
  filename = outputdir;
  filename += filenameTail;
  filename += ".root";
 	 }

else {
  filenameTail+="_CFDscan";
  filenameTail+="_Ch";
  filenameTail+=firstchannel;
  filenameTail+="vsCh";
  filenameTail+=secondchannel;
  filenameTail+="_nback::";
  filenameTail+=newTracker_string;
  filenameTail+="_filter";
   if(lowpass >= 0)   {
	  filenameTail+=lp_string;
   		}
   else if (lowpass < 0)  {
	   filenameTail+="_LPscan";
   		}
  filename = outputdir;
  filename += filenameTail;
  filename += ".root";
	}

  TFile * f_root = new TFile (filename.c_str(),"RECREATE");
  TString filenameTmp = outputdir;
  filenameTmp += filenameTail;
  filenameTmp += ".log";
  // std::ofstream f(filenameTmp);
  // f << "Test\n";
  // f.close();

  bool empty=false;
  bool full=false;
  bool low=false;

  std::cout<<"Filling "<<filename<<" with "<<filename<<std::endl;

  bool channelScan=true;
  int secondchannel_local = 0;
  while (channelScan && secondchannel_local<3) {
    if (secondchannel>=0) {
      channelScan=false;
      secondchannel_local = secondchannel;
    }
    else {
      ++secondchannel_local;
    }
    TString ch_dir_name("channel_");
    ch_dir_name += secondchannel_local;
    TDirectory* ch_dir = f_root->mkdir(ch_dir_name);
    std::cout<<"############ analyzing Ch " << secondchannel_local << std::endl;
    ch_dir->cd();

    if (cfd_threshold > 0) {
      if (lowpass>=0) {
        AlgorithmParameters par(0.5, cfd_threshold, threshold_MCP,threshold,lowpass,hysteresis,min_amplitude_MCP, max_amplitude_MCP, min_amplitude_ch2, max_amplitude_ch2,baseline_p);
        double timeres_ps = example_analyzeData.executeTimeDifferenceWithMCP<AlgorithmParameters>(f_root, ComputeExactTimeCFD, par, firstchannel, secondchannel_local)*1e12;
        // std::cout << "############ RESULTS: Time difference " << timeres_ps << " +- " << par.errorOnSigma << " ps, using " << par.found << " coincidences" << std::endl;

        if (!full) {
          if (par.sigmaOfCh1Amplitude < 0.001)
            empty=true;
          if (par.found>300)
            full=true;
          else
            low=true;
          }
      }
      else {
        TDirectory* lowpass_dir = ch_dir->mkdir("lowpass_scan");
        TGraphErrors lowpass_graph;
        lowpass_graph.SetName("lowpass_scan");
        int lowpass_counter=0;
        std::cout<< "Starting lowpass scan" <<std::endl;
        for (float lowpass_tmp=0.4e9; lowpass_tmp<1.5e9; lowpass_tmp+=std::abs(lowpass)) {
          std::cout<< "############ SCAN: Lowpass Frequency set to " << lowpass_tmp << " Hz ############" <<std::endl;
          TString lowpass_tmpdir_name("lowpass_");
          lowpass_tmpdir_name += (int) (1e-6*lowpass_tmp);
          lowpass_tmpdir_name += "_MHz";
          TDirectory* cfd_tmpdir = ch_dir->mkdir(lowpass_tmpdir_name);
          cfd_tmpdir->cd();
          AlgorithmParameters par( 0.5, cfd_threshold, threshold_MCP,threshold,lowpass_tmp,hysteresis,min_amplitude_MCP, max_amplitude_MCP, min_amplitude_ch2, max_amplitude_ch2,baseline_p);  //Fixed cfd_threshold for ch0
          double timeres_ps = example_analyzeData.executeTimeDifferenceWithMCP<AlgorithmParameters>(f_root, ComputeExactTimeCFD, par, firstchannel, secondchannel_local)*1e12;
          std::cout << "############ SCAN: Time difference " << timeres_ps << " +- " << par.errorOnSigma << " ps, using " << par.found << " coincidences ############" << std::endl;
          if (timeres_ps<100) {
            lowpass_graph.SetPoint(lowpass_counter, lowpass_tmp, timeres_ps);
            lowpass_graph.SetPointError(lowpass_counter++, 0, 1./std::sqrt(par.found));
          }
        }
       lowpass_dir->cd();
       lowpass_graph.Write();
      }
    }
    else {
      if (lowpass>=0) {
        TDirectory* cfd_dir = ch_dir->mkdir("cfd_scan");
        TGraphErrors cfd_graph;
        cfd_graph.SetName("cfd_scan");
        int cfd_counter=0;
        for (float cfd_th=std::abs(cfd_threshold); cfd_th<1; cfd_th+=std::abs(cfd_threshold)) {
          std::cout<< "############ SCAN: CFD fraction " << cfd_th << " ############" <<std::endl;
          TString cfd_tmpdir_name("cfd_");
          cfd_tmpdir_name += (int) (100*cfd_th);
          cfd_tmpdir_name += "_percent";
          TDirectory* cfd_tmpdir = ch_dir->mkdir(cfd_tmpdir_name);
          cfd_tmpdir->cd();
          AlgorithmParameters par( 0.5, cfd_th, threshold_MCP,threshold,lowpass,hysteresis,min_amplitude_MCP, max_amplitude_MCP, min_amplitude_ch2, max_amplitude_ch2,baseline_p);  //Fixed cfd_threshold for ch0
          double timeres_ps = example_analyzeData.executeTimeDifferenceWithMCP<AlgorithmParameters>(f_root, ComputeExactTimeCFD, par, firstchannel, secondchannel_local)*1e12;
          std::cout << "############ SCAN: Time difference " << timeres_ps << " +- " << par.errorOnSigma << " ps, using " << par.found << " coincidences ############" << std::endl;
          cfd_graph.SetPoint(cfd_counter, cfd_th, timeres_ps);
          cfd_graph.SetPointError(cfd_counter++, 0, timeres_ps/std::sqrt(par.found));
        }
        cfd_dir->cd();
        cfd_graph.Write();
      }
      else {
        TDirectory* bidim_dir = ch_dir->mkdir("2d_scan");
        TGraph2DErrors bidim_graph;
        bidim_graph.SetName("bidim_scan");
        int bidim_counter=0;
        for (float lowpass_tmp=std::abs(lowpass); lowpass_tmp<1.0e9; lowpass_tmp+=std::abs(lowpass)) {
          for (float cfd_th=std::abs(cfd_threshold); cfd_th<1; cfd_th+=std::abs(cfd_threshold)) {
            std::cout<< "############ SCAN: Lowpass Frequency set to " << lowpass_tmp << " Hz, CFD " << (int) (100*cfd_th) << " % ############" <<std::endl;
            TString bidim_tmpdir_name("bidim_");
            bidim_tmpdir_name += (int) (1e-6*lowpass_tmp);
            bidim_tmpdir_name += "_MHz_";
            bidim_tmpdir_name += (int) (100*cfd_th);
            bidim_tmpdir_name += "_percent";
            TDirectory* cfd_tmpdir = ch_dir->mkdir(bidim_tmpdir_name);
            cfd_tmpdir->cd();
            AlgorithmParameters par( 0.5, cfd_th, threshold_MCP,threshold,lowpass_tmp,hysteresis,min_amplitude_MCP, max_amplitude_MCP, min_amplitude_ch2, max_amplitude_ch2,baseline_p);  //Fixed cfd_threshold for ch0
            double timeres_ps = example_analyzeData.executeTimeDifferenceWithMCP<AlgorithmParameters>(f_root, ComputeExactTimeCFD, par, firstchannel, secondchannel_local)*1e12;
            std::cout << "############ SCAN: Time difference " << timeres_ps << " +- " << par.errorOnSigma << " ps, using " << par.found << " coincidences" << std::endl;
            if (timeres_ps<100) {
              bidim_graph.SetPoint(bidim_counter, lowpass_tmp, cfd_th, timeres_ps);
              bidim_graph.SetPointError(bidim_counter++, 0, 0, 1./std::sqrt(par.found));
            }
          }
        }
       bidim_dir->cd();
       bidim_graph.Write();
      }
    }
  }


  f_root->Close();

  // if (empty || full || low) {
  //   std::string newfilename(filename);
  //   if (newfilename.size () > 6)  newfilename.resize (newfilename.size () - 5);
  //   if (empty)
  //     newfilename.append("_empty.root");
  //   else if (low)
  //     newfilename.append("_low.root");
  //   else if (full)
  //     newfilename.append("_good.root");
  //   std::rename(filename.c_str(), newfilename.c_str());
  // }

  std::cout<<"Exiting..."<<std::endl;
  return 0;
}
