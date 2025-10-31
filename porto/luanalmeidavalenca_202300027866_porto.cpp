#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <cmath>

using namespace std;

struct Container {
    string codigo;
    string cnpj;
    int peso;
};

struct Divergencia {
    string codigo;
    string cnpj1;
    string cnpj2;
    int diffPeso;
    int perc;
    bool divergenciaCNPJ;
    int ordem;
};

// Função de leitura de arquivo
bool lerArquivo(
    const string& nomeArquivo,
    vector<Container>& cadastro,                    // mantém a ordem
    unordered_map<string, Container>& inspecionados // busca rápida
) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo) {
        cerr << "Erro ao abrir o arquivo: " << nomeArquivo << "\n";
        return false;
    }

    int n1, n2;
    if (!(arquivo >> n1)) return false;

    cadastro.reserve(n1);
    for (int i = 0; i < n1; i++) {
        Container r;
        arquivo >> r.codigo >> r.cnpj >> r.peso;
        cadastro.push_back(r);
    }

    if (!(arquivo >> n2)) return false;

    for (int i = 0; i < n2; i++) {
        Container r;
        arquivo >> r.codigo >> r.cnpj >> r.peso;
        inspecionados[r.codigo] = r;
    }

    arquivo.close();
    return true;
}

// Função MergeSort (estável)
void merge(vector<Divergencia>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<Divergencia> L(n1), R(n2);
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        bool cond = false;

        // Ordena primeiro por divergência de CNPJ
        if (L[i].divergenciaCNPJ != R[j].divergenciaCNPJ)
            cond = L[i].divergenciaCNPJ > R[j].divergenciaCNPJ;
        // Depois pela diferença percentual
        else if (L[i].perc != R[j].perc)
            cond = L[i].perc > R[j].perc;
        // Por último, pela ordem original
        else
            cond = L[i].ordem < R[j].ordem;

        if (cond)
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSort(vector<Divergencia>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

// Função principal
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " <entrada.txt> <saida.txt>\n";
        return 1;
    }

    vector<Container> cadastro;
    unordered_map<string, Container> inspecionados;

    if (!lerArquivo(argv[1], cadastro, inspecionados)) {
        cerr << "Erro na leitura do arquivo.\n";
        return 1;
    }

    vector<Divergencia> divergencias;
    divergencias.reserve(cadastro.size());

    for (size_t i = 0; i < cadastro.size(); ++i) {
        const auto& c1 = cadastro[i];
        if (inspecionados.find(c1.codigo) == inspecionados.end())
            continue;

        const auto& c2 = inspecionados[c1.codigo];

        bool cnpjDif = (c1.cnpj != c2.cnpj);
        int diff = abs(c2.peso - c1.peso);
        int perc = (int)round((diff * 100.0) / c1.peso);

        if (cnpjDif || perc > 10) {
            Divergencia d;
            d.codigo = c1.codigo;
            d.cnpj1 = c1.cnpj;
            d.cnpj2 = c2.cnpj;
            d.diffPeso = diff;
            d.perc = perc;
            d.divergenciaCNPJ = cnpjDif;
            d.ordem = (int)i;
            divergencias.push_back(d);
        }
    }

    // Ordena divergências
    mergeSort(divergencias, 0, (int)divergencias.size() - 1);

    // Escreve saída
    ofstream output(argv[2]);
    if (!output) {
        cerr << "Erro ao criar arquivo de saída.\n";
        return 1;
    }

    for (const auto& d : divergencias) {
        if (d.divergenciaCNPJ)
            output << d.codigo << ":" << d.cnpj1 << "<->" << d.cnpj2 << "\n";
        else
            output << d.codigo << ":" << d.diffPeso << "kg(" << d.perc << "%)\n";
    }

    output.close();
    return 0;
}
