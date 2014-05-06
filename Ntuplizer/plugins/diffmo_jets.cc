// hadronic actions

#include <memory>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "Analysis/Ntuplizer/plugins/diffmo_hadHelper.h"

#include <iostream>

using namespace HADDF;

// Class Def:
class DiFfMoHadronic : public edm::EDFilter 	
{ // open primary class def
   public:
      explicit DiFfMoHadronic(const edm::ParameterSet&);
     ~DiFfMoHadronic();
   private:
      virtual void beginJob() ;
      virtual bool filter(edm::Event&, const edm::EventSetup&); // this is essentially the Ntuplizer code
      virtual void endJob() ;
	// member data
	edm::InputTag	jetSrc_;
	edm::InputTag	genSrc_;
	edm::InputTag	npvSrc_;
	std::string	jetName_;
	std::string	btagType_;
	edm::InputTag   rhoSrc_;          /// mean pt per unit area
	std::string	useNsub_;
	std::string	subcorr_;
	signed int	basecorr_;
	bool 		doT;
	bool 		doB;
	bool 		subcorr;
  	double          scale_;
  	double          smear_;
  	double          etaSmear_;
  	boost::shared_ptr<FactorizedJetCorrector> jec_;
  	boost::shared_ptr<JetCorrectionUncertainty> jecUnc_;
	std::vector<std::string>  jecPayloads_; /// files for JEC payloads
}; // close primary class def


DiFfMoHadronic::DiFfMoHadronic(const edm::ParameterSet& iConfig) :
	jetSrc_ 	(iConfig.getParameter<edm::InputTag>("jetSrc")),
	genSrc_ 	(iConfig.getParameter<edm::InputTag>("genSrc")),
	npvSrc_ 	(iConfig.getParameter<edm::InputTag>("npvSrc")),
	jetName_	(iConfig.getParameter<std::string>("jetName")),
	btagType_	(iConfig.getParameter<std::string>("btagType")),
	useNsub_	(iConfig.getParameter<std::string>("useNsub")),
	subcorr_	(iConfig.getParameter<std::string>("subcorr")),
  	rhoSrc_       	(iConfig.getParameter<edm::InputTag>("rhoSrc")),
  	scale_        	(iConfig.getParameter<double>( "jetScale" ) ),
 	smear_        	(iConfig.getParameter<double>( "jetPtSmear") ),
  	etaSmear_     	(iConfig.getParameter<double>( "jetEtaSmear") ),
  	jecPayloads_  	(iConfig.getParameter<std::vector<std::string> >  ("jecPayloads")),
	basecorr_	(iConfig.getParameter<signed int>("basecorr"))
{
	subcorr = (subcorr_ == "yes" or subcorr_ == "y");
	doB = (btagType_ != "");
	doT = (useNsub_ == "yes" or useNsub_ == "y");


  	std::vector<JetCorrectorParameters> vPar;
  	for ( std::vector<std::string>::const_iterator ipayload = jecPayloads_.begin(), ipayloadEnd = jecPayloads_.end(); ipayload != ipayloadEnd - 1; ++ipayload ) 
	{
    		std::cout << "Adding payload " << *ipayload << std::endl;
    		JetCorrectorParameters pars(*ipayload);
    		vPar.push_back(pars);
  	}
  	jec_ = boost::shared_ptr<FactorizedJetCorrector> ( new FactorizedJetCorrector(vPar) );
	jecUnc_ = boost::shared_ptr<JetCorrectionUncertainty>( new JetCorrectionUncertainty(jecPayloads_.back()));

	produces<std::vector<reco::Candidate::PolarLorentzVector> > (jetName_);
	produces<std::vector<reco::Candidate::PolarLorentzVector> > (jetName_+"sub0");
	produces<std::vector<reco::Candidate::PolarLorentzVector> > (jetName_+"sub1");
	produces<std::vector<reco::Candidate::PolarLorentzVector> > (jetName_+"sub2");
	produces<std::vector<reco::Candidate::PolarLorentzVector> > (jetName_+"sub3");
	produces<std::vector<double> > (jetName_+"csv");
	if (doT)
	{
		produces<std::vector<double> > (jetName_+"tau1");
		produces<std::vector<double> > (jetName_+"tau2");
		produces<std::vector<double> > (jetName_+"tau3");
		produces<std::vector<double> > (jetName_+"tau4");
	}
	if (doB)
	{
		produces<std::vector<double> > (jetName_+"sub0csv");
		produces<std::vector<double> > (jetName_+"sub1csv");
		produces<std::vector<double> > (jetName_+"sub2csv");
		produces<std::vector<double> > (jetName_+"sub3csv");
	}
	produces<std::vector<unsigned int>> (jetName_+"nsub");
	produces<std::vector<reco::Candidate::PolarLorentzVector> > (jetName_+"CORR");
	produces<std::vector<reco::Candidate::PolarLorentzVector> > (jetName_+"sub0CORR");
	produces<std::vector<reco::Candidate::PolarLorentzVector> > (jetName_+"sub1CORR");
	produces<std::vector<reco::Candidate::PolarLorentzVector> > (jetName_+"sub2CORR");
	produces<std::vector<reco::Candidate::PolarLorentzVector> > (jetName_+"sub3CORR");	
}


void DiFfMoHadronic::beginJob()
{
	std::cout << "adding jets... "<<jetName_<<"\n";
}

void DiFfMoHadronic::endJob() 
{
}

bool DiFfMoHadronic::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{

	edm::Handle<std::vector<pat::Jet> > h_Jets;
	iEvent.getByLabel( jetSrc_, h_Jets);
  	edm::Handle<std::vector<reco::GenJet> > h_genJets;
  	iEvent.getByLabel( genSrc_, h_genJets);
 	edm::Handle< unsigned int > h_npv;
  	edm::Handle<double> h_rho;
  	iEvent.getByLabel( npvSrc_, h_npv );
  	iEvent.getByLabel( rhoSrc_, h_rho );
  	double rhoVal = *h_rho;
	unsigned int npv = *h_npv;
	
	std::auto_ptr<p4_vector> jets( new p4_vector() );
	std::auto_ptr<std::vector<double>> jetsCSV( new std::vector<double> );
	std::auto_ptr<std::vector<double>> jetstau1( new std::vector<double> );
	std::auto_ptr<std::vector<double>> jetstau2( new std::vector<double> );
	std::auto_ptr<std::vector<double>> jetstau3( new std::vector<double> );
	std::auto_ptr<std::vector<double>> jetstau4( new std::vector<double> );
	std::auto_ptr<std::vector<unsigned int>> nsub( new std::vector<unsigned int> );
	std::auto_ptr<p4_vector> sub0(new p4_vector());
	std::auto_ptr<p4_vector> sub1(new p4_vector());
	std::auto_ptr<p4_vector> sub2(new p4_vector());
	std::auto_ptr<p4_vector> sub3(new p4_vector());
	std::auto_ptr<std::vector<double>> sub0csv(new std::vector<double>);
	std::auto_ptr<std::vector<double>> sub1csv(new std::vector<double>);
	std::auto_ptr<std::vector<double>> sub2csv(new std::vector<double>);
	std::auto_ptr<std::vector<double>> sub3csv(new std::vector<double>);
	std::auto_ptr<p4_vector> jets_CORR( new p4_vector() );
	std::auto_ptr<p4_vector> sub0_CORR(new p4_vector());
	std::auto_ptr<p4_vector> sub1_CORR(new p4_vector());
	std::auto_ptr<p4_vector> sub2_CORR(new p4_vector());
	std::auto_ptr<p4_vector> sub3_CORR(new p4_vector());
	// uncorrected jets and their properties:
	for ( std::vector<pat::Jet>::const_iterator jetBegin = h_Jets->begin(), jetEnd = h_Jets->end(), ijet = jetBegin; ijet != jetEnd; ++ijet )
	{
		jetsCSV->push_back(ijet->bDiscriminator(btagType_));
		reco::Candidate::LorentzVector new_jet = ijet->correctedP4(basecorr_);
          	reco::Candidate::PolarLorentzVector uncorr_jet (new_jet.pt(), new_jet.eta(), new_jet.phi(), new_jet.mass());
		jets->push_back(uncorr_jet);
		unsigned int nsub = ijet->numberOfDaughters();
		if (doB) PopulateSubjets(ijet, sub0, sub1, sub2, sub3, nsub, btagType_, sub0csv, sub1csv, sub2csv, sub3csv);
		if (doT) CalculateTaus(ijet, jetstau1, jetstau2, jetstau3, jetstau4);
		ApplyJec(ijet, jec_, jecUnc_, h_genJets, jets_CORR, sub0_CORR, sub1_CORR, sub2_CORR, sub3_CORR, nsub, npv, rhoVal, scale_, smear_, etaSmear_, subcorr);
		
	}
	// corrected jets and their properties:
	iEvent.put(jets, jetName_);
	iEvent.put(jetsCSV, jetName_+"csv");
	if (doT)
	{
		iEvent.put(jetstau1, jetName_+"tau1");
		iEvent.put(jetstau2, jetName_+"tau2");
		iEvent.put(jetstau3, jetName_+"tau3");
		iEvent.put(jetstau4, jetName_+"tau4");
	}
	iEvent.put(nsub, jetName_+"nsub");
	if (doB)
	{
		iEvent.put(sub0, jetName_+"sub0");
		iEvent.put(sub1, jetName_+"sub1");
		iEvent.put(sub2, jetName_+"sub2");
		iEvent.put(sub3, jetName_+"sub3");
		iEvent.put(sub0csv, jetName_+"sub0csv");
		iEvent.put(sub1csv, jetName_+"sub1csv");
		iEvent.put(sub2csv, jetName_+"sub2csv");
		iEvent.put(sub3csv, jetName_+"sub3csv");
	}
	iEvent.put(jets_CORR, jetName_+"CORR");
	if (subcorr)
	{
		iEvent.put(sub0_CORR, jetName_+"sub0CORR");
		iEvent.put(sub1_CORR, jetName_+"sub1CORR");
		iEvent.put(sub2_CORR, jetName_+"sub2CORR");
		iEvent.put(sub3_CORR, jetName_+"sub3CORR");
	}
	return true;
}

DiFfMoHadronic::~DiFfMoHadronic(){}

//define this as a plug-in
DEFINE_FWK_MODULE(DiFfMoHadronic);