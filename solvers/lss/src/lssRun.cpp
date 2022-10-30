/*

The MIT License (MIT)

Copyright (c) 2017 Tim Warburton, Noel Chalmers, Jesse Chan, Ali Karakus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "lss.hpp"

void lss_t::Run(){

  dfloat startTime, finalTime;
  settings.getSetting("START TIME", startTime);
  settings.getSetting("FINAL TIME", finalTime);
  
  if(advection){  
  initialConditionKernel(mesh.Nelements,
                         startTime,
                         mesh.o_x,
                         mesh.o_y,
                         mesh.o_z,
                         o_q);

  setFlowFieldKernel(mesh.Nelements,
                    startTime,
                    mesh.o_x,
                    mesh.o_y,
                    mesh.o_z,
                    o_U);

  }else if(redistance){
    initialConditionKernel(mesh.Nelements,
                         startTime,
                         mesh.o_x,
                         mesh.o_y,
                         mesh.o_z,
                         o_phi);

    redistanceSetFieldsKernel(mesh.Nelements,
                           startTime,
                           mesh.o_x,
                           mesh.o_y,
                           mesh.o_z,
                           o_phi,
                           o_q);
  }


  dfloat cfl=1.0;
  settings.getSetting("CFL NUMBER", cfl);

  // set time step
  dfloat vmax = 1;

  dfloat dt = cfl/(vmax*(mesh.N+1.)*(mesh.N+1.));
  timeStepper.SetTimeStep(dt);

  stab.detectApply(o_q, o_q, 0.0); 

  postStep(o_q, startTime, 0);


  printf("rank: %d %d %d %d %d\n", mesh.rank, mesh.NinternalElements, mesh.NhaloElements, mesh.Nelements, mesh.totalHaloPairs); 


  timeStepper.Run(*this, o_q, startTime, finalTime);
 
  // output norm of final solution
  {
    //compute q.M*q
    mesh.MassMatrixApply(o_phi, o_Mq);

    dlong Nentries = mesh.Nelements*mesh.Np;
    dfloat norm2 = sqrt(platform.linAlg().innerProd(Nentries, o_phi, o_Mq, mesh.comm));

    if(mesh.rank==0)
      printf("Solution norm = %17.15lg\n", norm2);
  }
}
