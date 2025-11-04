#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Duplica un string. Debe contar la cantidad de caracteres totales de src y solicitar la memoria equivalente. Luego, debe copiar todos los caracteres a esta nueva ´area de memoria. Adem´as, como valor de retorno se debe retornar el puntero al nuevo string.
char *strDuplicate(char *src)
{
    int cont = 0;
    for (int i = 0; src[i] != '\0'; i++)
    {
        cont++;
    }
    char *duplica = (char *)malloc(sizeof(char) * cont + 1);
    for (int i = 0; src[i] != '\0'; i++)
    {
        duplica[i] = src[i];
    }
    duplica[cont] = '\0';

    return duplica;
}

// Compara dos strings lexicograficamente. Devuelve 0 si son iguales, 1 si el primer string es menor al segundo, -1 en otro caso.
int strCompare(char *s1, char *s2)
{
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') // se fija letra por letra si hay alguna menor lexicograficamente antes de que se terminen los strings para devolver el valor correspondiente
    {
        if (s1[i] < s2[i])
        {
            return 1;
        }
        else if (s1[i] > s2[i])
        {
            return -1;
        }
        i++;
    }
    // si algun string termina antes de que haya diferencias se evaluan los posibles casos
    if (s1[i] == '\0' && s2[i] == '\0') // si son iguales para devolver 0
    {
        return 0;
    }
    else if (s1[i] == '\0') // si termino primero s1 para devolver 1
    {
        return 1;
    }
    else // si termino primero s2 para devolver -1
    {
        return -1;
    }
}

// La funci´on toma dos strings src1 y src2, y retorna un nuevo string que contiene una copia de todos los caracteres de src1 seguidos de los caracteres de src2. Adem´as, la memoria de src1 y src2 debe ser liberada.
char *strConcatenate(char *src1, char *src2)
{
    int largo1 = 0;
    for (int i = 0; src1[i] != '\0'; i++)
    {
        largo1++;
    }
    int largo2 = 0;
    for (int i = 0; src2[i] != '\0'; i++)
    {
        largo2++;
    }
    char *ret = (char *)malloc(sizeof(char) * ((largo1 + largo2) + 1));

    int i = 0;
    for (; i < largo1; i++)
    {
        ret[i] = src1[i];
    }
    for (int j = 0; j < largo2; j++)
    {
        ret[largo1 + j] = src2[j];
    }
    ret[largo1 + largo2] = '\0';
    free(src1);
    free(src2);
    return ret;
}

int casos_test()
{
    printf("Duplicate vacío: '%s'\n", strDuplicate(""));
    printf("Duplicate A: '%s'\n", strDuplicate("A"));
    printf("Duplicate largo: '%s'\n", strDuplicate("ZXCVBNMASDFGHJKLQWERTYUIOPzxcvbnmasdfghjklqwertyuiop123456789"));

    printf("Compare iguales: %d\n", strCompare("AAAA", "AAAA"));
    printf("Compare vacíos: %d\n", strCompare("", ""));
    printf("Compare dos de un solo caracter: %d\n", strCompare("A", "B"));
    printf("Compare dos de un solo caracter: %d\n", strCompare("B", "A"));
    printf("Compare iguales hasta un caracter: %d\n", strCompare("Pedro", "Pedra"));
    printf("Compare iguales hasta un caracter: %d\n", strCompare("Pedra", "Pedro"));
    printf("Compare strings diferentes: %d\n", strCompare("Bautista", "Loisi"));
    printf("Compare strings diferentes: %d\n", strCompare("Loisi", "Bautista"));

    printf("Concat vacio y un string de 3 caracteres: '%s'\n", strConcatenate(strDuplicate(""), strDuplicate("ANA")));
    printf("Concat 3 caracteres y un string vacio: '%s'\n", strConcatenate(strDuplicate("ANA"), strDuplicate("")));
    printf("Concat dos vacíos: '%s'\n", strConcatenate(strDuplicate(""), strDuplicate("")));
    printf("Concat dos strings de 1 caracter: '%s'\n", strConcatenate(strDuplicate("P"), strDuplicate("A")));
    printf("Concat dos strings de 5 caracteres: '%s'\n", strConcatenate(strDuplicate("Pedro"), strDuplicate("Bauti")));

    return 0;
}