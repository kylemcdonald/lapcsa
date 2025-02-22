import numpy as np
import lapcsa

side = 128
n = side * side

lhs = np.random.uniform(low=-1, high=+1, size=(n, 2))
lhs = np.cumsum(lhs, axis=0)
lhs -= lhs.min(axis=0)
lhs /= lhs.max(axis=0)

xv, yv = np.meshgrid(np.linspace(0, 1, side), np.linspace(0, 1, side))
rhs = np.dstack((xv, yv)).reshape(-1, 2)

cost_csa, rows_csa, cols_csa = lapcsa.lapcsa(lhs, rhs)