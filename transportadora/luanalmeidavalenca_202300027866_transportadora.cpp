#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>

struct veiculo
{
    std::string placa;
    uint16_t max_peso;
    uint16_t max_volume;
};

struct pacote
{
    std::string id;
    uint16_t valor;
    uint16_t peso;
    uint16_t volume;
};

int main(int argc, char* argv[])
{
    ifstream input(argv[1]);
    ofstream output(argv[2]);
    if (!input || !output) return 1;


    return 0;
}