#!/usr/bin/env python3
"""
Script para análise gráfica dos resultados do benchmark MPI ping-pong.
Identifica regimes de latência vs largura de banda.
"""

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import sys
from scipy import stats
import argparse

def load_data(filename):
    """Carrega os dados do arquivo de resultados."""
    try:
        # Lê o arquivo, ignorando linhas de comentário
        data = pd.read_csv(filename, sep='\t', comment='#', 
                          names=['size_bytes', 'latency_us', 'bandwidth_mbps', 'total_time_us'])
        return data
    except Exception as e:
        print(f"Erro ao carregar dados: {e}")
        return None

def identify_regimes(data):
    """Identifica os regimes de latência e largura de banda."""
    
    # Encontra o ponto de transição aproximado
    # Regime de latência: largura de banda cresce linearmente com o tamanho
    # Regime de largura de banda: largura de banda se estabiliza
    
    sizes = data['size_bytes'].values
    bandwidths = data['bandwidth_mbps'].values
    
    # Calcula a derivada da largura de banda
    bandwidth_derivative = np.gradient(bandwidths, sizes)
    
    # Encontra onde a derivada começa a diminuir significativamente
    # (transição do regime de latência para largura de banda)
    
    # Suaviza a derivada para reduzir ruído
    from scipy.ndimage import gaussian_filter1d
    smooth_derivative = gaussian_filter1d(bandwidth_derivative, sigma=1)
    
    # Encontra o ponto onde a derivada cai abaixo de um threshold
    max_derivative = np.max(smooth_derivative)
    threshold = max_derivative * 0.1  # 10% do máximo
    
    transition_idx = None
    for i in range(len(smooth_derivative)):
        if smooth_derivative[i] < threshold and i > len(smooth_derivative) // 4:
            transition_idx = i
            break
    
    if transition_idx is None:
        transition_idx = len(sizes) // 2  # Fallback
    
    transition_size = sizes[transition_idx]
    
    return transition_size, transition_idx

def plot_analysis(data, output_prefix='mpi_analysis'):
    """Cria gráficos de análise dos resultados."""
    
    sizes = data['size_bytes'].values
    latencies = data['latency_us'].values
    bandwidths = data['bandwidth_mbps'].values
    
    # Identifica regimes
    transition_size, transition_idx = identify_regimes(data)
    
    # Configuração de estilo
    plt.style.use('seaborn-v0_8')
    plt.rcParams.update({'font.size': 12})
    
    # Figura 1: Latência vs Tamanho da Mensagem
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10))
    
    # Gráfico de latência
    ax1.loglog(sizes, latencies, 'bo-', linewidth=2, markersize=6, label='Latência medida')
    ax1.axvline(x=transition_size, color='red', linestyle='--', 
                label=f'Transição (~{transition_size} bytes)')
    
    # Regime de latência (antes da transição)
    lat_regime_sizes = sizes[:transition_idx+1]
    lat_regime_latencies = latencies[:transition_idx+1]
    if len(lat_regime_sizes) > 2:
        # Ajuste linear em escala log
        log_sizes = np.log10(lat_regime_sizes)
        log_latencies = np.log10(lat_regime_latencies)
        slope, intercept, r_value, _, _ = stats.linregress(log_sizes, log_latencies)
        
        fit_line = 10**(slope * np.log10(lat_regime_sizes) + intercept)
        ax1.loglog(lat_regime_sizes, fit_line, 'r--', linewidth=2, alpha=0.7,
                   label=f'Regime latência (slope={slope:.2f})')
    
    ax1.set_xlabel('Tamanho da Mensagem (bytes)')
    ax1.set_ylabel('Latência (μs)')
    ax1.set_title('Latência vs Tamanho da Mensagem')
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    
    # Gráfico de largura de banda
    ax2.semilogx(sizes, bandwidths, 'go-', linewidth=2, markersize=6, label='Largura de banda')
    ax2.axvline(x=transition_size, color='red', linestyle='--', 
                label=f'Transição (~{transition_size} bytes)')
    
    # Marca os regimes
    ax2.axvspan(sizes[0], transition_size, alpha=0.2, color='blue', 
                label='Regime dominado por latência')
    ax2.axvspan(transition_size, sizes[-1], alpha=0.2, color='green', 
                label='Regime dominado por largura de banda')
    
    # Largura de banda máxima sustentada
    max_bandwidth = np.max(bandwidths[-len(bandwidths)//4:])  # Últimos 25% dos pontos
    ax2.axhline(y=max_bandwidth, color='orange', linestyle=':', 
                label=f'Largura de banda máxima (~{max_bandwidth:.1f} MB/s)')
    
    ax2.set_xlabel('Tamanho da Mensagem (bytes)')
    ax2.set_ylabel('Largura de Banda (MB/s)')
    ax2.set_title('Largura de Banda vs Tamanho da Mensagem')
    ax2.grid(True, alpha=0.3)
    ax2.legend()
    
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_latency_bandwidth.png', dpi=300, bbox_inches='tight')
    plt.show()
    
    # Figura 2: Análise de eficiência
    fig, ax = plt.subplots(1, 1, figsize=(10, 6))
    
    # Eficiência = largura_banda_atual / largura_banda_máxima
    efficiency = bandwidths / max_bandwidth * 100
    
    ax.semilogx(sizes, efficiency, 'mo-', linewidth=2, markersize=6)
    ax.axvline(x=transition_size, color='red', linestyle='--', 
               label=f'Transição (~{transition_size} bytes)')
    ax.axhline(y=90, color='green', linestyle=':', alpha=0.7, label='90% eficiência')
    ax.axhline(y=50, color='orange', linestyle=':', alpha=0.7, label='50% eficiência')
    
    ax.set_xlabel('Tamanho da Mensagem (bytes)')
    ax.set_ylabel('Eficiência da Largura de Banda (%)')
    ax.set_title('Eficiência da Comunicação vs Tamanho da Mensagem')
    ax.grid(True, alpha=0.3)
    ax.legend()
    ax.set_ylim(0, 105)
    
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_efficiency.png', dpi=300, bbox_inches='tight')
    plt.show()
    
    return transition_size, max_bandwidth

def print_analysis_summary(data, transition_size, max_bandwidth):
    """Imprime um resumo da análise."""
    
    print("\n" + "="*60)
    print("RESUMO DA ANÁLISE MPI PING-PONG")
    print("="*60)
    
    sizes = data['size_bytes'].values
    latencies = data['latency_us'].values
    bandwidths = data['bandwidth_mbps'].values
    
    # Latência mínima (mensagens pequenas)
    min_latency = np.min(latencies)
    min_latency_size = sizes[np.argmin(latencies)]
    
    print(f"Latência mínima: {min_latency:.2f} μs (tamanho: {min_latency_size} bytes)")
    print(f"Largura de banda máxima: {max_bandwidth:.2f} MB/s")
    print(f"Ponto de transição: ~{transition_size} bytes")
    
    # Regime de latência
    lat_regime = data[data['size_bytes'] <= transition_size]
    print(f"\nREGIME DOMINADO POR LATÊNCIA (≤ {transition_size} bytes):")
    print(f"  - Faixa de latência: {lat_regime['latency_us'].min():.2f} - {lat_regime['latency_us'].max():.2f} μs")
    print(f"  - Largura de banda cresce de {lat_regime['bandwidth_mbps'].min():.2f} a {lat_regime['bandwidth_mbps'].max():.2f} MB/s")
    print(f"  - Neste regime, reduzir latência é mais importante que aumentar largura de banda")
    
    # Regime de largura de banda
    bw_regime = data[data['size_bytes'] > transition_size]
    if len(bw_regime) > 0:
        print(f"\nREGIME DOMINADO POR LARGURA DE BANDA (> {transition_size} bytes):")
        print(f"  - Largura de banda estabiliza em ~{bw_regime['bandwidth_mbps'].mean():.2f} MB/s")
        print(f"  - Variação de largura de banda: ±{bw_regime['bandwidth_mbps'].std():.2f} MB/s")
        print(f"  - Neste regime, maximizar largura de banda é mais importante")
    
    # Recomendações
    print(f"\nRECOMENDAÇÕES:")
    print(f"  - Para mensagens < {transition_size} bytes: otimize para baixa latência")
    print(f"  - Para mensagens > {transition_size} bytes: otimize para alta largura de banda")
    print(f"  - Considere usar comunicação assíncrona para mensagens grandes")
    print(f"  - Agrupe mensagens pequenas quando possível")

def main():
    parser = argparse.ArgumentParser(description='Analisa resultados do benchmark MPI ping-pong')
    parser.add_argument('input_file', help='Arquivo de dados de entrada')
    parser.add_argument('-o', '--output', default='mpi_analysis', 
                       help='Prefixo para arquivos de saída')
    
    args = parser.parse_args()
    
    # Carrega os dados
    data = load_data(args.input_file)
    if data is None:
        return 1
    
    print(f"Carregados {len(data)} pontos de dados.")
    print(f"Faixa de tamanhos: {data['size_bytes'].min()} - {data['size_bytes'].max()} bytes")
    
    # Análise gráfica
    transition_size, max_bandwidth = plot_analysis(data, args.output)
    
    # Resumo da análise
    print_analysis_summary(data, transition_size, max_bandwidth)
    
    return 0

if __name__ == '__main__':
    sys.exit(main())