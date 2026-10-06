/*
 * minios.c
 * Mini Sistema Operativo en C
 *
 * Los registros se guardan en el archivo "datos.dat" (simula el disco).
 * Create/Read/Update/Delete ya funcionan de verdad, leyendo y escribiendo
 * en ese archivo. Las opciones 5 y 6 (procesos y apagado) siguen igual.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

#define ARCHIVO "datos.dat"
#define TAM_NOMBRE 50
#define TAM_DESC   100

typedef struct {
    int id;
    char nombre[TAM_NOMBRE];
    char descripcion[TAM_DESC];
} Registro;

void limpiarBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void crearRegistro() {
    Registro r;
    printf("\n--- CREATE: nuevo registro ---\n");
    printf("ID: ");
    if (scanf("%d", &r.id) != 1) { printf("ID invalido.\n"); limpiarBuffer(); return; }
    limpiarBuffer();
    printf("Nombre: ");
    fgets(r.nombre, TAM_NOMBRE, stdin);
    r.nombre[strcspn(r.nombre, "\n")] = '\0';
    printf("Descripcion: ");
    fgets(r.descripcion, TAM_DESC, stdin);
    r.descripcion[strcspn(r.descripcion, "\n")] = '\0';

    FILE *f = fopen(ARCHIVO, "ab");
    if (f == NULL) { printf("Error: no se pudo abrir el archivo.\n"); return; }
    fwrite(&r, sizeof(Registro), 1, f);
    fclose(f);
    printf("Registro con ID %d guardado en disco.\n", r.id);
}

void leerRegistros() {
    Registro r;
    int contador = 0;
    FILE *f = fopen(ARCHIVO, "rb");
    if (f == NULL) { printf("\nNo hay registros aun (el archivo no existe).\n"); return; }
    printf("\n--- READ: registros en disco ---\n");
    while (fread(&r, sizeof(Registro), 1, f) == 1) {
        printf("ID: %d | Nombre: %s | Descripcion: %s\n", r.id, r.nombre, r.descripcion);
        contador++;
    }
    fclose(f);
    if (contador == 0) printf("El archivo existe pero no tiene registros.\n");
}

void actualizarRegistro() {
    int idBuscado;
    Registro r;
    int encontrado = 0;
    printf("\n--- UPDATE: modificar registro ---\n");
    printf("ID a buscar: ");
    if (scanf("%d", &idBuscado) != 1) { printf("ID invalido.\n"); limpiarBuffer(); return; }
    limpiarBuffer();

    FILE *f = fopen(ARCHIVO, "rb+");
    if (f == NULL) { printf("No hay registros guardados todavia.\n"); return; }

    long posicion;
    while (fread(&r, sizeof(Registro), 1, f) == 1) {
        if (r.id == idBuscado) {
            encontrado = 1;
            posicion = ftell(f) - sizeof(Registro);
            printf("Registro encontrado -> Nombre actual: %s | Descripcion actual: %s\n", r.nombre, r.descripcion);
            printf("Nuevo nombre: ");
            fgets(r.nombre, TAM_NOMBRE, stdin);
            r.nombre[strcspn(r.nombre, "\n")] = '\0';
            printf("Nueva descripcion: ");
            fgets(r.descripcion, TAM_DESC, stdin);
            r.descripcion[strcspn(r.descripcion, "\n")] = '\0';
            fseek(f, posicion, SEEK_SET);
            fwrite(&r, sizeof(Registro), 1, f);
            break;
        }
    }
    fclose(f);
    if (encontrado) printf("Registro con ID %d actualizado.\n", idBuscado);
    else printf("No se encontro ningun registro con ID %d.\n", idBuscado);
}

void eliminarRegistro() {
    int idBuscado;
    Registro r;
    int encontrado = 0;
    printf("\n--- DELETE: eliminar registro ---\n");
    printf("ID a eliminar: ");
    if (scanf("%d", &idBuscado) != 1) { printf("ID invalido.\n"); limpiarBuffer(); return; }
    limpiarBuffer();

    FILE *f = fopen(ARCHIVO, "rb");
    if (f == NULL) { printf("No hay registros guardados todavia.\n"); return; }
    FILE *temp = fopen("temp.dat", "wb");
    if (temp == NULL) { printf("Error: no se pudo crear archivo temporal.\n"); fclose(f); return; }

    while (fread(&r, sizeof(Registro), 1, f) == 1) {
        if (r.id == idBuscado) { encontrado = 1; continue; }
        fwrite(&r, sizeof(Registro), 1, temp);
    }
    fclose(f);
    fclose(temp);
    remove(ARCHIVO);
    rename("temp.dat", ARCHIVO);

    if (encontrado) printf("Registro con ID %d eliminado. El resto del archivo quedo intacto.\n", idBuscado);
    else printf("No se encontro ningun registro con ID %d.\n", idBuscado);
}

#define SNAKE_ANCHO 40
#define SNAKE_ALTO  20
#define SNAKE_MAX   (SNAKE_ANCHO * SNAKE_ALTO)

typedef struct { int x, y; } Punto;

void jugarSnake() {
    Punto cuerpo[SNAKE_MAX];
    int largo = 3;
    int dx = 1, dy = 0;
    int puntaje = 0;
    int comidaX, comidaY;
    int tecla;
    int perdio = 0;
    int i;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(120);

    for (i = 0; i < largo; i++) {
        cuerpo[i].x = SNAKE_ANCHO / 2 - i;
        cuerpo[i].y = SNAKE_ALTO / 2;
    }

    srand(time(NULL));
    comidaX = rand() % (SNAKE_ANCHO - 2) + 1;
    comidaY = rand() % (SNAKE_ALTO - 2) + 1;

    while (!perdio) {
        tecla = getch();
        switch (tecla) {
            case KEY_UP:    if (dy != 1)  { dx = 0; dy = -1; } break;
            case KEY_DOWN:  if (dy != -1) { dx = 0; dy = 1;  } break;
            case KEY_LEFT:  if (dx != 1)  { dx = -1; dy = 0; } break;
            case KEY_RIGHT: if (dx != -1) { dx = 1; dy = 0;  } break;
            case 'q': case 'Q': perdio = 1; continue;
        }

        for (i = largo - 1; i > 0; i--) cuerpo[i] = cuerpo[i - 1];
        cuerpo[0].x += dx;
        cuerpo[0].y += dy;

        if (cuerpo[0].x <= 0 || cuerpo[0].x >= SNAKE_ANCHO - 1 ||
            cuerpo[0].y <= 0 || cuerpo[0].y >= SNAKE_ALTO - 1) {
            perdio = 1;
        }

        for (i = 1; i < largo; i++) {
            if (cuerpo[0].x == cuerpo[i].x && cuerpo[0].y == cuerpo[i].y) perdio = 1;
        }

        if (cuerpo[0].x == comidaX && cuerpo[0].y == comidaY) {
            puntaje += 10;
            if (largo < SNAKE_MAX) largo++;
            comidaX = rand() % (SNAKE_ANCHO - 2) + 1;
            comidaY = rand() % (SNAKE_ALTO - 2) + 1;
        }

        clear();
        for (i = 0; i <= SNAKE_ANCHO; i++) { mvaddch(0, i, '#'); mvaddch(SNAKE_ALTO, i, '#'); }
        for (i = 0; i <= SNAKE_ALTO; i++)  { mvaddch(i, 0, '#'); mvaddch(i, SNAKE_ANCHO, '#'); }
        mvaddch(comidaY, comidaX, '*');
        for (i = 0; i < largo; i++) mvaddch(cuerpo[i].y, cuerpo[i].x, i == 0 ? '@' : 'o');
        mvprintw(SNAKE_ALTO + 1, 0, "Puntaje: %d   (usa las flechas, 'q' para salir)", puntaje);
        refresh();
    }

    clear();
    mvprintw(SNAKE_ALTO / 2, SNAKE_ANCHO / 2 - 10, "JUEGO TERMINADO");
    mvprintw(SNAKE_ALTO / 2 + 1, SNAKE_ANCHO / 2 - 10, "Puntaje final: %d", puntaje);
    mvprintw(SNAKE_ALTO / 2 + 2, SNAKE_ANCHO / 2 - 10, "Presiona cualquier tecla...");
    refresh();
    timeout(-1);
    getch();
    endwin();
}

void mostrarMenu() {
    printf("\n===== MINI SISTEMA OPERATIVO EN C =====\n");
    printf("--- BIENVENIDO A NUESTRO SISTEMA OPERATIVO---\n");
    printf("1. Create\n");
    printf("2. Read\n");
    printf("3. Update\n");
    printf("4. Delete\n");
    printf("--- Gestion de Procesos ---\n");
    printf("5. Iniciar Reto BSDGames\n");
    printf("--- Control del Sistema ---\n");
    printf("6. Apagar Sistema\n");
    printf("===================================\n");
    printf("Seleccione una opcion: ");
}

int main() {
    int opcion;
    int sistemaActivo = 1;
    setvbuf(stdout, NULL, _IONBF, 0);

    while (sistemaActivo) {
        mostrarMenu();
        if (scanf("%d", &opcion) != 1) {
            printf("Entrada invalida. Intente de nuevo.\n");
            limpiarBuffer();
            continue;
        }
        limpiarBuffer();

        switch (opcion) {
            case 1: crearRegistro(); break;
            case 2: leerRegistros(); break;
            case 3: actualizarRegistro(); break;
            case 4: eliminarRegistro(); break;
            case 5:
                printf("Iniciando Reto BSDGames: Snake...\n");
                jugarSnake();
                printf("De vuelta en el Mini Sistema Operativo.\n");
                break;
            case 6:
                printf("Apagando el sistema de forma segura...\n");
                sistemaActivo = 0;
                break;
            default:
                printf("Opcion no valida. Por favor elija entre 1 y 6.\n");
                break;
        }
    }

    printf("Sistema apagado. Hasta pronto.\n");
    return 0;
}
