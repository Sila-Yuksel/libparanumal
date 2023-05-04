/*

The MIT License (MIT)

Copyright (c) 2017-2022 Tim Warburton, Noel Chalmers, Jesse Chan, Ali Karakus

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

//Mean flow
#define p_RBAR 1.4
#define p_UBAR 3.0
#define p_VBAR 0.0
#define p_PBAR 1.0


// Initial conditions (p is ignored for isothermal)
#define cnsInitialConditions2D(gamma, mu, t, x, y, r, u, v, p) \
{                                         \
  *(r) = p_RBAR;        \
  *(u) = p_UBAR;            \
  *(v) = p_VBAR;            \
  *(p) = p_PBAR;            \
}

// // Initial conditions (p is ignored for isothermal)
// #define cnsInitialConditions2D(gamma, mu, t, x, y, r, u, v, p) \
// {                                         \
//   *(r) = x*x;        \
//   *(u) = x*x*y;            \
//   *(v) = y*y*y;            \
//   *(p) = y*y;            \
// }

// Body force
#define cnsBodyForce2D(gamma, mu, t, x, y, r, u, v, p, fx, fy) \
{                                                   \
  *(fx) = 0.0;                                      \
  *(fy) = 0.0;                                      \
}

// Boundary conditions for double check with FLEXI
// Adiabatic Wall, Weak Dirichlet for Inflow / Outflow
// + 0.5*(gamma-1)*rM*(uM*uM + vM*vM - uP*uP - vP*vP);
// dfloat + 0.5*(gamma-1)*rM*(uM*uM + vM*vM - ruP*ruP - rvP*rvP); \
// dfloat vP = (rM*vM -2.0*ny*(nx*rM*uM + ny*rM*vM))/rM;   \
// /* wall 1, inflow 2, outflow 3, x-slip 4, y-slip 5 */
#define cnsBoundaryConditions2D(bc, gamma, R, CP, CV, mu, \
                                  t, x, y, nx, ny, \
                                  rM, uM, vM, pM, uxM, uyM, vxM, vyM, \
                                  rB, uB, vB, pB, uxB, uyB, vxB, vyB) \
{                                                                     \
const dfloat um = (rM*uM -2.0*nx*(nx*rM*uM + ny*rM*vM))/rM;            \
const dfloat vm = (rM*vM -2.0*ny*(nx*rM*uM + ny*rM*vM))/rM;            \
  if(bc==1){                                                           \
    *(rB) = rM;                                                        \
    *(uB) = um;                                                        \
    *(vB) = vm;                                                        \
    *(pB) = pM + 0.5*(gamma -1.0)*rM*(uM*uM + vM*vM -um*um-vm*vm);     \                                                     \
    *(uxB) = 0.0;                      \
    *(uyB) = 0.0;                      \
    *(vxB) = 0.0;                      \
    *(vyB) = 0.0;                      \
  } else if(bc==2){                    \
    *(rB) = p_RBAR;                    \
    *(uB) = p_UBAR;                    \
    *(vB) = p_VBAR;                    \
    *(pB) = p_RBAR;                    \
    *(uxB) = uxM;                      \
    *(uyB) = uyM;                      \
    *(vxB) = vxM;                      \
    *(vyB) = vyM;                      \
  } else if(bc==3){                    \
    *(rB) = rM;                        \
    *(uB) = uM;                        \
    *(vB) = vM;                         \
    *(pB) = pM;                    \
    *(uxB) = 0.0;                      \
    *(uyB) = 0.0;                      \
    *(vxB) = 0.0;                      \
    *(vyB) = 0.0;                      \
  } else if(bc==4||bc==5){             \
    *(rB) = rM;                        \
    *(uB) = uM - (nx*uM+ny*vM)*nx;     \
    *(vB) = vM - (nx*uM+ny*vM)*ny;     \
    *(pB) = pM;                        \
    *(uxB) = uxM;                      \
    *(uyB) = uyM;                      \
    *(vxB) = vxM;                      \
    *(vyB) = vyM;                      \
  }                                    \
}