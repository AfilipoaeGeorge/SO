// Afilipoae George-Marian 2C.1.1
// gcc -Wall -o p proiectLab.c
// ./p -o dirOutput -s dirIzolare dir dir2 dir3
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>
//Structura pentru a stoca metadatele fiecărui fișier/director
typedef struct
{
    char name[256]; //Numele fișierului/directorului
    mode_t mode;    //Permisiunile fișierului/directorului
    off_t size;     //Dimensiunea fișierului
    time_t mtime;   //Data ultimei modificări
} FileMetadata;

//compar noile metadate cu cele din fisierul anterior de snapshot
//in caz de modificari le afisez la stdin
//se verifica si daca a fost adaugat/sters un fisier
//functia primeste vectorul de metadate, cate sunt, dar si filePath-ul care contine snapshot-ul trecut pentru a fi comparate metadatele
void compare(FileMetadata *metadata, int countMetadata, char *filePathCompare)
{
    int ok = 0, matches = 0;
    int indexMetadata[countMetadata];
    int foFilePathCompare = open(filePathCompare, O_RDONLY); //deschidem fisierul pentru citire
    if (foFilePathCompare == -1)
    {
        perror("Error: open filePath in compare\n");
        exit(1);
    }

    FileMetadata aux;
    while (read(foFilePathCompare, &aux, sizeof(FileMetadata)) > 0) //citim cate o metadata din fisier
    {
        ok = 0;
        for (int i = 0; i < countMetadata; i++) //parcurgem vectorul de metadate
        {
            if (strcmp(metadata[i].name, aux.name) == 0) //cand gasim metadata din vector care are acelasi nume cu cea din vechiul snapshot verificam modificarile
            {
                indexMetadata[matches] = i;
                matches++;  //contorizam cate potriviri de nume de metadate intalnim
                ok = 1;
                if (metadata[i].mode != aux.mode) //daca si-a modificat permisiunile vom afisa acest lucru
                {
                    fprintf(stdout, "Permissions for %s changed from %o to: %o\n", metadata[i].name, aux.mode & 0777, metadata[i].mode & 0777);
                }
                if (metadata[i].size != aux.size) //daca si-a modificat size-ul vom afisa acest lucru
                {
                    fprintf(stdout, "Size for %s changed from %ld to: %ld\n", metadata[i].name, aux.size, metadata[i].size);
                }
                if (metadata[i].mtime != aux.mtime) //daca s-a schimbat timpul ultimei modificari afisam acest lucru
                {
                    char oldTime[100], newTime[100];
                    strftime(oldTime, sizeof(oldTime), "%a %b %e %T %Y", localtime(&aux.mtime));
                    strftime(newTime, sizeof(newTime), "%a %b %e %T %Y", localtime(&metadata[i].mtime));
                    fprintf(stdout, "Last modified time for %s changed from %s to: %s\n", metadata[i].name, oldTime, newTime);
                }
            }
        }
        //daca ok ramane pe 0 inseamna ca numele metadatei din snapshot-ul anterior nu se mai gaseste in vectorul nou
        //inseamna ca a fost sters acest fisier din director si afisam acest lucru
        if (ok == 0)
        {
            fprintf(stdout, "The file %s has been deleted from this directory.\n", aux.name);
        }
    }

    if (close(foFilePathCompare) == -1) //inchidem fisierul cu snapshot-ul anterior
    {
        perror("Error: close filePath in compare\n");
        exit(1);
    }
    //verificam daca exista metadate in plus fata de snapshot-ul anterior
    //daca da, inseamna ca a/au fost adaugat/e fisier/e nou/noi si afisam acest lucru 
    for (int i = 0; i < countMetadata; i++)
    {
        int found = 0;
        for (int j = 0; j < matches; j++)
        {
            if (i == indexMetadata[j])
            {
                found = 1;
                break;
            }
        }
        if (!found)
        {
            fprintf(stdout, "The file %s has been added to this directory.\n", metadata[i].name);
        }
    }
}

//fac un fisier auxiliar snapshot.txt sa pot observa mai usor snapshot-ul
//scriu vectorul de metadate in fisier, care reprezinta si snaspsot-ul pentru directorul dirPath
//primeste ca parametrii:
// -vectorul de metadate, numarul de metadate, directorul de output, fisierul specific pentru snapshot, numele directorului pentru care s-a facut snapshot-ul
void saveSnapshot(FileMetadata *metadata, const char *dirPath, const char *dirPathOutput, char *filePath, int countMetadata)
{
    FILE *file = fopen("snapshot.txt", "aw");
    if (file == NULL)
    {
        perror("Error: open snapshot.txt\n");
        exit(1);
    }
    fprintf(file, "Snapshot pentru directorul: %s\n", dirPath);
    fprintf(file, "----------------------------------------------\n");
    for (int i = 0; i < countMetadata; i++)
    {
        fprintf(file, "Nume: %s\n", metadata[i].name);
        fprintf(file, "Permisiuni: %o\n", metadata[i].mode & 0777);
        fprintf(file, "Dimensiune: %ld bytes\n", metadata[i].size);
        fprintf(file, "Ultima modificare: %s", ctime(&metadata[i].mtime));
        fprintf(file, "----------------------------------------------\n");
    }

    if (fclose(file))
    {
        perror("Error: close secondary file\n");
        exit(1);
    }
    //deschidem directorul de output
    DIR *dir = opendir(dirPathOutput);
    if (dir == NULL)
    {
        perror("Error: open dir in save\n");
        exit(1);
    }

    int foFileOutput;
    foFileOutput = open(filePath, O_TRUNC); //deschidem fisierul pentru snapshot cu TRUNC pentru a sterge continutul
    close(foFileOutput); //il inchidem inapoi
    foFileOutput = open(filePath, O_WRONLY); //il redeschidem cu WRONLY pentru a scrie noul snapshot in el
    if (foFileOutput == -1)
    {
        perror("Error: open write file in save\n");
        exit(1);
    }
    //parcurgem vectorul de metadate si scriem pe rand cu write in fisier
    for (int i = 0; i < countMetadata; i++)
    {
        if (write(foFileOutput, &metadata[i], sizeof(FileMetadata)) == -1)
        {
            perror("Error: writing metadata in the output file in save\n");
            exit(1);
        }
    }

    if (close(foFileOutput) == -1) //inchidem fisierul
    {
        perror("Error: close write file in save\n");
        exit(1);
    }

    if (closedir(dir) == -1) //inchidem directorul
    {
        perror("Error: close dir in save\n");
        exit(1);
    }
}

//obtin metadatele pentru fisierul path
void getMetadata(const char *path, FileMetadata *metadata)
{
    struct stat st;
    if (lstat(path, &st) == -1)
    {
        perror("Error: metadata-getMetadata");
        exit(EXIT_FAILURE);
    }

    strcpy(metadata->name, path);
    metadata->mode = st.st_mode;
    metadata->size = st.st_size;
    metadata->mtime = st.st_mtime;
}

//verific permisiunile fisierului filePath
//daca are permisiuni 000, se porneste un proces nepot in care apelez script-ul verify_for_malicious.sh
//in caz ca este periculos il mut in dirPathIzolat
void checkPermissions(const char *filePath, const char *dirPathIzolat, int* isSave, int* countUnsafeFile)
{
    int status;
    struct stat st;
    if (stat(filePath, &st) == -1)
    {
        perror("Error: metadata-checkPermissions");
        exit(EXIT_FAILURE);
    }
    //se verifica permisiunile de citire,scriere,executie
    if ((st.st_mode & S_IRWXU) == 0 && (st.st_mode & S_IRWXG) == 0 && (st.st_mode & S_IRWXO) == 0)
    {
        (*isSave) = 0;
        (*countUnsafeFile) ++;
        int pfd[2];
        char string[100];
        //facem pipe pentru a putea lua datele(din procesul fiu) scrise de script in procesul parinte
        if (pipe(pfd) < 0)
        {
            perror("Error: pipe-checkPermissions");
            exit(1);
        }
        int pid = fork(); //facem procesul nepot in care se executa script-ul pentru analiza fisierului
        if (pid < -1)
        {
            perror("Error: fork-checkPermissions");
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            printf("GrandChild Process %d startes with PID %d for file %s.\n",(*countUnsafeFile),getpid(),filePath);
            close(pfd[0]); //inchidem capatul de citire
            dup2(pfd[1], 1); //redirecteaza iesirea standard spre pipe
            execlp("./verify_for_malicious.sh", "verify_for_malicious.sh", filePath, NULL); //executam script-ul pentru analiza
            perror("Error: execlp-checkPermissions"); //daca se ajunge aici inseamna ca a fost o eroare la execlp
            exit(1);
        }
        else
        {
            close(pfd[1]); //inchidem capatul de scriere
            int status;
            waitpid(pid, &status, 0); //asteptam pid-ul pid
            read(pfd[0], string, sizeof(string) - 1); //citim din pipe ce a scris script-ul
            close(pfd[0]); //inchidem capatul de citire
            string[strcspn(string, "\n")] = 0; // elimină newline-ul dacă există
            //printf("%s-%s\n", filePath, string);
            if (strcmp(string, "SAFE") != 0) //daca script-ul nu a scris SAFE fisierul trebuie mutat in directorul de izolare
            {
                chmod(filePath, 777); //ii dam permisiuni
                char newPath[100];
                char copyFilePath[100];
                strcpy(copyFilePath, filePath); //facem o copie a numelui pentru a o putea "sparge" si a lua ce avem nevoie
                char *p = strtok(copyFilePath, "/");
                p = strtok(NULL, "/");
                //construim newPath-ul
                strcpy(newPath, dirPathIzolat);
                strcat(newPath, "/");
                strcat(newPath, p);
                if (rename(filePath, newPath) == 0) //prin rename mutam fisierul in directorul de izolare
                {
                    printf("File renamed and moved successfully\n");
                    chmod(newPath, 000); //ii stergem inapoi permisiunile, facandu-le inapoi 000
                }
                else
                {
                    printf("Unable to rename files\n");
                }
            }
        }
        waitpid(pid,&status,0);
        //afisam cand se incheie procesul nepot, cuu ce pid si cu ce status
        //daca fisierul va fi evaluat ca SAFE procesul nepot se va incheia cu status=2, altfel daca nu e SAFE cu status=1
        printf("GradChild Process %d terminated with PID %d and exit code %d.\n",(*countUnsafeFile),pid,WEXITSTATUS(status));
    }
}

//parcurg directorul recursiv si construiesc vectorul de metadate
void traverseDirectory(FileMetadata *metadata, int *countMetadata, char *nameDir, const char *dirPath, const char *dirPathOutput, int countExt, const char *dirPathIzolat, int *countSuspiciousFile)
{
    DIR *dir = opendir(dirPath); //se deschide directorul
    if (dir == NULL)
    {
        perror("Error: open dir in traverse\n");
        exit(1);
    }

    struct dirent *entry;
    struct stat fileStat;
    int isSave = 1;
    int countUnsafeFile = 0;

    while ((entry = readdir(dir)) != NULL) //cat timp se mai este ceva in director care nu a fost citit
    {
        isSave = 1;
        snprintf(nameDir, sizeof(char) * 512, "%s/%d.txt", dirPathOutput, countExt);
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char filePath[512];
            snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);

            stat(filePath, &fileStat);
            if (S_ISDIR(fileStat.st_mode))
            {
                //daca este subdirector
                getMetadata(filePath, &metadata[(*countMetadata)++]); //se iau metadatele corespunzatoare lui
                traverseDirectory(metadata, countMetadata, nameDir, filePath, dirPathOutput, countExt, dirPathIzolat, countSuspiciousFile); //se apeleaza recursiv pentru acest subdirector
            }
            else
            {
                //daca este fisier
                checkPermissions(filePath, dirPathIzolat, &isSave, &countUnsafeFile); //verificam permisiunile
                if (isSave == 1) //daca fisierul este SAFE
                {
                    getMetadata(filePath, &metadata[(*countMetadata)++]); //se iau metadatele corespunzatoare lui
                }
                else //trecem peste el si marim numarul de fisiere suspicioase
                {
                    (*countSuspiciousFile)++;
                    continue;
                }
            }
        }
    }

    if (closedir(dir) == -1) //se inchide directorul
    {
        perror("Error: close dir in traverse\n");
        exit(1);
    }
}

//in cazul in care nu exista director il creez
void check_or_create_dir(const char *dirPath)
{
    struct stat st;
    if (stat(dirPath, &st) == -1) //daca nu exista
    {
        if (mkdir(dirPath, 0777) == -1) //il cream si ii dam permisiuni 0777
        {
            perror("Error: create dir out\n");
            exit(1);
        }
    }
}

//in cazul in care nu exista fisierul -> filePath il cream cu drepturi si cu permisiuni de 0777
void check_or_create_file(char *filePath)
{
    int foFilePath = open(filePath, O_WRONLY);
    if (foFilePath == -1)
    {
        creat(filePath, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
        chmod(filePath, 0777);
    }
}

int main(int argc, char *argv[])
{
    if (argc > 15 || argc < 4) //verificam numarul de argumente
    {
        perror("Wrong number of arguments\n");
        exit(1);
    }

    const char *dirPathOutput = argv[2]; //luam directorul de output
    const char *dirPathIzolat = argv[4]; //luam directorul de izolare
    check_or_create_dir(dirPathOutput); //verificam daca exista directorul de output, daca nu il cream
    check_or_create_dir(dirPathIzolat); //verificam daca exista directorul de izolare, daca nu il cream
    struct stat auxStat;
    //countExt il folosim pentru a transmite in functia traverseDirectory un numar corespunzator pe care sa il folosim la crearea numelui pentru snapshot
    //pentru a contoriza input-urile valide care sunt directoare nu fisiere
    int countExt = 0, countValidInputDir = 0;
    FileMetadata metadata[100];   //vector de maxim 100(poate stoca metadatele pentru maxim 100 directoare)
    int countProcesses = argc - 5;  //-5 pentru ca avem ./p(1) -o(2) dirOutput(3) -s(4) dirIzolare(5)
    char paux[countProcesses][6];   //un vector de cuvinte(adica o matrice de caractere) pentru a citi in el
    int *pids = malloc(sizeof(int) * countProcesses);   //un vector pentru a retine pid-urile in el
    int *validInputDir = malloc(sizeof(int) * countProcesses);   //un vector in care punem 1 la indexul care corespunde cu un director

    for (int i = 5; i < argc; i++) //parcurgem lista de argumente
    {
        stat(argv[i], &auxStat);
        if (!(S_ISDIR(auxStat.st_mode))) //verificam daca argumentul este director
        {
            continue; //daca nu trecem mai departe la urmatorul argument
        }
        const char *dirPath = argv[i]; //luam numele directorului cu numarul i si il punem in dirPath
        int countMetadata = 0; //contor pentru numarul de metadate dintr-un director
        char nameDir[512];
        char filePath[512]; 
        countExt++; countValidInputDir++; //daca a fost valid marim contoarele
        validInputDir[i - 5] = 1; //punem in vector 1 pe pozitia corespunzatoare

        int pid; 
        int pfd[2]; //vectorul pentru pipe
        if (pipe(pfd) < 0) //facem un pipe pentru a putea lua numarul de fisiere suspicioase care se calculeaza in procesul fiu
        {
            perror("Error: pipe-main");
            exit(1);
        }
        if ((pid = fork()) < 0) //facem un proces fiu pentru a parcurge directorul si pentru a-i face un snapshot
        {
            perror("Error: fork-main\n");
            exit(1);
        }
        
        if (pid == 0) //daca suntem in procesul fiu
        {
            int countSuspiciousFile = 0; //initializam numarul fisierelor suspicioase din director cu 0
            char p[6]; //un vector de caractere in care sa scriem numarul procesului(directorului), numarul de fisiere suspicioase
            close(pfd[0]); //inchidem capatul de citire
            printf("Child Process %d started with PID %d for directory %s\n",countValidInputDir, getpid(), dirPath); //printam un ,esaj cand incepe procesul fiu
            traverseDirectory(metadata, &countMetadata, nameDir, dirPath, dirPathOutput, countExt - 1, dirPathIzolat, &countSuspiciousFile); //traversam racursiv directorul, construim vectorul de metadate si numarul de fisiere suspicioase
            sprintf(p, "p%d%02d", i - 5, countSuspiciousFile); //scriem in vectorul de caractere
            write(pfd[1], p, sizeof(char) * 6); //il scriem  in pipe
            //printf("%d-%s\n", getpid(), p);
            close(pfd[1]); //iinchidem capatul de scriere
            snprintf(filePath, sizeof(filePath), "%s/%d.txt", dirPathOutput, countExt - 1); //compunem numele fisierului in care se fa afla snapshot-ul pentru un anumit director(directorul cu numarul countExt-1)
            check_or_create_file(filePath); //verificam daca deja exista acel nume, daca nu cream acel fisier
            compare(metadata, countMetadata, filePath); //comparam snapshot-ul anterior cu actualul vector care contine noile metadate despre director(practic noul snapshot)
            saveSnapshot(metadata, dirPath, dirPathOutput, filePath, countMetadata); //salvam snapshot-ul in fisierul cu numele filePath

            printf("Snapshot for directory %s created successfully!\n", dirPath); //afisam mesaj ca snapshot-ul a fost creat
            exit(0);
        }
        else
        {
            close(pfd[1]); //inchidem capatul de scriere
            pids[i - 5] = pid; //punem pid-ul corespunzator actualului proces in vectorul pids, pentru a putea folosi waitpid si a sti ce proces asteptam
            sleep(1); //pentru a se putea observa cum se executa
        }
        read(pfd[0], paux[i - 5], sizeof(char) * 6); //citim din pipe si punem in matricea de caractere(astfel avand numarul de fisiere suspicioase)
        close(pfd[0]); //inchidem capatul de citire
    }
    //un contor folosit doar pentru afisare
    //il crestem doar daca intalnim un input valid, ceea ce inseamna ca va afisa corect numarul procesului chiar daca vor fi intalnite si fisiere in argumentele din linia de comanda
    int countForPrint = 0;
    for (int i = 0; i < countProcesses; i++)
    {
        if(validInputDir[i] == 1){ //daca input-ul este valid
            int status;
            countForPrint ++; //marim contorul pentru afisare
            waitpid(pids[i], &status, 0); //asteptam pid-ul pids[i]
            //afisam un mesaj de incheiere a procesului fiu
            printf("Child Process %d terminated with PID %d and exit code %d with %c%c potential suspicious file.\n", countForPrint, pids[i], WEXITSTATUS(status), paux[i][2], paux[i][3]);
            sleep(1); //pentru a se observa mai bine cum se termina
        }
    }
    
    return 0;
}