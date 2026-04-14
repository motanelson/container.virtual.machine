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
    char cmdLine[32767];
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD exitCode;
    BOOL ok;

    while (1) {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole == INVALID_HANDLE_VALUE) return 1;

        HANDLE hPipeRead, hPipeWrite;

        if (!CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0)) {
            fprintf(stderr, "CreatePipe falhou\n");
            return 1;
        }

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hPipeWrite;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        printf("_:>");
        fflush(stdout);

        memset(cmdLine, 0, sizeof(cmdLine));
        fgets(cmdLine, sizeof(cmdLine), stdin);
        if(strncmp(cmdLine,"exit",4)==0)break;
        if(strncmp(cmdLine,"EXIT",4)==0)break;
        // remover newline
        cmdLine[strcspn(cmdLine, "\n")] = 0;

        ok = CreateProcess(NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
        CloseHandle(hPipeWrite);

        if (!ok) {
            fprintf(stderr, "CreateProcess falhou: %d\n", GetLastError());
            CloseHandle(hPipeRead);
            continue;
        }

        CloseHandle(pi.hThread);

        filter_pipe(hPipeRead);

        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(pi.hProcess);
        CloseHandle(hPipeRead);
    }

    set_color(7, 0);
    return 0;
}