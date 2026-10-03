#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*ALUMNO: Rodríguez Luna Paulina
Código: 223378663
Materia: Compiladores 
Hands-on 1: Análisis Léxico y Autómatas*/
// Datos reconocidos

typedef struct {
    char operacion[10];

    char registros[2][5];
    int cantidadRegistros;

    int direccion;
    int tieneDireccion;
} Instruccion;

//Estados del AFD 
typedef enum {

    Q0,     // Entrada de datos
    Q1,     // Reconocimiento inicial
    Q2,     // VALIDADA
    Q3,     // ERROR

    /* MOV AL,d */
    Q4,     // M
    Q5,     // O
    Q6,     // V
    Q7,     // espacio
    Q8,     // A
    Q9,     // L
    Q10,    // ,
    Q11,    // espacio
    Q12,    // numero

    /* MOV BL,d */
    Q13,    // B
    Q14,    // L
    Q15,    // espacio

    /* ADD AL,BL */
    Q16,    // A
    Q17,    // D
    Q18,    // D
    Q19,    // espacio
    Q20,    // A
    Q21,    // L
    Q22,    // ,
    Q23,    // B
    Q24,    // L

    /* STO d */
    Q25,    // S
    Q26,    // T
    Q27,    // O
    Q28,    // espacio
    Q29,    // numero

    /* END */
    Q30,    // E
    Q31,    // N
    Q32     // D

} EstadoAFD;

   //FUNCIONES AUXILIARES

/* Verifica si un carácter es un dígito */
int esDigito(char c)
{
    return c >= '0' && c <= '9';
}


/* Verifica si un carácter es espacio */
int esEspacio(char c)
{
    return c == ' ' || c == '\t';
}


/* Muestra el nombre de un estado */
void mostrarEstado(EstadoAFD estado)
{
    printf("  Estado q%d\n", estado);
}

/* FUNCION PARA MOSTRAR TOKENS */
	int i;
void mostrarTokens(Instruccion *inst)
{
	
    printf("\n========== TOKENS RECONOCIDOS ==========\n");
    printf("MOV/ADD/STO/END -> Mnemónico: %s\n",
           inst->operacion);
    for (i = 0; i < inst->cantidadRegistros; i++)
    {
        printf("REGISTRO(\"%s\")\n",
               inst->registros[i]);
    }
    if (inst->tieneDireccion)
    {
        printf("NUMERO(\"%d\")\n",
               inst->direccion);
    }
    if (strcmp(inst->operacion, "MOV") == 0 ||
        strcmp(inst->operacion, "ADD") == 0)
    {
        printf("COMA(\",\")\n");
    }
}

/* MOSTRAR OBJETO INSTRUCCION */

void mostrarObjeto(Instruccion *inst)
{
    printf("\n========== OBJETO INSTRUCCION ==========\n");
    printf("{\n");
    printf("  operacion: \"%s\",\n",
           inst->operacion);
    printf("  registros: [");
    for (i = 0; i < inst->cantidadRegistros; i++)
    {
        printf("\"%s\"", inst->registros[i]);

        if (i < inst->cantidadRegistros - 1)
            printf(", ");
    }

    printf("],\n");
    if (inst->tieneDireccion)
        printf("  direccion: %d\n", inst->direccion);
    else
        printf("  direccion: null\n");

    printf("}\n");
}


/* 
   MAQUINA DE MOORE
   ========================================================= */

typedef enum
{
    M_INICIO,

    M_MAR,
    M_MBR,
    M_REGISTRO,

    M_ACC,

    M_STO_MAR,
    M_STO_MBR,
    M_STO_MEM,

    M_HALT,

    M_FIN

} EstadoMoore;


/* Inicializa la máquina de Moore dependiendo
   de la instrucción recibida. */

EstadoMoore iniciarMoore(Instruccion *inst)
{
    if (strcmp(inst->operacion, "MOV") == 0)
    {
        return M_MAR;
    }
    if (strcmp(inst->operacion, "ADD") == 0)
    {
        return M_ACC;
    }
    if (strcmp(inst->operacion, "STO") == 0)
    {
        return M_STO_MAR;
    }
    if (strcmp(inst->operacion, "END") == 0)
    {
        return M_HALT;
    }
    return M_FIN;
}


/* Cada estado tiene una salida asociada.
   La máquina avanza al siguiente estado. */

EstadoMoore siguientePaso(EstadoMoore estado, Instruccion *inst, int *paso)
{
    switch (estado)
    {
        case M_MAR:
            printf("%d. MAR <- %d\n",
                   (*paso)++,
                   inst->direccion);
            return M_MBR;
            
        case M_MBR:
            printf("%d. MBR <- M[MAR]\n",
                   (*paso)++);
                   
            return M_REGISTRO;

        case M_REGISTRO:

            printf("%d. %s <- MBR\n",
                   (*paso)++,
                   inst->registros[0]);
                   
            return M_FIN;
            
        case M_ACC:

            printf("%d. ACC <- AL + BL\n",
                   (*paso)++);
                   
            return M_FIN;
            
        case M_STO_MAR:

            printf("%d. MAR <- %d\n",
                   (*paso)++,
                   inst->direccion);
                   
            return M_STO_MBR;

        case M_STO_MBR:

            printf("%d. MBR <- ACC\n",
                   (*paso)++);

            return M_STO_MEM;

        case M_STO_MEM:

            printf("%d. M[MAR] <- MBR\n",
                   (*paso)++);

            return M_FIN;

        case M_HALT:

            printf("%d. HALT <- 1\n",
                   (*paso)++);

            return M_FIN;

        case M_FIN:

            return M_FIN;

        default:

            return M_FIN;
    }
}


/* Ejecuta la máquina de Moore */

void ejecutarMoore(Instruccion *inst)
{
    printf("\n========== MICROOPERACIONES GENERADAS POR MOORE ==========\n");

    EstadoMoore estado;
    int paso = 1;

    estado = iniciarMoore(inst);

    while (estado != M_FIN)
    {
        estado = siguientePaso(estado, inst, &paso);
    }

    printf("Generación terminada.\n");
}


/* AFD */

int ejecutarAFD(char entrada[], Instruccion *inst)
{
    EstadoAFD estado = Q0;

    int i = 0;
    int longitud = strlen(entrada);

    /* Variables temporales */
    char registroTemporal[5] = "";
    char numeroTemporal[50] = "";
    int indiceNumero = 0;

    /* Inicializar estructura */
    strcpy(inst->operacion, "");
    inst->cantidadRegistros = 0;
    inst->direccion = 0;
    inst->tieneDireccion = 0;


    printf("\n========== RECORRIDO DEL AFD ==========\n");
    printf("q0 -> q1 : entrada de datos\n");
    estado = Q1; //Decide qué instrucción comienza.
    mostrarEstado(estado);
    
    //en caso de ingrasar un caracter no validado marca error
    if (i >= longitud)
    {
        estado = Q3;
        printf("Error: no se introdujo ninguna instrucción.\n");
        return 0;
    }

    /* esatados para MOV*/

    if (entrada[i] == 'M')
    {
        printf("\nCaracter '%c': q1 -> q4\n", entrada[i]);
        estado = Q4;
        i++;

        /* q4 -> q5 : O */
        if (i < longitud && entrada[i] == 'O')
        {
            printf("Caracter '%c': q4 -> q5\n", entrada[i]);
            estado = Q5;
            i++;
        }
        else
        {
            printf("Error en q4: se esperaba 'O'.\n");
            return 0;
        }

        /* q5 -> q6 : V */
        if (i < longitud && entrada[i] == 'V')
        {
            printf("Caracter '%c': q5 -> q6\n", entrada[i]);
            estado = Q6;
            i++;
        }
        else
        {
            printf("Error en q5: se esperaba 'V'.\n");
            return 0;
        }

        /* q6 -> q7* : espacio */
        if (i < longitud && esEspacio(entrada[i]))
        {
            printf("Caracter espacio: q6 -> q7\n");
            estado = Q7;

            while (i < longitud && esEspacio(entrada[i]))
            {
                printf("q7 -- espacio --> q7\n");
                i++;
            }
        }
        else
        {
            printf("Error en q6: se esperaba uno o más espacios.\n");
            return 0;
        }
        
        /* q7 decide entre AL y BL */

        if (i < longitud && entrada[i] == 'A')
        {
            /* MOV AL */

            printf("Caracter 'A': q7 -> q8\n");
            estado = Q8;
            i++;

            /* q8 -> q9 : L */
            if (i < longitud && entrada[i] == 'L')
            {
                printf("Caracter 'L': q8 -> q9\n");
                estado = Q9;
                i++;
            }
            else
            {
                printf("Error en q8: se esperaba 'L'.\n");
                return 0;
            }

            strcpy(registroTemporal, "AL");


            /* q9 -> q10 : espacios opcionales */
            while (i < longitud && esEspacio(entrada[i]))
            {
                printf("q9 -- espacio --> q9\n");
                i++;
            }

            /* q10 -> q10 / q11 : coma */
            if (i < longitud && entrada[i] == ',')
            {
                printf("Caracter ',': q9 -> q10\n");
                estado = Q10;
                i++;
            }
            else
            {
                printf("Error en q10: falta la coma.\n");
                return 0;
            }

            /* espacios después de coma */
            while (i < longitud && esEspacio(entrada[i]))
            {
                printf("q11 -- espacio --> q11\n");
                estado = Q11;
                i++;
            }

            /* d un número */
            if (i < longitud && esDigito(entrada[i]))
            {
                estado = Q12;

                indiceNumero = 0;

                while (i < longitud && esDigito(entrada[i]))
                {
                    numeroTemporal[indiceNumero++] =
                        entrada[i];

                    printf("Caracter '%c': q11 -> q12\n",
                           entrada[i]);

                    i++;
                }

                numeroTemporal[indiceNumero] = '\0';

                inst->direccion = atoi(numeroTemporal);
                inst->tieneDireccion = 1;
            }
            else
            {
                printf("Error en q11: se esperaba un número.\n");
                return 0;
            }


            /* Reconocimiento completo */
            strcpy(inst->operacion, "MOV");

            strcpy(inst->registros[0],
                   registroTemporal);

            inst->cantidadRegistros = 1;

            estado = Q12;
        }


        else if (i < longitud && entrada[i] == 'B')
        {
            /* MOV BL */

            printf("Caracter 'B': q7 -> q13\n");
            estado = Q13;
            i++;


            /* q13 -> q14 : L */
            if (i < longitud && entrada[i] == 'L')
            {
                printf("Caracter 'L': q13 -> q14\n");
                estado = Q14;
                i++;
            }
            else
            {
                printf("Error en q13: se esperaba 'L'.\n");
                return 0;
            }

            strcpy(registroTemporal, "BL");

            /* espacios antes de coma */
            while (i < longitud && esEspacio(entrada[i]))
            {
                printf("q14 -- espacio --> q15\n");
                estado = Q15;
                i++;
            }

            /* coma */
            if (i < longitud && entrada[i] == ',')
            {
                printf("Caracter ',': se reconoce la coma.\n");
                i++;
            }
            else
            {
                printf("Error: falta la coma.\n");
                return 0;
            }

            /* espacios después de coma */
            while (i < longitud && esEspacio(entrada[i]))
            {
                printf("q15 -- espacio --> q15\n");
                i++;
            }

            /* número */
            if (i < longitud && esDigito(entrada[i]))
            {
                indiceNumero = 0;

                while (i < longitud && esDigito(entrada[i]))
                {
                    numeroTemporal[indiceNumero++] =
                        entrada[i];

                    printf("Caracter '%c': q15 -> q12\n",
                           entrada[i]);

                    i++;
                }

                numeroTemporal[indiceNumero] = '\0';

                inst->direccion = atoi(numeroTemporal);
                inst->tieneDireccion = 1;
            }
            else
            {
                printf("Error: se esperaba una dirección.\n");
                return 0;
            }

            strcpy(inst->operacion, "MOV");

            strcpy(inst->registros[0],
                   registroTemporal);

            inst->cantidadRegistros = 1;

            estado = Q12;
        }

        else
        {
            printf("Error en q7: se esperaba 'A' o 'B'.\n");
            return 0;
        }

        /*  Verificar fin de MOV */

        while (i < longitud && esEspacio(entrada[i]))
            i++;

        if (i == longitud)
        {
            printf("q12 -> q2\n");
            estado = Q2;
        }
        else
        {
            printf("Error: existen caracteres adicionales.\n");
            return 0;
        }
    }

    /* estados ADD
       ===================================================== */

    else if (entrada[i] == 'A')
    {
        /* q1 -> q16 */

        printf("\nCaracter 'A': q1 -> q16\n");
        estado = Q16;
        i++;

        /* q16 -> q17 : D */

        if (i < longitud && entrada[i] == 'D')
        {
            printf("Caracter 'D': q16 -> q17\n");
            estado = Q17;
            i++;
        }
        else
        {
            printf("Error en q16: se esperaba 'D'.\n");
            return 0;
        }

        /* q17 -> q18 : D */

        if (i < longitud && entrada[i] == 'D')
        {
            printf("Caracter 'D': q17 -> q18\n");
            estado = Q18;
            i++;
        }
        else
        {
            printf("Error en q17: se esperaba 'D'.\n");
            return 0;
        }

        /* espacios */

        if (i < longitud && esEspacio(entrada[i]))
        {
            printf("q18 -> q19: espacio\n");

            while (i < longitud && esEspacio(entrada[i]))
            {
                printf("q19 -- espacio --> q19\n");
                i++;
            }
        }
        else
        {
            printf("Error: se esperaba espacio después de ADD.\n");
            return 0;
        }


        /* AL */

        if (i < longitud && entrada[i] == 'A')
        {
            printf("q19 -> q20: A\n");
            estado = Q20;
            i++;
        }
        else
        {
            printf("Error en q19: se esperaba 'A'.\n");
            return 0;
        }


        if (i < longitud && entrada[i] == 'L')
        {
            printf("q20 -> q21: L\n");
            estado = Q21;
            i++;
        }
        else
        {
            printf("Error en q20: se esperaba 'L'.\n");
            return 0;
        }


        /* espacios opcionales */

        while (i < longitud && esEspacio(entrada[i]))
        {
            printf("q21 -- espacio --> q21\n");
            i++;
        }


        /* coma */

        if (i < longitud && entrada[i] == ',')
        {
            printf("q21 -> q22: ','\n");
            estado = Q22;
            i++;
        }
        else
        {
            printf("Error en q21: falta la coma.\n");
            return 0;
        }


        /* espacios */

        while (i < longitud && esEspacio(entrada[i]))
        {
            printf("q22 -- espacio --> q22\n");
            i++;
        }


        /* B */

        if (i < longitud && entrada[i] == 'B')
        {
            printf("q22 -> q23: B\n");
            estado = Q23;
            i++;
        }
        else
        {
            printf("Error en q22: se esperaba 'B'.\n");
            return 0;
        }


        /* L */

        if (i < longitud && entrada[i] == 'L')
        {
            printf("q23 -> q24: L\n");
            estado = Q24;
            i++;
        }
        else
        {
            printf("Error en q23: se esperaba 'L'.\n");
            return 0;
        }


        strcpy(inst->operacion, "ADD");

        strcpy(inst->registros[0], "AL");
        strcpy(inst->registros[1], "BL");

        inst->cantidadRegistros = 2;
        inst->tieneDireccion = 0;


        while (i < longitud && esEspacio(entrada[i]))
            i++;

        if (i == longitud)
        {
            printf("q24 -> q2\n");
            estado = Q2;
        }
        else
        {
            printf("Error: existen operandos adicionales.\n");
            return 0;
        }
    }


    /* estaso STO */

    else if (entrada[i] == 'S')
    {
        /* q1 -> q25 */

        printf("\nCaracter 'S': q1 -> q25\n");
        estado = Q25;
        i++;


        /* T */

        if (i < longitud && entrada[i] == 'T')
        {
            printf("q25 -> q26: T\n");
            estado = Q26;
            i++;
        }
        else
        {
            printf("Error en q25: se esperaba 'T'.\n");
            return 0;
        }

        /* O */

        if (i < longitud && entrada[i] == 'O')
        {
            printf("q26 -> q27: O\n");
            estado = Q27;
            i++;
        }
        else
        {
            printf("Error en q26: se esperaba 'O'.\n");
            return 0;
        }

        /* espacio */

        if (i < longitud && esEspacio(entrada[i]))
        {
            printf("q27 -> q28: espacio\n");

            while (i < longitud && esEspacio(entrada[i]))
            {
                printf("q28 -- espacio --> q28\n");
                i++;
            }
        }
        else
        {
            printf("Error en q27: se esperaba espacio.\n");
            return 0;
        }


        /* número */

        if (i < longitud && esDigito(entrada[i]))
        {
            indiceNumero = 0;

            while (i < longitud && esDigito(entrada[i]))
            {
                numeroTemporal[indiceNumero++] =
                    entrada[i];

                printf("q28 -> q29: '%c'\n",
                       entrada[i]);

                i++;
            }

            numeroTemporal[indiceNumero] = '\0';

            inst->direccion = atoi(numeroTemporal);
            inst->tieneDireccion = 1;
        }
        else
        {
            printf("Error en q28: se esperaba una dirección.\n");
            return 0;
        }


        strcpy(inst->operacion, "STO");
        inst->cantidadRegistros = 0;


        while (i < longitud && esEspacio(entrada[i]))
            i++;

        if (i == longitud)
        {
            printf("q29 -> q2\n");
            estado = Q2;
        }
        else
        {
            printf("Error: existen caracteres adicionales.\n");
            return 0;
        }
    }


    /* estados END
       ===================================================== */

    else if (entrada[i] == 'E')
    {
        /* q1 -> q30 */

        printf("\nCaracter 'E': q1 -> q30\n");
        estado = Q30;
        i++;


        /* N */

        if (i < longitud && entrada[i] == 'N')
        {
            printf("q30 -> q31: N\n");
            estado = Q31;
            i++;
        }
        else
        {
            printf("Error en q30: se esperaba 'N'.\n");
            return 0;
        }


        /* D */

        if (i < longitud && entrada[i] == 'D')
        {
            printf("q31 -> q32: D\n");
            estado = Q32;
            i++;
        }
        else
        {
            printf("Error en q31: se esperaba 'D'.\n");
            return 0;
        }


        strcpy(inst->operacion, "END");

        inst->cantidadRegistros = 0;
        inst->tieneDireccion = 0;


        while (i < longitud && esEspacio(entrada[i]))
            i++;

        if (i == longitud)
        {
            printf("q32 -> q2\n");
            estado = Q2;
        }
        else
        {
            printf("Error: END no debe tener operandos.\n");
            return 0;
        }
    }


    /* CARACTER NO RECONOCIDO */

    else
    {
        printf("\nq1 -> q3\n");
        printf("ERROR: caracter '%c' no reconocido.\n",
               entrada[i]);

        estado = Q3;

        return 0;
    }


    /*  ESTADO q2
       ===================================================== */

    if (estado == Q2)
    {
        printf("\n========================================\n");
        printf("q2 -> INSTRUCCION VALIDADA\n");
        printf("========================================\n");

        return 1;
    }

    return 0;
}


int main()
{
    char entrada[200];

    Instruccion instruccion;


    printf("========================================\n");
    printf("   ANALISIS LEXICO Y AUTOMATAS\n");
    printf("   HANDS-ON 1\n");
    printf("========================================\n");


    printf("\nIntroduce una instruccion:\n> ");

    fgets(entrada, sizeof(entrada), stdin);


    /* Eliminar salto de línea */
    entrada[strcspn(entrada, "\n")] = '\0';


    /* =====================================================
       EJECUTAR AFD */

    printf("\nVALIDACION MEDIANTE AFD\n");

    int valida = ejecutarAFD(entrada,
                             &instruccion);


    //VALIDACIÓN 

    if (!valida)
    {
        printf("\n========================================\n");
        printf("INSTRUCCION INVALIDA\n");
        printf("No se construye el objeto instruccion.\n");
        printf("No se generan microoperaciones.\n");
        printf("========================================\n");

        return 0;
    }


    /* TOKENS */

    mostrarTokens(&instruccion);


    /* COMPONENTES */

    printf("\n========== COMPONENTES IDENTIFICADOS ==========\n");

    printf("Mnemónico: %s\n",
           instruccion.operacion);

    if (instruccion.cantidadRegistros > 0)
    {
        for (i = 0;
             i < instruccion.cantidadRegistros;
             i++)
        {
            printf("Registro %d: %s\n",
                   i + 1,
                   instruccion.registros[i]);
        }
    }


    if (instruccion.tieneDireccion)
    {
        printf("Dirección de memoria: %d\n",
               instruccion.direccion);

        printf("Direccionamiento: directo\n");
    }


    /* OBJETO */

    mostrarObjeto(&instruccion);

    /* 
     LLAMADA DE MÁQUINA MORE
       ===================================================== */

    ejecutarMoore(&instruccion);

    return 0;
}
