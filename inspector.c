/**
 * @file
 *
 * System inspector implementation: a Unix utility that inspects the system it
 * runs on and creates a summarized report for the user using the proc pseudo
 * file system.
 *
 * See specification here: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-1.html
 */

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"

#define BUF_SZ 1024


/* Function prototypes */
void print_usage(char *argv[]);

/**
 * This struct is a collection of booleans that controls whether or not the
 * various sections of the output are enabled.
 */
struct view_opts {
    bool hardware;
    bool live_view;
    bool system;
    bool task_list;
};


/** 
* Function that reads each line in a file using the read system call.
*/
int read_line(char *str, size_t size, int fd)
{
    int i = 0;
    ssize_t read_sz;
    while ((read_sz = read(fd, &str[i], 1)) > 0 && i < size)
    {
        if (str[i] == '\n' || str[i] == '\0')
        {
            i++;
            break;
        }
        i++;
    }
    return i;
}

/**
* Function that uses read_line to read a line in a file
*/
char * read_file(char *file)
{

    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        perror("open");
    }

    static char buf[BUF_SZ];
    ssize_t read_sz;
    while ((read_sz = read_line(buf, BUF_SZ, fd)) > 0) 
    {
        buf[read_sz+1] = '\0';
        buf[strcspn(buf, "\n")] = ' ';
    }

    close(fd);
    return buf;
}

/**
 * Retrieves the next token from a string.
 *
 * Parameters:
 * - str_ptr: maintains context in the string, i.e., where the next token in the
 *   string will be. If the function returns token N, then str_ptr will be
 *   updated to point to token N+1. To initialize, declare a char * that points
 *   to the string being tokenized. The pointer will be updated after each
 *   successive call to next_token.
 *
 * - delim: the set of characters to use as delimiters
 *
 * Returns: char pointer to the next token in the string.
 */
char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  == 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}

/**
* Function to find and print the system info from the proc file system (default or other)
* System info: Hostname, kernel version, uptime
*/
void sys_info()
{
    //call read_file on each relative path
    printf("System Information\n------------------\n");
    //Hostname: /proc/sys/kernel/hostname
    printf("Hostname: ");
    printf("%s\n", read_file ("sys/kernel/hostname"));
    //Kernel version: /proc/version
    printf ("Kernel Version: ");
    printf ("%s\n", read_file ("sys/kernel/osrelease"));
    //Uptime: /proc/uptime
    printf ("Uptime: ");
    int time;
    int tokens = 0;
    char *curr_tok;
    char *next_tok_up =read_file ("uptime");
    while ((curr_tok = next_token(&next_tok_up, " |,?!\n")) != NULL) {
        if (tokens++ == 0)
        {
            //format into an integer (seconds)
            time = atoi(curr_tok);
            //convert seconds into years, days, hours, minutes, seconds
            int y, d, h, m, s;
            y = time/60/60/24/365;
            time -= y*60*60*24*365;
            d = time/(24*60*60);
            time -= d*24*60*60;
            h = time/3600;
            time -= h*3600;
            m = time/60;
            time -= m*60;
            s = time;
            //need to handle edge cases (0's) when printing
            if (y == 0) {
                if (d == 0) {
                    if (h == 0) {
                        printf("%d minutes, %d seconds\n", m, s);
                    } else {
                        printf("%d hours, %d minutes, %d seconds\n", h, m, s);
                    } 
                } else {
                    if (h == 0) {
                        printf(" %d days, %d minutes, %d seconds\n", d, m, s);
                    } else {
                        printf("%d days, %d hours, %d minutes, %d seconds\n", d, h, m, s);
                    } 
                }
            } else {
                printf("%d years, ", y);

                if (d == 0) {
                    if (h == 0) {
                        printf("%d minutes, %d seconds\n", m, s);
                    } else {
                        printf("%d hours, %d minutes, %d seconds\n", h, m, s);
                    } 
                } else {
                    if (h == 0) {
                        printf(" %d days, %d minutes, %d seconds\n", d, m, s);
                    } else {
                        printf("%d days, %d hours, %d minutes, %d seconds\n", d, h, m, s);
                    } 
                }
            }
        }
    }
}

/**
* Function to get and print hardware info.
* Information needed: CPU Model, Processing Units, Load Average, CPU Usage, and Memory Usage
*/
void hardware_info()
{
    printf ("Hardware Information\n");
    printf ("--------------------\n");

    printf ("CPU Model: ");
    int fd = open("cpuinfo", O_RDONLY);
    if (fd == -1) {
        perror("open");
    }

    static char buf[BUF_SZ];
    size_t read_sz;
    int units = 0;
    while ((read_sz = read_line(buf, BUF_SZ, fd)) > 0) 
    {
        buf[read_sz+1] = '\0';
        buf[strcspn(buf, "\n")] = ' ';
        if (strstr (buf, "model name"))
        {
            int tokens = 0;
            char *curr_tok;
            char *next_tok_cpu = buf;
            while ((curr_tok = next_token(&next_tok_cpu, " :,?!")) != NULL) 
            {
                if (tokens++ == 2)
                {
                    printf ("%s", curr_tok);
                    printf (" %s", next_tok_cpu);
                }
            }
            break;
        }
    }
    close (fd);

    fd = open("cpuinfo", O_RDONLY);
    if (fd == -1) {
        perror("open");
    }
    while ((read_sz = read_line(buf, BUF_SZ, fd)) > 0) 
    {   
        if (strstr(buf, "processor"))
        {
            units++;
        }
    }
    close(fd);
    printf ("\n");
    printf ("Processing Units: %d\n", units);

    printf ("Load Average (1/5/15 min): ");
    int tokens = 0;
    char *curr_tok;
    char *next_tok_load = read_file("loadavg");
    while ((curr_tok = next_token(&next_tok_load, " ")) != NULL) 
    {
        if (tokens++ < 3)
        {
            printf ("%s", curr_tok);
        }
    }
    
    printf ("\n");

    //calculate cpu usage: /proc/stat
    //read from stat-- all the numbers in the first line are the total, the 4th column is the idle
    //1 - ( (idle2 - idle1) / (total2 - total1) )

    printf ("CPU Usage:    [");
    int total[2]={0};
    int idle[2]={0};
    for (int i = 0; i < 2; i++)
    {
        if (i == 1)
        {
            sleep (1);
        }
        static char buf_1[BUF_SZ];
        fd = open("stat", O_RDONLY);
        if (fd == -1) {
            perror("open");
        }
        while ((read_sz = read_line(buf_1, BUF_SZ, fd)) > 0) 
        {   
            if (strstr(buf_1, "cpu"))
            {
                //token buf_1 and get the total and idle times for each loop
                int tok = 0;
                char *next_tok_usage = buf_1;
                char *curr_tok_usage;
                /* Tokenize. Note that ' ,?!' will all be removed. */
                while ((curr_tok_usage = next_token(&next_tok_usage, " ,?!\n")) != NULL) 
                {
                    if (tok>0 && tok<10)
                    {
                        total[i] += atoi(curr_tok_usage);
                        if (tok == 4)
                        {
                            idle[i] = atoi (curr_tok_usage);
                        }
                        tok++;
                    }
                    else
                    {
                        tok++;
                    }
                }
            break;
            }
        }
        close(fd);
    }

    int t_diff = (total[1] - total[0]);
    int i_diff = (idle[1] - idle[0]);

    float cpu_usage;
    cpu_usage = ((float) i_diff / (float) t_diff);

    float c_usage;

    if (isnan(cpu_usage)) 
    {
        c_usage = 0;
    } else 
    {
        c_usage = ((float) 1 - cpu_usage) * 100;
    }

    int count = (int) c_usage / 5;

    for (int i = 0; i < 100; i+=5)
    {
        if (count != 0) 
        {
            printf("#");
            count --;
        } else 
        {
            printf("-");
        }
    }

    printf ("] %.1f%%\n", c_usage);

    float tot;
    float active;
    static char buf_2[BUF_SZ];
    fd = open("meminfo", O_RDONLY);
    if (fd == -1) 
    {
        perror("open");
    }

    while ((read_sz = read_line(buf_2, BUF_SZ, fd)) > 0) 
    {   
        if (strstr(buf_2, "MemTotal:"))
        {
            //tokenize buf_2 and get total mem
            int tok_m = 0;
            char *next_tok_mem = buf_2;
            char *curr_tok_mem;
            /* Tokenize. Note that ' ,?!' will all be removed. */
            while ((curr_tok_mem = next_token(&next_tok_mem, " ,?!\n")) != NULL) 
            {
                if (tok_m == 1)
                {
                    tot = atof(curr_tok_mem);
                    tok_m++;
                }
                else
                {
                    tok_m++;
                }
            }
        }
        if (strstr(buf_2, "Active:"))
        {
            //tokenize buf_2 and get active mem
            int tok_a = 0;
            char *next_tok_act = buf_2;
            char *curr_tok_act;
            /* Tokenize. Note that ' ,?!' will all be removed. */
            while ((curr_tok_act = next_token(&next_tok_act, " ,?!\n")) != NULL) 
            {
                if (tok_a == 1)
                {
                    active = atof(curr_tok_act);
                    tok_a++;
                }
                else
                {
                    tok_a++;
                }
            }
        }
    }
    //convert kb to gb
    tot = tot/1024/1024;
    active = active/1024/1024;
    float mem_usage = 100 * (active/tot);

    printf ("Memory Usage: [");
    int countm = round(mem_usage) / 5;

    for (int j = 0; j < 100; j+=5)
    {
        if (countm != 0) 
        {
            printf("#");
            countm --;
        } else 
        {
            printf("-");
        }
    }
    //printf ("%f\n", (mem_usage/100)*tot);
    printf ("] %.1f%% (%.1f GB / %.1f GB)\n", mem_usage, active, tot);
}

/**
* Function to display live view
*/
void live_info()
{
    printf ("Live CPU/Memory View\n");
    printf ("--------------------\n");
    printf ("\033[?25l");

    //infinite while loop printing load average, cpu usage, and memory usage
    int i = 0;
    int fd;
    while (true)
    {
        int idle[2]={0};
        int total[2]={0};
        printf ("Load Average (1/5/15 min): ");
        int tokens = 0;
        ssize_t read_sz;
        char *curr_tok;
        char *next_tok_load = read_file("loadavg");
        while ((curr_tok = next_token(&next_tok_load, " ")) != NULL) 
        {
            if (tokens++ < 3)
            {
                printf ("%s ", curr_tok);
            }
            else
            {
                break;
            }
        }
        
        printf ("\n");

        //cpu usage
        printf ("CPU Usage:    [");
        
        for (i = 0; i < 2; i++)
        {
            if (i == 1)
            {
                sleep (1);
            }
            static char buf_1[BUF_SZ];
            fd = open("stat", O_RDONLY);
            if (fd == -1) {
                perror("open");
            }
            while ((read_sz = read_line(buf_1, BUF_SZ, fd)) > 0) 
            {   
                if (strstr(buf_1, "cpu"))
                {
                    //printf ("%s", buf_1);
                    //token buf_1 and get the total and idle times for each loop
                    int tok = 0;
                    char *next_tok_usage = buf_1;
                    char *curr_tok_usage;
                    /* Tokenize. Note that ' ,?!' will all be removed. */
                    while ((curr_tok_usage = next_token(&next_tok_usage, " ,?!\n")) != NULL) 
                    {
                        if (tok>0 && tok<10)
                        {
                            total[i] += atoi(curr_tok_usage);
                            if (tok == 4)
                            {
                                idle[i] = atoi (curr_tok_usage);
                            }
                            tok++;
                        }
                        else
                        {
                            tok++;
                        }
                    }
                break;
                }
            }
            close(fd);
        }
        int t_diff = (total[1] - total[0]);
        int i_diff = (idle[1] - idle[0]);

        float cpu_usage;
        cpu_usage = ((float) i_diff / (float) t_diff);

        float c_usage;

        if (isnan(cpu_usage)) 
        {
            c_usage = 0;
        } else 
        {
            c_usage = ((float) 1 - cpu_usage) * 100;
        }

        int count = (int) c_usage / 5;

        for (int k = 0; k < 100; k+=5)
        {
            if (count != 0) 
            {
                printf("#");
                count --;
            } else 
            {
                printf("-");
            }
        }
        printf ("] %.1f%%\n", c_usage);

        //memory usage
        float tot;
        float active;
        static char buf_2[BUF_SZ];
        fd = open("meminfo", O_RDONLY);
        if (fd == -1) 
        {
            perror("open");
        }

        while ((read_sz = read_line(buf_2, BUF_SZ, fd)) > 0) 
        {   
            if (strstr(buf_2, "MemTotal:"))
            {
                //tokenize buf_2 and get total mem
                int tok_m = 0;
                char *next_tok_mem = buf_2;
                char *curr_tok_mem;
                /* Tokenize. Note that ' ,?!' will all be removed. */
                while ((curr_tok_mem = next_token(&next_tok_mem, " ,?!\n")) != NULL) 
                {
                    if (tok_m == 1)
                    {
                        tot = atof(curr_tok_mem);
                        tok_m++;
                    }
                    else
                    {
                        tok_m++;
                    }
                }
            }
            if (strstr(buf_2, "Active:"))
            {
                //tokenize buf_2 and get active mem
                int tok_a = 0;
                char *next_tok_act = buf_2;
                char *curr_tok_act;
                /* Tokenize. Note that ' ,?!' will all be removed. */
                while ((curr_tok_act = next_token(&next_tok_act, " ,?!\n")) != NULL) 
                {
                    if (tok_a == 1)
                    {
                        active = atof(curr_tok_act);
                        tok_a++;
                    }
                    else
                    {
                        tok_a++;
                    }
                }
            }
        }
        //convert kb to gb
        
        tot = tot/1024/1024;
        active = active/1024/1024;
        float mem_usage = 100 * (active/tot);

        printf ("Memory Usage: [");
        int countm = round(mem_usage) / 5;

        for (int j = 0; j < 100; j+=5)
        {
            if (countm != 0) 
            {
                printf("#");
                countm --;
            } else 
            {
                printf("-");
            }
        }
        printf ("] %.1f%% (%.1f GB / %.1f GB)\n\033[A\033[A\033[A\r", mem_usage, active, tot);
    }
}


/**
* Function to display task info
*/
void task_info()
{
    printf ("Task Information\n");
    printf ("----------------\n");

    //readdir

    DIR *directory;
    if ((directory = opendir(".")) == NULL) {
        perror("opendir");
    }

    int tasks_count=0;
    int fd=0;
    static char buf_t[BUF_SZ];

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) 
    {
        if (atoi(entry->d_name) !=0)
        {
            char path[1000];
            char status[10] = ("/status");
            strcpy(path, entry->d_name);
            strcat(path, status);
            //printf ("%s\n", path);
            fd = open(path, O_RDONLY);
            if (fd == -1) 
            {
                //perror("open");
                continue;
            }
            else
            {
                tasks_count++;
            }
        }
        close(fd);
    }
    closedir(directory);
    if ((directory = opendir(".")) == NULL) {
        perror("opendir");
    }

    printf ("Tasks Running: %d\n\n", tasks_count);

    printf ("  PID |        State |                 Task Name |            User | Tasks\n");
    printf ("------+--------------+---------------------------+-----------------+-------\n");


    while ((entry = readdir(directory)) != NULL)
    {
        ssize_t read_sz=0;
        char threads[10];
        char pid[20];
        char name[26];
        char state[100];
    
        //printf ("%s\n", entry->d_name);
        char path[1000];
        char status[10] = ("/status");
        strcpy(path, entry->d_name);
        strcat(path, status);
        //printf ("%s\n", path);
 

        struct passwd *pw;
        struct stat stat_buf;
        stat(path, &stat_buf);
        pw = getpwuid(stat_buf.st_uid);

        fd = open(path, O_RDONLY);
        if (fd == -1)
        {
            continue;
        }


        while ((read_sz = read_line(buf_t, BUF_SZ, fd)) > 0)
        {
            if (strstr(buf_t, "Name:"))
            {
                //tokeninze and get token 2 (task name)
                //printf ("%s\n", buf_t);
                int tok_n = 0;
                char *next_tok_n = buf_t;
                char *curr_tok_n;
                /* Tokenize. Note that ' ,?!' will all be removed. */
                while ((curr_tok_n = next_token(&next_tok_n, "\t ():,?!\n")) != NULL) 
                {
                    if (tok_n == 1)
                    {
                        strcpy(name, curr_tok_n);
                        if (strlen(curr_tok_n) > 25)
                        {
                            name[25] = '\0';
                        }
                        //printf ("%s\n", name);
                        //printf ("%s\n", curr_tok_n);
                        tok_n++;
                    }
                    else
                    {
                        tok_n++;
                    }
                }
            }
            if (strstr(buf_t, "State:"))
            {
                //tokeninze and get token 2 (task name)
                //printf ("%s\n", buf);
                int tok_s = 0;
                char *next_tok_s = buf_t;
                char *curr_tok_s;
                /* Tokenize. Note that ' ,?!' will all be removed. */
                while ((curr_tok_s = next_token(&next_tok_s, "\t():,?!\n")) != NULL) 
                {
                    if (tok_s == 2)
                    {
                        strcpy (state, curr_tok_s);
                        //printf ("%s\n", state);
                        //printf ("%s\n", curr_tok_s);
                        tok_s++;
                    }
                    else
                    {
                        tok_s++;
                    }
                }
            }
            if (strstr(buf_t, "Pid:") && !strstr(buf_t, "PPid:") && !strstr(buf_t, "TracerPid"))
            {
                //tokeninze and get token 2 (task name)
                int tok_id = 0;
                char *next_tok_id = buf_t;
                char *curr_tok_id;
        
                /* Tokenize. Note that ' ,?!' will all be removed. */
                while ((curr_tok_id = next_token(&next_tok_id, "\t :,?!\n")) != NULL) 
                {
                    if (tok_id == 1)
                    {
                        strcpy(pid, curr_tok_id);
                        tok_id++;
                    }
                    else
                    {
                        tok_id++;
                    }
                }
            }
            if (strstr(buf_t, "Threads:"))
            {
                //tokeninze and get token 2 (task name)
                int tok_thread = 0;
                char *next_tok_thread = buf_t;
                char *curr_tok_thread;
                /* Tokenize. Note that ' ,?!' will all be removed. */
                while ((curr_tok_thread = next_token(&next_tok_thread, "\t :,?!\n")) != NULL) 
                {
                    if (tok_thread == 1)
                    {
                        strcpy (threads, curr_tok_thread);
                        tok_thread++;
                    }
                    else
                    {
                        tok_thread++;
                    }
                }
            }
        }
        close(fd);

        printf("%5s | %12s | %25s | %15s | %5s \n", pid, state, name, pw->pw_name, threads);

        //open d_name (status) and get all the info you need and store it in the struct
    }

    closedir(directory);

    //status file -- process id, state, name, uid (user), threads (processes running)
}

/**
 * Prints help/program usage information.
 *
 * This output is displayed if there are issues with command line option parsing
 * or the user passbuffes in the -h flag.
 */
void print_usage(char *argv[])
{
    printf("Usage: %s [-ahrst] [-l] [-p procfs_dir]\n" , argv[0]);
    printf("\n");
    printf("Options:\n"
"    * -a              Display all (equivalent to -rst, default)\n"
"    * -h              Help/usage information\n"
"    * -l              Live view. Cannot be used with other view options.\n"
"    * -p procfs_dir   Change the expected procfs mount point (default: /proc)\n"
"    * -r              Hardware Information\n"
"    * -s              System Information\n"
"    * -t              Task Information\n");
    printf("\n");
}

/**
 * Main program entrypoint. Reads command line options and runs the appropriate
 * subroutines to display system information.
 */
int main(int argc, char *argv[])
{
    /* Default location of the proc file system */
    char *procfs_loc = "/proc";

    /* Set to true if we are using a non-default proc location */
    bool alt_proc = false;

    struct view_opts defaults = { true, false, true, true };
    struct view_opts options = { false, false, false, false };

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "ahlp:rst")) != -1) {
        switch (c) {
            case 'a':
                options = defaults;
                break;
            case 'h':
                print_usage(argv);
                return 0;
            case 'l':
                options.live_view = true;
                break;
            case 'p':
                procfs_loc = optarg;
                alt_proc = true;
                break;
            case 'r':
                options.hardware = true;
                break;
            case 's':
                options.system = true;
                break;
            case 't':
                options.task_list = true;
                break;
            case '?':
                if (optopt == 'p') {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n", optopt);
                }
                print_usage(argv);
                return 1;
            default:
                abort();
        }
    }

    if (alt_proc == true) {
        LOG("Using alternative proc directory: %s\n", procfs_loc);

        /* Remove two arguments from the count: one for -p, one for the
         * directory passed in: */
        argc = argc - 2;
    }

    if (argc <= 1) {
        /* No args (or -p only). Enable default options: */
        options = defaults;
    }

    if (options.live_view == true) {
        /* If live view is enabled, we will disable any other view options that
         * were passed in. */
        options = defaults;
        options.live_view = true;
        LOGP("Live view enabled. Ignoring other view options.\n");
    } else {
        LOG("View options selected: %s%s%s\n",
                options.hardware ? "hardware " : "",
                options.system ? "system " : "",
                options.task_list ? "task_list" : "");
    }

    if (chdir(procfs_loc) == -1) {
        return -1;
    }

    if(options.live_view)
    {
        live_info();
    }

    if (options.system) 
    {
        sys_info();
    }

    if (options.hardware)
    {
        hardware_info();
    }

    if (options.task_list)
    {
        task_info();
    }

    return 0;
}
