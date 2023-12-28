//bouguettaia imane 
//groupe 2
//projet se tp
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define N 3 // Taille du tampon
#define n1 3 // Nombre de lignes de B
#define m1 3 // Nombre de colonnes de B (également nombre de lignes de C)
#define n2 3 // Nombre de colonnes de C

// Matrices
double B[n1][m1], C[m1][n2], A[n1][n2];
// Tampon
double T[N];

// Synchronisation
pthread_mutex_t mutex;
sem_t empty;
sem_t full;

// Prototypes des fonctions
int getNextRow();
void calculateRow(int row);
void insertRow(int row);
int removeItem();

// Fonction pour saisir une matrice à partir de l'utilisateur
void inputMatrixFromUser(double matrix[][n2], int rows, int cols, const char *matrixName) {
    printf("Enter matrix %s (%d x %d):\n", matrixName, rows, cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            printf("Enter element at position (%d, %d): ", i + 1, j + 1);
            scanf("%lf", &matrix[i][j]);
        }
    }
}

// Fonction pour afficher une matrice
void printMatrixToConsole(double matrix[][n2], int rows, int cols, const char *matrixName) {
    printf("Matrix %s:\n", matrixName);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }
}

// Producteur
void *producer(void *arg) {
    int row;
    while (1) {
        // Obtenir la prochaine ligne à calculer
        pthread_mutex_lock(&mutex);
        row = getNextRow();
        pthread_mutex_unlock(&mutex);

        // Vérifier la condition de terminaison
        if (row == -1)
            break;

        // Calculer la ligne de la matrice résultante A et la stocker dans le tampon T
        calculateRow(row);

        sem_wait(&empty); // Attente d'une place libre
        pthread_mutex_lock(&mutex);
        // Section critique
        insertRow(row);
        pthread_mutex_unlock(&mutex);
        sem_post(&full); // Il y a une place remplie en plus
    }

    pthread_exit(NULL);
}

// Consommateur
void *consumer(void *arg) {
    int row;

    while (1) {
        sem_wait(&full); // Attente d'une place remplie
        pthread_mutex_lock(&mutex);
        // Section critique
        row = removeItem();
        pthread_mutex_unlock(&mutex);
        sem_post(&empty); // Il y a une place libre en plus

        // Vérifier la condition de terminaison et quitter si nécessaire
        if (row == -1)
            break;

        // Afficher la ligne résultante
        printMatrixToConsole(A + row, 1, n2, "A (Row)");
    }

    pthread_exit(NULL);
}

// Fonction pour obtenir la prochaine ligne à calculer
int getNextRow() {
    static int currentRow = 0;
    if (currentRow < n1) {
        return currentRow++;
    } else {
        return -1; // Retourner -1 pour indiquer que toutes les lignes ont été calculées
    }
}

// Fonction pour calculer la ligne de la matrice résultante A et la stocker dans le tampon T
void calculateRow(int row) {
    for (int col = 0; col < n2; ++col) {
        double result = 0.0;
        for (int i = 0; i < m1; ++i) {
            result += B[row][i] * C[i][col];
        }
        T[col] = result;
    }
}

// Fonction pour insérer une ligne dans la matrice résultante A
void insertRow(int row) {
    for (int col = 0; col < n2; ++col) {
        A[row][col] = T[col];
    }
}

// Fonction pour retirer un élément du tampon T
int removeItem() {
    static int currentRow = 0;
    if (currentRow < n1) {
        return currentRow++;
    } else {
        return -1; // Retourner -1 pour indiquer que toutes les lignes ont été consommées
    }
}

// Fonction principale
int main() {
    // Saisir les matrices B et C à partir de l'utilisateur
    inputMatrixFromUser(B, n1, m1, "B");
    inputMatrixFromUser(C, m1, n2, "C");

    // Initialisation
    sem_init(&empty, 0, N);  // Buffer vide
    sem_init(&full, 0, 0);   // Buffer vide
    pthread_mutex_init(&mutex, NULL); // Exclusion mutuelle

    // Création des threads producteurs
    pthread_t producers[n1];
    for (int i = 0; i < n1; ++i) {
        pthread_create(&producers[i], NULL, producer, NULL);
    }

    // Création des threads consommateurs
    pthread_t consumers[n1];
    for (int i = 0; i < n1; ++i) {
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }

    // Attente des threads producteurs
    for (int i = 0; i < n1; ++i) {
        pthread_join(producers[i], NULL);
    }

    // Indiquer aux consommateurs que la production est terminée
    for (int i = 0; i < n1; ++i) {
        sem_post(&full);
    }

    // Attente des threads consommateurs
    for (int i = 0; i < n1; ++i) {
        pthread_join(consumers[i], NULL);
    }

    // Afficher les matrices et le résultat à la fin du programme
    printMatrixToConsole(B, n1, m1, "B");
    printMatrixToConsole(C, m1, n2, "C");
    printMatrixToConsole(A, n1, n2, "A");

    // Destruction des ressources
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}
