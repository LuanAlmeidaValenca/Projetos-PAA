#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cmath>
#include <cstdlib>
using namespace std;

struct Resultado {
    char nome[3];
    int trocas;
    int chamadas;
    int total() const { return trocas + chamadas; }
};

// lomuto padrão
int partitionLomuto(int arr[], int low, int high, int &trocas) {
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(arr[i], arr[j]);
            trocas++;
        }
    }
    swap(arr[i + 1], arr[high]);
    trocas++;
    return i + 1;
}

void quicksortLomuto(int arr[], int low, int high, int &trocas, int &chamadas) {
    chamadas++;
    if (low < high) {
        int pi = partitionLomuto(arr, low, high, trocas);
        quicksortLomuto(arr, low, pi - 1, trocas, chamadas);
        quicksortLomuto(arr, pi + 1, high, trocas, chamadas);
    }
}

// lomuto mediana de 3
int medianaDe3(int a, int b, int c) {
    if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
    else if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
    else return c;
}

int partitionLomutoMediana(int arr[], int low, int high, int &trocas) {
    int mid = low + (high - low) / 2;
    int pivotValue = medianaDe3(arr[low], arr[mid], arr[high]);
    // Move o pivô para o final
    if (pivotValue == arr[mid]) swap(arr[mid], arr[high]);
    else if (pivotValue == arr[low]) swap(arr[low], arr[high]);
    trocas++;
    return partitionLomuto(arr, low, high, trocas);
}

void quicksortLomutoMediana(int arr[], int low, int high, int &trocas, int &chamadas) {
    chamadas++;
    if (low < high) {
        int pi = partitionLomutoMediana(arr, low, high, trocas);
        quicksortLomutoMediana(arr, low, pi - 1, trocas, chamadas);
        quicksortLomutoMediana(arr, pi + 1, high, trocas, chamadas);
    }
}

// lomuto aleatório
int partitionLomutoRandom(int arr[], int low, int high, int &trocas) {
    int randomIndex = low + rand() % (high - low + 1);
    swap(arr[randomIndex], arr[high]);
    trocas++;
    return partitionLomuto(arr, low, high, trocas);
}

void quicksortLomutoRandom(int arr[], int low, int high, int &trocas, int &chamadas) {
    chamadas++;
    if (low < high) {
        int pi = partitionLomutoRandom(arr, low, high, trocas);
        quicksortLomutoRandom(arr, low, pi - 1, trocas, chamadas);
        quicksortLomutoRandom(arr, pi + 1, high, trocas, chamadas);
    }
}

// hoare padrão
int partitionHoare(int arr[], int low, int high, int &trocas) {
    int pivot = arr[low];
    int i = low - 1, j = high + 1;
    while (true) {
        do { i++; } while (arr[i] < pivot);
        do { j--; } while (arr[j] > pivot);
        if (i >= j) return j;
        swap(arr[i], arr[j]);
        trocas++;
    }
}

void quicksortHoare(int arr[], int low, int high, int &trocas, int &chamadas) {
    chamadas++;
    if (low < high) {
        int p = partitionHoare(arr, low, high, trocas);
        quicksortHoare(arr, low, p, trocas, chamadas);
        quicksortHoare(arr, p + 1, high, trocas, chamadas);
    }
}

// hoare mediana de 3
int partitionHoareMediana(int arr[], int low, int high, int &trocas) {
    int mid = low + (high - low) / 2;
    int pivotValue = medianaDe3(arr[low], arr[mid], arr[high]);
    if (pivotValue == arr[mid]) swap(arr[mid], arr[low]);
    else if (pivotValue == arr[high]) swap(arr[high], arr[low]);
    trocas++;
    return partitionHoare(arr, low, high, trocas);
}

void quicksortHoareMediana(int arr[], int low, int high, int &trocas, int &chamadas) {
    chamadas++;
    if (low < high) {
        int p = partitionHoareMediana(arr, low, high, trocas);
        quicksortHoareMediana(arr, low, p, trocas, chamadas);
        quicksortHoareMediana(arr, p + 1, high, trocas, chamadas);
    }
}

// hoare aleatório
int partitionHoareRandom(int arr[], int low, int high, int &trocas) {
    int randomIndex = low + rand() % (high - low + 1);
    swap(arr[randomIndex], arr[low]);
    trocas++;
    return partitionHoare(arr, low, high, trocas);
}

void quicksortHoareRandom(int arr[], int low, int high, int &trocas, int &chamadas) {
    chamadas++;
    if (low < high) {
        int p = partitionHoareRandom(arr, low, high, trocas);
        quicksortHoareRandom(arr, low, p, trocas, chamadas);
        quicksortHoareRandom(arr, p + 1, high, trocas, chamadas);
    }
}


int main(int argc, char* argv[]) {
    ifstream input(argv[1]);
    ofstream output(argv[2]);
    if (!input || !output) return 1;

    srand(42);

    const char nomes[6][3] = {"LP","LM","LA","HP","HM","HA"};   

    int totalVetores;
    input >> totalVetores;

    for (int v = 0; v < totalVetores; v++) {
        int N;
        input >> N;
        int *dados = new int[N];
        for (int i = 0; i < N; i++)
            input >> dados[i];

        Resultado resultados[6];

        for (int i = 0; i < 6; i++) {
            strcpy(resultados[i].nome, nomes[i]);
            resultados[i].trocas = 0;
            resultados[i].chamadas = 0;

            int *copia = new int[N];
            memcpy(copia, dados, N * sizeof(int));

            switch (i) {
                case 0: quicksortLomuto(copia, 0, N - 1, resultados[i].trocas, resultados[i].chamadas); break;
                case 1: quicksortLomutoMediana(copia, 0, N - 1, resultados[i].trocas, resultados[i].chamadas); break;
                case 2: quicksortLomutoRandom(copia, 0, N - 1, resultados[i].trocas, resultados[i].chamadas); break;
                case 3: quicksortHoare(copia, 0, N - 1, resultados[i].trocas, resultados[i].chamadas); break;
                case 4: quicksortHoareMediana(copia, 0, N - 1, resultados[i].trocas, resultados[i].chamadas); break;
                case 5: quicksortHoareRandom(copia, 0, N - 1, resultados[i].trocas, resultados[i].chamadas); break;
            }

            delete[] copia;
        }

        output << "[" << N << "]:";
        for (int i = 0; i < 6; i++) {
            output << resultados[i].nome << "(" << resultados[i].total() << ")";
            if (i < 5) output << ",";
        }
        output << "\n";

        delete[] dados;
    }

    return 0;
}