#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <regex.h>
#include <sys/wait.h>
#define MACOMS 4
#define CHINT 1024
#define MANUMPIP 7
#define MAARGMINI 5
#define MATRUNOUTP 1
#define CHOUTAPP 2

//Analysis BackSlash
int Maescapebackslash(const char* MAst, int Chindex) {
if (Chindex > 0 && MAst[Chindex-1] == '\\') {
    int MAcnt = 0;
        while (Chindex > 0 && MAst[Chindex-1] == '\\') {
            MAcnt++;
            Chindex--;
        }return MAcnt % 2 != 0;
    }
return 0;
}
char* MAwhitespaceTrim(char *CHstrwhite)//Whitspace Analysis and Trim 
{
    int m;
    while (isspace (*CHstrwhite)) {  
        CHstrwhite++;  
    }
    for (m = strlen (CHstrwhite) - 1; (isspace (CHstrwhite[m])); m--) ;   
    CHstrwhite[m + 1] = '\0';
return CHstrwhite;
}
//introducing spaces between special characters
char* MAformatCmdspecial(const char* MAcmdpromp) {
    //Variable to store SpecialChar
    const char* CHspecialcha = "<>|&;";
    const char* CHspecialcha2 = ">|&";
    size_t Malen2 = strlen(MAcmdpromp);
    char* MAcmdnew = malloc(Malen2*2 + 1); 
    size_t m = 0, n = 0;

    //for continous commands
    while (m < Malen2) {
        if (strchr(CHspecialcha, MAcmdpromp[m]) != NULL && !Maescapebackslash(MAcmdpromp, m)) { 
            if (strchr(CHspecialcha2, MAcmdpromp[m]) != NULL &&
                m + 1 < Malen2 &&
                MAcmdpromp[m] == MAcmdpromp[m+1]) {
                MAcmdnew[n++] = ' ';
                MAcmdnew[n++] = MAcmdpromp[m++];
                MAcmdnew[n++] = MAcmdpromp[m];
                MAcmdnew[n++] = ' ';
                MAcmdnew[n++] = ' ';
            } else {
                MAcmdnew[n++] = ' ';
                MAcmdnew[n++] = MAcmdpromp[m];
                MAcmdnew[n++] = ' ';
            }
        } else {   
            MAcmdnew[n++] = MAcmdpromp[m];
        }
        m++;
    }
    MAcmdnew[n] = '\0';
    return MAcmdnew;//results
}

//number of arg in cmd checker
int CHcmdcheck (char* MAcmdpromp) {
    char* cmd = strdup(MAcmdpromp);//store cmds
    cmd = MAwhitespaceTrim(cmd);
    char* MAst = strtok (cmd," ");//checkdelimiter
    int MAarcg = 0;
    while (MAst != NULL) {
        MAst = strtok (NULL, " ");
        MAarcg++;
    }
    return MAarcg <= MAARGMINI && MAarcg > 0;
}

int MATokenizerSting(char *MAst, char **MATok)//Creates tokens form sgtring
{
int ChToKnew = 0; 
int Malen2 = strlen(MAst);
    int m;
    int Chindx = 0;
    for (m = 0; m < Malen2; m++)
    {
    if (strncmp(MAst + m, "&&", 2) == 0)//checks for "&&"
        {
            MATok[ChToKnew] = malloc((m - Chindx + 1) * sizeof(char)); 
            strncpy(MATok[ChToKnew++], MAst + Chindx, m - Chindx);
            MATok[ChToKnew++] = "&&";
            m++;
            Chindx = m + 2;
        }
        else if (strncmp(MAst + m, "||", 2) == 0)
        {
            MATok[ChToKnew] = malloc((m - Chindx + 1) * sizeof(char));
            // copying
            strncpy(MATok[ChToKnew++], MAst + Chindx, m - Chindx);
            MATok[ChToKnew++] = "||";
            m++;
            Chindx = m + 2;
        }
        else if (MAst[m] == '|')
        {
            MATok[ChToKnew] = malloc((m - Chindx + 1) * sizeof(char));
            strncpy(MATok[ChToKnew++], MAst + Chindx, m - Chindx);
            MATok[ChToKnew++] = "|";
            Chindx = m + 1;
        }
        else if (MAst[m] == ';')
        {
            MATok[ChToKnew] = malloc((m - Chindx + 1) * sizeof(char));
            strncpy(MATok[ChToKnew++], MAst + Chindx, m - Chindx);
            MATok[ChToKnew++] = ";";
            Chindx = m + 1;
        }
    }
    MATok[ChToKnew] = malloc((Malen2 - Chindx + 1) * sizeof(char));
    strncpy(MATok[ChToKnew++], MAst + Chindx, Malen2 - Chindx);
    return ChToKnew;
}
//execute cmd individually
int CHexecmd1(char* cmd, int CHforkcheck) {
    char *MAdel = " "; //split
    char *MAFileDist = NULL; 
    int CHback = 0; 
    char *MAFileIn = NULL; 
    char *MATok;
    char *MAarcg[10];
    char* MAcmdpromp = strdup(cmd);
    MAcmdpromp = MAwhitespaceTrim(MAcmdpromp);
    int MAmodOut = MATRUNOUTP;
    //look "&"
    if (MAcmdpromp[strlen(MAcmdpromp) - 1] == '&') {
        CHback = 1;
        MAcmdpromp[strlen(MAcmdpromp) - 1] = '\0';
        MAcmdpromp = MAwhitespaceTrim(MAcmdpromp);}
    // look "exit" 
    if (strcmp(MAcmdpromp, "exit") == 0) {
        exit(0);
    }
    // look "cd"
    if (strcmp(MAcmdpromp, "cd") == 0) {
        chdir(getenv("HOME"));
        return 0;
    } else if (strstr(MAcmdpromp, "cd ") == MAcmdpromp) //for cd change dir
    {
        char* Madir = MAcmdpromp + 3; 
        Madir = MAwhitespaceTrim(Madir);
        chdir(Madir);
        return 0;
    }
    int m = 0;
    char* MAnextTok;
    MATok = strtok_r(MAcmdpromp, MAdel, &MAnextTok);
    while (MATok != NULL) {
        // look "<"
        if (strcmp(MATok, "<") == 0) {
            MAFileIn = strtok_r(NULL, MAdel, &MAnextTok);
        } else if (strcmp(MATok, ">") == 0)//look ">"
         {
            MAFileDist = strtok_r(NULL, MAdel, &MAnextTok);
            // output mode
            MAmodOut = MATRUNOUTP;
        } else if (strcmp(MATok, ">>") == 0)//look ">>"
         {
            MAFileDist = strtok_r(NULL, MAdel, &MAnextTok);
            MAmodOut = CHOUTAPP;
        } else {
            MAarcg[m++] = MATok;
        }
        MATok = strtok_r(NULL, MAdel, &MAnextTok);
    }
    MAarcg[m] = NULL;
    int CHfork = 0;
    if (CHforkcheck) {
        CHfork = fork();
    }
    if (CHfork < 0) {
        fprintf(stderr, "Unable to perform fork\n");//message
        exit(1);
    } else if (CHfork == 0) {
        int MAfileDistIN, CHfileDistOut;
        if (MAFileIn) {
            MAfileDistIN = open(MAFileIn, O_RDONLY);
            if (MAfileDistIN < 0) {
                fprintf(stderr, "Unable to open file\n");
                exit(1);
            }
            dup2(MAfileDistIN, STDIN_FILENO);
            close(MAfileDistIN);
        }
        if (MAFileDist) {
            if (MAmodOut == MATRUNOUTP) {
                CHfileDistOut = open(MAFileDist, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            } else if (MAmodOut == CHOUTAPP) {
                CHfileDistOut = open(MAFileDist, O_WRONLY | O_CREAT | O_APPEND, 0666);
            }
            if (CHfileDistOut < 0) {
                fprintf(stderr, "Unable to open file\n");
                exit(1);
            }
            dup2(CHfileDistOut, STDOUT_FILENO);

            close(CHfileDistOut);
        }
        if (execvp(MAarcg[0], MAarcg) < 0) {
            fprintf(stderr, "Uable to Exec\n");
            exit(1);
        }
    } else {
        if (!CHback) {
            int MAsts;
            wait(&MAsts);
            return MAsts;
        } else {
            printf("Process running in Back =  %d\n", CHfork);
            return 0;
        }
    }
    if (CHforkcheck == 0) {
        if (!CHback) {
            int MAsts;
            wait(&MAsts);
            return MAsts;
        } else {
            printf("Process running in Back = %d\n", CHfork);
            return 0;
        }
    }
    return 0;
    }
//Pipe proces exec
int ChpipExe (char** MAcmds, int CHnoCmd) {
    int CHfork = fork();
    int m=0;
    if (!CHfork) {
        for(  m=0; m<CHnoCmd-1; m+=2)
        {
            int CHpip[2];
            pipe(CHpip);
            if (!fork()) {
                dup2(CHpip[1], 1);//redirection
                CHexecmd1(MAcmds[m], 0);
                perror("exec");
            }
            dup2(CHpip[0], 0);//redirection out to intput
            close(CHpip[1]);
        }
        CHexecmd1(MAcmds[m], 0);
    } else if (CHfork > 0) {
        char* MAcmdpromp = strdup(MAcmds[CHnoCmd - 1]);
        MAcmdpromp = MAwhitespaceTrim(MAcmdpromp);
        if (MAcmdpromp[strlen(MAcmdpromp) - 1] != '&')//waiting 
        {
            waitpid(CHfork, NULL, 0);
        } else {
            printf("Process running in Back = %d\n", CHfork);
        }
    }
}

void MAExecCmds(char **MAcmds, int Malen2) {
    int ChResult = 1;
    int m = 0;
    int MAfinal = 0;
    int MAfileDis[2];
    m=0;
    while (m<Malen2) {
        // look "||"" 
        if (strcmp(MAcmds[m], "||") == 0)//skip to cmd next
         {
            if (ChResult == 0) {
                m++;
                while (m<Malen2 && strcmp(MAcmds[m], "&&") != 0 && 
                strcmp(MAcmds[m], "|") != 0 &&
                strcmp(MAcmds[m], ";") != 0) {
                    m++;
                }
                if (m>0 && m<Malen2 && strcmp(MAcmds[m], "|") == 0) m+=2;
            } else {
                m++;
            }
        } else if (strcmp(MAcmds[m], "&&") == 0) {
            if (ChResult != 0) {
                // look && 
                m++;
                while (m<Malen2 && strcmp(MAcmds[m], "||") != 0 && 
                strcmp(MAcmds[m], "|") != 0 &&
                strcmp(MAcmds[m], ";") != 0) {
                    m++;
                }
                if (m>0 && m<Malen2 && strcmp(MAcmds[m], "|") == 0) m+=2;
            } else {
                m++;
            }
        } else if (strcmp(MAcmds[m], "|") == 0) {
            m++;
        } else if (strcmp(MAcmds[m], ";") == 0) {
            m++;
        } 
        else if (m<Malen2-1 && strcmp(MAcmds[m+1], "|") == 0) {
            int Chstrt = m;
            while(m<Malen2-1 && strcmp(MAcmds[m+1], "|") == 0) m+=2;
            ChpipExe(MAcmds+Chstrt, m-Chstrt+1);
            m++;
        }
        else {
            MAfinal = CHexecmd1(MAcmds[m], 1);
            ChResult = MAfinal;
            m++;
        }
    }
}
int ChvalCmds(char** CHintUsercmd, int CHnoCmds) {
    if (CHnoCmds%2 == 0)//correction of user input
     {
        printf("Please write correct input\n");
        return 0;
    }
    if (CHnoCmds > 2*MACOMS - 1)//no more than 5
     {
        printf("Please insert upto five arg\n");//error
        return 0;
    }
    for (int m=0;m<CHnoCmds;m++){
        if (m%2) continue;
        int CHValCheck = CHcmdcheck(CHintUsercmd[m]);
        if (!CHValCheck) {
            printf("Please write correct input\n");//error
            return 0;
        }
    }
    return 1;
}
int main()
{
    while (1) {
        char* Mintdat = malloc(4096);
        fflush(stdout);
        printf(" mshell$ ");// shell name
        fgets(Mintdat, CHINT, stdin);
        Mintdat = MAwhitespaceTrim(Mintdat);
        if (strlen(Mintdat) == 0) {
            continue;
        }
        char* MAintNew = MAformatCmdspecial(Mintdat);
        char *MATok[150];
        int MATokNew = MATokenizerSting(MAintNew, MATok);
        int CHValCheck = ChvalCmds(MATok, MATokNew);
        if (CHValCheck) MAExecCmds(MATok, MATokNew);
    }
    return 0;
}
