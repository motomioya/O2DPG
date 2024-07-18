// usage (fwdy) :
//o2-sim -j 4 -n 10 -g external -t external -m "PIPE ITS TPC" -o sgn --configFile GeneratorHF_bbbar_fwdy.ini 
// usage (midy) :
//o2-sim -j 4 -n 10 -g external -t external -m "PIPE ITS TPC" -o sgn --configFile GeneratorHF_bbbar_midy.ini 
//
//
R__ADD_INCLUDE_PATH($PYTHIA_ROOT/include)
R__ADD_INCLUDE_PATH($O2DPG_ROOT/MC/config/PWGDQ/EvtGen)

#include "GeneratorEvtGen.C"
#include "Pythia8/Pythia.h"

namespace o2
{
namespace eventgen
{

class GeneratorVirtualPhoton : public GeneratorPythia8
{

 public:
  GeneratorVirtualPhoton() : GeneratorPythia8(){};
  ~GeneratorVirtualPhoton() = default;

  // We initialise the local Pythia8 event where we store the particles
  // of the signal event that is the sum of multiple Pythia8 events
  // generated according to the generateEvent() function below.
  Bool_t Init() override
  {
    mOutputEvent.init("(GeneratorHF output event)", &mPythia.particleData);
    return GeneratorPythia8::Init();
  }

  // This function is called by the primary generator
  // for each event in case we are in embedding mode.
  // We use it to setup the number of signal events
  // to be generated and to be embedded on the background.
  void notifyEmbedding(const o2::dataformats::MCEventHeader* bkgHeader) override
  {
    mEvents = mFormula.Eval(bkgHeader->GetB());
    std::cout << " --- notify embedding: impact parameter is " << bkgHeader->GetB() << ", generating " << mEvents << " signal events " << std::endl;
  };

  // We override this function to be able to generate multiple
  // events and build an output event that is the sum of them
  // where we have stripped out only the sub-event starting from
  // the c-cbar ancestor particle
  Bool_t generateEvent() override
  {

    // reset counter and event
    mOutputEvent.reset();

    // loop over number of events to be generated
    int nEvents = 0;
    while (nEvents < mEvents) {

      // generate event
      if (!GeneratorPythia8::generateEvent())
        return false;

      // find the c-cbar ancestor
      auto ancestor = findAncestor(mPythia.event);
      if (ancestor < 0)
        continue;

      // append ancestor and its daughters to the output event
      selectFromAncestor(ancestor, mPythia.event, mOutputEvent);
      nEvents++;
    }

    if (mVerbose)
      mOutputEvent.list();

    return true;
  };

  // We override this event to import the particles from the
  // output event that we have constructed as the sum of multiple
  // Pythia8 sub-events as generated above
  Bool_t importParticles() override
  {
    return GeneratorPythia8::importParticles(mOutputEvent);
  }

  // search for c-cbar mother with at least one c at midrapidity
  int findAncestor(Pythia8::Event& event)
  {
    for (int ipa = 0; ipa < event.size(); ++ipa) {
      auto daughterList = event[ipa].daughterList();
      bool hasvp = false, atDecaySelectedY = false;
      for (auto ida : daughterList) {
        if (event[ida].id() == mPDG)
          hasvp = true;
        auto gdaughterList = event[ida].daughterList();
        for (auto igd : gdaughterList) {
          if ((event[igd].y() > mDecayRapidityMin) && (event[igd].y() < mDecayRapidityMax))
            atDecaySelectedY = true;
        }
      }
      if (hasvp && atDecaySelectedY)
        return ipa;
    }
    return -1;
  };

  void setPDG(int val) { mPDG = val; };
  void setDecayPDG(int val) { mDecayPDG = val; };
  void setDecayRapidity(double valMin, double valMax)
  {
    mDecayRapidityMin = valMin;
    mDecayRapidityMax = valMax;
  };
  void setVerbose(bool val) { mVerbose = val; };
  void setFormula(std::string val) { mFormula.Compile(val.c_str()); };

 private:
  TFormula mFormula;
  int mEvents = 1;
  Pythia8::Event mOutputEvent;
  int mPDG = 23;
  int mDecayPDG = 13;
  double mDecayRapidityMin = -1.5;
  double mDecayRapidityMax = 1.5;
  bool mVerbose = false;
};

} // namespace eventgen
} // namespace o2


FairGenerator*
GeneratorVirtualPhotonToMu_EvtGenFwdY(double rapidityMin = -4.3, double rapidityMax = -2.2, bool ispp = true, bool verbose = false)
{
  auto gen = new o2::eventgen::GeneratorEvtGen<o2::eventgen::GeneratorVirtualPhoton>();
  gen->setDecayRapidity(rapidityMin,rapidityMax);
  gen->setPDG(23);
  gen->setDecayPDG(13);

  gen->setVerbose(verbose);
  if(ispp) gen->setFormula("1");
  else gen->setFormula("max(1.,120.*(x<5.)+80.*(1.-x/20.)*(x>5.)*(x<11.)+240.*(1.-x/13.)*(x>11.))");
  gen->readString("ProcessLevel:all = off");
  gen->readString("23:onMode = off");
  gen->readString("23:onIfMatch = 13 13");
  gen->readString("23:m0 = 5.0");
  gen->readString("23:m0 = 5.0");
  // set random seed
  uint random_seed;
  unsigned long long int random_value = 0; 
  ifstream urandom("/dev/urandom", ios::in|ios::binary);
  urandom.read(reinterpret_cast<char*>(&random_value), sizeof(random_seed));
  gen->readString(Form("Random:seed = %llu", random_value % 900000001));
  // print debug
  // gen->PrintDebug();

  return gen;
}
*/
