__kernel void multiply_matrix(const int M, const int N, const int K,
                      const __global int* A,
                      const __global int* B,
                      __global int* C)
{
  const int Row = get_global_id(0); // Row ID of C (0..M)
  const int Column = get_global_id(1); // Col ID of C (0..N)
  int index=Row+Column*M;
  if (Row < M && Column < N) {
    int i, sum = 0;
    for (i = 0 ; i < M ; ++i) {
      sum += A[Column*M+i] * B[Row+i*M];
    }
    C[index] = sum;
  }
}
