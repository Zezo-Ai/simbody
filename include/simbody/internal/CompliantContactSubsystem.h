#ifndef SimTK_SIMBODY_COMPLIANT_CONTACT_SUBSYSTEM_H_
#define SimTK_SIMBODY_COMPLIANT_CONTACT_SUBSYSTEM_H_

/* -------------------------------------------------------------------------- *
 *                      SimTK Core: SimTK Simbody(tm)                         *
 * -------------------------------------------------------------------------- *
 * This is part of the SimTK Core biosimulation toolkit originating from      *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2008-10 Stanford University and the Authors.        *
 * Authors: Peter Eastman, Michael Sherman                                    *
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

#include "simbody/internal/common.h"
#include "simbody/internal/ForceSubsystem.h"
#include "simbody/internal/Contact.h"

#include <cassert>

namespace SimTK {

class MultibodySystem;
class SimbodyMatterSubsystem;
class ContactTrackerSubsystem;
class ContactForceGenerator;
class Contact;
class ContactForce;
class ContactPatch;



//==============================================================================
//                        COMPLIANT CONTACT SUBSYSTEM
//==============================================================================
/** This is a force subsystem that implements a compliant contact model to
respond to Contact objects as detected by a ContactTrackerSubsystem. The
subsystem contains an extendable collection of ContactForceGenerator 
objects, one per type of Contact. For example, a point contact would be
handled by a different force generator than would a mesh contact. **/
class SimTK_SIMBODY_EXPORT CompliantContactSubsystem : public ForceSubsystem {
public:
/** Default constructor creates an empty handle. **/
CompliantContactSubsystem() {}

/** Add a new CompliantContactSubsystem to the indicated MultibodySystem,
specifying the ContactTrackerSubsystem that we will use to obtain the list
of ActiveContacts from which we will generate compliant forces. The
MultibodySystem takes over ownership of the new subsystem object, but the
handle we construct here retains a reference to it. **/
CompliantContactSubsystem(MultibodySystem&, 
                          const ContactTrackerSubsystem&);

/** Get the transition velocity (vt) of the friction model. **/
Real getTransitionVelocity() const;
/** Set the transition velocity (vt) of the friction model. **/
void setTransitionVelocity(Real vt);

/** Determine how many of the reported Contacts are currently generating
contact forces. **/
int getNumContactForces(const State& state) const;
/** For each active Contact, get a reference to the most recently calculated
force there; the ContactId that produced this force is returned also. You
can call this at Velocity stage or later. **/
const ContactForce& getContactForce(int n) const;

/** Obtain the total amount of energy dissipated by all the contact responses
that were mediated by this subsystem since some arbitrary starting point. This 
is the time integral of all the power dissipated during any of the contacts
by material dissipative and friction forces. For a system whose only 
non-conservative forces are contacts, the sum of potential, kinetic, and 
dissipated energies should be conserved with an exception noted below. This is 
particularly useful for debugging new ContactForceGenerators. This is a 
State variable so you can obtain its value any time after it is allocated.
@pre \a state realized to Stage::Model
@param[in]          state    
    The State from which to obtain the current value of the dissipated energy.
@return
    The total dissipated energy (a nonnegative scalar). 

@note The Hunt and Crossley dissipation model used by many contact force
generators can occasionally detect that a body is being "yanked" out of 
a contact (this is more likely with very large dissipation coefficients). 
To track all the energy in that case we would have to allow the
surfaces to stick together, which is unreasonable. Instead, some of the 
energy will be lost to unmodeled effects because the surface is unable to
transfer energy back to the bodies and will instead vibrate or ring until
the energy is dissipated. **/
Real getDissipatedEnergy(const State& state) const;

/** Set the accumulated dissipated energy to an arbitrary value. Typically
this is used only to reset the dissipated energy to zero, but non-zero
values can be useful if you are trying to match some existing data or
continuing a simulation. This is a State variable so you can set its 
value any time after it is allocated.
@pre \a state realized to Stage::Model
@param[in,out]      state    
    The State whose dissipated energy variable for this subsystem is to
    be modified.
@param[in]          energy   
    The new value for the accumulated dissipated energy (must be a 
    nonnegative scalar). **/
void setDissipatedEnergy(State& state, Real energy) const;

/** Calculate detailed information about the current set of active contact
patches, including deformed geometric information, pressure and friction
force distribution, and resultant forces and moments. This detailed 
information is only calculated when requested because it may be expensive
for some ContactForceGenerators; for simulation purposes only the resultants 
are needed. The input to this calculation is the current set of 
ActiveContacts as obtained for this State from the associated 
ContactTrackerSubsystem. You can call this at Velocity stage or higher. Each 
contact patch is the result of a single Contact, but not all active Contacts 
will produce patches. **/
void calcContactPatchDetails(const State& state, 
                             Array_<ContactPatch>& patches) const;

/** Attach a new generator to this subsystem as the responder to be used when
we see the kind of Contact type for which this generator is defined,
replacing the previous generator for this Contact type if there was one. The 
subsystem takes over ownership of the generator; don't delete it yourself. **/
void adoptForceGenerator(ContactForceGenerator* generator);

/** Attach a new generator to this subsystem as the responder to be used when
we see a Contact type for which no suitable generator has been defined,
replacing the previous default generator type if there was one. The 
subsystem takes over ownership of the generator; don't delete it yourself. **/
void adoptDefaultForceGenerator(ContactForceGenerator* generator);

/** Return true if this subsystem has a force generator registered that can
respond to this kind of Contact. **/
bool hasForceGenerator(ContactTypeId contact) const;

/** Return true if this subsystem has a force generator registered that can
be used for response to an unrecognized type of Contact (typically this will
be a "do nothing" or "throw an error" generator. **/
bool hasDefaultForceGenerator() const;

/** Return the force generator to be used for a Contact of the indicated
type. If no generator was registered for this type of contact, this will
be the default generator. **/
const ContactForceGenerator& 
    getContactForceGenerator(ContactTypeId contact) const; 

/** Return the force generator to be used for a Contact type for which no
suitable force generator has been registered. **/
const ContactForceGenerator& getDefaultForceGenerator() const; 

/** Get a read-only reference to the ContactTrackerSubsystem associated
with this CompliantContactSubsystem. This is the contact tracker that is
maintaining the list of contacts for which this subsystem will be providing
the response forces. **/
const ContactTrackerSubsystem& getContactTrackerSubsystem() const;

/** Every Subsystem is owned by a System; a CompliantContactSubsystem expects
to be owned by a MultibodySystem. This method returns a const reference
to the containing MultibodySystem and will throw an exception if there is
no containing System or it is not a MultibodySystem. **/
const MultibodySystem& getMultibodySystem() const;

/** @cond **/   // don't show in Doxygen docs
SimTK_PIMPL_DOWNCAST(CompliantContactSubsystem, ForceSubsystem);
/** @endcond **/

//--------------------------------------------------------------------------
                                 private:
class CompliantContactSubsystemImpl& updImpl();
const CompliantContactSubsystemImpl& getImpl() const;
};



//==============================================================================
//                               CONTACT FORCE
//==============================================================================
/** This is a simple class containing the basic force information for a 
single contact point between deformable surfaces S1 and S2 mounted on rigid
bodies B1 and B2. Every contact interaction between two rigid bodies, however 
complex, can be expressed as a resultant that can be contained in this class 
and is sufficient for advancing a simulation. Optionally, you may be able to 
get more details about the deformed geometry and pressure distribution over the
patch but you have to ask for that separately because it can be expensive.

The information stored here is:
  - A point in space at which equal and opposite forces will be applied to
    corresponding stations of the two interacting rigid bodies. This is called
    the "center of pressure".
  - The force vector to be applied there (to each body with opposite sign).
  - A moment vector to be applied to both bodies (with opposite signs).
  - The potential energy currently stored by the elasticity of the contacting
    materials.
  - The instantaneous power dissipation due to inelastic behavior such as 
    friction and internal material damping.

<h3>Definition of center of pressure</h3>

When the contact patch itself involves many distributed contact points, the
center of pressure might not be a particularly meaningful quantity but 
serves to provide enough information to proceed with a simulation, and to
display resultant contact forces, without requiring detailed patch geometry
information. We define the location r_c of the center of pressure like this:
@verbatim
           sum_i (r_i * |r_i X Fn_i|) 
     r_c = --------------------
              sum_i |r_i X Fn_i|
@endverbatim
where r_i is the vector locating contact point i, F_i=Fn_i+Ft_i is the 
contact force at point i resolved into locally-normal and locally-tangential
components, and M_i is a pure moment if any generated by the contact force 
model as a result of the i'th contact point. Note that the locally-
tangent contact force Ft_i (presumably from friction) and pure moment M_i
do not contribute to the center of pressure calculation; we're choosing
the point that minimizes the net moment generated by normal ("pressure")
forces only. Note that the normal force Fn_i includes both stiffness and
dissipation contributions, so the center of pressure can be 
velocity-dependent. **/
class ContactForce {
public:
ContactForce() {} // invalid
void clear() {m_contactId.invalidate();}
bool isValid() const {return m_contactId.isValid();}
ContactId       m_contactId;            // Which Contact produced this force?
Vec3            m_centerOfPressureInG;
SpatialVec      m_forceOnSurface2InG;   // at COP; negate for Surface 1
Real            m_potentialEnergy;      // > 0 when due to compression
Real            m_power;                // > 0 means dissipation
};

inline std::ostream& operator<<(std::ostream& o, const ContactForce& f) {
    o << "ContactForce for ContactId " << f.m_contactId << " (ground frame):\n";
    o << "  ctr of pressure=" << f.m_centerOfPressureInG << "\n";
    o << "  force on surf2 =" << f.m_forceOnSurface2InG << "\n";
    o << "  pot. energy=" << f.m_potentialEnergy << "  power=" << f.m_power;
    return o << "\n";
}

//==============================================================================
//                              CONTACT DETAIL
//==============================================================================
/** This provides geometric and force details for one element of a contact
patch. **/
class ContactDetail {
public:
// This is the element local frame. The force is applied at this point; the
// local normal is z pointing away from Surface 1 and towards Surface 2, 
// x is the long and y the short subpatch direction if that makes sense for 
// this kind of contact, otherwise they are an arbitrary frame for the contact 
// tangent plane.
unsigned        m_elementId;
Transform       m_patchFrame;           // X_GP
Vec2            m_patchHalfDimensions;  // x >= y >= 0
Vec2            m_deformations;         // surf1, surf2 along -z,z at OP, > 0
Vec2            m_deformationRates;     // > 0 means compressing
SpatialVec      m_forceOnSurface2InP;   // applied at OP
Real            m_potentialEnergy;      // > 0 when due to compression
Real            m_power;                // > 0 means dissipation
};



//==============================================================================
//                               CONTACT PATCH
//==============================================================================
/** A ContactPatch is the description of the forces and the deformed shape of
the contact surfaces that result from compliant contact interactions. This
should not be confused with the Contact object that describes only the 
overlap in \e undeformed surface geometry and knows nothing of forces.
Although there are several qualitatively different kinds of compliant contact
models, we assume that each can be described by some number of contact
"elements" and report the detailed results in terms of those elements. A 
Hertz contact will have only a single element while an elastic foundation
contact will have many. Only the elments that are currently participating in
contact will have entries here; the element id is stored with each
piece of ContactDetail information. There is also some basic information 
needed to advance the simulation and that is common to all contact patch types
and stored as a ContactForce resultant. **/
class SimTK_SIMBODY_EXPORT ContactPatch {
public:
ContactForce            m_resultant;
Array_<ContactDetail>   m_elements;
};



//==============================================================================
//                          CONTACT FORCE GENERATOR
//==============================================================================
/** A ContactForceGenerator implements an algorithm for responding to overlaps 
or potential overlaps between pairs of ContactSurface objects, as detected by
a ContactTrackerSubsystem. This class is used internally by 
CompliantContactSubsystem and there usually is no reason to access it directly.
The exception is if you are defining a new Contact subclass (very rare). In 
that case, you will also need to define one or more ContactForceGenerators to 
respond to Contacts with the new type, then register it with the 
CompliantContactSubsystem. **/
class SimTK_SIMBODY_EXPORT ContactForceGenerator {
public:
class ElasticFoundation;        // for TriangleMeshContact
class HertzCircular;            // for PointContact
class HertzElliptical;          // for EllipticalPointContact

// These are for response to unknown ContactTypeIds.
class DoNothing;     // do nothing if called
class ThrowError;    // throw an error if called

/** Base class constructor for use by the concrete classes. **/
explicit ContactForceGenerator(ContactTypeId type): m_contactType(type) {}

/** Return the ContactTypeId handled by this force generator. ContactTypeId(0)
is reserved and is used here for fallback force generators that deal with
unrecognized ContactTypeIds. **/
ContactTypeId getContactTypeId() const {return m_contactType;}

const CompliantContactSubsystem& getCompliantContactSubsystem() const
{   assert(m_compliantContactSubsys); return *m_compliantContactSubsys; }
void setCompliantContactSubsystem(const CompliantContactSubsystem* sub)
{   m_compliantContactSubsys = sub; }

/** Base class destructor is virtual but does nothing. **/
virtual ~ContactForceGenerator() {}

/** The CompliantContactSubsystem will invoke this method on any 
active contact pair of the right Contact type for which there is overlapping 
undeformed geometry. The force generator is expected to calculate a point
in space where equal and opposite contact forces should be applied to the two
contacting rigid bodies, the potential energy currently stored in this 
contact, and the power (energy dissipation rate). State should be used
for instance info only; use position information from \a overlapping and
velocity information from the supplied arguments. That allows this method
to be used as an operator, for example to calculate potential energy when
velocities are not yet available. **/
virtual void calcContactForce
   (const State&            state,
    const Contact&          overlapping,
    const SpatialVec&       V_GS1,  // surface velocities
    const SpatialVec&       V_GS2,
    ContactForce&           contactForce) const = 0;

/** The CompliantContactSubsystem will invoke this method in response to a user
request for contact patch information; this returns force, potential energy, 
and power as above but may also require expensive computations that can be 
avoided in calcContactForce(). Don't use the state for position or
velocity information; the only allowed positions are in the Contact object
and the velocities are supplied explicitly. **/
virtual void calcContactPatch
   (const State&      state,
    const Contact&    overlapping,
    const SpatialVec& V_GS1,  // surface velocities
    const SpatialVec& V_GS2,
    ContactPatch&     patch) const = 0;


//--------------------------------------------------------------------------
private:
    // This generator should be called only for Contact objects of the 
    // indicated type id.
    ContactTypeId                       m_contactType;
    // This is a reference to the owning CompliantContactSubsystem if any;
    // don't delete on destruction.
    const CompliantContactSubsystem*    m_compliantContactSubsys;
};




//==============================================================================
//                         HERTZ CIRCULAR GENERATOR
//==============================================================================

/** This ContactForceGenerator handles contact between non-conforming
objects that meet at a point and generate a circular contact patch; those
generate a PointContact tracking object. **/
class SimTK_SIMBODY_EXPORT ContactForceGenerator::HertzCircular 
:   public ContactForceGenerator {
public:
HertzCircular() 
:   ContactForceGenerator(CircularPointContact::classTypeId()) {}

virtual ~HertzCircular() {}
virtual void calcContactForce
   (const State&            state,
    const Contact&          overlapping,
    const SpatialVec&       V_GS1,  // surface velocities
    const SpatialVec&       V_GS2,
    ContactForce&           contactForce) const;

virtual void calcContactPatch
   (const State&      state,
    const Contact&    overlapping,
    const SpatialVec& V_GS1,  // surface velocities
    const SpatialVec& V_GS2,
    ContactPatch&     patch) const;
};




//==============================================================================
//                       ELASTIC FOUNDATION GENERATOR
//==============================================================================
/** This ContactForceGenerator handles contact between a TriangleMesh
and a variety of other geometric objects, all of which produce a
TriangleMeshContact tracking object. **/
class SimTK_SIMBODY_EXPORT ContactForceGenerator::ElasticFoundation 
:   public ContactForceGenerator {
public:
ElasticFoundation() 
:   ContactForceGenerator(TriangleMeshContact::classTypeId()) {}
virtual ~ElasticFoundation() {}
virtual void calcContactForce
   (const State&            state,
    const Contact&          overlapping,
    const SpatialVec&       V_GS1,  // surface velocities
    const SpatialVec&       V_GS2,
    ContactForce&           contactForce) const
{   SimTK_ASSERT_ALWAYS(!"implemented",
        "ContactForceGenerator::ElasticFoundation::calcContactForce() not implemented yet."); }
virtual void calcContactPatch
   (const State&      state,
    const Contact&    overlapping,
    const SpatialVec& V_GS1,  // surface velocities
    const SpatialVec& V_GS2,
    ContactPatch&     patch) const
{   SimTK_ASSERT_ALWAYS(!"implemented",
        "ContactForceGenerator::ElasticFoundation::calcContactPatch() not implemented yet."); }

};




//==============================================================================
//                        DO NOTHING FORCE GENERATOR
//==============================================================================
/** This ContactForceGenerator does nothing silently. It can be used as a way
to explicitly ignore a certain ContactTypeId, or more commonly it is used as
the fallback generator for unrecognized ContactTypeIds. **/
class SimTK_SIMBODY_EXPORT ContactForceGenerator::DoNothing 
:   public ContactForceGenerator {
public:
explicit DoNothing(ContactTypeId type = ContactTypeId(0)) 
:   ContactForceGenerator(type) {}
virtual ~DoNothing() {}
virtual void calcContactForce
   (const State&            state,
    const Contact&          overlapping,
    const SpatialVec&       V_GS1,  // surface velocities
    const SpatialVec&       V_GS2,
    ContactForce&           contactForce) const
{   SimTK_ASSERT_ALWAYS(!"implemented",
        "ContactForceGenerator::DoNothing::calcContactForce() not implemented yet."); }
virtual void calcContactPatch
   (const State&      state,
    const Contact&    overlapping,
    const SpatialVec& V_GS1,  // surface velocities
    const SpatialVec& V_GS2,
    ContactPatch&     patch) const
{   SimTK_ASSERT_ALWAYS(!"implemented",
        "ContactForceGenerator::DoNothing::calcContactPatch() not implemented yet."); }
};



//==============================================================================
//                       THROW ERROR FORCE GENERATOR
//==============================================================================
/** This ContactForceGenerator throws an error if it is every invoked. It can 
be used as a way to explicitly catch a certain ContactTypeId and complain, or 
more commonly it is used as the fallback generator for unrecognized 
ContactTypeIds. **/
class SimTK_SIMBODY_EXPORT ContactForceGenerator::ThrowError 
:   public ContactForceGenerator {
public:
explicit ThrowError(ContactTypeId type = ContactTypeId(0)) 
:   ContactForceGenerator(type) {}
virtual ~ThrowError() {}
virtual void calcContactForce
   (const State&            state,
    const Contact&          overlapping,
    const SpatialVec&       V_GS1,  // surface velocities
    const SpatialVec&       V_GS2,
    ContactForce&           contactForce) const
{   SimTK_ASSERT_ALWAYS(!"implemented",
        "ContactForceGenerator::ThrowError::calcContactForce() not implemented yet."); }
virtual void calcContactPatch
   (const State&      state,
    const Contact&    overlapping,
    const SpatialVec& V_GS1,  // surface velocities
    const SpatialVec& V_GS2,
    ContactPatch&     patch) const
{   SimTK_ASSERT_ALWAYS(!"implemented",
        "ContactForceGenerator::ThrowError::calcContactPatch() not implemented yet."); }
};

} // namespace SimTK

#endif // SimTK_SIMBODY_COMPLIANT_CONTACT_SUBSYSTEM_H_