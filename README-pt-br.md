<div align="right">
Leia isto em outros idiomas: <a href="README.md">English</a> 🇬🇧🇺🇸
</div>

# Simulador de Alocação de Recursos em Redes

![Linguagem](https://img.shields.io/badge/Linguagem-C%2B%2B-blue.svg)
![Padrão](https://img.shields.io/badge/C%2B%2B-17-blue.svg)

Uma estrutura de simulação em C++ para otimizar a alocação de recursos em uma rede de dispositivos, servidores de borda (edge) e servidores em nuvem (cloud). Este projeto avalia diferentes estratégias, desde modelos matemáticos a meta-heurísticas, para minimizar os custos operacionais enquanto garante a qualidade do serviço.

## Principais Funcionalidades

-   **Geração de Dados:** Gera dinamicamente conjuntos de dados para dispositivos, serviços e servidores (Edge e Cloud).
-   **Fase de Pré-cálculo:** Realiza análises de rede, incluindo cobertura de dispositivos, latência e cálculos de tempo de resposta.
-   **Múltiplas Abordagens de Otimização:**
    -   **Modelo Matemático:** Implementa um modelo de Programação Linear Inteira (ILP) usando o IBM ILOG CPLEX para encontrar a solução ótima.
    -   **Heurísticas:** Inclui algoritmos rápidos de alocação como Aleatório (Random) e diversas variações do Guloso (Greedy).
    -   **Meta-heurísticas:** Utiliza *Simulated Annealing* (SA) para encontrar soluções próximas da ótima em um tempo razoável.
-   **Métricas Detalhadas:** Coleta e salva métricas abrangentes para cada execução da simulação, permitindo uma análise de desempenho detalhada.

## Estrutura do Projeto

```
.
├── include/              # Arquivos de cabeçalho (.h)
├── src/                  # Arquivos de código-fonte (.cpp)
├── data/                 # Arquivos de dados base para geração
├── Results/              # Diretório de saída para os resultados (ignorado pelo git)
├── analysis/             # Arquivos de análise (ex: planilhas com gráficos)
├── build/                # Diretório de compilação (ignorado pelo git)
├── .gitignore            # Arquivo do Git para ignorar arquivos
└── README.md             # Este arquivo
```

## Pré-requisitos

Antes de começar, garanta que você possui os seguintes requisitos:

* **Compilador C++17:** Um compilador C++ moderno (como GCC ou Clang) com suporte ao padrão C++17.
* **CMake:** Versão 3.10 ou superior é recomendada para compilar o projeto.
* **IBM ILOG CPLEX:** Este projeto depende das bibliotecas de otimização do CPLEX. Você precisa ter o CPLEX instalado no seu sistema.
    * Garanta que as variáveis de ambiente do CPLEX (`CPLEX_DIR`, etc.) estão configuradas corretamente, ou que o instalador o integrou ao path do seu sistema.

## Como Compilar e Executar

1.  **Clone o repositório:**
    ```bash
    git clone https://github.com/MrBruninm/Edge-Cloud_Resource_Allocation_for_IoT.git
    cd Edge-Cloud_Resource_Allocation_for_IoT
    ```

2.  **Configure o projeto com o CMake:**
    ```bash
    cmake -S . -B build
    ```
    *Se o CMake não encontrar o CPLEX automaticamente, você talvez precise fornecer o caminho para a instalação.*

3.  **Compile o projeto:**
    ```bash
    cmake --build build
    ```

4.  **Execute a simulação:**
    O arquivo `main.cpp` está configurado para rodar um conjunto padrão de simulações. Para executá-las, rode:
    ```bash
    ./build/main_app
    ```

## Como Funciona

A simulação segue um processo claro e multifásico:

1.  **Carregamento e Geração de Dados:** O programa primeiro carrega ou gera os dados necessários para dispositivos e servidores.
2.  **Pré-cálculo:** Em seguida, determina quais dispositivos estão dentro da área de cobertura dos servidores de borda e pré-calcula métricas essenciais como tempos de conexão e processamento para todos os pares potenciais de dispositivo-servidor.
3.  **Execução:** O estado da simulação é passado para um dos algoritmos selecionados:
    * **Matemático:** Resolve o problema para a otimalidade usando o CPLEX.
    * **Heurístico:** Aplica um método rápido e baseado em regras para encontrar uma boa solução rapidamente.
    * **Meta-heurístico:** Começa com uma solução de uma heurística e a melhora iterativamente.
4.  **Resultados:** Após cada execução, um objeto `Metrics` é preenchido, exibido no console e salvo em um arquivo `.txt` no diretório `Results/`.

## Algoritmos Implementados

-   **Modelo Matemático:**
    -   `Minimize_Cost`: Um modelo ILP que minimiza os custos operacionais totais.
-   **Heurísticas:**
    -   `Random`: Aloca dispositivos a servidores disponíveis de forma aleatória.
    -   `Greedy`: Aloca dispositivos com base em listas ordenadas de dispositivos (por custo) e servidores (por tempo de resposta). Variações incluem `Greedy_AscAsc`, `Greedy_DescAsc`, etc.
-   **Meta-heurística:**
    -   `SA` (Simulated Annealing): Um método probabilístico para encontrar um ótimo global.