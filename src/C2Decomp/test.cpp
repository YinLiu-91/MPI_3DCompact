#include <mpi.h>
#include <cmath>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <fstream>
#include <assert.h>

using namespace std;

#include "C2Decomp.hpp"
 
int main(int argc, char *argv[]){
   int ierr, totRank, mpiRank;

    //Initialize MPI
    ierr = MPI_Init( &argc, &argv);

    //Get the number of processes
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &totRank);

    //Get the local rank
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);

    if(!mpiRank){
        cout << endl;
        cout << "-------------------" << endl;
    	cout << " C2Decomp Testing " << endl;
    	cout << "-------------------" << endl;
    	cout << endl;

    }
 
    int nx = 100, ny = 100, nz = 100;
    int pRow = 0, pCol = 0;
    bool periodicBC[3] = {true, true, true};

    if(!mpiRank) cout << "initializing " << endl;
    C2Decomp *c2d;
    c2d = new C2Decomp(nx, ny, nz, pRow, pCol, periodicBC);
    if(!mpiRank) cout << "done initializing " << endl;
    
    bool errorFlag, errorFlagGlobal;

    int m = 1;
    double data1[nz][ny][nx];
    for(int kp = 0; kp < nz; kp++){
	for(int jp = 0; jp < ny; jp++){
	    for(int ip = 0; ip < nx; ip++){
		data1[kp][jp][ip] = (double)m;
		m++;
	    }
	}
    }

    double xSize[3], ySize[3], zSize[3];
    xSize[0] = c2d->xSize[0];
    xSize[1] = c2d->xSize[1];
    xSize[2] = c2d->xSize[2];
    ySize[0] = c2d->ySize[0];
    ySize[1] = c2d->ySize[1];
    ySize[2] = c2d->ySize[2];
    zSize[0] = c2d->zSize[0];
    zSize[1] = c2d->zSize[1];
    zSize[2] = c2d->zSize[2];

    double *u1, *u2, *u3;


    c2d->allocX(u1);
    c2d->allocY(u2);
    c2d->allocZ(u3);

    for(int kp = 0; kp < xSize[2]; kp++){
	for(int jp = 0; jp < xSize[1]; jp++){
	    for(int ip = 0; ip < xSize[0]; ip++){
		int ii = kp*xSize[1]*xSize[0] + jp*xSize[0] + ip;
		u1[ii] = data1[c2d->xStart[2]+kp][c2d->xStart[1]+jp][c2d->xStart[0]+ip];
	    }
	}
    }

    double t1, t2, t3;
    t1 = MPI_Wtime();
    c2d->transposeX2Y(u1, u2);
    t2 = MPI_Wtime();

    if(mpiRank == 0){
	printf( "X2Y Elapsed time is %f\n", t2 - t1 );
    }

    //Testing transposition

    errorFlag = false;
    errorFlagGlobal = false;
    for(int kp = 0; kp < ySize[2]; kp++){
	for(int jp = 0; jp < ySize[1]; jp++){
	    for(int ip = 0; ip < ySize[0]; ip++){
	  	int ii = kp*ySize[1]*ySize[0] + jp*ySize[0] + ip;
		double temp = u2[ii];
		double temp1= data1[c2d->yStart[2]+kp][c2d->yStart[1]+jp][c2d->yStart[0]+ip];  
		if(fabs(temp-temp1)>1E-16){
		    cout << "Error in blocking X2Y tranposition" << endl;
		}
	    }
	}
    }


    t1 = MPI_Wtime();
    c2d->transposeY2Z(u2, u3);
    t2 = MPI_Wtime();
    if(mpiRank == 0){
	printf( "Y2Z Elapsed time is %f\n", t2 - t1 );
    }


    //Testing transposition
    for(int kp = 0; kp < zSize[2]; kp++){
	for(int jp = 0; jp < zSize[1]; jp++){
	    for(int ip = 0; ip < zSize[0]; ip++){
		int ii = kp*zSize[1]*zSize[0] + jp*zSize[0] + ip;
		double temp = u3[ii];
		double temp1= data1[c2d->zStart[2]+kp][c2d->zStart[1]+jp][c2d->zStart[0]+ip]; 
		if(fabs(temp-temp1)>1E-16){
		    cout << "Error in blocking Y2Z tranposition" << endl;
		}
 	    }
  	}
    }
    

    t1 = MPI_Wtime();
    c2d->transposeZ2Y(u3, u2);
    t2 = MPI_Wtime();
    if(mpiRank == 0){
	printf( "Z2Y Elapsed time is %f\n", t2 - t1 );
    }


    //Testing transposition
    for(int kp = 0; kp < ySize[2]; kp++){
	for(int jp = 0; jp < ySize[1]; jp++){
	    for(int ip = 0; ip < ySize[0]; ip++){
	  	int ii = kp*ySize[1]*ySize[0] + jp*ySize[0] + ip;
		double temp = u2[ii];
		double temp1= data1[c2d->yStart[2]+kp][c2d->yStart[1]+jp][c2d->yStart[0]+ip]; 
		if(fabs(temp-temp1)>1E-16){
		    cout << "Error in blocking Z2Y tranposition" << endl;
		}
	    }
 	}
    }


    t1 = MPI_Wtime();
    c2d->transposeY2X(u2, u1);
    t2 = MPI_Wtime();
    if(mpiRank == 0){
	printf( "Y2X Elapsed time is %f\n", t2 - t1 );
    }

    if(mpiRank==0){
      //Testing transposition
      for(int kp = 0; kp < xSize[2]; kp++){
	  for(int jp = 0; jp < xSize[1]; jp++){
	      for(int ip = 0; ip < xSize[0]; ip++){
	  	  int ii = kp*xSize[1]*xSize[0] + jp*xSize[0] + ip;
		  double temp = u1[ii];
		  double temp1= data1[c2d->xStart[2]+kp][c2d->xStart[1]+jp][c2d->xStart[0]+ip]; 
		  if(fabs(temp-temp1)>1E-16){
		      cout << "Error in blocking Y2X transposition" << endl;
		  }
	      }
 	  }
       }
    }

    //allocate new buffers for non-blocking comms
    double *sbuf = new double[c2d->decompBufSize];
    double *rbuf = new double[c2d->decompBufSize];

    MPI_Request x2yHandle;

    t1 = MPI_Wtime();
    c2d->transposeX2Y_Start(x2yHandle, u1, u2, sbuf, rbuf);
    t2 = MPI_Wtime();
    c2d->transposeX2Y_Wait(x2yHandle, u1, u2, sbuf, rbuf);
    t3 = MPI_Wtime();
    if(mpiRank == 0){
	printf( "X2Y Nonblocking Start Elapsed time is %f\n", t2 - t1 );
	printf( "X2Y Nonblocking Wait Elapsed time is %f\n", t3 - t2 );
    }


    //Testing transposition
    for(int kp = 0; kp < ySize[2]; kp++){
	for(int jp = 0; jp < ySize[1]; jp++){
	    for(int ip = 0; ip < ySize[0]; ip++){
	  	int ii = kp*ySize[1]*ySize[0] + jp*ySize[0] + ip;
		double temp = u2[ii];
		double temp1= data1[c2d->yStart[2]+kp][c2d->yStart[1]+jp][c2d->yStart[0]+ip]; 
		if(fabs(temp-temp1)>1E-16){
		    cout << "Error in nonblocking X2Y transposition" << endl;
		}
	    }
        }
    }

    MPI_Request y2zHandle;
    t1 = MPI_Wtime();
    c2d->transposeY2Z_Start(y2zHandle, u2, u3, sbuf, rbuf);
    t2 = MPI_Wtime();
    c2d->transposeY2Z_Wait(y2zHandle, u2, u3, sbuf, rbuf);
    t3 = MPI_Wtime();
    if(mpiRank == 0){
	printf( "Y2Z Nonblocking Start Elapsed time is %f\n", t2 - t1 );
	printf( "Y2Z Nonblocking Wait Elapsed time is %f\n", t3 - t2 );
    }


    //Testing transposition
    for(int kp = 0; kp < zSize[2]; kp++){
        for(int jp = 0; jp < zSize[1]; jp++){
	    for(int ip = 0; ip < zSize[0]; ip++){
		int ii = kp*zSize[1]*zSize[0] + jp*zSize[0] + ip;
		double temp = u3[ii];
		double temp1= data1[c2d->zStart[2]+kp][c2d->zStart[1]+jp][c2d->zStart[0]+ip];
		if(fabs(temp-temp1)>1E-16){  
		    cout << "Error in nonblocking Y2Z transposition" << endl;
		}
 	    }
  	}
    }


    MPI_Request z2yHandle;
    t1 = MPI_Wtime();
    c2d->transposeZ2Y_Start(z2yHandle, u3, u2, sbuf, rbuf);
    t2 = MPI_Wtime();
    c2d->transposeZ2Y_Wait(z2yHandle, u3, u2, sbuf, rbuf);
    t3 = MPI_Wtime();
    if(mpiRank == 0){
	printf( "Z2Y Nonblocking Start Elapsed time is %f\n", t2 - t1 );
	printf( "Z2Y Nonblocking Wait Elapsed time is %f\n", t3 - t2 );
    }


    //Testing transposition
    for(int kp = 0; kp < ySize[2]; kp++){
	for(int jp = 0; jp < ySize[1]; jp++){
	    for(int ip = 0; ip < ySize[0]; ip++){
	  	int ii = kp*ySize[1]*ySize[0] + jp*ySize[0] + ip;
		double temp = u2[ii];
		double temp1= data1[c2d->yStart[2]+kp][c2d->yStart[1]+jp][c2d->yStart[0]+ip]; 
		if(fabs(temp-temp1)>1E-16){  
		    cout << "Error in nonblocking Z2Y transposition" << endl;
	   	}
	    }
 	}
    }


    MPI_Request y2xHandle;
    t1 = MPI_Wtime();
    c2d->transposeY2X_Start(y2xHandle, u2, u1, sbuf, rbuf);
    t2 = MPI_Wtime();
    c2d->transposeY2X_Wait(y2xHandle, u2, u1, sbuf, rbuf);
    t3 = MPI_Wtime();
    if(mpiRank == 0){
	printf( "Y2X Nonblocking Start Elapsed time is %f\n", t2 - t1 );
	printf( "Y2X Nonblocking Wait Elapsed time is %f\n", t3 - t2 );
    }


    //Testing transposition
    for(int kp = 0; kp < xSize[2]; kp++){
	for(int jp = 0; jp < xSize[1]; jp++){
	    for(int ip = 0; ip < xSize[0]; ip++){
	  	int ii = kp*xSize[1]*xSize[0] + jp*xSize[0] + ip;
		double temp = u1[ii];
		double temp1= data1[c2d->xStart[2]+kp][c2d->xStart[1]+jp][c2d->xStart[0]+ip]; 
		if(fabs(temp-temp1)>1E-16){  
		    cout << "Error in nonblocking Y2X transposition" << endl;
	   	}
	    }
 	}
    }


    double *u4;
    c2d->allocX(u4);


    t1 = MPI_Wtime();
    c2d->writeOne(0, u1, "test.out");
    MPI_Barrier(MPI_COMM_WORLD);
    c2d->readOne(0, u4, "test.out");
    t2 = MPI_Wtime();

      //Testing write and read one...
    for(int kp = 0; kp < xSize[2]; kp++){
	for(int jp = 0; jp < xSize[1]; jp++){
	    for(int ip = 0; ip < xSize[0]; ip++){
	  	int ii = kp*xSize[1]*xSize[0] + jp*xSize[0] + ip;
		double temp = u1[ii];
		double temp1= u4[ii];
		if(fabs(temp-temp1)>1E-16){
		    //cout << "Error in write/readOne()" << endl;
		}
	    }
 	}
    }
    
    if(!mpiRank){
 	printf("writeOne/readOne elapsed time is %f\n", t2-t1);
    }   




    t1 = MPI_Wtime();

    //Write out to large file
    MPI_File fh;
    MPI_Offset disp, filesize;

    string filename = "vartest.out";
    MPI_File_open(MPI_COMM_WORLD, filename.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    filesize = 0;
    MPI_File_set_size(fh, filesize);

    disp = 0;

    c2d->writeVar(fh, disp, 0, u1);
    c2d->writeVar(fh, disp, 1, u2);
    c2d->writeVar(fh, disp, 2, u3);
    c2d->writeVar(fh, disp, 0, u4);
    
    MPI_File_close(&fh);

    //Now lets read in that data back in
    double *u1a, *u2a, *u3a, *u4a;

    c2d->allocX(u1a);
    c2d->allocY(u2a);
    c2d->allocZ(u3a);
    c2d->allocX(u4a);

    MPI_File_open(MPI_COMM_WORLD, filename.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    disp = 0;    

    c2d->readVar(fh, disp, 0, u1a);
    c2d->readVar(fh, disp, 1, u2a);
    c2d->readVar(fh, disp, 2, u3a);
    c2d->readVar(fh, disp, 0, u4a);

    MPI_File_close(&fh);

    t2 = MPI_Wtime();

    if(!mpiRank){
 	printf("writeVar/readVar w/ four variables, elapsed time is %f\n", t2-t1);
    }   



    delete[] sbuf;
    delete[] rbuf;
    c2d->deallocXYZ(u1);
    c2d->deallocXYZ(u1a);
    c2d->deallocXYZ(u2);
    c2d->deallocXYZ(u2a);
    c2d->deallocXYZ(u3);
    c2d->deallocXYZ(u3a);
    c2d->deallocXYZ(u4);
    c2d->deallocXYZ(u4a);


    //Testing chunking
    double *u1chunk1; c2d->allocX(u1chunk1);
    double *u1chunk2; c2d->allocX(u1chunk2);
    double *u1chunk3; c2d->allocX(u1chunk3);
    double *u1chunk4; c2d->allocX(u1chunk4);
    double *u1chunk5; c2d->allocX(u1chunk5);

    double *u2chunk1; c2d->allocY(u2chunk1);
    double *u2chunk2; c2d->allocY(u2chunk2);
    double *u2chunk3; c2d->allocY(u2chunk3);
    double *u2chunk4; c2d->allocY(u2chunk4);
    double *u2chunk5; c2d->allocY(u2chunk5);

    double *u1chunk1_nc; c2d->allocX(u1chunk1_nc);
    double *u1chunk2_nc; c2d->allocX(u1chunk2_nc);
    double *u1chunk3_nc; c2d->allocX(u1chunk3_nc);
    double *u1chunk4_nc; c2d->allocX(u1chunk4_nc);
    double *u1chunk5_nc; c2d->allocX(u1chunk5_nc);

    double *u2chunk1_nc; c2d->allocY(u2chunk1_nc);
    double *u2chunk2_nc; c2d->allocY(u2chunk2_nc);
    double *u2chunk3_nc; c2d->allocY(u2chunk3_nc);
    double *u2chunk4_nc; c2d->allocY(u2chunk4_nc);
    double *u2chunk5_nc; c2d->allocY(u2chunk5_nc);



    if(!mpiRank) cout << " > Setting up array of arrays chunk..." << endl; 
    double *p_src[5] = {u1chunk1, u1chunk2, u1chunk3, u1chunk4, u1chunk5};;
    double *p_dst[5] = {u2chunk1, u2chunk2, u2chunk3, u2chunk4, u2chunk5};

    if(!mpiRank) cout << " > Allocating big buffers..." << endl; 
    double *big_sbuf = new double[(int)(xSize[0]*xSize[1]*xSize[2]*5)];
    double *work1     = new double[(int)(xSize[0]*xSize[1]*xSize[2]*5)];
    double *big_rbuf = new double[(int)(ySize[0]*ySize[1]*ySize[2]*5)];
    double *work2    = new double[(int)(ySize[0]*ySize[1]*ySize[2]*5)];


    if(!mpiRank) cout << " > Filling chunk..." << endl; 
    for(int kp = 0; kp < xSize[2]; kp++){
        for(int jp = 0; jp < xSize[1]; jp++){
            for(int ip = 0; ip < xSize[0]; ip++){
                int ii = kp*xSize[1]*xSize[0] + jp*xSize[0] + ip;
		int totalSize = xSize[0]*xSize[1]*xSize[2];
                u1chunk1[ii] = data1[c2d->xStart[2]+kp][c2d->xStart[1]+jp][c2d->xStart[0]+ip];
                u1chunk1_nc[ii] = u1chunk1[ii];
                u1chunk2[ii] = totalSize*2 + data1[c2d->xStart[2]+kp][c2d->xStart[1]+jp][c2d->xStart[0]+ip];
                u1chunk2_nc[ii] = u1chunk2[ii];
                u1chunk3[ii] = totalSize*3 + data1[c2d->xStart[2]+kp][c2d->xStart[1]+jp][c2d->xStart[0]+ip];
                u1chunk3_nc[ii] = u1chunk3[ii];
                u1chunk4[ii] = totalSize*4 + data1[c2d->xStart[2]+kp][c2d->xStart[1]+jp][c2d->xStart[0]+ip];
                u1chunk4_nc[ii] = u1chunk4[ii];
                u1chunk5[ii] = totalSize*5 + data1[c2d->xStart[2]+kp][c2d->xStart[1]+jp][c2d->xStart[0]+ip];
                u1chunk5_nc[ii] = u1chunk5[ii];
            }
        }
    }
    
    MPI_Request handle_chunk;

    t1 = MPI_Wtime();
    c2d->transposeChunkX2Y_MajorIndex_Start(handle_chunk, p_src, p_dst, big_sbuf, big_rbuf, 5, work1);
    c2d->transposeChunkX2Y_MajorIndex_Wait(handle_chunk, p_src, p_dst, big_sbuf, big_rbuf, 5, work2);
    t2 = MPI_Wtime();

    
    if(!mpiRank)  cout << " > DONE..." << endl; 

    if(!mpiRank){
 	printf("chunk elapsed time is %f\n", t2-t1);
	cout << "Running blocking chunk send..." << endl;
    }   

    t1 = MPI_Wtime();
    c2d->transposeChunkX2Y_MajorIndex(p_src, p_dst, 5, work1, work2);
    t2 = MPI_Wtime();

    if(!mpiRank){
 	printf("Blocking chunk send elapsed time is %f\n", t2-t1);
	cout << "Running regular, non-chunking transposes..." << endl;
    }

    t1 = MPI_Wtime();
    c2d->transposeX2Y_MajorIndex(u1chunk1_nc, u2chunk1_nc);
    c2d->transposeX2Y_MajorIndex(u1chunk2_nc, u2chunk2_nc);
    c2d->transposeX2Y_MajorIndex(u1chunk3_nc, u2chunk3_nc);
    c2d->transposeX2Y_MajorIndex(u1chunk4_nc, u2chunk4_nc);
    c2d->transposeX2Y_MajorIndex(u1chunk5_nc, u2chunk5_nc);
    t2 = MPI_Wtime();
    if(!mpiRank){
 	printf("nonchunk elapsed time is %f\n", t2-t1);
    }   



    c2d->deallocXYZ(u1chunk1);
    c2d->deallocXYZ(u1chunk2);
    c2d->deallocXYZ(u1chunk3);
    c2d->deallocXYZ(u1chunk4);
    c2d->deallocXYZ(u1chunk5);

    c2d->deallocXYZ(u2chunk1);
    c2d->deallocXYZ(u2chunk2);
    c2d->deallocXYZ(u2chunk3);
    c2d->deallocXYZ(u2chunk4);
    c2d->deallocXYZ(u2chunk5);

    delete[] big_sbuf;
    delete[] big_rbuf;
    

    //Now lets kill MPI
    MPI_Finalize();

    return 0;
}









