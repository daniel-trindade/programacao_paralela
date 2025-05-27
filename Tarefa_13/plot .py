import matplotlib.pyplot as plt
import pandas as pd
import re
import io

# Read the data from the file
# This path assumes the file 'true.txt' is uploaded in the current turn.
file_path = "/results/true.txt" # Standard path for uploaded files

try:
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
except FileNotFoundError:
    print(f"Erro: O arquivo {file_path} não foi encontrado. Por favor, faça o upload do arquivo 'true.txt' novamente.")
    # Exit or raise to prevent further execution if file not found
    raise

# Prepare data for DataFrame
parsed_data = []
current_matrix_size = None
pending_threads = None

for line in content.splitlines():
    line = line.strip()
    if not line:
        continue

    matrix_match = re.match(r"Matri[zx]=(.*)", line, re.IGNORECASE)
    if matrix_match:
        current_matrix_size = matrix_match.group(1).strip()
        pending_threads = None # Reset pending threads when matrix size changes
        continue

    threads_time_match = re.match(r"Threads: (\d+) \| Tempo de execução: ([\d\.]+) segundos", line, re.IGNORECASE)
    if threads_time_match and current_matrix_size:
        threads = int(threads_time_match.group(1))
        time = float(threads_time_match.group(2))
        parsed_data.append([current_matrix_size, threads, time])
        pending_threads = None
        continue

    threads_only_match = re.match(r"Threads: (\d+) \|", line, re.IGNORECASE)
    if threads_only_match and current_matrix_size:
        pending_threads = int(threads_only_match.group(1))
        continue

    time_only_match = re.search(r"Tempo de execução: ([\d\.]+) segundos", line, re.IGNORECASE)
    if time_only_match and current_matrix_size and pending_threads is not None:
        time = float(time_only_match.group(1))
        parsed_data.append([current_matrix_size, pending_threads, time])
        pending_threads = None
        continue

df = pd.DataFrame(parsed_data, columns=['Matrix_Size', 'Threads', 'Time'])

if df.empty:
    print("Erro: Nenhum dado foi extraído do arquivo. Verifique o formato do arquivo.")
else:
    print("Dados extraídos com sucesso. DataFrame inicial:")
    print(df.head())
    df.info()

    # Calculate Speedup
    df_list = []
    unique_matrix_sizes = df['Matrix_Size'].unique()

    for size in unique_matrix_sizes:
        subset = df[df['Matrix_Size'] == size].copy()
        subset.sort_values('Threads', inplace=True)
        
        t1_series = subset[subset['Threads'] == 1]['Time']
        if t1_series.empty:
            print(f"Aviso: Dados para T1 (1 thread) não encontrados para a matriz {size}. Cálculo de speedup para esta matriz será ignorado.")
            continue
        
        t1 = t1_series.iloc[0]
        subset['Speedup'] = t1 / subset['Time']
        df_list.append(subset)

    if not df_list:
        print("Erro: Não há dados disponíveis para o cálculo de speedup após o processamento. Verifique os dados de T1.")
    else:
        speedup_df = pd.concat(df_list)
        print("\nDataFrame com Speedup calculado:")
        print(speedup_df.head())
        speedup_df.info()

        # Save the processed data to a CSV file
        output_csv_path = "/mnt/data/speedup_data.csv"
        speedup_df.to_csv(output_csv_path, index=False)
        print(f"\nDados processados salvos em: {output_csv_path}")

        # Create the plot
        plt.figure(figsize=(12, 8))
        plot_matrix_sizes = sorted(speedup_df['Matrix_Size'].unique(), key=lambda x: int(x.split('x')[-1]))

        for size in plot_matrix_sizes:
            subset_plot = speedup_df[speedup_df['Matrix_Size'] == size]
            plt.plot(subset_plot['Threads'], subset_plot['Speedup'], marker='o', linestyle='-', label=f'Matriz {size}')

        plt.xlabel('Número de Threads')
        plt.ylabel('Speedup (<span class="math-inline">T\_1/T\_N</span>)')
        plt.title('Gráfico de Escalabilidade Forte')
        plt.xscale('log', base=2)
        
        unique_threads = sorted(speedup_df['Threads'].unique())
        plt.xticks(unique_threads, labels=[str(t) for t in unique_threads])
        
        if unique_threads: # Add ideal speedup line
            plt.plot(unique_threads, unique_threads, linestyle='--', color='gray', label='Speedup Ideal')

        plt.legend(title='Tamanho da Matriz')
        plt.grid(True, which="both", ls="--")
        plt.tight_layout()
        
        plot_image_path = "/mnt/data/escalabilidade_forte.png"
        plt.savefig(plot_image_path)
        # plt.show() # Not strictly necessary when saving to file
        print(f"\nGráfico gerado e salvo como: {plot_image_path}")
        print("Por favor, verifique os arquivos gerados.")