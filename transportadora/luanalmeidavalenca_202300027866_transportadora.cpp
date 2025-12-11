#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring> 
#include <new>     
#include <iomanip>

#define SET_BIT(array, idx)   (array[(idx) >> 3] |= (1 << ((idx) & 7)))
#define GET_BIT(array, idx)   (array[(idx) >> 3] & (1 << ((idx) & 7)))

struct Veiculo {
    char placa[10];
    uint32_t capPeso;
    uint32_t capVol;
};

struct Pacote {
    char codigo[16];
    float valor;    
    uint32_t peso;  
    uint32_t volume;
    bool disponivel;
};

// Otimização
void processarCarga(const Veiculo& caminhao, Pacote* pacotes, int total_pacotes, std::ofstream& output) {
    int32_t W = (int32_t)caminhao.capPeso;
    int32_t V = (int32_t)caminhao.capVol;

    // Alocação
    uint32_t strideW = (uint32_t)V + 1;
    uint32_t totalCelulas = (uint32_t)(W + 1) * strideW;

    float* dp = new(std::nothrow) float[totalCelulas];
    
    uint32_t tamanhoPlano = totalCelulas;
    uint32_t totalBits = (uint32_t)total_pacotes * tamanhoPlano;
    uint32_t tamanhoBytesHistory = (totalBits >> 3) + 1;

    uint8_t* history = new(std::nothrow) uint8_t[tamanhoBytesHistory];

    if (!dp || !history) {
        delete[] dp;
        delete[] history;
        return; 
    }

    std::memset(dp, 0, totalCelulas * sizeof(float));
    std::memset(history, 0, tamanhoBytesHistory);

    // Programação Dinâmica
    for (int i = 0; i < total_pacotes; ++i) {
        if (!pacotes[i].disponivel) continue;

        int32_t pPeso = (int32_t)pacotes[i].peso;
        int32_t pVol  = (int32_t)pacotes[i].volume;
        float pValor  = pacotes[i].valor;
        uint32_t offsetItem = (uint32_t)i * tamanhoPlano;

        for (int32_t w = W; w >= pPeso; --w) {
            uint32_t rowOffset = (uint32_t)w * strideW;
            uint32_t prevRowOffset = (uint32_t)(w - pPeso) * strideW;

            for (int32_t v = V; v >= pVol; --v) {
                uint32_t currIdx = rowOffset + v;
                uint32_t prevIdx = prevRowOffset + (v - pVol);

                float valorSem = dp[currIdx];
                float valorCom = dp[prevIdx] + pValor;

                if (valorCom > valorSem) {
                    dp[currIdx] = valorCom;
                    SET_BIT(history, offsetItem + currIdx);
                }
            }
        }
    }

    // Recuperação e Cálculos Finais
    int32_t* indicesEscolhidos = new(std::nothrow) int32_t[total_pacotes];
    int contagem = 0;
    
    int32_t wRestante = W;
    int32_t vRestante = V;
    float valorFinal = dp[totalCelulas - 1];

    for (int i = total_pacotes - 1; i >= 0; --i) {
        if (!pacotes[i].disponivel) continue;

        uint32_t idxPosicao = (uint32_t)wRestante * strideW + vRestante;
        uint32_t idxBit = (uint32_t)i * tamanhoPlano + idxPosicao;

        if (GET_BIT(history, idxBit)) {
            indicesEscolhidos[contagem++] = i;
            pacotes[i].disponivel = false; //Marca como carregado
            wRestante -= (int32_t)pacotes[i].peso;
            vRestante -= (int32_t)pacotes[i].volume;
        }
    }

    // Calcula usados
    int32_t pesoUsado = W - wRestante;
    int32_t volUsado = V - vRestante;
    
    //Calcula porcentagens (evita divisão por zero se capacidade for 0, o que não deve ocorrer)
    int percPeso = (caminhao.capPeso > 0) ? (pesoUsado * 100) / caminhao.capPeso : 0;
    int percVol  = (caminhao.capVol > 0) ? (volUsado * 100) / caminhao.capVol : 0;

    // Saída
    
    output << "[" << caminhao.placa << "]";
    output << "R$" << std::fixed << std::setprecision(2) << valorFinal << ",";
    output << std::defaultfloat; // Remove formatação de float para os inteiros
    output << pesoUsado << "KG(" << percPeso << "%),";
    output << volUsado << "L(" << percVol << "%)";
    output << "->";

    for (int k = 0; k < contagem; ++k) {
        if (k > 0) output << ","; // Virgula entre códigos
        output << pacotes[indicesEscolhidos[k]].codigo;
    }
    output << "\n";

    delete[] dp;
    delete[] history;
    delete[] indicesEscolhidos;
}

int main(int argc, char* argv[])
{
    if (argc < 3) return 1;

    std::ifstream input(argv[1]);
    std::ofstream output(argv[2]);

    if (!input || !output) return 1;

    // Leitura Veículos
    int total_veiculos;
    input >> total_veiculos;
    Veiculo* veiculos = new Veiculo[total_veiculos];
    for (int i = 0; i < total_veiculos; i++) {
        input >> veiculos[i].placa >> veiculos[i].capPeso >> veiculos[i].capVol;
    }

    // Leitura Pacotes
    int total_pacotes;
    input >> total_pacotes;
    Pacote* pacotes = new Pacote[total_pacotes];
    for (int i = 0; i < total_pacotes; i++) {
        input >> pacotes[i].codigo >> pacotes[i].valor >> pacotes[i].peso >> pacotes[i].volume;
        pacotes[i].disponivel = true;
    }

    // Processa cada veículo
    for (int i = 0; i < total_veiculos; i++) {
        processarCarga(veiculos[i], pacotes, total_pacotes, output);
    }

    // PENDENTES
    // Calcula totais dos itens que sobraram (disponivel == true)
    float valorPend = 0;
    uint32_t pesoPend = 0;
    uint32_t volPend = 0;
    bool primeiroPendente = true;

    // Buffers para evitar múltiplas iterações ou strings complexas
    // Imprimir o cabeçalho primeiro, depois iterar imprimindo os códigos
    
    // Calcular somas para o cabeçalho
    for(int i = 0; i < total_pacotes; i++) {
        if(pacotes[i].disponivel) {
            valorPend += pacotes[i].valor;
            pesoPend += pacotes[i].peso;
            volPend += pacotes[i].volume;
        }
    }

    if (valorPend > 0 || pesoPend > 0) {
        output << "PENDENTE:R$" << std::fixed << std::setprecision(2) << valorPend << ",";
        output << std::defaultfloat;
        output << pesoPend << "KG," << volPend << "L->";
        
        // Imprimir códigos
        for(int i = 0; i < total_pacotes; i++) {
            if(pacotes[i].disponivel) {
                if (!primeiroPendente) output << ",";
                output << pacotes[i].codigo;
                primeiroPendente = false;
            }
        }
        output << "\n"; // Linha no final do arquivo
    } else {
        // Se não sobrar nada, imprime linha vazia ou mensagem
        // não imprimime a linha ou imprimimos zerada. 
        // Assumo que só imprime se houver pendencia ou zerada.
        output << "PENDENTE:R$0.00,0KG,0L->\n";
    }

    delete[] veiculos;
    delete[] pacotes;

    return 0;
}