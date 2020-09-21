#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* readFile(const char* filePath) {
    FILE* fp = fopen(filePath, "rb");
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    rewind(fp);

    // +1 to give us a null terminator
    char* buffer = calloc(length + 1, 1);
    fread(buffer, 1, length, fp);
    fclose(fp);

    return buffer;
}

void writeFile(const char* filePath, char* body) {
    FILE* fp = fopen(filePath, "wb");
    fwrite(body, 1, strlen(body), fp);
    fclose(fp);
}

char* indent(const char* input) {
    char* indented = calloc(strlen(input) * 4, 1);
    char* out = indented;
    while (*input) {
        if (*input == '\n') {
            *(out++) = '\n';
            *(out++) = ' ';
            *(out++) = ' ';
        } else {
            *(out++) = *input;
        }
        input++;
    }
    return indented;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: insert-cp437-diagram name diagrams/name.txt help.txt");
        return -1;
    }

    char* name = argv[1];
    char* diagram = indent(readFile(argv[2]));
    char* dia = malloc(sizeof("dia[") + strlen(name) + sizeof("]"));
    sprintf(dia, "dia[%s]", name);

    while (1) {
        char* help = readFile(argv[3]);
        char* found = strstr(help, dia);
        if (found) {
            char* newHelp = calloc(strlen(help) + strlen(diagram), 1);
            memcpy(newHelp, help, found - help);
            strcat(newHelp, diagram);
            strcat(newHelp, found + strlen(dia));
            writeFile(argv[3], newHelp);
        } else {
            break;
        }
    }

    return 0;
}
