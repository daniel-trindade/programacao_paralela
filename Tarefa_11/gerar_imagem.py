import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import ImageGrid

def read_velocity_field(filename):
    """Lê o arquivo de saída 3D e retorna um array numpy (NX, NY, NZ)"""
    with open(filename, 'r') as f:
        lines = [line.strip() for line in f.readlines() if line.strip() != '']
    
    data = []
    i_block = []
    for line in lines:
        if len(i_block) < NY:
            row = list(map(float, line.split()))
            i_block.append(row)
        if len(i_block) == NY:
            data.append(i_block)
            i_block = []
    
    return np.array(data)

# Parâmetros
NX, NY, NZ = 20, 20, 20
k_slice = NZ // 2
steps = [0, 2500, 4900]
files = [f"data/saida3d_{step:04d}.dat" for step in steps]

# Configuração da figura com espaçamento ajustado
fig = plt.figure(figsize=(18, 6))  # Aumentei a largura para 18 polegadas
grid = ImageGrid(fig, 111, 
                 nrows_ncols=(1, 3),
                 axes_pad=0.7,      # Aumentei o espaçamento entre subplots
                 cbar_location="right",
                 cbar_mode="each",
                 cbar_size="5%",
                 cbar_pad=0.2)

for ax, step, file in zip(grid, steps, files):
    try:
        u = read_velocity_field(file)
        u_slice = u[:, :, k_slice]
        
        im = ax.imshow(u_slice.T, origin='lower', cmap='viridis',
                       extent=[0, NX*0.01, 0, NY*0.01])
        ax.set_title(f"Passo {step}\nZ = {k_slice*0.01:.2f}m", pad=20)
        ax.set_xlabel("X (m)", labelpad=10)
        ax.set_ylabel("Y (m)", labelpad=10)
        
        # Adiciona barra de cores individual para cada subplot
        cbar = plt.colorbar(im, cax=ax.cax, orientation='vertical')
        cbar.set_label("Velocidade (m/s)", labelpad=10)
        
    except FileNotFoundError:
        print(f"Arquivo {file} não encontrado.")

plt.suptitle("Evolução da Perturbação no Plano Central Z", y=1.05, fontsize=14)
plt.tight_layout(pad=3.0)  # Aumentei o padding global
plt.savefig("evolucao_fluido.png", bbox_inches='tight', dpi=300)
plt.show()