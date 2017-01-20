/*************************************************************************************

Grid physics library, www.github.com/paboyle/Grid

Source file: ./lib/qcd/modules/ActionModules.h

Copyright (C) 2016

Author: Guido Cossu <guido.cossu@ed.ac.uk>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

See the full license in the file "LICENSE" in the top level distribution
directory
*************************************************************************************/
/*  END LEGAL */
#ifndef ACTION_MODULES_H
#define ACTION_MODULES_H

/*
Define loadable, serializable modules
for the HMC execution
*/

namespace Grid {

//////////////////////////////////////////////
//              Actions
//////////////////////////////////////////////

template <class Product, class R>
class ActionModuleBase: public HMCModuleBase<Product>{
public:
  typedef R Resource;
  virtual void acquireResource(R& ){};

};


template <class ActionType, class APar>
class ActionModule
    : public Parametrized<APar>,
      public ActionModuleBase< QCD::Action<typename ActionType::GaugeField> , QCD::GridModule > {
 public:
  typedef ActionModuleBase< QCD::Action<typename ActionType::GaugeField>, QCD::GridModule > Base;
  typedef typename Base::Product Product;
  typedef APar Parameters;

  std::unique_ptr<ActionType> ActionPtr;

  ActionModule(APar Par) : Parametrized<APar>(Par) {}

  template <class ReaderClass>
  ActionModule(Reader<ReaderClass>& Reader) : Parametrized<APar>(Reader){};


  virtual void print_parameters(){
    Parametrized<APar>::print_parameters();
  }

  Product* getPtr() {
    if (!ActionPtr) initialize();

    return ActionPtr.get();
  }

 private:
  virtual void initialize() = 0;

};

//////////////////////////
// Modules
//////////////////////////

namespace QCD{

class PlaqPlusRectangleGaugeActionParameters : Serializable {
 public:
  GRID_SERIALIZABLE_CLASS_MEMBERS(PlaqPlusRectangleGaugeActionParameters, 
    RealD, c_plaq,
    RealD, c_rect);

};

class RBCGaugeActionParameters : Serializable {
 public:
  GRID_SERIALIZABLE_CLASS_MEMBERS(RBCGaugeActionParameters, 
    RealD, beta,
    RealD, c1);

};

class BetaGaugeActionParameters : Serializable {
 public:
  GRID_SERIALIZABLE_CLASS_MEMBERS(BetaGaugeActionParameters, 
    RealD, beta);
};




template <class Impl >
class WilsonGModule: public ActionModule<WilsonGaugeAction<Impl>, BetaGaugeActionParameters> {
  typedef ActionModule<WilsonGaugeAction<Impl>, BetaGaugeActionParameters> ActionBase;
  using ActionBase::ActionBase; // for constructors

  // acquire resource
  virtual void initialize(){
    this->ActionPtr.reset(new WilsonGaugeAction<Impl>(this->Par_.beta));
  }

};

template <class Impl >
class PlaqPlusRectangleGModule: public ActionModule<PlaqPlusRectangleAction<Impl>, PlaqPlusRectangleGaugeActionParameters> {
  typedef ActionModule<PlaqPlusRectangleAction<Impl>, PlaqPlusRectangleGaugeActionParameters> ActionBase;
  using ActionBase::ActionBase; // for constructors

  // acquire resource
  virtual void initialize(){
    this->ActionPtr.reset(new PlaqPlusRectangleAction<Impl>(this->Par_.c_plaq, this->Par_.c_rect));
  }

};

template <class Impl >
class RBCGModule: public ActionModule<RBCGaugeAction<Impl>, RBCGaugeActionParameters> {
  typedef ActionModule<RBCGaugeAction<Impl>, RBCGaugeActionParameters> ActionBase;
  using ActionBase::ActionBase; // for constructors

  // acquire resource
  virtual void initialize(){
    this->ActionPtr.reset(new RBCGaugeAction<Impl>(this->Par_.beta, this->Par_.c1));
  }

};




template <class Impl >
class SymanzikGModule: public ActionModule<SymanzikGaugeAction<Impl>, BetaGaugeActionParameters> {
  typedef ActionModule<SymanzikGaugeAction<Impl>, BetaGaugeActionParameters> ActionBase;
  using ActionBase::ActionBase; // for constructors

  // acquire resource
  virtual void initialize(){
    this->ActionPtr.reset(new SymanzikGaugeAction<Impl>(this->Par_.beta));
  }

};

template <class Impl >
class IwasakiGModule: public ActionModule<IwasakiGaugeAction<Impl>, BetaGaugeActionParameters> {
  typedef ActionModule<IwasakiGaugeAction<Impl>, BetaGaugeActionParameters> ActionBase;
  using ActionBase::ActionBase; // for constructors

  // acquire resource
  virtual void initialize(){
    this->ActionPtr.reset(new IwasakiGaugeAction<Impl>(this->Par_.beta));
  }

};


template <class Impl >
class DBW2GModule: public ActionModule<DBW2GaugeAction<Impl>, BetaGaugeActionParameters> {
  typedef ActionModule<DBW2GaugeAction<Impl>, BetaGaugeActionParameters> ActionBase;
  using ActionBase::ActionBase; // for constructors

  // acquire resource
  virtual void initialize(){
    this->ActionPtr.reset(new DBW2GaugeAction<Impl>(this->Par_.beta));
  }

};

/////////////////////////////////////////
// Fermion Actions
/////////////////////////////////////////


template <class Impl, template <typename> class FermionA >
class PseudoFermionModuleBase: public ActionModule<FermionA<Impl>, NoParameters> {
protected:
  typedef ActionModule<FermionA<Impl>, NoParameters> ActionBase;
  using ActionBase::ActionBase; // for constructors

  typedef std::unique_ptr<FermionOperatorModuleBase<FermionOperator<Impl>> > operator_type;
  typedef std::unique_ptr<HMCModuleBase<OperatorFunction<typename Impl::FermionField> > > solver_type;

  template <class ReaderClass>
  void getFermionOperator(Reader<ReaderClass>& Reader, operator_type &fo, std::string section_name){
    auto &FOFactory = HMC_FermionOperatorModuleFactory<fermionop_string, Impl, ReaderClass>::getInstance();
    Reader.push(section_name);
    std::string op_name;
    read(Reader,"name", op_name);
    fo = FOFactory.create(op_name, Reader);
    Reader.pop();  
  }

  template <class ReaderClass>
  void getSolverOperator(Reader<ReaderClass>& Reader, solver_type &so, std::string section_name){
    auto& SolverFactory = HMC_SolverModuleFactory<solver_string, typename Impl::FermionField, ReaderClass>::getInstance();
    Reader.push(section_name);
    std::string solv_name;
    read(Reader,"name", solv_name);
    so = SolverFactory.create(solv_name, Reader);
    Reader.pop();    
  }
};


template <class Impl >
class TwoFlavourFModule: public PseudoFermionModuleBase<Impl, TwoFlavourPseudoFermionAction>{
  typedef PseudoFermionModuleBase<Impl, TwoFlavourPseudoFermionAction> Base;
  using Base::Base;

  typename Base::operator_type fop_mod;
  typename Base::solver_type   solver_mod;

 public:
  virtual void acquireResource(typename Base::Resource& GridMod){
    fop_mod->AddGridPair(GridMod);
  }

   // constructor
   template <class ReaderClass>
   TwoFlavourFModule(Reader<ReaderClass>& R): PseudoFermionModuleBase<Impl, TwoFlavourPseudoFermionAction>(R) {
    this->getSolverOperator(R, solver_mod, "Solver");
    this->getFermionOperator(R, fop_mod, "Operator");
    R.pop();
   } 

  // acquire resource
  virtual void initialize() {
    // here temporarily assuming that the force and action solver are the same
    this->ActionPtr.reset(new TwoFlavourPseudoFermionAction<Impl>(*(this->fop_mod->getPtr()), *(this->solver_mod->getPtr()), *(this->solver_mod->getPtr())));
  }

};

// very similar, I could have templated this but it is overkilling
template <class Impl >
class TwoFlavourEOFModule: public PseudoFermionModuleBase<Impl, TwoFlavourEvenOddPseudoFermionAction>{
  typedef PseudoFermionModuleBase<Impl, TwoFlavourEvenOddPseudoFermionAction> Base;
  using Base::Base;

  typename Base::operator_type fop_mod;
  typename Base::solver_type   solver_mod;

 public:
  virtual void acquireResource(typename Base::Resource& GridMod){
    fop_mod->AddGridPair(GridMod);
  }

   // constructor
   template <class ReaderClass>
   TwoFlavourEOFModule(Reader<ReaderClass>& R): PseudoFermionModuleBase<Impl, TwoFlavourEvenOddPseudoFermionAction>(R) {
    this->getSolverOperator(R, solver_mod, "Solver");
    this->getFermionOperator(R, fop_mod, "Operator");
    R.pop();
   } 

  // acquire resource
  virtual void initialize() {
    // here temporarily assuming that the force and action solver are the same
    this->ActionPtr.reset(new TwoFlavourEvenOddPseudoFermionAction<Impl>(*(this->fop_mod->getPtr()), *(this->solver_mod->getPtr()), *(this->solver_mod->getPtr())));
  }

};





typedef WilsonGModule<PeriodicGimplR> WilsonGMod;
typedef SymanzikGModule<PeriodicGimplR> SymanzikGMod;
typedef IwasakiGModule<PeriodicGimplR> IwasakiGMod;
typedef DBW2GModule<PeriodicGimplR> DBW2GMod;
typedef RBCGModule<PeriodicGimplR> RBCGMod;
typedef PlaqPlusRectangleGModule<PeriodicGimplR> PlaqPlusRectangleGMod;

}// QCD temporarily here







////////////////////////////////////////
// Factories specialisations
////////////////////////////////////////



// use the same classed defined by Antonin, does not make sense to rewrite
// Factory is perfectly fine
// Registar must be changed because I do not want to use the ModuleFactory

// explicit ref to LatticeGaugeField must be changed or put in the factory
typedef ActionModuleBase< QCD::Action< QCD::LatticeGaugeField >, QCD::GridModule > HMC_LGTActionModBase;

template <char const *str, class ReaderClass >
class HMC_LGTActionModuleFactory
    : public Factory < HMC_LGTActionModBase , Reader<ReaderClass> > {
 public:
  typedef Reader<ReaderClass> TheReader; 
  // use SINGLETON FUNCTOR MACRO HERE
  HMC_LGTActionModuleFactory(const HMC_LGTActionModuleFactory& e) = delete;
  void operator=(const HMC_LGTActionModuleFactory& e) = delete;
  static HMC_LGTActionModuleFactory& getInstance(void) {
    static HMC_LGTActionModuleFactory e;
    return e;
  }

 private:
  HMC_LGTActionModuleFactory(void) = default;
    std::string obj_type() const {
        return std::string(str);
  }
};

extern char gauge_string[];
static Registrar<QCD::WilsonGMod,            HMC_LGTActionModuleFactory<gauge_string, XmlReader> > __WGmodXMLInit("Wilson"); 
static Registrar<QCD::SymanzikGMod,          HMC_LGTActionModuleFactory<gauge_string, XmlReader> > __SymGmodXMLInit("Symanzik"); 
static Registrar<QCD::IwasakiGMod,           HMC_LGTActionModuleFactory<gauge_string, XmlReader> > __IwGmodXMLInit("Iwasaki"); 
static Registrar<QCD::DBW2GMod,              HMC_LGTActionModuleFactory<gauge_string, XmlReader> > __DBW2GmodXMLInit("DBW2"); 
static Registrar<QCD::RBCGMod,               HMC_LGTActionModuleFactory<gauge_string, XmlReader> > __RBCGmodXMLInit("RBC"); 
static Registrar<QCD::PlaqPlusRectangleGMod, HMC_LGTActionModuleFactory<gauge_string, XmlReader> > __PPRectGmodXMLInit("PlaqPlusRect"); 


// FIXME more general implementation
static Registrar<QCD::TwoFlavourFModule<QCD::WilsonImplR> , HMC_LGTActionModuleFactory<gauge_string, XmlReader> > __TwoFlavourFmodXMLInit("TwoFlavours"); 
static Registrar<QCD::TwoFlavourEOFModule<QCD::WilsonImplR> , HMC_LGTActionModuleFactory<gauge_string, XmlReader> > __TwoFlavourEOFmodXMLInit("TwoFlavoursEvenOdd"); 



// add here the registration for other implementations and readers


} // Grid


#endif //HMC_MODULES_H