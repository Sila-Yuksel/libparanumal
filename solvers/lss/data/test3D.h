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


// Level-Set function
#define lssInitialConditions3D(t, x, y, z, q) \
{                                       \
  const dfloat xc = -1.0;           \
  const dfloat yc = -1.0;         \
  const dfloat zc = -1.0;          \
  const dfloat rc = 1.0;          \
  (*q) = (sqrt((x-xc)*(x-xc) +(y-yc)*(y-yc)+(z-zc)*(z-zc))-rc) <=0.0 ? 1 :-1 ; \
}

  // (*q) = (sqrt((x-xc)*(x-xc) +(y-yc)*(y-yc)+(z-zc)*(z-zc))-rc) <=0.0 ? 1 :-1 ; \

// Boundary conditions
/* wall 1, outflow 2 */
#define lssDirichletConditions3D(bc, t, x, y, z, nx, ny, nz, qM, qB) \
{                                       \
  if(bc==1){                            \
    *(qB) = qM;                        \
  } else if(bc==2){                     \
    *(qB) = qM;                         \
  }                                     \
}