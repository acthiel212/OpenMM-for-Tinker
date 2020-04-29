#include "AmoebaReferenceGKCavitationForce.h"
using namespace OpenMM;


double AmoebaReferenceGKCavitationForce::calculateForceAndEnergy(vector<RealVec>& pos, vector<RealVec>& force, int numParticles,
                                                                 vector<int> ishydrogen, vector<RealOpenMM> radii_large, vector<RealOpenMM> radii_vdw, vector<RealOpenMM> gammas,
                                                                 double roffset, vector<RealVec> vol_force, vector<RealOpenMM> vol_dv, vector<RealOpenMM> free_volume,
                                                                 vector<RealOpenMM> self_volume){
    //create and saves GaussVol instance
    //radii, volumes, etc. will be set in execute()
    gvol = new GaussVol(numParticles, ishydrogen);

    //sequence: volume1->volume2
    //weights
    RealOpenMM w_evol = 1.0;
    RealOpenMM energy = 0.0;
    vector<RealOpenMM> nu(numParticles);

    // volume energy function 1 (large radii)
    RealOpenMM volume1, vol_energy1;
    gvol->setRadii(radii_large);

    vector<RealOpenMM> volumes_large(numParticles);
    for(int i = 0; i < numParticles; i++){
        volumes_large[i] = ishydrogen[i]>0 ? 0.0 : 4.*M_PI*pow(radii_large[i],3)/3.;
    }
    gvol->setVolumes(volumes_large);

    for(int i = 0; i < numParticles; i++){
        nu[i] = gammas[i]/roffset;
    }
    gvol->setGammas(nu);
    gvol->compute_tree(pos);
    gvol->compute_volume(pos, volume1, vol_energy1, vol_force, vol_dv, free_volume, self_volume);

    //returns energy and gradients from volume energy function
    for(int i = 0; i < numParticles; i++){
        force[i] += vol_force[i] * w_evol;
    }
    energy += vol_energy1 * w_evol;

    // volume energy function 2 (small radii)
    RealOpenMM vol_energy2, volume2;
    gvol->setRadii(radii_vdw);

    vector<RealOpenMM> volumes_vdw(numParticles);
    for(int i = 0; i < numParticles; i++){
        volumes_vdw[i] = ishydrogen[i]>0 ? 0.0 : 4.*M_PI*pow(radii_vdw[i],3)/3.;
    }

    gvol->setVolumes(volumes_vdw);

    for(int i = 0; i < numParticles; i++){
        nu[i] = -gammas[i]/roffset;
    }

    gvol->setGammas(nu);
    gvol->rescan_tree_volumes(pos);
    gvol->compute_volume(pos, volume2, vol_energy2, vol_force, vol_dv, free_volume, self_volume);

    for(int i = 0; i < numParticles; i++){
        force[i] += vol_force[i] * w_evol;
    }
    energy += vol_energy2 * w_evol;

    //returns energy
    return (double)energy;
}

