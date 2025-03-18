/* 
Sew: create binary files from text files
By: Paul Clarke
Created: 3/17/2025
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint64_t u64;

void errexit(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args );
	va_end(args);
	exit(1);
}

struct char_dynarray {
    char *data;
    u64 size;
    u64 capacity;
};

struct char_dynarray new_char_dynarray(void)
{
    struct char_dynarray arr;
    arr.data = malloc(8);
    arr.size = 0;
    arr.capacity = 8;
    return arr;
}

void char_dynarray_add(struct char_dynarray *arr, char byte)
{
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, arr->capacity);
    }
    arr->data[arr->size++] = byte;
}

char *read_in_chunks(FILE *in, u64 chunk_size, u64 *out_size)
{
    char *data = malloc(chunk_size);
    u64 size = 0;
    u64 read;
    while ((read = fread(data+size, 1, chunk_size, in)) == chunk_size) {
        size += read;
        data = realloc(data, size+chunk_size);
    }
    size += read;
    *out_size = size;
    return data;
}

enum mode {
    MODE_HEX,
    MODE_BINARY,
    MODE_HEX_LITTLE,
    MODE_BINARY_LITTLE,
};

const char *usage = "usage: sew [-o outfile] infile\n";

int main(int argc, char **argv)
{
    char *infile, *outfile = NULL;
    for (int i=1; i<argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'o':
                    if (i+1 >= argc) errexit(usage);
                    outfile = argv[i+1];
                    i++;
                    break;
            }
        } else {
            infile = argv[i];
        }
    }
    if (!infile) errexit(usage);
    FILE *in = fopen(infile, "r");
    if (!in) errexit ("Failed to open %s.\n", infile);
    if (!outfile) outfile = "a.out";
    FILE *outf = fopen(outfile, "w+");
    if (!outf) errexit("Failed to open %s.\n", outfile);

    struct char_dynarray out = new_char_dynarray();

    u64 size;
    char *data = read_in_chunks(in, 1<<20, &size);
    u64 i = 0;
    enum mode mode = MODE_HEX;
    int byte_i = 0;
    char byte = 0;
    while (i < size) {
        char c = data[i];
        if (c == ' ' || c == '\n' || c=='\t') {
            i++;
            continue;
        }
        else if (c == '.') {
            if (byte_i) {
                errexit("Commands not allowed in the middle of a byte.\n");
            }
            char *next = &data[++i];
            if (strncmp(next, "format hex_little", 17)==0) {
                mode = MODE_HEX_LITTLE;
                i += 17;
            } else if (strncmp(next, "format binary_little", 20)==0) {
                mode = MODE_BINARY_LITTLE;
                i += 20;
            } else if (strncmp(next, "format hex", 10)==0) {
                mode = MODE_HEX;
                i += 10;
            } else if (strncmp(next, "format binary", 13)==0) {
                mode = MODE_BINARY;
                i += 13;
            } else {
                // TODO: line number
                errexit("Invalid command.\n");
            }
        } else if (c == ';') {
            i++;
            while (data[i] != '\n') 
                i++;
            continue;
        } else if (mode == MODE_HEX || mode == MODE_HEX_LITTLE) {
            char char_val;
            if (c >= '0' && c <= '9') {
                char_val = c - '0';
            } else if (c >= 'a' && c <= 'f') {
                char_val = c - 'a' + 10;
            } else if (c >= 'A' && c <= 'F') {
                char_val = c - 'A' + 10;
            } else {
                errexit("Invalid character: %c.\n", c);
            }
            if (mode == MODE_HEX) {
                byte |= (char_val << (4 - 4*byte_i));
            } else {
                byte |= (char_val << 4*byte_i);
            }
            if (byte_i == 1) {
                char_dynarray_add(&out, byte);
                byte = 0;
            }
            byte_i = (byte_i + 1) % 2;
            i++;
        } else if (mode == MODE_BINARY || mode == MODE_BINARY_LITTLE) {
            char char_val;
            if (c == '0' || c == '1') {
                char_val = c - '0';
            } else {
                errexit("Invalid character: %c.\n", c);
            }
            if (mode == MODE_BINARY) {
                byte |= (char_val << (7 - byte_i));
            } else {
                byte |= (char_val << byte_i);
            }
            if (byte_i == 7) {
                char_dynarray_add(&out, byte);
                byte = 0;
            }
            byte_i = (byte_i + 1) % 8;
            i++;
        }
    }
    fwrite(out.data, 1, out.size, outf);
    fclose(in);
    fclose(outf);
}