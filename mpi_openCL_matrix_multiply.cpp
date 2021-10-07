#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <omp.h>
#include <time.h>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <iostream>
#include <CL/cl.h>
using namespace std::chrono;

#define SZ 1000 //defining matrix size

/* holding the matrices */
int A[SZ][SZ];
int B[SZ][SZ];
int C[SZ][SZ];

cl_mem bufA, bufB, bufC;

cl_device_id device_id;
cl_context context;
cl_program program;
cl_kernel kernel;
cl_command_queue  queue;
cl_event event = NULL;

int err;
const int max=SZ;
//local size memory of 2D array.
const int TS = 2;
const size_t local[2] = { (size_t)TS, (size_t)TS };
//global size for 2D array.
const size_t global[2] = { (size_t) SZ, (size_t)SZ }; 

cl_device_id create_device();
void setup_openCL_device_context_queue_kernel(char* filename, char* kernelname);
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename);
void setup_kernel_memory();
void copy_kernel_args();
void free_memory();

/* declaring function used */
void head(int num_processes, int start, int end);
void node(int num_processes, int start, int end);
void init(int A[SZ][SZ]);
void print(int B[SZ][SZ]);
void checkResult(int B[SZ][SZ]);

int process_rank, proc, start, end, num_threads;

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &proc);    
    int num_threads = 1;
    //distributing data evenly across nodes
    start = process_rank * SZ / proc;
    end = ((process_rank + 1) * SZ / proc);
    //start the timer
    auto startT = high_resolution_clock::now();
    //call the nodes.
    if (process_rank == 0)
    {
        head(proc, start, end);
    }
    else
    {
        node(proc, start, end);
    }
    //end the time
    auto stopT = high_resolution_clock::now();
    //calculate the runtime
    auto duration = duration_cast<microseconds>(stopT - startT);

   //print to the console Size of matrix and time in microseconds.
   std::cout << "Time taken by function: "
            << duration.count() << " microseconds" << std::endl;
   std::cout<<"SIZE: "<<SZ<<std::endl;
    MPI_Finalize();

    free_memory();
    return 0;
}

void head(int proc, int start, int end){
    init(A); init(B);
    /*
        Idea is to broadcast all of the elements of Second matrix and,
        scatter copies of first matrix, containing certain number of rows based on the load balance, then perform
    */
    MPI_Bcast(B, SZ * SZ, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(&A[0][0], SZ * SZ / proc, MPI_INT, MPI_IN_PLACE, SZ * SZ / proc, MPI_INT, 0, MPI_COMM_WORLD);
    /*
        head perform some multiplication with number of rows being end-start
    */
        
   //gather result from other nodes.
   MPI_Gather(MPI_IN_PLACE, SZ*SZ/proc, MPI_INT, &C[0][0], SZ*SZ/proc, MPI_INT, 0, MPI_COMM_WORLD);
   /*
      The rows left are calculate using Kernel in the openCL
   */
   setup_openCL_device_context_queue_kernel((char *)"./matrix_ops.cl", (char *)"multiply_matrix");


   //for your program as this varies from one implementation to another
    setup_kernel_memory();
    copy_kernel_args();


    //submit the kernel for execution 
    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, &event);
    clWaitForEvents(1, &event);


   //copying data from the device back to host c matrix
   clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, SZ * SZ *sizeof(int), &C[0][0], 0, NULL, NULL);
   //check the answer
   checkResult(C);
}

void node(int proc, int start, int end){

   MPI_Bcast(B, SZ * SZ, MPI_INT, 0, MPI_COMM_WORLD);
   MPI_Scatter(&A[0][0], SZ * SZ / proc, MPI_INT, &A[start], SZ * SZ / proc, MPI_INT, 0, MPI_COMM_WORLD);
    /*
        head perform some multiplication with number of rows being end-start
    */
   MPI_Gather(&C[start], SZ*SZ/proc, MPI_INT, &C, SZ*SZ/proc, MPI_INT, 0, MPI_COMM_WORLD);
    //print(C);
   setup_openCL_device_context_queue_kernel((char *)"./matrix_ops.cl", (char *)"multiply_matrix");


   //this function is used to load/copy memory and link arguments -- you will need to change this 
   //for your program as this varies from one implementation to another
    setup_kernel_memory();
    copy_kernel_args();


    //submit the kernel for execution 
    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, &event);
    clWaitForEvents(1, &event);


   //copying data from the device back to host c matrix
    clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, SZ * SZ *sizeof(int), &C[0][0], 0, NULL, NULL);
}

void init(int matrix[SZ][SZ])
{
  for (int i=0; i<SZ; i++)
    for (int j=0; j<SZ; j++)
      matrix[i][j] = rand() % 99;
}

void print(int matrix[SZ][SZ])
{
  for (int i=0; i<SZ; i++) {
    for (int j=0; j<SZ; j++)
      std::cout<<matrix[i][j]<<"  ";
   std::cout<<std::endl;
   
  }
}
void checkResult(int matrix[SZ][SZ])
{
   int t=1;
  for (int i=0; i<SZ; i++) {
    for (int j=0; j<SZ; j++)
    if(C[i][j]==0){
      std::cout<<matrix[i][j]<<"  ";
      t=0;
    }
  }
  std::cout<<std::endl;
  if(t==1){
     std::cout<<"Result is Valid"<<std::endl;
  }
}


void free_memory() {
   //free the buffers
   clReleaseMemObject(bufA);
   clReleaseMemObject(bufB);
   clReleaseMemObject(bufC);

    //free opencl objects
   clReleaseKernel(kernel);
   clReleaseCommandQueue(queue);
   clReleaseProgram(program);
   clReleaseContext(context);
   
}
void copy_kernel_args() {
    clSetKernelArg(kernel, 0, sizeof(int), (void*)&max);
    clSetKernelArg(kernel, 1, sizeof(int), (void*)&max);
    clSetKernelArg(kernel, 2, sizeof(int), (void*)&max);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&bufA);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&bufB);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&bufC);
    if(err < 0) {
      perror("Couldn't create a kernel argument");
      printf("error = %d", err);
      exit(1);
   }
}
void setup_kernel_memory() {
     bufA = clCreateBuffer(context, CL_MEM_READ_ONLY,  SZ*SZ*sizeof(int), NULL, NULL);
     bufB = clCreateBuffer(context, CL_MEM_READ_ONLY,  SZ*SZ*sizeof(int), NULL, NULL);
     bufC = clCreateBuffer(context, CL_MEM_READ_WRITE, SZ*SZ*sizeof(int), NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufA, CL_TRUE, 0, SZ*SZ*sizeof(int), &A[0][0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufB, CL_TRUE, 0, SZ*SZ*sizeof(int), &B[0][0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufC, CL_TRUE, 0, SZ*SZ*sizeof(int), &C[0][0], 0, NULL, NULL);

}

void setup_openCL_device_context_queue_kernel(char* filename, char* kernelname) {
    device_id = create_device();
    cl_int err;
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
   if(err < 0) {
      perror("Couldn't create a context");
      exit(1);   
    }

    program = build_program(context, device_id, filename );
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if(err < 0) {
      perror("Couldn't create a command queue");
      exit(1);   
    };

    kernel = clCreateKernel(program, kernelname, &err);
    if(err < 0) {
      perror("Couldn't create a kernel");
      printf("error =%d", err);
      exit(1);
    };

}
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

   cl_program program;
   FILE *program_handle;
   char *program_buffer, *program_log;
   size_t program_size, log_size;
  

   /* Read program file and place content into buffer */
   program_handle = fopen(filename, "r");
   if(program_handle == NULL) {
      perror("Couldn't find the program file");
      exit(1);
   }
   fseek(program_handle, 0, SEEK_END);
   program_size = ftell(program_handle);
   rewind(program_handle);
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   fread(program_buffer, sizeof(char), program_size, program_handle);
   fclose(program_handle);

   /* Create program from file 
   Creates a program from the source code in the add_numbers.cl file. 
   Specifically, the code reads the file's content into a char array 
   called program_buffer, and then calls clCreateProgramWithSource.
   */
   program = clCreateProgramWithSource(ctx, 1, 
      (const char**)&program_buffer, &program_size, &err);
   if(err < 0) {
      perror("Couldn't create the program");
      exit(1);
   }
   free(program_buffer);

   /* Build program 
   The fourth parameter accepts options that configure the compilation. 
   These are similar to the flags used by gcc. For example, you can 
   define a macro with the option -DMACRO=VALUE and turn off optimization 
   with -cl-opt-disable.
   */
   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if(err < 0) {

      /* Find size of log and print to std output */
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            0, NULL, &log_size);
      program_log = (char*) malloc(log_size + 1);
      program_log[log_size] = '\0';
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            log_size + 1, program_log, NULL);
      printf("%s\n", program_log);
      free(program_log);
      exit(1);
   }

   return program;
}

cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   // Access a device
   // GPU
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      // CPU
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }
   return dev;
}