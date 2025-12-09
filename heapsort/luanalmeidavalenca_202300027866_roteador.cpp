#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cctype>

static const size_t OUTBUF_SZ = 1 << 20;
static const size_t INBUF_EXTRA = 16;

static inline int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

int main(int argc, char **argv) {
    if (argc != 3) return 1;

    FILE *f = std::fopen(argv[1], "rb");
    if (!f) return 1;
    std::fseek(f, 0, SEEK_END);
    long fsize = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (fsize < 0) return 1;

    size_t bufsize = (size_t)fsize + INBUF_EXTRA;
    char *inbuf = (char*) std::malloc(bufsize);
    if (!inbuf) return 1;
    size_t read = std::fread(inbuf, 1, (size_t)fsize, f);
    std::fclose(f);
    inbuf[read] = '\0';

    char *p = inbuf;
    auto skip_sep = [&]() {
        while (*p && ( *p==' ' || *p=='\t' || *p=='\r' || *p=='\n' || *p==',' )) ++p;
    };

    auto parse_uint = [&]() -> unsigned long {
        skip_sep();
        unsigned long v = 0;
        while (*p && *p>='0' && *p<='9') {
            v = v*10 + (unsigned long)(*p - '0');
            ++p;
        }
        return v;
    };

    auto parse_byte = [&]() -> int {
        skip_sep();
        if (!*p) return -1;
        char *s = p;
        char *t = p;
        while (*t && !( *t==' '||*t=='\t'||*t=='\r'||*t=='\n'||*t==',' )) ++t;
        bool is_hex = false;
        if ((t-s)>=2 && s[0]=='0' && (s[1]=='x'||s[1]=='X')) is_hex = true;
        for (char *q=s; q<t; ++q) {
            if ((*q>='A'&&*q<='F')||(*q>='a'&&*q<='f')||*q=='x'||*q=='X') { is_hex = true; break; }
        }
        int value = 0;
        if (is_hex) {
            if ((t-s)>=2 && s[0]=='0' && (s[1]=='x'||s[1]=='X')) s+=2;
            for (char *q=s; q<t; ++q) {
                int hv = hexval(*q);
                if (hv<0) break;
                value = (value<<4) | hv;
            }
        } else {
            for (char *q=s; q<t; ++q) {
                if (*q<'0'||*q>'9') break;
                value = value*10 + (*q - '0');
            }
        }
        p = t;
        return value & 0xFF;
    };

    unsigned long n = parse_uint();
    unsigned long capacity = parse_uint();
    if (n == 0) {
        std::free(inbuf);
        FILE *outf = std::fopen(argv[2], "wb");
        if (outf) std::fclose(outf);
        return 0;
    }

    int *pkt_size = (int*) std::malloc(sizeof(int)*n);
    int *pkt_prio = (int*) std::malloc(sizeof(int)*n);
    int *pkt_offset = (int*) std::malloc(sizeof(int)*n);
    int *next_in_bucket = (int*) std::malloc(sizeof(int)*n);
    if (!pkt_size || !pkt_prio || !pkt_offset || !next_in_bucket) return 1;

    int head_bucket[64], tail_bucket[64];
    for (int i=0;i<64;i++){ head_bucket[i]=-1; tail_bucket[i]=-1; }

    size_t data_buf_cap = (size_t)capacity + 512;
    if (data_buf_cap<1024) data_buf_cap=1024;
    uint8_t *data_buf = (uint8_t*) std::malloc(data_buf_cap);
    if (!data_buf) return 1;

    char *outbuf = (char*) std::malloc(OUTBUF_SZ);
    if (!outbuf) return 1;
    size_t outpos = 0;

    auto flush_to_file = [&](FILE *outf, bool) {
        if (outpos>0) {
            std::fwrite(outbuf,1,outpos,outf);
            outpos=0;
        }
    };

    auto out_write = [&](const char *s, size_t len, FILE *outf) {
        if (outpos + len >= OUTBUF_SZ) flush_to_file(outf,false);
        if (len >= OUTBUF_SZ) {
            std::fwrite(s,1,len,outf);
            return;
        }
        std::memcpy(outbuf+outpos,s,len);
        outpos += len;
    };

    auto out_write_byte_hex = [&](uint8_t b, FILE *outf) {
        char t[2];
        const char *hex = "0123456789ABCDEF";
        t[0] = hex[(b>>4)&0xF];
        t[1] = hex[b&0xF];
        out_write(t,2,outf);
    };

    auto flush_print = [&](FILE *outf) {
        bool any=false;
        for(int pr=63;pr>=0;--pr){ if(head_bucket[pr]!=-1){ any=true; break; } }
        if(!any) return;

        for(int pr=63;pr>=0;--pr){
            int idx = head_bucket[pr];
            while(idx!=-1){
                out_write("|",1,outf);
                int off = pkt_offset[idx];
                int sz = pkt_size[idx];
                for(int i=0;i<sz;i++){
                    out_write_byte_hex(data_buf[off+i],outf);
                    if(i+1<sz) out_write(",",1,outf);
                }
                out_write("|",1,outf);
                idx = next_in_bucket[idx];
            }
        }
        out_write("\n",1,outf);

        for(int pr=0;pr<64;pr++){ head_bucket[pr]=tail_bucket[pr]=-1; }
    };

    unsigned long buffered_count=0;
    size_t data_pos=0;
    unsigned long total_bytes_buffered=0;

    FILE *outf = std::fopen(argv[2], "wb");
    if(!outf) return 1;

    for(unsigned long pkt_i=0;pkt_i<n;pkt_i++){
        unsigned long priority = parse_uint();
        if(priority>63) priority=63;
        unsigned long size = parse_uint();
        if(size>512) size=512;

        if(data_pos + size > data_buf_cap){
            flush_print(outf);
            flush_to_file(outf,false);
            data_pos=0;
            total_bytes_buffered=0;
            for(int pr=0;pr<64;pr++) head_bucket[pr]=tail_bucket[pr]=-1;
            buffered_count=0;
        }

        int offset_here = (int)data_pos;
        for(unsigned long b=0;b<size;b++){
            int bv = parse_byte();
            if(bv<0) bv=0;
            data_buf[data_pos++] = (uint8_t)bv;
        }

        pkt_offset[buffered_count] = offset_here;
        pkt_size[buffered_count] = (int)size;
        pkt_prio[buffered_count] = (int)priority;
        next_in_bucket[buffered_count] = -1;

        int idx = (int)buffered_count;
        if(head_bucket[priority]==-1){
            head_bucket[priority] = tail_bucket[priority] = idx;
        } else {
            next_in_bucket[ tail_bucket[priority] ] = idx;
            tail_bucket[priority] = idx;
        }

        buffered_count++;
        total_bytes_buffered += size;

        if(total_bytes_buffered >= capacity){
            flush_print(outf);
            flush_to_file(outf,false);
            data_pos=0;
            total_bytes_buffered=0;
            buffered_count=0;
            for(int pr=0;pr<64;pr++) head_bucket[pr]=tail_bucket[pr]=-1;
        }
    }

    {
        bool any=false;
        for(int pr=0;pr<64;pr++) if(head_bucket[pr]!=-1){ any=true; break; }
        if(any){
            flush_print(outf);
            flush_to_file(outf,false);
        }
    }

    flush_to_file(outf,true);
    std::fclose(outf);

    std::free(inbuf);
    std::free(pkt_size);
    std::free(pkt_prio);
    std::free(pkt_offset);
    std::free(next_in_bucket);
    std::free(data_buf);
    std::free(outbuf);

    return 0;
}