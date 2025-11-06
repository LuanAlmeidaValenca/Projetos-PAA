#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cmath>
using namespace std;

struct Resultado {
    char nome[3];
    int trocas;
    int chamadas;
    int total() const { return trocas + chamadas; }
};

//quicksort lomuto padrão
int partition(Resultado arr[], int low, int high, int &trocas) {
    Resultado pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (arr[j].total() < pivot.total() || 
           (arr[j].total() == pivot.total() && strcmp(arr[j].nome, pivot.nome) < 0)) {
            i++;
            swap(arr[i], arr[j]);
            trocas++;
        }
    }
    swap(arr[i + 1], arr[high]);
    trocas++;
    return i + 1;
}

//quicksort lomuto por mediana de 3
int partitionMedian(Resultado arr[], int low, int high, int &trocas) {
    int mid = low + (high - low) / 2;
    // Ordena low, mid, high para encontrar a mediana
    if (arr[low].total() > arr[mid].total() || 
       (arr[low].total() == arr[mid].total() && strcmp(arr[low].nome, arr[mid].nome) > 0)) {
        swap(arr[low], arr[mid]);
        trocas++;
    }
    if (arr[low].total() > arr[high].total() || 
       (arr[low].total() == arr[high].total() && strcmp(arr[low].nome, arr[high].nome) > 0)) {
        swap(arr[low], arr[high]);
        trocas++;
    }
    if (arr[mid].total() > arr[high].total() || 
       (arr[mid].total() == arr[high].total() && strcmp(arr[mid].nome, arr[high].nome) > 0)) {
        swap(arr[mid], arr[high]);
        trocas++;
    }
    // Coloca a mediana no final
    swap(arr[mid], arr[high - 1]);
    trocas++;
    Resultado pivot = arr[high - 1];
    int i = low;
    for (int j = low; j < high - 1; j++) {
        if (arr[j].total() < pivot.total() || 
           (arr[j].total() == pivot.total() && strcmp(arr[j].nome, pivot.nome) < 0)) {
            swap(arr[i], arr[j]);
            trocas++;
            i++;
        }
    }
    swap(arr[i], arr[high - 1]);
    trocas++;
    return i;
}

//quicksort lomuto por pivo aleatório
int partitionRandom(Resultado arr[], int low, int high, int &trocas) {
    int randomIndex = low + rand() % (high - low + 1);
    swap(arr[randomIndex], arr[high]);
    trocas++;
    return partition(arr, low, high, trocas);
}

//quicksort hoare padrão
int partitionHoare(Resultado arr[], int low, int high, int &trocas) {
    Resultado pivot = arr[low];
    int i = low - 1;
    int j = high + 1;
    while (true) {
        do {
            i++;
        } while (arr[i].total() < pivot.total() || 
                (arr[i].total() == pivot.total() && strcmp(arr[i].nome, pivot.nome) < 0));
        do {
            j--;
        } while (arr[j].total() > pivot.total() || 
                (arr[j].total() == pivot.total() && strcmp(arr[j].nome, pivot.nome) > 0));
        if (i >= j) return j;
        swap(arr[i], arr[j]);
        trocas++;
    }
}

//quicksort hoare por mediana de 3
int partitionHoareMedian(Resultado arr[], int low, int high, int &trocas) {
    int mid = low + (high - low) / 2;
    // Ordena low, mid, high para encontrar a mediana
    if (arr[low].total() > arr[mid].total() || 
       (arr[low].total() == arr[mid].total() && strcmp(arr[low].nome, arr[mid].nome) > 0)) {
        swap(arr[low], arr[mid]);
        trocas++;
    }
    if (arr[low].total() > arr[high].total() || 
       (arr[low].total() == arr[high].total() && strcmp(arr[low].nome, arr[high].nome) > 0)) {
        swap(arr[low], arr[high]);
        trocas++;
    }
    if (arr[mid].total() > arr[high].total() || 
       (arr[mid].total() == arr[high].total() && strcmp(arr[mid].nome, arr[high].nome) > 0)) {
        swap(arr[mid], arr[high]);
        trocas++;
    }
    // Coloca a mediana no início
    swap(arr[mid], arr[low]);
    trocas++;
    return partitionHoare(arr, low, high, trocas);
}

//quicksort hoare por pivo aleatório
int partitionHoareRandom(Resultado arr[], int low, int high, int &trocas) {
    int randomIndex = low + rand() % (high - low + 1);
    swap(arr[randomIndex], arr[low]);
    trocas++;
    return partitionHoare(arr, low, high, trocas);
}

int main(int argc, char* argv[]) {
    ifstream input(argv[1]);
    ofstream output(argv[2]);
    if (!input || !output) return 1;

    int totalVetores;
    input >> totalVetores;   // lê o primeiro número (4)

    cout << totalVetores << endl;

    for (int v = 0; v < totalVetores; v++) {
        int N;
        input >> N;          // lê o próximo número após o último lido

        cout << N << endl;

        int *dados = new int[N];
        for (int i = 0; i < N; i++)
            input >> dados[i]; // lê N números sequenciais

        
    }
    return 0;
}