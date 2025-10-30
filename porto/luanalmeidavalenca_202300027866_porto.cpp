#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct Container
{
    string codigo;
    string cnpj;
    int peso;
};

/* Entrada
6
QOZJ7913219 34.699.211/9365-11 13822
FCCU4584578 50.503.434/5731-28 16022
KTAJ0603546 20.500.522/6013-58 25279
ZYHU3978783 43.172.263/4442-14 24543
IKQZ7582839 51.743.446/1183-18 12160
HAAZ0273059 25.699.428/4746-79 16644
5
ZYHU3978783 43.172.263/4442-14 29765
IKQZ7582839 51.743.446/1113-18 18501
KTAJ0603546 20.500.522/6113-58 17842
QOZJ7913219 34.699.211/9365-11 16722
FCCU4584578 50.503.434/5731-28 16398
*/

vector<Container> c1;
unordered_map<string, int> c2;

bool lerArquivo(
    const string& nomeArquivo,
    vector<Container>& cadastro,                    // saída: mantém a ordem
    unordered_map<string, Container>& inspecionados // saída: busca rápida
) {
    ifstream arquivo(nomeArquivo);
    if (!arquivo) {
        cerr << "Erro ao abrir o arquivo: " << nomeArquivo << "\n";
        return false;
    }

    int n1, n2;

    // ======== BLOCO 1: CADASTRO ORIGINAL ========
    if (!(arquivo >> n1)) {
        cerr << "Erro ao ler quantidade do primeiro bloco.\n";
        return false;
    }

    cadastro.reserve(n1); // otimiza memória
    for (int i = 0; i < n1; i++) {
        Container r;
        arquivo >> r.codigo >> r.cnpj >> r.peso;
        if (!arquivo) {
            cerr << "Erro ao ler registro do primeiro bloco na linha " << (i + 1) << ".\n";
            return false;
        }
        cadastro.push_back(r);
    }

    // ======== BLOCO 2: INSPECIONADOS ========
    if (!(arquivo >> n2)) {
        cerr << "Erro ao ler quantidade do segundo bloco.\n";
        return false;
    }

    for (int i = 0; i < n2; i++) {
        Container r;
        arquivo >> r.codigo >> r.cnpj >> r.peso;
        if (!arquivo) {
            cerr << "Erro ao ler registro do segundo bloco na linha " << (i + 1) << ".\n";
            return false;
        }
        inspecionados[r.codigo] = r;
    }

    arquivo.close();
    return true;
}

int main(int argc, char* argv[]){
    ifstream input(argv[1]);
    ofstream output(argv[2]);



    return 0;
}