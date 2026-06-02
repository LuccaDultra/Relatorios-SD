/*
    Implementação do algoritmo de Quine-McCluskey com interpretador de PLA (Programmable Logic Array)
*/

// bibliotecas usadas no projeto
#include <iostream>             // permite lidar com entrada e saída (std::cout e std::cin)
#include <fstream>              // permite lidar com leitura de arquivos já criados
#include <filesystem>           // permite lidar com escrita, exclusão e criação de arquivos
#include <vector>               // lista de elementos
#include <set>                  // lista de elementos ordenados e únicos
#include <string>               // lista de caracteres
#include <algorithm>            // permite lidar com algoritmos já prontos



// Representa a estrutura de um termo lógico / Implicante
struct Implicant {
    std::string bits;            // representação binária do implicante. Lembramdo que o caractere '-' don't care.
    std::vector<int> mintermos;  // lista de mintermos cobertos por este implicante.
    bool usado = false;          // indica se o implicante já foi combinado
};


// função que transforma um valor inteiro (int) em bits com n casas
// exemplo: 2, em binário, é escrito como '10' usando duas casas, mas com n = 3, 2 em binário passa a ser escrito como 010 
std::string toBits(int val, int n) {
    std::string str(n, '0');
    for (int i = n - 1; i >= 0; i--) {
        str[i] = '0' + (val & 1);
        val >>= 1;
    }
    return str;
}


// countOne(te) kk
// Conta quantos números '1' existem na string passada como parâmetro para a função
int countOne(const std::string& str) {
    int count = 0;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '1') {
            count++;
        }
    }
    return count;
}


// Verifica se, no vetor passado como primeiro parâmetro, há o número inteiro fornecido no segundo parâmetro da função.
// No algoritmo de Quine-McCluskey, o mintermo representa a(s) linha(s) que da tabela verdade em que a saída é 1.
bool temMintermo(const std::vector<int>& vector, int mintermo) {
    for (size_t i = 0; i < vector.size(); i++) {
        if (vector[i] == mintermo) {
            return true;
        }
    }
    return false;
}


// função responsável por tentar combinar dois implicantes. A combinação só é possível se eles diferirem em apenas 1 único bit.
bool tryMerge(const Implicant& input_a, const Implicant& input_b, Implicant& output) {
    
    int differentBits = 0;
    std::string merged = input_a.bits;

    // compara os bits dos implicantes
    for (size_t i = 0; i < input_a.bits.size(); i++) {
        if (input_a.bits[i] != input_b.bits[i]) {

            // impede combinações inválidas que envolvam don't cares: "-"
            if (input_a.bits[i] == '-' || input_b.bits[i] == '-') {
                return false;
            }

            differentBits++;
            merged[i] = '-';
        }
    }

    // Só permite a combinação se houver uma única diferença de um único bit
    if (differentBits != 1) {
        return false;
    }

    output.bits = merged;                   // armazena os bits do novo implicante
    output.mintermos = input_a.mintermos;   // copia os mintermos do primeiro implicante


    // junta os mintermos dos dois implicantes
    for (size_t i = 0; i < input_b.mintermos.size(); i++) {
        if (!temMintermo(output.mintermos, input_b.mintermos[i])) {
            output.mintermos.push_back(input_b.mintermos[i]);
        }
    }
    return true;
}


// interpretadro do PLA: função é responsável por ler o arquivo PLA e extrair os mintermos cujo valor de saída é 1
// a função recebe o caminho para o arquivo .pla (path), a quantidade de entradas e o vetor onde os mintermos serão armazenados
void readPLA(const std::string& path, int& num_in, std::vector<int>& on_set) {

    // identificando o arquivo e, caso não seja possível ler, fecha o programa automaticamente informando o erro
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error: Não foi possível abrir o arquivo " << path << "\n";
        exit(EXIT_FAILURE);
    }

    // lê linha por linha do arquivo
    std::string line;
    while (std::getline(file, line)) {

        // ignora comentários: começam com "#" e linhas vazias
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // lê comandos do PLA :: interpretador
        if (line[0] == '.') {
            if (line.substr(0, 2) == ".i") {
                num_in = std::stoi(line.substr(3)); // qual o número que está após o '.i '?

                // se o número após o .i está vazio ou com valor inválido, então...
                if(num_in <= 0){
                    std::cerr << "Error (.i): O número de entradas precisa ser um número inteiro maior que 0\n";
                    exit(EXIT_FAILURE);
                }

            }
            continue;
        }

        // obtem os bits de entrada e o valor da saida
        std::string in_bits = line.substr(0, num_in);
        char out = line[num_in + 1];
        if (out != '1') {
            continue;
        }

        // converte os valores de entrada em mintermos
        std::vector<int> vals = {0};
        for (int i = 0; i < (int)in_bits.size(); i++) {
            char c = in_bits[i];
            std::vector<int> next;
            for (size_t j = 0; j < vals.size(); j++) {
                if (c == '-') {
                    next.push_back(vals[j] * 2);
                    next.push_back(vals[j] * 2 + 1);
                } else {
                    next.push_back(vals[j] * 2 + (c - '0'));
                }
            }
            vals = next;
        }

        // adiciona os mintermos ao conjunto de entradas em que a função booleana vale 1: on_set
        for (size_t i = 0; i < vals.size(); i++) {
            on_set.push_back(vals[i]);
        }
    }
}


// responsável por retornar todos os implicantes primos da função booleana 
// um implicante primo significa dizer que o mesmo não pode ser simplificado ou não foi possível combinar com nenhum outro
std::vector<Implicant> getPrimeImplicants(const std::vector<int>& on_set, int n) {

    // pega os mintermos puros e transforma em implicantes binários
    std::vector<Implicant> current;

    for (size_t i = 0; i < on_set.size(); i++) {
        Implicant implicant;

        implicant.bits = toBits(on_set[i], n);
        implicant.mintermos = {on_set[i]};

        current.push_back(implicant);
    }

    // armazena os primos implicantes encontrados durante cada ciclo
    std::vector<Implicant> primes;

    // loop responsável por ordernar os ciclos. Continua processando enquanto há implicantes para processar
    // tenta combinar os implicantes e gerar a próxima rodada simplificada (Não fiz de maneira funcional, mas acredito que daria para fazer usando recursão)
    while (!current.empty()) {
        std::vector<Implicant> next;                        // armazena os novos implicantes simplificados
        std::vector<bool> used(current.size(), false);      // indica quais implicantes já participaram de combinações


        // escolhe o primeiro implicante e compara com os próximos logo em seguida
        for (size_t i = 0; i < current.size(); i++) {
            for (size_t j = i + 1; j < current.size(); j++) {
                // evita comparações inúteis: Só aceita diferença de 1 bit
                if (abs(countOne(current[i].bits) - countOne(current[j].bits)) != 1) {
                    continue;
                }

                // tenta combinar os implicantes 
                Implicant merged;
                if (tryMerge(current[i], current[j], merged)) {
                    used[i] = used[j] = true;
                    bool duplicate = false;

                    for (size_t k = 0; k < next.size(); k++) {
                        if (next[k].bits == merged.bits) {
                            duplicate = true;
                            break;
                        }
                    }

                    // adiciona o novo implicante caso ele não seja duplicado
                    if (!duplicate) {
                        next.push_back(merged);
                    }
                }
            }
        }

        // percorre todos os implicantes do ciclo atual e, caso não tenha conseguido ser usado, coloca na lista de implicantes primos (prime)  
        for (size_t i = 0; i < current.size(); i++) {
            if (!used[i]) {
                primes.push_back(current[i]);
            }
        }

        // ir para o próximo ciclo
        current = next;
    }

    // retorna os implicantes primos (prime implicants) no final desta etapa
    return primes;
}


// função responsável por escolher quais implicantes primos realmente serão usados na expressão final
std::vector<Implicant> selectCover(const std::vector<Implicant>& primes, const std::vector<int>& on_set) {
    std::set<int> covered;                                  // lista que guarda, de maneira ordenada e com elementos únicos, quais mintermos já estão cobertos
    std::vector<bool> selected(primes.size(), false);       // lista que marca quais implicantes primos já foram resolvidos

    // pecorre todos os mintermos para identificar os implicantes essenciais (aqueles que possuem apenas 1 único implicante que cobre o mintermo)
    for (size_t i = 0; i < on_set.size(); i++) {

        int count = 0;
        int last = -1;

        // verifica quantos implicantes cobrem o mintermo atual.
        for (size_t j = 0; j < primes.size(); j++) {
            if (temMintermo(primes[j].mintermos, on_set[i])) {
                count++;
                last = j;
            }
        }

        if (count == 1 && !selected[last]) {
            selected[last] = true;
            for (size_t k = 0; k < primes[last].mintermos.size(); k++) {
                covered.insert(primes[last].mintermos[k]);
            }
        }
    }


    // cobre os mintermos restantes
    for (size_t i = 0; i < on_set.size(); i++) {
        if (covered.count(on_set[i])) {
            continue; // ignora os que já estão cobertos
        }

        int best = -1;
        int best_score = 0;


        // escolhe o implicante que cobre mais mintermos ainda não cobertos.
        for (size_t j = 0; j < primes.size(); j++) {
            if (selected[j]) {
                continue;
            }

            int score = 0;
            for (size_t k = 0; k < primes[j].mintermos.size(); k++) {
                if (!covered.count(primes[j].mintermos[k])) {
                    score++;
                }
            }

            // atualiza o melhor "candidato" implicante
            if (score > best_score) {
                best_score = score;
                best = j;
            }
        }

        // seleciona o melhor implicante encontrado
        if (best != -1) {
            selected[best] = true;

            // atualiza os mintermos cobertos
            for (size_t k = 0; k < primes[best].mintermos.size(); k++) {
                covered.insert(primes[best].mintermos[k]);
            }
        }
    }

    // monta o resultado final com os implicantes escolhidos.
    std::vector<Implicant> result;
    for (size_t i = 0; i < primes.size(); i++) {
        if (selected[i]) {
            result.push_back(primes[i]);
        }
    }
    return result;
}


// responsável pela formalização da resposta: transforma o resultado final do algoritmo em expressão booleana e em um arquivo PLA de saída
void printResult(const std::vector<Implicant>& cover, int n) {

    // garante que a pasta output existe
    std::filesystem::create_directory("output");

    std::ofstream file("output/result.pla");

    if (!file) {
        std::cerr << "Erro ao criar arquivo de saída\n";
        return;
    }

    // expressão booleana
    file << "\nF = ";
    bool first = true;

    for (size_t i = 0; i < cover.size(); i++) {

        if (!first) {
            file << " + ";
        }
        first = false;

        bool empty = true;

        for (int j = 0; j < n; j++) {

            if (cover[i].bits[j] == '1') {
                file << char('A' + j);
                empty = false;

            } else if (cover[i].bits[j] == '0') {
                file << char('A' + j) << "'";
                empty = false;
            }
        }

        if (empty) {
            file << "1";
        }
    }

    // PLA output
    file << "\n\nPLA output:\n.i " << n << "\n.o 1\n";

    for (size_t i = 0; i < cover.size(); i++) {
        file << cover[i].bits << " 1\n";
    }

    file << ".e\n";

    file.close();
}



// função principal: leitura do arquivo, processamento e saída do resultado.
int main(int argc, char* argv[]) {
    
    // verificando se o arquivo PLA foi fornecido como argumento
    if (argc < 2) {
        std::cerr << "Usage: ./qm <file.pla>\n";
        return EXIT_FAILURE;
    }


    int n = 0;
    std::vector<int> on_set;

    // lê o arquivo PLA e extrai numero de entradas e mintermos on_set
    readPLA(argv[1], n, on_set);

    // ordena e remove duplicata dos mintermos
    std::sort(on_set.begin(), on_set.end());
    on_set.erase(std::unique(on_set.begin(), on_set.end()), on_set.end());

    // exibe dados de entrada (debug)
    std::cout << "inputs: " << n << "\n";
    std::cout << "mintermos: ";

    for (size_t i = 0; i < on_set.size(); i++) {
        std::cout << on_set[i] << " ";
    }

    std::cout << "\n";


    // etapa 1: encontra os implicantes primos
    std::vector<Implicant> primes = getPrimeImplicants(on_set, n);

    // etapa 2: seleciona a cobertura mínima da função
    std::vector<Implicant> cover  = selectCover(primes, on_set);


    // mostra o resultado em PLA e fecha o programa adequadamente mostrando sucesso na aplicação
    printResult(cover, n);
    return EXIT_SUCCESS;
}