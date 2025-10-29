// Duplica un string. Debe contar la cantidad de caracteres totales de src y solicitar la memoria equivalente. Luego, debe copiar todos los caracteres a esta nueva ´area de memoria. Adem´as, como valor de retorno se debe retornar el puntero al nuevo string.
char* strDuplicate(char* src) {
    int cont = 0;
    for (int i = 0; src[i] != '\0'; i++) {
        cont++;
    }
    char* duplica = (char*) malloc(sizeof(char) * cont + 1);
    for (int i = 0; src[i] != '\0'; i++) {
        duplica[i] = src[i];
    }
    duplica[cont] = '\0';

    return duplica;
}

// Compara dos strings lexicograficamente. Devuelve 0 si son iguales, 1 si el primer string es menor al segundo, -1 en otro caso.
int strCompare(char* s1, char* s2) {
    int largo1 = 0;
    for (int i = 0; s1[i] != '\0'; i++) {
        largo1++;
    }
    int largo2 = 0;
    for (int i = 0; s2[i] != '\0'; i++) {
        largo2++;
    }
    if (largo1 < largo2) {
        return 1;
    }
    else if (largo1 == largo2) {
        for (int i = 0; i < largo1; i++) {
            if (s1[i] != s2[i]){
                if (s1[i] < s2[i]){
                    return 1;
                }
                return -1;
            }
        }
        return 0;
    }
    else {
        return -1;
    }
}

// La funci´on toma dos strings src1 y src2, y retorna un nuevo string que contiene una copia de todos los caracteres de src1 seguidos de los caracteres de src2. Adem´as, la memoria de src1 y src2 debe ser liberada.
char* strConcatenate(char* src1, char* src2) {
    int largo1 = 0;
    for (int i = 0; src1[i] != '\0'; i++) {
        largo1++;
    }
    int largo2 = 0;
    for (int i = 0; src2[i] != '\0'; i++) {
        largo2++;
    }
    char* ret = (char*) malloc(sizeof(char) * (largo1 + largo2) + 1);

    int i = 0;
    for (; i < largo1; i++) {
        ret[i] = src1[i];
    }
    for (int j = 0; j < largo2; j++) {
        ret[i + j] = src2[j];
    }
    ret[largo1 + largo2] = '\0';
    free(src1);
    free(src2);
    return ret;
}