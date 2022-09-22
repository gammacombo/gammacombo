#include "ParametersAbs.h"

Parameter* ParametersAbs::newParameter(TString name)
{
    Parameter *p = new Parameter();
    p->name = name;
    m_parameters.push_back(p);
    return p;
}


Parameter::Range ParametersAbs::range(float min, float max)
{
    Parameter::Range r = {min, max};
    return r;
}


Parameter* ParametersAbs::var(TString name)
{
    for ( int i=0; i<m_parameters.size(); i++ ){
        if ( m_parameters[i]->name == name ) return m_parameters[i];
    }
    cout << "ParametersAbs::var() : ERROR : no such parameter '"+name+"'." << endl;
    return 0;
}

///
/// Ranges:
///  free - here is where Feldman-Cousins-like forbidden regions get implemented
///
RooRealVar* ParametersAbs::get(TString name)
{
    for ( int i=0; i<m_parameters.size(); i++ ){
        if ( m_parameters[i]->name == name ){
            RooRealVar* r = new RooRealVar(m_parameters[i]->name, m_parameters[i]->title, 
                m_parameters[i]->startvalue, m_parameters[i]->unit);
            RooMsgService::instance().setGlobalKillBelow(WARNING); // else we get messages for range creation
            r->setRange("free",  m_parameters[i]->free.min,  m_parameters[i]->free.max);
            r->setRange("phys",  m_parameters[i]->phys.min,  m_parameters[i]->phys.max);
            r->setRange("scan",  m_parameters[i]->scan.min,  m_parameters[i]->scan.max);
            r->setRange("force", m_parameters[i]->force.min, m_parameters[i]->force.max);
            r->setRange("bboos", m_parameters[i]->bboos.min, m_parameters[i]->bboos.max);
            RooMsgService::instance().setGlobalKillBelow(INFO);
            return r;
        }
    }
    cout << "ParametersAbs::get() : ERROR : no such parameter '"+name+"'." << endl;
    return 0;
}
