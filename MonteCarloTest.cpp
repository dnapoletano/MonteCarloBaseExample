#include <cmath>
#include <vector>

#include "MonteCarloBase.hpp"
#include "Random.hpp"

/**
 * Sistema immaginario di una catena di particelle monodimensionale
 */ 
class MolecularDynamics : public ModelBase
{
private:
  size_t randpos{0};
  double Delta {0.5};
  double EActual{};
  double ETrial{};

  double TestPos{};

  /// Particelle = collezione di posizione e velocita (monodim!)
  std::vector<std::pair<double,double> > Particles;
  Random * _rand;
  double Beta;

public:
  MolecularDynamics(const std::string &name, Observable *oss,
                    const size_t NumberOfParticles,Random *random,
                    const double beta = 1.0)
      : ModelBase(name, oss), _rand{random}, Beta{beta} 
  {
    Particles.resize(NumberOfParticles);
  }

  bool Init() override
  {
    /// metto tutte le particelle equidistintanti
    for(size_t i{0}; i < Particles.size(); ++i){
      Particles[i] = {i,0.0};
    }
    return true;
  }

  int TrialStep() override
  {
    /// seleziona una particella a caso e spostala
    randpos = _rand->randint() % Particles.size();
    double Delta{0.5};
    EActual = pow(Particles[randpos].first - Particles[randpos + 1].first, 2) +
              pow(Particles[randpos - 1].first - Particles[randpos].first, 2);

    TestPos = Particles[randpos].first + Delta;
    ETrial = pow(TestPos - Particles[randpos + 1].first, 2) +
             pow(Particles[randpos - 1].first - TestPos, 2);
    return 1;
  }

  bool AcceptProb() override
  {
    return (*_rand)() < std::min(1.0,exp(-ETrial + EActual) * Beta);
  }

  bool Update() override
  {
    Particles[randpos].first = TestPos;
    return true;
  }

  void SetDelta(const double delta) {Delta = delta;}

  void UpdateObservable() override
  {
    Osservabile->steps += 1;
    double Pressione = ETrial;
    Osservabile->w += Pressione;
    Osservabile->w2 += Pressione * Pressione;
    Osservabile->Value = Osservabile->w / Osservabile->steps;
    Osservabile->ValueErr = sqrt((Osservabile->w2 / Osservabile->steps 
      - Osservabile->Value)/Osservabile->steps);
  }
};

int main(int argc, char ** argv)
{
  std::shared_ptr<Observable> Pressure{new Observable};
  std::shared_ptr<Random> rand{new Random{0}};
  MolecularDynamics MD{"MolecularDynamics", Pressure.get(), 100, rand.get()};
  MonteCarlo MC{&MD, rand.get()};

  MC.Evolve(10000);

  std::cout << "Number of steps: " << Pressure->steps << std::endl;
  std::cout << "Sum of weights : " << Pressure->w << std::endl;
  std::cout << "Sum of weights2: " << Pressure->w2 << std::endl;
  std::cout << "Value          : " << Pressure->Value << " \u00B1 "
            << Pressure->ValueErr << std::endl;
}