import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from mpl_toolkits.mplot3d import Axes3D
import glob

NX, NY, NZ = 20, 20, 20
files = sorted(glob.glob("data/saida3d_*.dat"))

def load_data(filename):
    with open(filename, 'r') as f:
        data = f.read().split()
    return np.array(data, dtype=float).reshape((NX, NY, NZ))

# Cria malha de coordenadas
x = np.arange(NX)
y = np.arange(NY)
z = np.arange(NZ)
X, Y = np.meshgrid(x, y)

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
surf = [None]

def update(frame_idx):
    ax.clear()
    u = load_data(files[frame_idx])
    ax.set_title(f"Passo {frame_idx * 100}")

    # Plotando múltiplas fatias XY ao longo de Z
    for k in range(0, NZ, 4):  # Fatias espaçadas
        ax.plot_surface(X, Y, k + 0*X, facecolors=plt.cm.hot(u[:, :, k]/u.max()), rstride=1, cstride=1, shade=False)

    ax.set_xlim(0, NX)
    ax.set_ylim(0, NY)
    ax.set_zlim(0, NZ)
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")

    return []

ani = animation.FuncAnimation(fig, update, frames=len(files), interval=200)
plt.show()
