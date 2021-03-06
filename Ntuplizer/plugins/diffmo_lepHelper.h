#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"

// Helper functions for diffmo's lepton module
namespace LEPDF
{

  	typedef std::vector<reco::Candidate::PolarLorentzVector> p4_vector;

	double lepPFIso(std::vector<pat::Muon>::const_iterator muon)
	{
		double chIso = muon->userIsolation(pat::PfChargedHadronIso);
		double nhIso = muon->userIsolation(pat::PfNeutralHadronIso);
		double gIso  = muon->userIsolation(pat::PfGammaIso);
		double pt    = muon->pt() ;
		double pfIso = (chIso + nhIso + gIso) / pt;
		return pfIso;
	}

	double lepPFIso(std::vector<pat::Electron>::const_iterator electron)
	{
		double chIso = electron->userIsolation(pat::PfChargedHadronIso);
		double nhIso = electron->userIsolation(pat::PfNeutralHadronIso);
		double gIso  = electron->userIsolation(pat::PfGammaIso);
		double pt    = electron->pt() ;
		double pfIso = (chIso + nhIso + gIso) / pt;
		return pfIso;
	}

	unsigned int lepTight(std::vector<pat::Muon>::const_iterator muon)
	{
		unsigned int is_tight_muon = 0;
		if (muon->isGlobalMuon()) 
		    {
			if (muon->isTrackerMuon()) 
			{
			    if (static_cast<int>( muon->numberOfValidHits()) > 10) 
			    {
				if (muon->dB() < 0.2) 
				{
				    if (muon->normChi2() < 10) 
				    {
					if (muon->numberOfMatchedStations() > 0) 
					{
					    if (muon->track()->hitPattern().numberOfValidPixelHits() > 0) 
					    {
						is_tight_muon = 1;
					    } 
					}
				    }
				}
			    }
			}
		    }
		return is_tight_muon;
	}

	unsigned int lepTight(std::vector<pat::Electron>::const_iterator electron)
	{
		unsigned int is_tight_ele = 0;
		if (electron->dB() < 0.2) 
		{
			if (not electron->gsfTrack()->trackerExpectedHitsInner().numberOfHits() < 1) 
			{
				is_tight_ele = 1;
			}
		}
		return is_tight_ele;
	}

	unsigned int lepModTight(std::vector<pat::Electron>::const_iterator electron)
	{
		double abseta = std::abs(electron->superCluster()->eta());
		if (abseta <= 1.4442 || abseta >= 1.5660) {
			return 0;
		}
		else {
			if (electron->dB() >= 0.02) {
				return 0;
			}
			else {
				//not sure about this one
				if (electron->gsfTrack()->dz() >= 0.01) {
					return 0;
				}
				else return 1;
			}
		}
	}
}
