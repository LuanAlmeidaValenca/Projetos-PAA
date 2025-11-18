#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>

#define PRIORITY_LEVELS 64

struct Packet {
    uint8_t *data;
    unsigned short size;
    Packet *next;
};

static Packet *head_bucket[PRIORITY_LEVELS];
static Packet *tail_bucket[PRIORITY_LEVELS];
static unsigned int total_bytes = 0;
static FILE *inFile = nullptr;
static FILE *outFile = nullptr;

static Packet* create_packet(const uint8_t *buf, unsigned short size) {
    Packet *p = (Packet*) std::malloc(sizeof(Packet));
    if (!p) { std::fprintf(stderr, "malloc fail\n"); std::exit(1); }

    p->data = (uint8_t*) std::malloc(size);
    if (!p->data) { std::fprintf(stderr, "malloc fail\n"); std::exit(1); }

    std::memcpy(p->data, buf, size);
    p->size = size;
    p->next = nullptr;
    return p;
}

static void free_packet(Packet *p) {
    if (!p) return;
    if (p->data) std::free(p->data);
    std::free(p);
}

static void push_packet(unsigned priority, Packet *p) {
    if (priority >= PRIORITY_LEVELS) priority = 0;

    if (!head_bucket[priority]) {
        head_bucket[priority] = tail_bucket[priority] = p;
    } else {
        tail_bucket[priority]->next = p;
        tail_bucket[priority] = p;
    }
    total_bytes += p->size;
}

static void flush_and_print() {
    bool any = false;
    for (int pr = 63; pr >= 0; pr--) {
        if (head_bucket[pr]) { any = true; break; }
    }
    if (!any) return;

    // Imprime tudo em UMA única linha
    for (int pr = 63; pr >= 0; pr--) {
        Packet *cur = head_bucket[pr];
        while (cur) {
            std::fputc('|', outFile);
            for (unsigned short i = 0; i < cur->size; i++) {
                std::fprintf(outFile, "%02X", cur->data[i]);
                if (i + 1 < cur->size)
                    std::fputc(',', outFile);
            }
            std::fputc('|', outFile);
            cur = cur->next;
        }
    }
    std::fputc('\n', outFile);

    // Liberar pacotes
    for (int pr = 0; pr < PRIORITY_LEVELS; pr++) {
        Packet *cur = head_bucket[pr];
        while (cur) {
            Packet *n = cur->next;
            free_packet(cur);
            cur = n;
        }
        head_bucket[pr] = tail_bucket[pr] = nullptr;
    }
    total_bytes = 0;
}

static int read_token(char *out, int max) {
    int c;

    // pular separadores
    do {
        c = std::fgetc(inFile);
        if (c == EOF) return 0;
    } while (c==' ' || c=='\n' || c=='\t' || c=='\r' || c==',');

    int idx = 0;
    while (c!=EOF && c!=' ' && c!='\n' && c!='\t' && c!='\r' && c!=',') {
        if (idx+1 < max)
            out[idx++] = (char)c;
        c = std::fgetc(inFile);
    }
    out[idx] = '\0';
    return 1;
}

static int token_to_byte(const char *tok, uint8_t *out) {
    if (!tok || tok[0] == '\0') return 0;
    char *end = nullptr;
    long v;

    // detecta hexadecimal
    bool hex = false;
    for (const char *p = tok; *p; p++)
        if ((*p>='A'&&*p<='F') || (*p>='a'&&*p<='f'))
            hex = true;

    if ((tok[0]=='0' && (tok[1]=='x'||tok[1]=='X')) || hex)
        v = std::strtol(tok, &end, 16);
    else
        v = std::strtol(tok, &end, 10);

    if (end == tok) return 0;
    if (v < 0) v = 0;
    if (v > 255) v &= 0xFF;

    *out = (uint8_t)v;
    return 1;
}

int main(int argc, char **argv) {

    if (argc != 3) {
        std::fprintf(stderr, "Uso: %s <entrada> <saida>\n", argv[0]);
        return 1;
    }

    inFile = std::fopen(argv[1], "r");
    if (!inFile) {
        std::fprintf(stderr, "Erro ao abrir arquivo de entrada\n");
        return 1;
    }

    outFile = std::fopen(argv[2], "w");
    if (!outFile) {
        std::fprintf(stderr, "Erro ao abrir arquivo de saída\n");
        return 1;
    }

    for (int i = 0; i < PRIORITY_LEVELS; i++)
        head_bucket[i] = tail_bucket[i] = nullptr;

    int n;
    unsigned int capacity;

    if (std::fscanf(inFile, "%d %u", &n, &capacity) != 2) {
        std::fprintf(stderr, "Erro lendo n e capacidade\n");
        return 1;
    }

    char token[64];
    int trash = std::fgetc(inFile); // limpar newline

    for (int i = 0; i < n; i++) {

        // PRIORIDADE
        if (!read_token(token, sizeof(token))) {
            std::fprintf(stderr, "Erro lendo prioridade\n");
            return 1;
        }
        int priority = std::atoi(token);
        if (priority < 0) priority = 0;
        if (priority > 63) priority = 63;

        // TAMANHO
        if (!read_token(token, sizeof(token))) {
            std::fprintf(stderr, "Erro lendo tamanho\n");
            return 1;
        }
        int size = std::atoi(token);
        if (size < 0) size = 0;
        if (size > 512) size = 512;

        uint8_t *buf = (uint8_t*) std::malloc(size);
        if (size > 0 && !buf) { std::fprintf(stderr,"malloc fail\n"); return 1; }

        // BYTES
        for (int b = 0; b < size; b++) {
            if (!read_token(token, sizeof(token))) {
                std::fprintf(stderr, "Erro lendo byte\n");
                return 1;
            }
            uint8_t val;
            if (!token_to_byte(token, &val)) {
                std::fprintf(stderr, "Byte invalido: %s\n", token);
                return 1;
            }
            buf[b] = val;
        }

        Packet *p = create_packet(buf, size);
        std::free(buf);
        push_packet(priority, p);

        if (total_bytes >= capacity)
            flush_and_print();
    }

    if (total_bytes > 0)
        flush_and_print();

    std::fclose(inFile);
    std::fclose(outFile);
    return 0;
}
