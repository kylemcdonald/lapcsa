import numpy as np
import lapcsa
import matplotlib.pyplot as plt

def generate_test_data(side):
    n = side * side
    
    # Generate source points using random walk
    source = np.random.uniform(low=-1, high=+1, size=(n, 2))
    source = np.cumsum(source, axis=0)
    source -= source.min(axis=0)
    source /= source.max(axis=0)
    
    # Generate target points as uniform grid
    x, y = np.meshgrid(np.linspace(0, 1, side), np.linspace(0, 1, side))
    target = np.dstack((x, y)).reshape(-1, 2)
    
    return source, target

def visualize_matching(source, target, matching):
    plt.figure(figsize=(10, 10))
    
    for i in range(len(source)):
        match_idx = matching[i]
        plt.arrow(source[i, 0], source[i, 1],
                 target[match_idx, 0] - source[i, 0],
                 target[match_idx, 1] - source[i, 1],
                 head_width=0.01, head_length=0.01,
                 fc='black', ec='none', alpha=0.5)
    
    plt.legend()
    plt.title('LAPCSA Matching Visualization')
    plt.savefig('matching.png')
    plt.close()

if __name__ == '__main__':
    source, target = generate_test_data(64)
    cost, lhs_sol, rhs_sol = lapcsa.ebm(source, target)
    print(f"Matching cost: {cost}")
    visualize_matching(source, target, lhs_sol)