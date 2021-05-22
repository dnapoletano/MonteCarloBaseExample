#ifndef MONTECARLO_BASE_HPP
#define MONTECARLO_BASE_HPP

#include <iostream>
#include <memory>
#include <string>

struct Observable {
  size_t steps;
  double w,w2;
  double Value, ValueErr;
};

/** class ModelBase
 * Questa classe definisce le funzioni base che ogni modello
 * deve avere affinché la classe montecarlo possa usarlo.
 */
class ModelBase {
protected:
  std::string Name;
  Observable * Osservabile;
public:
  ModelBase(const std::string& name, Observable * oss) 
  : Name{name}, Osservabile{oss}
  {
    Osservabile->steps = 0;
    Osservabile->w = Osservabile->w2 = Osservabile->Value = Osservabile->ValueErr = 0.0;
  }
  inline std::string GetName() const {return Name;}

  /**
   * Inizializza il modello e ritorna vero
   * se lo ha fatto senza problemi
   */  
  virtual bool Init() = 0;

  /**
   * Genera uno step di prova
   */
  virtual int TrialStep() = 0;

  /**
   * Accept or reject del trial
   */
  virtual bool AcceptProb() = 0;

  /**
   * Accept or reject del trial
   */
  virtual bool Update() = 0;

  /**
   * Update the value of the observable
   */
  virtual void UpdateObservable() = 0;
  inline Observable * GetObservable() const {return Osservabile;}
};

/** class MonteCarlo
 * L'algoritmo di Metropolis-Hastings
 * nella sua versione minima
 */
class MonteCarlo {
private:
  size_t NSteps;
  ModelBase * Model;
  class Random * _rand;
public:
  MonteCarlo(ModelBase * model, Random * random);
  bool Evolve(const size_t NSteps);
};

inline MonteCarlo::MonteCarlo(ModelBase *model, Random * random)
: Model{model}, _rand{random}
{
  Model->Init();
}

inline bool MonteCarlo::Evolve(const size_t NSteps)
{
  bool res {false}; 
  /// do a few cycles just to warm up!
  for(size_t i{0}; i < NSteps/100; ++i){
    Model->TrialStep();
    if(Model->AcceptProb()){
      Model->Update();
    }
  }

  /// real cycle
  for (size_t i{0}; i < NSteps; ++i) {
    Model->TrialStep();
    if (Model->AcceptProb()) {
      Model->Update();
    }
    Model->UpdateObservable();
  }
  return res;
}

#endif