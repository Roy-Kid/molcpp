#include "molecule.h"

namespace MolCpp
{

    Molecule::Molecule(): Topology{}, _flags{0}
    {
        // ctor
    }

    Molecule::Molecule(const Pattern *p) : Molecule{}
    {
        
    }

    bool Molecule::add_new_hydrogens(HydrogenType whichHydrogen, bool correctForPH, double pH)
    {
        if (!is_corrected_for_ph() && correctForPH)
            CorrectForPH(pH);

        if (has_hydrogens_added())
            return (true);

        bool hasChiralityPerceived = this->has_hydrogens_added(); // remember

        /*
        //
        // This was causing bug #1892844 in avogadro. We also want to add hydrogens if the molecule has no bonds.
        //
        if(NumBonds()==0 && NumAtoms()!=1)
        {
        obErrorLog.ThrowError(__FUNCTION__,
        "Did not run OpenBabel::AddHydrogens on molecule with no bonds", obAuditMsg);
        return true;
        }
        */
        if (whichHydrogen == AllHydrogen)
            LOG_INFO("Adding hydrogens to molecule");
        else if (whichHydrogen == PolarHydrogen)
            LOG_INFO("Adding polar hydrogens to molecule");
        else
            LOG_INFO("Adding non-polar hydrogens to molecule");

        // Make sure we have conformers (PR#1665519)
        if (!_vconf.empty() && !Empty())
        {
            OBAtom *atom;
            vector<OBAtom *>::iterator i;
            for (atom = BeginAtom(i); atom; atom = NextAtom(i))
            {
                atom->SetVector();
            }
        }

        SetHydrogensAdded(); // This must come after EndModify() as EndModify() wipes the flags
        // If chirality was already perceived, remember this (to avoid wiping information
        if (hasChiralityPerceived)
            this->SetChiralityPerceived();

        // count up number of hydrogens to add
        OBAtom *atom, *h;
        int hcount, count = 0;
        vector<pair<OBAtom *, int>> vhadd;
        vector<OBAtom *>::iterator i;
        for (atom = BeginAtom(i); atom; atom = NextAtom(i))
        {
            if (whichHydrogen == PolarHydrogen && !AtomIsNSOP(atom))
                continue;
            if (whichHydrogen == NonPolarHydrogen && AtomIsNSOP(atom))
                continue;

            hcount = atom->GetImplicitHCount();
            atom->SetImplicitHCount(0);

            if (hcount)
            {
                vhadd.push_back(pair<OBAtom *, int>(atom, hcount));
                count += hcount;
            }
        }

        if (count == 0)
        {
            // Make sure to clear SSSR and aromatic flags we may have tripped above
            _flags &= (~(OB_SSSR_MOL | OB_AROMATIC_MOL));
            return (true);
        }
        bool hasCoords = HasNonZeroCoords();

        // realloc memory in coordinate arrays for new hydrogens
        double *tmpf;
        vector<double *>::iterator j;
        for (j = _vconf.begin(); j != _vconf.end(); ++j)
        {
            tmpf = new double[(NumAtoms() + count) * 3];
            memset(tmpf, '\0', sizeof(double) * (NumAtoms() + count) * 3);
            if (hasCoords)
                memcpy(tmpf, (*j), sizeof(double) * NumAtoms() * 3);
            delete[] * j;
            *j = tmpf;
        }

        IncrementMod();

        int m, n;
        vector3 v;
        vector<pair<OBAtom *, int>>::iterator k;
        double hbrad = CorrectedBondRad(1, 0);

        for (k = vhadd.begin(); k != vhadd.end(); ++k)
        {
            atom = k->first;
            double bondlen = hbrad + CorrectedBondRad(atom->GetAtomicNum(), atom->GetHyb());
            for (m = 0; m < k->second; ++m)
            {
                int badh = 0;
                for (n = 0; n < NumConformers(); ++n)
                {
                    SetConformer(n);
                    if (hasCoords)
                    {
                        // Ensure that add hydrogens only returns finite coords
                        // atom->GetNewBondVector(v,bondlen);
                        v = OBBuilder::GetNewBondVector(atom, bondlen);
                        if (isfinite(v.x()) || isfinite(v.y()) || isfinite(v.z()))
                        {
                            _c[(NumAtoms()) * 3] = v.x();
                            _c[(NumAtoms()) * 3 + 1] = v.y();
                            _c[(NumAtoms()) * 3 + 2] = v.z();
                        }
                        else
                        {
                            _c[(NumAtoms()) * 3] = 0.0;
                            _c[(NumAtoms()) * 3 + 1] = 0.0;
                            _c[(NumAtoms()) * 3 + 2] = 0.0;
                            obErrorLog.ThrowError(__FUNCTION__,
                                                  "Ran OpenBabel::AddHydrogens -- no reasonable bond geometry for desired hydrogen.",
                                                  obAuditMsg);
                            badh++;
                        }
                    }
                    else
                        memset((char *)&_c[NumAtoms() * 3], '\0', sizeof(double) * 3);
                }
                if (badh == 0 || badh < NumConformers())
                {
                    // Add the new H atom to the appropriate residue list
                    // but avoid doing perception by checking for existence of residue
                    // just in case perception is trigger, make sure GetResidue is called
                    // before adding the hydrogen to the molecule
                    OBResidue *res = atom->HasResidue() ? atom->GetResidue() : nullptr;
                    h = NewAtom();
                    h->SetType("H");
                    h->SetAtomicNum(1);
                    string aname = "H";

                    if (res)
                    {
                        res->AddAtom(h);
                        res->SetAtomID(h, aname);

                        // hydrogen should inherit hetatm status of heteroatom (default is false)
                        if (res->IsHetAtom(atom))
                        {
                            res->SetHetAtom(h, true);
                        }
                    }

                    int bondFlags = 0;
                    AddBond(atom->GetIdx(), h->GetIdx(), 1, bondFlags);
                    h->SetCoordPtr(&_c);
                    OpenBabel::ImplicitRefToStereo(*this, atom->GetId(), h->GetId());
                }
            }
        }

        DecrementMod();

        // reset atom type and partial charge flags
        _flags &= (~(OB_PCHARGE_MOL | OB_ATOMTYPES_MOL | OB_SSSR_MOL | OB_AROMATIC_MOL | OB_HYBRID_MOL));

        return (true);
    }

}