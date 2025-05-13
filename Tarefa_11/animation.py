import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import glob

NX, NY, NZ = 20, 20, 20
z_slice = NZ // 2

def load_data(filename):
    with open(filename, 'r') as f:
        data = f.read().split()
    data = np.array([float(val) for val in data])
    return data.reshape((NX, NY, NZ))

# Lista de arquivos ordenados
files = sorted(glob.glob("data/saida3d_*.dat"))

fig, ax = plt.subplots()
frame = ax.imshow(np.zeros((NX, NY)), cmap='hot', origin='lower')
plt.colorbar(frame)

def update(i):
    u = load_data(files[i])
    frame.set_data(u[:, :, z_slice])
    ax.set_title(f'Passo {i}')
    return [frame]

ani = animation.FuncAnimation(fig, update, frames=len(files), blit=True)
plt.show()
