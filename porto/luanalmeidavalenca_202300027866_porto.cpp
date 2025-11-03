#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;

struct Container {
    string codigo;
    string cnpj;
    int peso;
};

struct Divergencia {
    string codigo;
    string info;
    int prioridade;
    double diff;
};

struct HashItem {
    string codigo;
    string cnpj;
    int peso;
    bool ocupado;
};

int hashFunc(const string& codigo, int tamanho) {
    unsigned long h = 5381;
    for (char c : codigo)
        h = ((h << 5) + h) + c;
    return h % tamanho;
}

void inserirHash(HashItem* tabela, int tamanho, const Container& c) {
    int idx = hashFunc(c.codigo, tamanho);
    while (tabela[idx].ocupado) {
        idx = (idx + 1) % tamanho;
    }
    tabela[idx].codigo = c.codigo;
    tabela[idx].cnpj = c.cnpj;
    tabela[idx].peso = c.peso;
    tabela[idx].ocupado = true;
}

bool buscarHash(HashItem* tabela, int tamanho, const string& codigo, Container& resultado) {
    int idx = hashFunc(codigo, tamanho);
    int inicio = idx;
    while (tabela[idx].ocupado) {
        if (tabela[idx].codigo == codigo) {
            resultado.codigo = tabela[idx].codigo;
            resultado.cnpj = tabela[idx].cnpj;
            resultado.peso = tabela[idx].peso;
            return true;
        }
        idx = (idx + 1) % tamanho;
        if (idx == inicio) break;
    }
    return false;
}

void merge(Divergencia arr[], int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;
    Divergencia* L = new Divergencia[n1];
    Divergencia* R = new Divergencia[n2];
    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int j = 0; j < n2; j++) R[j] = arr[m + 1 + j];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        bool menor = false;
        if (L[i].prioridade < R[j].prioridade) menor = true;
        else if (L[i].prioridade == R[j].prioridade && L[i].diff > R[j].diff) menor = true;
        if (menor) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    delete[] L;
    delete[] R;
}

void mergeSort(Divergencia arr[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

int main(int argc, char* argv[]) {
    ifstream input(argv[1]);
    ofstream output(argv[2]);
    if (!input || !output) return 1;

    int n1, n2;
    input >> n1;
    Container* cadastro = new Container[n1];
    for (int i = 0; i < n1; i++)
        input >> cadastro[i].codigo >> cadastro[i].cnpj >> cadastro[i].peso;

    input >> n2;
    Container* inspecionados = new Container[n2];
    for (int i = 0; i < n2; i++)
        input >> inspecionados[i].codigo >> inspecionados[i].cnpj >> inspecionados[i].peso;

    int tamanhoHash = n2 * 2 + 1;
    HashItem* tabela = new HashItem[tamanhoHash];
    for (int i = 0; i < tamanhoHash; i++) tabela[i].ocupado = false;
    for (int i = 0; i < n2; i++) inserirHash(tabela, tamanhoHash, inspecionados[i]);

    Divergencia* diverg = new Divergencia[n1];
    int count = 0;

    for (int i = 0; i < n1; i++) {
        Container achado;
        if (buscarHash(tabela, tamanhoHash, cadastro[i].codigo, achado)) {
            if (cadastro[i].cnpj != achado.cnpj) {
                diverg[count].codigo = cadastro[i].codigo;
                diverg[count].info = cadastro[i].cnpj + "<->" + achado.cnpj;
                diverg[count].prioridade = 1;
                diverg[count].diff = 0;
                count++;
            } else {
                double diff = fabs(cadastro[i].peso - achado.peso);
                double perc = (diff / cadastro[i].peso) * 100.0;
                if (perc > 10.0) {
                    diverg[count].codigo = cadastro[i].codigo;
                    diverg[count].info = to_string((int)diff) + "kg(" + to_string((int)round(perc)) + "%)";
                    diverg[count].prioridade = 2;
                    diverg[count].diff = perc;
                    count++;
                }
            }
        }
    }

    mergeSort(diverg, 0, count - 1);

    for (int i = 0; i < count; i++)
        output << diverg[i].codigo << ":" << diverg[i].info << "\n";

    delete[] cadastro;
    delete[] inspecionados;
    delete[] diverg;
    delete[] tabela;
    input.close();
    output.close();
    return 0;
}