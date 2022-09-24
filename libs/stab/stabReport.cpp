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

#include "stab.hpp"

namespace libp {

void stab_t::Report(dfloat time, int tstep){

  static int frame=0;

  dlong NdElements =  GetElementNumber(o_eList); 

  if(mesh.rank==0)
    printf("%5.2f (%d), %d (time, timestep, # of detected elements)\n", time, tstep, NdElements);

  if (settings.compareSetting("STAB OUTPUT TO FILE","TRUE")) {
    char fname[BUFSIZ];
    sprintf(fname, "detect_%04d_%04d.vtu", mesh.rank, frame);
	
	  // copy data back to host
  	o_eList.copyTo(eList);
    if(viscRamp.length()!=0){
      o_viscRamp.copyTo(viscRamp);
    }
  	PlotElements(eList, std::string(fname));

    sprintf(fname, "stab_%04d_%04d.vtu", mesh.rank, frame);
    if(visc.length()!=0){
      o_visc.copyTo(visc);
    }    
    if(qd.length()!=0){
      o_qd.copyTo(qd);
    }    
    
    PlotFields(qd, fname);
    frame++; 
  }
  
}


dlong stab_t::GetElementNumber(deviceMemory<dlong>& o_list){
  // Tweak linAlg sum function
  deviceMemory<dfloat> o_tmp = platform.malloc<dfloat>(mesh.Nelements*dNfields); 
  
  // Copy o_list to tmp in the size of dfloat
  copyFloatKernel(mesh.Nelements*dNfields, o_list, o_tmp); 
  dfloat Nelm = platform.linAlg().sum(mesh.Nelements*dNfields, o_tmp, comm); 

  return dlong(Nelm);
}


// interpolate data to plot nodes and save to file (one per processor)
void stab_t::PlotElements(memory<dlong> ElementList, const std::string fileName){

  FILE *fp;

  fp = fopen(fileName.c_str(), "w");

  fprintf(fp, "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"BigEndian\">\n");
  fprintf(fp, "  <UnstructuredGrid>\n");
  fprintf(fp, "    <Piece NumberOfPoints=\"%d\" NumberOfCells=\"%d\">\n",
          mesh.Nelements*mesh.Nverts, mesh.Nelements);

  // write out nodes
  fprintf(fp, "      <Points>\n");
  fprintf(fp, "        <DataArray type=\"Float32\" NumberOfComponents=\"3\" Format=\"ascii\">\n");

  // compute plot node coordinates on the fly
  for(dlong e=0;e<mesh.Nelements;++e){
    for(int n=0;n<mesh.Nverts;++n){
      const int vnode = mesh.vertexNodes[n]; 
      if(mesh.dim==2){
      fprintf(fp, "       ");
      fprintf(fp, "%g %g %g\n", mesh.x[e*mesh.Np + vnode], 
      							mesh.y[e*mesh.Np + vnode],
      							0.0);
  		}else{
  			fprintf(fp, "       ");
      fprintf(fp, "%g %g %g\n", mesh.x[e*mesh.Np + vnode], 
      							mesh.y[e*mesh.Np + vnode],
      							mesh.z[e*mesh.Np + vnode]);
  		}
    }
  }
  fprintf(fp, "        </DataArray>\n");
  fprintf(fp, "      </Points>\n");


  // write out field
  fprintf(fp, "      <PointData Scalars=\"scalars\">\n");
  if(ElementList.length()!=0){
	 fprintf(fp, "        <DataArray type=\"Float32\" Name=\"Elements\" NumberOfComponents=\"%d\" Format=\"ascii\">\n", dNfields);
	for(dlong e=0;e<mesh.Nelements;++e){
    	for(int n=0;n<mesh.Nverts;++n){
      	  fprintf(fp, "       ");
      	   for(int fld=0; fld<dNfields; fld++){
      		   fprintf(fp, "%g ", dfloat(ElementList[e*dNfields + fld]));
      		}
      		fprintf(fp, "\n");
    	}
  	}
 	fprintf(fp, "       </DataArray>\n");
  }



  if(viscRamp.length()!=0){
 fprintf(fp, "        <DataArray type=\"Float32\" Name=\"Ramp\" NumberOfComponents=\"%d\" Format=\"ascii\">\n", dNfields);
  for(dlong e=0;e<mesh.Nelements;++e){
      for(int n=0;n<mesh.Nverts;++n){
          fprintf(fp, "       ");
           for(int fld=0; fld<dNfields; fld++){
             fprintf(fp, "%g ", dfloat(viscRamp[e*dNfields + fld]));
          }
          fprintf(fp, "\n");
      }
    }
  fprintf(fp, "       </DataArray>\n");
  }


  fprintf(fp, "     </PointData>\n");


  fprintf(fp, "    <Cells>\n");
  fprintf(fp, "      <DataArray type=\"Int32\" Name=\"connectivity\" Format=\"ascii\">\n");
  for(dlong e=0;e<mesh.Nelements;++e){
    fprintf(fp, "       ");
    for(int m=0;m<mesh.Nverts;++m){
      fprintf(fp, "%d ", e*mesh.Nverts + m);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "        </DataArray>\n");

  
  fprintf(fp, "        <DataArray type=\"Int32\" Name=\"offsets\" Format=\"ascii\">\n");
  dlong cnt = 0;
  for(dlong e=0;e<mesh.Nelements;++e){
    cnt += mesh.Nverts;
    fprintf(fp, "       ");
    fprintf(fp, "%d\n", cnt);
  }
  fprintf(fp, "       </DataArray>\n");

  
  fprintf(fp, "       <DataArray type=\"Int32\" Name=\"types\" Format=\"ascii\">\n");
  for(dlong e=0;e<mesh.Nelements;++e){
    if(mesh.elementType==Mesh::TRIANGLES)
      fprintf(fp, "5\n");
    else if(mesh.elementType==Mesh::TETRAHEDRA)
      fprintf(fp, "10\n");
    else if(mesh.elementType==Mesh::QUADRILATERALS)
      fprintf(fp, "9\n");
    else if(mesh.elementType==Mesh::HEXAHEDRA)
      fprintf(fp, "12\n");

  }
  
  fprintf(fp, "        </DataArray>\n");
  fprintf(fp, "      </Cells>\n");
  fprintf(fp, "    </Piece>\n");
  fprintf(fp, "  </UnstructuredGrid>\n");
  fprintf(fp, "</VTKFile>\n");
  fclose(fp);
}





// interpolate data to plot nodes and save to file (one per processor)
void stab_t::PlotFields(memory<dfloat> Q, const std::string fileName){

FILE *fp;

  fp = fopen(fileName.c_str(), "w");

  fprintf(fp, "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"BigEndian\">\n");
  fprintf(fp, "  <UnstructuredGrid>\n");
  fprintf(fp, "    <Piece NumberOfPoints=\"%d\" NumberOfCells=\"%d\">\n",
          mesh.Nelements*mesh.plotNp,
          mesh.Nelements*mesh.plotNelements);

  // write out nodes
  fprintf(fp, "      <Points>\n");
  fprintf(fp, "        <DataArray type=\"Float32\" NumberOfComponents=\"3\" Format=\"ascii\">\n");

  //scratch space for interpolation
  size_t Nscratch = std::max(mesh.Np, mesh.plotNp);
  memory<dfloat> scratch(2*Nscratch);

  memory<dfloat> Ix(mesh.plotNp);
  memory<dfloat> Iy(mesh.plotNp);
  memory<dfloat> Iz(mesh.plotNp);

  // compute plot node coordinates on the fly
  for(dlong e=0;e<mesh.Nelements;++e){
    mesh.PlotInterp(mesh.x + e*mesh.Np, Ix, scratch);
    mesh.PlotInterp(mesh.y + e*mesh.Np, Iy, scratch);
    if(mesh.dim==3)
      mesh.PlotInterp(mesh.z + e*mesh.Np, Iz, scratch);

    if (mesh.dim==2) {
      for(int n=0;n<mesh.plotNp;++n){
        fprintf(fp, "       ");
        fprintf(fp, "%g %g %g\n", Ix[n],Iy[n],0.0);
      }
    } else {
      for(int n=0;n<mesh.plotNp;++n){
        fprintf(fp, "       ");
        fprintf(fp, "%g %g %g\n", Ix[n],Iy[n],Iz[n]);
      }
    }
  }
  fprintf(fp, "        </DataArray>\n");
  fprintf(fp, "      </Points>\n");

  memory<dfloat> Iu(mesh.plotNp);
  memory<dfloat> Iv(mesh.plotNp);
  memory<dfloat> Iw(mesh.plotNp);
  fprintf(fp, "      <PointData Scalars=\"scalars\">\n");
 
  if (Q.length()!=0) {
    // write out pressure
     fprintf(fp, "        <DataArray type=\"Float32\" Name=\"DetectField\" NumberOfComponents=\"%d\" Format=\"ascii\">\n", dNfields);
    for(dlong e=0;e<mesh.Nelements;++e){
      mesh.PlotInterp(Q + 0*mesh.Np + e*mesh.Np*dNfields, Iu, scratch);
       if(dNfields>1)
        mesh.PlotInterp(Q + 1*mesh.Np  + e*mesh.Np*dNfields, Iv, scratch);

      for(int n=0;n<mesh.plotNp;++n){
        fprintf(fp, "       ");
        if(dNfields==1)
          fprintf(fp, "%g \n", Iu[n]);
        else if(dNfields==2)
          fprintf(fp, "%g %g\n", Iu[n], Iv[n]);
      }
    }
    fprintf(fp, "       </DataArray>\n");
  }



 if (visc.length()!=0) {
  // write out pressure
     fprintf(fp, "        <DataArray type=\"Float32\" Name=\"Viscosity\" NumberOfComponents=\"%d\" Format=\"ascii\">\n", dNfields);
    for(dlong e=0;e<mesh.Nelements;++e){

      mesh.PlotInterp(visc + 0*mesh.Np  + e*mesh.Np*dNfields, Iu, scratch);

      if(dNfields>1)
        mesh.PlotInterp(visc + 1*mesh.Np  + e*mesh.Np*dNfields, Iv, scratch);
      if(dNfields>2)
        mesh.PlotInterp(visc + 2*mesh.Np  + e*mesh.Np*dNfields, Iw, scratch);

      for(int n=0;n<mesh.plotNp;++n){
        fprintf(fp, "       ");
        fprintf(fp, "       ");
        if(dNfields==1)
          fprintf(fp, "%g \n", Iu[n]);
        else if(dNfields==2)
          fprintf(fp, "%g %g\n", Iu[n], Iv[n]);
        else if(dNfields==3)
          fprintf(fp, "%g %g %g\n", Iu[n], Iv[n], Iw[n]);
      }
    }
    fprintf(fp, "       </DataArray>\n");
  }


// if (q.length()!=0) {
//     // write out pressure
//     fprintf(fp, "        <DataArray type=\"Float32\" Name=\"qm\" Format=\"ascii\">\n");
//     for(dlong e=0;e<mesh.Nelements;++e){
//       mesh.PlotInterp(q + 1*mesh.Np  + e*mesh.Np*2, Iu, scratch);

//       for(int n=0;n<mesh.plotNp;++n){
//         fprintf(fp, "       ");
//         fprintf(fp, "%g\n", Iu[n]);
//       }
//     }
//     fprintf(fp, "       </DataArray>\n");
//   }

  fprintf(fp, "     </PointData>\n");



  fprintf(fp, "    <Cells>\n");
  fprintf(fp, "      <DataArray type=\"Int32\" Name=\"connectivity\" Format=\"ascii\">\n");

  for(dlong e=0;e<mesh.Nelements;++e){
    for(int n=0;n<mesh.plotNelements;++n){
      fprintf(fp, "       ");
      for(int m=0;m<mesh.plotNverts;++m){
        fprintf(fp, "%d ", e*mesh.plotNp + mesh.plotEToV[n*mesh.plotNverts+m]);
      }
      fprintf(fp, "\n");
    }
  }
  fprintf(fp, "        </DataArray>\n");

  fprintf(fp, "        <DataArray type=\"Int32\" Name=\"offsets\" Format=\"ascii\">\n");
  dlong cnt = 0;
  for(dlong e=0;e<mesh.Nelements;++e){
    for(int n=0;n<mesh.plotNelements;++n){
      cnt += mesh.plotNverts;
      fprintf(fp, "       ");
      fprintf(fp, "%d\n", cnt);
    }
  }
  fprintf(fp, "       </DataArray>\n");

  fprintf(fp, "       <DataArray type=\"Int32\" Name=\"types\" Format=\"ascii\">\n");
  for(dlong e=0;e<mesh.Nelements;++e){
    for(int n=0;n<mesh.plotNelements;++n){
      if(mesh.dim==2)
        fprintf(fp, "5\n");
      else
        fprintf(fp, "10\n");
    }
  }
  fprintf(fp, "        </DataArray>\n");
  fprintf(fp, "      </Cells>\n");
  fprintf(fp, "    </Piece>\n");
  fprintf(fp, "  </UnstructuredGrid>\n");
  fprintf(fp, "</VTKFile>\n");
  fclose(fp);
}







}