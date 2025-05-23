#!/bin/bash
#SBATCH --partition=intel-128
#SBATCH --job-name=mpi_pigpong
#SBATCH --nodes=1
#SBATCH --ntasks=2
#SBATCH --cpus-per-task=32
#SBATCH --time=00:30:00
#SBATCH --output=mpi_pingpong_%j.out
#SBATCH --error=mpi_pingpong_%j.err

# Carrega módulos necessários (ajuste conforme o sistema)
# module load openmpi/4.1.0
# module load gcc/9.3.0
# module load python/3.8.0

# Informações do job
echo "=== Informações do Job ==="
echo "Job ID: $SLURM_JOB_ID"
echo "Nós alocados: $SLURM_JOB_NODELIST"
echo "Número de tasks: $SLURM_NTASKS"
echo "Início: $(date)"
echo

# Informações do sistema
echo "=== Informações do Sistema ==="
echo "Hostname: $(hostname)"
echo "Arquitetura: $(uname -m)"
echo "Kernel: $(uname -r)"
echo "Compilador MPI: $(mpicc --version | head -1)"
echo

# Compilação
echo "=== Compilação ==="
make clean
make optimize-intel  # ou optimize-amd, dependendo da arquitetura
echo

# Verifica se a compilação foi bem-sucedida
if [ ! -f mpi_pingpong ]; then
    echo "ERRO: Falha na compilação!"
    exit 1
fi

# Informações sobre afinidade de CPU (importante para performance)
echo "=== Configuração de Execução ==="
echo "Binding de CPU: $SLURM_CPU_BIND"
echo "Afinidade de CPU:"
cat /proc/cpuinfo | grep "processor\|model name" | head -4

# Execução do benchmark
echo
echo "=== Executando Benchmark ==="
echo "Início da execução: $(date)"

# Executa com binding de CPU para melhor performance
mpirun --bind-to core --map-by core --report-bindings \
       ./mpi_pingpong > results_${SLURM_JOB_ID}.txt 2>&1

# Verifica se a execução foi bem-sucedida
if [ $? -eq 0 ]; then
    echo "Benchmark concluído com sucesso!"
    echo "Fim da execução: $(date)"
    
    # Mostra primeiras linhas do resultado
    echo
    echo "=== Primeiros Resultados ==="
    head -10 results_${SLURM_JOB_ID}.txt
    
    echo
    echo "=== Últimos Resultados ==="
    tail -5 results_${SLURM_JOB_ID}.txt
    
    # Estatísticas do arquivo
    echo
    echo "=== Estatísticas ==="
    wc -l results_${SLURM_JOB_ID}.txt
    
else
    echo "ERRO: Falha na execução do benchmark!"
    exit 1
fi

echo
echo "Arquivo de resultados: results_${SLURM_JOB_ID}.txt"
echo "Para análise, execute: python3 analyze_results.py results_${SLURM_JOB_ID}.txt"