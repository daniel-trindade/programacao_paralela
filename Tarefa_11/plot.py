import numpy as np
import matplotlib.pyplot as plt

# Define o tamanho da malha (igual ao do C!)
NX = 20
NY = 20
NZ = 20

# Carrega os dados
with open('saida3d.dat', 'r') as f:
    data = f.read().split()
    data = np.array(data, dtype=float)

# Reconstrói o array 3D
u = data.reshape((NX, NY, NZ))

# Escolhe uma fatia para visualizar
z_slice = NZ // 2  # Metade do domínio

# Pega a fatia
slice_2d = u[:, :, z_slice]

# Plota a fatia 2D como mapa de calor
plt.figure(figsize=(8,6))
plt.imshow(slice_2d, origin='lower', extent=[0, NX, 0, NY], cmap='viridis')
plt.colorbar(label='Velocidade')
plt.title(f'Fatia em z = {z_slice}')
plt.xlabel('x')
plt.ylabel('y')
plt.show()
