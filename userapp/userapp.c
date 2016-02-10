#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <popt.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

#define BENCHMOD_NAME "/proc/fget_lock_module"

struct popt_args {
    int clear;
    int loop;
    int nthreads;
    char *outfile;
};

struct popt_args popt_args;

struct poptOption options[] = {
    {
        NULL, 'n',
        POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL,
        &popt_args.loop, 0,
        "Number of times to run the system call", "Desc"
    },
    {
        "output", 'o',
        POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL,
        &popt_args.outfile, 0,
        "File to append to", "Desc file"
    },
    {
        "threads", 'p',
        POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL,
        &popt_args.nthreads, 0, "Nthreads"
    },
    {
        "clear", 'c',
        POPT_ARG_NONE | POPT_ARGFLAG_OPTIONAL,
        &popt_args.clear, 0,
        "Clear file", "clear"
    },
    POPT_AUTOHELP
};

pthread_barrier_t barrier;

struct ioctl_args {
    int fd;
    int ntimes;
};

struct timespec do_ts_diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000 + end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

static void parse_args(int argc, char **argv, poptContext *pc)
{
    int val;

    *pc = poptGetContext(NULL, argc, (const char **)argv, options, 0);

    if (argc < 2) {
        poptPrintUsage(*pc, stderr, 0);
        return;
    }

    while ((val = poptGetNextOpt(*pc)) >= 0) {
        printf("poptGetNextOpt returned val %d\n", val);
    }
}

static void dump(char *outfile, struct timespec timespan)
{
    FILE *file;
    unsigned long totaltime;

    totaltime = timespan.tv_sec * 1000000000 + timespan.tv_nsec;

    file = fopen(outfile, popt_args.clear ? "w+" : "a+");

    if(!file) {
        fprintf(stderr, "Error opening file %s\n", outfile);
        return;
    }

    if(popt_args.clear) {
        fprintf(file, "total time,number of events,number of threads\n");
    }
    fprintf(file, "%ld,%d,%d\n", totaltime, popt_args.loop, popt_args.nthreads);
    fprintf(stdout, "%ld\n", timespan.tv_nsec);

    fclose(file);
}

void *do_work(void *args)
{
    struct ioctl_args *arg = args;
    int i;

    pthread_barrier_wait(&barrier);

    for(i = 0; i < arg->ntimes; i++) {
        ioctl(arg->fd, 0, arg->ntimes);
    }
}

int main(int argc, char **argv)
{
    popt_args.outfile = "default";
    popt_args.clear = 0;
    popt_args.nthreads = 1;
    popt_args.loop = 1;

    int i, fd, ret = 0;
    struct timespec ts_beg, ts_end, ts_elapsed;
    struct ioctl_args ioctl_arg;
    poptContext pc;
    pthread_t *threads;

    parse_args(argc, argv, &pc);
    printf("nthreads = %d, loop = %d\n", popt_args.nthreads, popt_args.loop);

    /*
     * Open procfs entry for our module
     */
    fd = open(BENCHMOD_NAME, O_RDONLY);
    if(fd == -1) {
        printf("Error opening %s\n", BENCHMOD_NAME);
        return 1;
    }

    ioctl_arg.fd = fd;
    ioctl_arg.ntimes = popt_args.loop / popt_args.nthreads;

    /*
     * Initiliaze the barrier and create the threads
     */
    pthread_barrier_init(&barrier, NULL, popt_args.nthreads);
    threads = (pthread_t*)malloc(popt_args.nthreads * sizeof(pthread_t));
    for(i = 0; i < popt_args.nthreads; i++) {
        pthread_create(&threads[i], NULL, do_work, (void*)&ioctl_arg);
    }

    /*
     * Wait until all threads have finished and measure elapsed time
     * Could use some refinement to make sure we don't start the measurement
     * too late.
     */
    clock_gettime(CLOCK_MONOTONIC, &ts_beg);
    for(i = 0; i < popt_args.nthreads; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    /*
     * Cleaning
     */
    pthread_barrier_destroy(&barrier);
    close(fd);
    free(threads);

    /*
     * Output results
     */
    ts_elapsed = do_ts_diff(ts_beg, ts_end);
    dump(popt_args.outfile, ts_elapsed);

    return ret;
}
