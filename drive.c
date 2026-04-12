#include <windows.h>
#include <stdio.h>

HANDLE hConsole;

// Funções do filtro ANSI (as mesmas do seu código)
int ansi_to_win_color(int ansi_color) {
    int r = (ansi_color >> 0) & 1;
    int g = (ansi_color >> 1) & 1;
    int b = (ansi_color >> 2) & 1;
    return (r << 2) | (g << 1) | (b << 0);
}

void set_color(int fg, int bg) {
    SetConsoleTextAttribute(hConsole, (bg << 4) | fg);
}

void process_ansi(const char *str) {
    while (*str) {
        if (*str == 27 && *(str+1) == '[') {
            str += 2;
            int fg = 7, bg = 0;
            while (*str && *str != 'm') {
                int code = atoi(str);
                if (code >= 30 && code <= 37)
                    fg = ansi_to_win_color(code - 30);
                if (code >= 40 && code <= 47)
                    bg = ansi_to_win_color(code - 40);
                while (*str && *str != ';' && *str != 'm') str++;
                if (*str == ';') str++;
            }
            if (*str == 'm') {
                set_color(fg, bg);
                str++;
            }
        } else {
            putchar(*str);
            str++;
        }
    }
}

// Função que lê do pipe e aplica o filtro
void filter_pipe(HANDLE hPipeRead) {
    char buffer[4096];
    DWORD bytesRead;
    while (ReadFile(hPipeRead, buffer, sizeof(buffer)-1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        process_ansi(buffer);
    }
}

int main(int argc, char *argv[]) {
    /*if (argc < 2) {
        fprintf(stderr, "Uso: %s <comando> [argumentos...]\n", argv[0]);
        return 1;
    }
    */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return 1;

    // Criar pipe para capturar stdout do filho
    HANDLE hPipeRead, hPipeWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE }; // herdável
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0)) {
        fprintf(stderr, "CreatePipe falhou\n");
        return 1;
    }

    // Garantir que a leitura seja não-bloqueante? Não necessário, mas vamos configurar
    // Prepara o processo filho
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hPipeWrite;   // stdout do filho vai para o pipe
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE); // stderr não redirecionado (opcional)
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE); // stdin do filho = nosso stdin

    // Montar linha de comando
    char cmdLine[32767] = "";
    for (int i = 1; i < argc; i++) {
        strcat(cmdLine, argv[i]);
        if (i < argc-1) strcat(cmdLine, " ");
    }
    strcpy(cmdLine,"command");
    // Criar o processo filho
    BOOL ok = CreateProcess(NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    CloseHandle(hPipeWrite); // o filho herdou, podemos fechar nossa cópia

    if (!ok) {
        fprintf(stderr, "CreateProcess falhou: %d\n", GetLastError());
        CloseHandle(hPipeRead);
        return 1;
    }

    // Fechar o handle do filho (não precisamos esperar explicitamente, mas vamos esperar o pipe terminar)
    CloseHandle(pi.hThread);

    // Filtrar a saída do pipe até o fim
    filter_pipe(hPipeRead);

    // Aguardar o processo terminar (opcional, para pegar o exit code)
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(hPipeRead);

    // Restaurar cor padrão
    set_color(7, 0);
    return exitCode;
}