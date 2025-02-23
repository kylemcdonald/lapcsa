import numpy as np
from scipy.spatial.distance import cdist
import time

cdef extern from "csa.c" nogil:
    double csa(double* cost, int n, unsigned int* lhs_sol, unsigned int* rhs_sol, double default_scale_factor, double min_epsilon_factor)

# need to clean up after all the mallocs somehow
def lap(double[:,::1] cost_matrix, scale_factor=10.0, min_epsilon_factor=2.0):
    if cost_matrix.shape[0] != cost_matrix.shape[1]:
        raise ValueError("Cost matrix must be square")
    dim = cost_matrix.shape[0]
    lhs_sol = np.empty(dim, dtype=np.uint32)
    rhs_sol = np.empty(dim, dtype=np.uint32)
    cdef double sf = scale_factor
    cdef double mef = min_epsilon_factor
    cdef unsigned int[::1] lhs_sol_view = lhs_sol
    cdef unsigned int[::1] rhs_sol_view = rhs_sol
    cdef double cost_sol
    cdef int n = dim
    with nogil:
        cost_sol = csa(&cost_matrix[0,0], n, &lhs_sol_view[0], &rhs_sol_view[0], sf, mef)
    return cost_sol, lhs_sol, rhs_sol

def ebm(lhs, rhs, scale_factor=10.0, min_epsilon_factor=2.0, verbose=False):
    if verbose:
        print('Calculating distances...')
    start_time = time.time()
    dist = np.ascontiguousarray(cdist(lhs, rhs, 'euclidean'))
    calc_time = time.time() - start_time
    if verbose:
        print(f'Done in {calc_time:.2f} seconds')
    return lap(dist, scale_factor, min_epsilon_factor)