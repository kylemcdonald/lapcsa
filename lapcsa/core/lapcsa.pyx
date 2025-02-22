import numpy as np
from scipy.spatial.distance import cdist

cdef extern from "csa.c":
    double csa(double* cost, int n, unsigned int* lhs_sol, unsigned int* rhs_sol)

# what about nogil?
# what about avoiding memory copies?
# need to clean up after all the mallocs somehow
def lapcsa(lhs, rhs, scale_factor=10.0):
    dim = len(lhs)
    print('Calculating distances...')
    dist = cdist(lhs, rhs, 'euclidean')
    print('Done.')
    lhs_sol = np.ndarray(dim, dtype=np.uint32)
    rhs_sol = np.ndarray(dim, dtype=np.uint32)
    cdef double[:,::1] dist_view = dist
    cdef unsigned int[::1] lhs_sol_view = lhs_sol
    cdef unsigned int[::1] rhs_sol_view = rhs_sol
    cost_sol = csa(&dist_view[0,0], dist.shape[0], &lhs_sol_view[0], &rhs_sol_view[0], scale_factor)
    return cost_sol, lhs_sol, rhs_sol