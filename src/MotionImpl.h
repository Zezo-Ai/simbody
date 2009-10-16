#ifndef SimTK_SIMBODY_MOTION_IMPL_H_
#define SimTK_SIMBODY_MOTION_IMPL_H_
/* -------------------------------------------------------------------------- *
 *                      SimTK Core: SimTK Simbody(tm)                         *
 * -------------------------------------------------------------------------- *
 * This is part of the SimTK Core biosimulation toolkit originating from      *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2009 Stanford University and the Authors.           *
 * Authors: Michael Sherman                                                   *
 * Contributors:                                                              *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,    *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 * -------------------------------------------------------------------------- */

#include "SimTKcommon.h"
#include "simbody/internal/common.h"
#include "simbody/internal/Motion.h"

#include "SimbodyTreeState.h"

namespace SimTK {

class MobilizedBodyImpl;

class MotionImpl : public PIMPLImplementation<Motion, MotionImpl> {
public:
	MotionImpl() : mobodImpl(0) {}

    // Base class copy constructor just clears out the mobilized body
    // reference because there can be only one Motion associated with a
    // particular mobilized body. Presumably this one is being copied so
    // it can be duplicated on another mobilized body.
    MotionImpl(const MotionImpl& src) : mobodImpl(0) {}

    bool hasMobilizedBody() const {return mobodImpl != 0;}
	const MobilizedBodyImpl& 
         getMobilizedBodyImpl() const {assert(mobodImpl); return *mobodImpl;}
    MobilizedBodyIndex getMobilizedBodyIndex() const;

	void setMobilizedBodyImpl(MobilizedBodyImpl* mbi) {assert(!mobodImpl); mobodImpl = mbi;}
	void invalidateTopologyCache() const;

    const SimbodyMatterSubsystem& getMatterSubsystem() const; 

    template <class T>
    const T& getVar(const State& s, DiscreteVariableIndex vx) const {
        return Value<T>::downcast(getMatterSubsystem().getDiscreteVariable(s, vx));
    }

    template <class T>
    T& updVar(State& s, DiscreteVariableIndex vx) const {
        return Value<T>::updDowncast(getMatterSubsystem().updDiscreteVariable(s, vx));
    }

    template <class T>
    DiscreteVariableIndex allocVar(State& state, const T& initVal, 
                                   const Stage& stage=Stage::Instance) const {
        return getMatterSubsystem().allocateDiscreteVariable
                                        (state, stage, new Value<T>(initVal)); 
    }

    virtual ~MotionImpl() {}
    virtual MotionImpl* clone() const = 0;

    // This reports whether this Motion is holonomic (Level::Position), nonholonomic
    // (Level::Velocity), or acceleration (Level::Acceleration).
    Motion::Level getLevel(const State& s) const 
    {   return getLevelVirtual(s); }
    virtual Motion::Level getLevelVirtual(const State&) const = 0;

    Motion::Method getLevelMethod(const State& s) const 
    {   return getLevelMethodVirtual(s); }
    virtual Motion::Method getLevelMethodVirtual(const State&) const 
    {   return Motion::Prescribed; }

    // These operators calculate prescribed positions, velocities, or accelerations
    // given a State realized to the previous Stage.
    void calcPrescribedPosition      (const State& s, int nq, Real* q)       const
    {   calcPrescribedPositionVirtual(s,nq,q); }
    void calcPrescribedPositionDot   (const State& s, int nq, Real* qdot)    const
    {   calcPrescribedPositionVirtual(s,nq,qdot); }
    void calcPrescribedPositionDotDot(const State& s, int nq, Real* qdotdot) const
    {   calcPrescribedPositionVirtual(s,nq,qdotdot); }
    void calcPrescribedVelocity      (const State& s, int nu, Real* u)       const
    {   calcPrescribedPositionVirtual(s,nu,u); }
    void calcPrescribedVelocityDot   (const State& s, int nu, Real* udot)    const
    {   calcPrescribedPositionVirtual(s,nu,udot); }
    void calcPrescribedAcceleration  (const State& s, int nu, Real* udot)    const
    {   calcPrescribedPositionVirtual(s,nu,udot); }

    void realizeTopology(State& state) const
    {   realizeTopologyVirtual(state); }
    void realizeModel(State& state) const 
    {   realizeModelVirtual(state); }
    void realizeInstance(const State& state) const
    {   realizeInstanceVirtual(state); }
    void realizeTime(const State& state) const 
    {   realizeTimeVirtual(state); }
    void realizePosition(const State& state) const 
    {   realizePositionVirtual(state); }
    void realizeVelocity(const State& state) const 
    {   realizeVelocityVirtual(state); }
    void realizeDynamics(const State& state) const 
    {   realizeDynamicsVirtual(state); }
    void realizeAcceleration(const State& state) const 
    {   realizeAccelerationVirtual(state); }
    void realizeReport(const State& state) const 
    {   realizeReportVirtual(state); }

    virtual void calcPrescribedPositionVirtual      (const State&, int nq, Real* q)       const;
    virtual void calcPrescribedPositionDotVirtual   (const State&, int nq, Real* qdot)    const;
    virtual void calcPrescribedPositionDotDotVirtual(const State&, int nq, Real* qdotdot) const;
    virtual void calcPrescribedVelocityVirtual      (const State&, int nu, Real* u)       const;
    virtual void calcPrescribedVelocityDotVirtual   (const State&, int nu, Real* udot)    const;
    virtual void calcPrescribedAccelerationVirtual  (const State&, int nu, Real* udot)    const;

    virtual void realizeTopologyVirtual    (State&)       const {}
    virtual void realizeModelVirtual       (State&)       const {}
    virtual void realizeInstanceVirtual    (const State&) const {}
    virtual void realizeTimeVirtual        (const State&) const {}
    virtual void realizePositionVirtual    (const State&) const {}
    virtual void realizeVelocityVirtual    (const State&) const {}
    virtual void realizeDynamicsVirtual    (const State&) const {}
    virtual void realizeAccelerationVirtual(const State&) const {}
    virtual void realizeReportVirtual      (const State&) const {}
private:
    MobilizedBodyImpl* mobodImpl;	// just a reference; don't delete on destruction
};

class Motion::SteadyImpl : public MotionImpl {
public:
    // no default constructor
    explicit SteadyImpl(const Vec6& u) : defaultU(u) {}

    SteadyImpl* clone() const { 
        SteadyImpl* copy = new SteadyImpl(*this);
        copy->currentU.invalidate(); // no sharing state variables
        return copy; 
    }

    void setDefaultRates(const Vec6& u) {
        invalidateTopologyCache();
        defaultU = u;
    }
    void setOneDefaultRate(UIndex ux, Real u) {
        invalidateTopologyCache();
        defaultU[ux] = u;
    }

    const Vec6& getDefaultRates() const {return defaultU;}

    void setRates(State& s, const Vec6& u) const {
        updVar<Vec6>(s, currentU) = u;
    }
    void setOneRate(State& s, UIndex i, Real u) const {
        updVar<Vec6>(s, currentU)[i] = u;
    }

    Motion::Level  getLevelVirtual (const State&) const {return Motion::Velocity;}
    Motion::Method getLevelMethodVirtual(const State&) const {return Motion::Prescribed;}

    // Allocate a discrete variable to hold the constant rates.
    void realizeTopologyVirtual(State& state) const {
        // This is in the Topology-stage "cache" so we can write to it,
        // but only here.
        const_cast<DiscreteVariableIndex&>(currentU) = 
            allocVar(state, defaultU);
    }

    // We're done at Instance stage: set the prescribed udot's to zero and the
    // prescribed u's to the value currently in our discrete state variable.
    void realizeInstanceVirtual(const State& state, 
        int nu, Real* udot, Real* u, // udot, u set here
        int nq, Real* q) const       // q's are ignored
    {
        assert(0 <= nu && nu <= 6);
        assert(nu==0 || (u && udot));
        const Vec6& uval = getVar<Vec6>(state, currentU);
        for (int i=0; i<nu; ++i) { 
            u[i]    = uval[i];
            udot[i] = 0;
        }
    }


private:
        // TOPOLOGY "STATE"
    Vec6                  defaultU;

        // TOPOLOGY "CACHE"
    DiscreteVariableIndex currentU;
};

class Motion::CustomImpl : public MotionImpl {
public:
    // Take over ownership of the supplied heap-allocated object.
    explicit CustomImpl(Motion::Custom::Implementation* implementation);

    CustomImpl(const CustomImpl& src) : implementation(0) {
        if (src.implementation) 
            implementation = src.implementation->clone();
    }

    CustomImpl* clone() const { return new CustomImpl(*this); }

    ~CustomImpl() {
        delete implementation;
    }

    Motion::Level getLevelVirtual(const State& s) const {
        return getImplementation().getLevel(s);
    }
    Motion::Method getLevelMethodVirtual(const State& s) const {
        return getImplementation().getLevelMethod(s);
    }

    const Motion::Custom::Implementation& getImplementation() const {
        assert(implementation); return *implementation;
    }
    Motion::Custom::Implementation& updImplementation() {
        assert(implementation); return *implementation;
    }

    void calcPrescribedPositionVirtual(const State& s, int nq, Real* q) const
    {  getImplementation().calcPrescribedPosition(s,nq,q); }
    void calcPrescribedPositionDotVirtual(const State& s, int nq, Real* qdot) const
    {  getImplementation().calcPrescribedPositionDot(s,nq,qdot); }
    void calcPrescribedPositionDotDotVirtual(const State& s, int nq, Real* qdotdot) const
    {  getImplementation().calcPrescribedPositionDotDot(s,nq,qdotdot); }

    void calcPrescribedVelocityVirtual(const State& s, int nu, Real* u) const
    {  getImplementation().calcPrescribedVelocity(s,nu,u); }
    void calcPrescribedVelocityDotVirtual(const State& s, int nu, Real* udot) const
    {  getImplementation().calcPrescribedVelocityDot(s,nu,udot); }

    void calcPrescribedAccelerationVirtual(const State& s, int nu, Real* udot) const 
    {  getImplementation().calcPrescribedAcceleration(s,nu,udot); }

    void realizeTopology(State& state) const {
        getImplementation().realizeTopology(state);
    }
    void realizeModel(State& state) const {
        getImplementation().realizeModel(state);
    }
    void realizeInstance(const State& state) const {
        getImplementation().realizeInstance(state);
    }
    void realizeTime(const State& state) const {
        getImplementation().realizeTime(state);
    }
    void realizePosition(const State& state) const {
        getImplementation().realizePosition(state);
    }
    void realizeVelocity(const State& state) const {
        getImplementation().realizeVelocity(state);
    }
    void realizeDynamics(const State& state) const {
        getImplementation().realizeDynamics(state);
    }
    void realizeAcceleration(const State& state) const {
        getImplementation().realizeAcceleration(state);
    }
    void realizeReport(const State& state) const {
        getImplementation().realizeReport(state);
    }
private:
    Motion::Custom::Implementation* implementation;
};

} // namespace SimTK

#endif // SimTK_SIMBODY_MOTION_IMPL_H_