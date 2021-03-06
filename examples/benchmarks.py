#!/usr/bin/env python3

#
# Configuration options for benchmarks
#

# Platforms

PYVIENNACL = True  # PyViennaCL benchmarks
NUMPY_SCIPY = True # NumPy / SciPy benchmarks
CUDA = True        # Only if gnumpy is installed

# Operations

ADD = True     # Dense matrix elementwise addition
GEMM = True    # GEMM
GEMV = True    # Dense GEMV
SPGEMV = True  # Sparse GEMV

# Matrix structure parameters

ADD_SIZES = [2**x for x in range(3,15)]
GEMM_SIZES = [2**x for x in range(3,13)]
GEMV_SIZES = [2**x for x in range(3,13)]
SPGEMV_SIZES = [10**n for n in range(2,9)]
SPARSITY = 0.02

################################################################################

from timeit import timeit
WITH_CUDA = True
try: import gnumpy
except ImportError: WITH_CUDA = False

def do_benchmark(setup, stmt, sizes, num_iters=10, sparsity=None):
    for size in sizes:
        if sparsity is None:
            setup_fmt = setup%(size,stmt)
        else:
            setup_fmt = setup%(size,sparsity,stmt)
        try:
            per_call = timeit(stmt,
                              setup=setup_fmt,
                              number=num_iters) / num_iters
            print(size, per_call)
        except Exception as e:
            print("Exception with size %d: %s" % (size, e))
            raise

setup_dense_pyvcl = """import numpy as np
import pyviennacl as p

from pyviennacl.backend import OpenCLMemory, Context
ctx = Context(OpenCLMemory)

dtype = np.float32

size = %d

A = np.random.rand(size,size).astype(dtype)
B = np.random.rand(size,size).astype(dtype)

A = p.Matrix(A, context=ctx)
B = p.Matrix(B, context=ctx)

x = np.random.rand(size).astype(dtype)
x = p.Vector(x, context=ctx)

for i in range(3):
    %s
"""

setup_sparse_pyvcl = """import numpy as np
import pyviennacl as p

from pyviennacl.backend import OpenCLMemory, Context
ctx = Context(OpenCLMemory)

import math,random

dtype = np.float32

size = %d
sparsity = %f
print(sparsity, math.ceil((size*size)*sparsity))
nnz = int(max(1, math.ceil((size*size)*sparsity)))
mod = nnz

x = np.random.rand(size).astype(dtype)
x = p.Vector(x, context=ctx)

values = np.array([], dtype=dtype)
max_size = 10**6
while mod > 0:
    if mod < max_size:
        values = np.append(values, np.random.rand(mod).astype(dtype))
        mod = 0
    else:
        values = np.append(values, np.random.rand(max_size).astype(dtype))
        mod -= max_size
data = []
while len(data) < nnz:
    r = np.random.randint(0, size-1)
    c = np.random.randint(0, size-1)
    if (r, c) not in data:
        data.append((r, c))
data = list(map(list, zip(*data)))
data.append(values)
del values
data = tuple(data)

# TODO: CompressedMatrix
A = p.CoordinateMatrix(data, shape=(size, size, nnz),
                       dtype=dtype, context=ctx)
A.flush()

for i in range(3):
    %s
"""

setup_dense_gnumpy = """import numpy as np
import gnumpy as gnp

dtype = np.float32

size = %d

A = np.random.rand(size,size).astype(dtype)
B = np.random.rand(size,size).astype(dtype)

A = gnp.garray(A)
B = gnp.garray(B)

x = np.random.rand(size).astype(dtype)
x = gnp.garray(x)

for i in range(3):
    %s
"""

setup_dense_numpy = """import numpy as np

dtype = np.float32

size = %d

A = np.random.rand(size,size).astype(dtype)
B = np.random.rand(size,size).astype(dtype)
x = np.random.rand(size).astype(dtype)

for i in range(3):
    %s
"""

setup_sparse_scipy = """import numpy as np
import scipy.sparse as sp
import math,random

dtype = np.float32

size = %d
sparsity = %f
nnz = int(math.ceil((size*size)*sparsity))
mod = nnz

x = np.random.rand(size).astype(dtype)

values = np.array([], dtype=dtype)
max_size = 10**6
while mod > 0:
    if mod < max_size:
        values = np.append(values, np.random.rand(mod).astype(dtype))
        mod = 0
    else:
        values = np.append(values, np.random.rand(max_size).astype(dtype))
        mod -= max_size
rows = np.random.randint(0, size-1, size=nnz)
cols = np.random.randint(0, size-1, size=nnz)

A = sp.coo_matrix((values, (rows, cols)), shape=(size, size), dtype=dtype)

for i in range(3):
    %s
"""

#
# Dense matrix elementwise addition
#

if ADD:

    if PYVIENNACL:
        print("Dense matrix elementwise addition -- PyViennaCL")
        stmt = "(A+B).execute()"
        do_benchmark(setup_dense_pyvcl, stmt, ADD_SIZES)
        print("")

    if NUMPY_SCIPY:
        print("Dense matrix elementwise addition -- NumPy")
        stmt = "C=A+B"
        do_benchmark(setup_dense_numpy, stmt, ADD_SIZES)
        print("")

    if WITH_CUDA and CUDA:
        print("Dense matrix elementwise addition -- gnumpy (CUDA)")
        do_benchmark(setup_dense_gnumpy, stmt, ADD_SIZES)
        print("")

#
# Dense matrix multiplication
#

if GEMM:

    if PYVIENNACL:
        print("Dense matrix multiplication -- PyViennaCL")
        stmt = "(A*B).execute()"
        do_benchmark(setup_dense_pyvcl, stmt, GEMM_SIZES)
        print("")

    if NUMPY_SCIPY:
        print("Dense matrix multiplication -- NumPy")
        stmt = "A.dot(B)"
        do_benchmark(setup_dense_numpy, stmt, GEMM_SIZES)
        print("")
    
    if WITH_CUDA and CUDA:
        print("Dense matrix multiplication -- gnumpy (CUDA)")
        do_benchmark(setup_dense_gnumpy, stmt, GEMM_SIZES)
        print("")


#
# Dense matrix-vector multiplication

if GEMV:

    if PYVIENNACL:
        print("Dense matrix-vector multiplication -- PyViennaCL")
        stmt = "(A*x).execute()"
        do_benchmark(setup_dense_pyvcl, stmt, GEMM_SIZES)
        print("")

    if NUMPY_SCIPY:
        print("Dense matrix-vector multiplication -- NumPy")
        stmt = "A.dot(x)"
        do_benchmark(setup_dense_numpy, stmt, GEMM_SIZES)
        print("")
    
    if WITH_CUDA and CUDA:
        print("Dense matrix-vector multiplication -- gnumpy (CUDA)")
        do_benchmark(setup_dense_gnumpy, stmt, GEMM_SIZES)
        print("")

#


#
# Sparse matrix-vector multiplication
#

if SPGEMV:

    if PYVIENNACL:
        print("Sparse matrix-vector multiplication -- PyViennaCL")
        print("Sparsity: %f" % SPARSITY)
        stmt = "(A*x).execute()"
        do_benchmark(setup_sparse_pyvcl, stmt, SPGEMV_SIZES, sparsity=SPARSITY)
        print("")

    if NUMPY_SCIPY:
        print("Sparse matrix-vector multiplication -- SciPy")
        print("Sparsity: %f" % SPARSITY)
        stmt = "A.dot(x)"
        do_benchmark(setup_sparse_scipy, stmt, SPGEMV_SIZES, sparsity=SPARSITY)
        print("")

    # TODO: sparse-vec mult (theano)

