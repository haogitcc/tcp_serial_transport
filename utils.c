#include "utils.h"


/*
 * safe_asprintf();
 */
int safe_asprintf(char **strp, const char *fmt, ...) 
{
    va_list ap;
    int retval;

    va_start(ap, fmt);
    retval = safe_vasprintf(strp, fmt, ap);
    va_end(ap);

    return retval;
}

/*
 * safe_vasprintf();
 */
int safe_vasprintf(char **strp, const char *fmt, va_list ap) 
{
    int retval;

    retval = vasprintf(strp, fmt, ap);
    if (retval == -1) 
    {
        printf("Failed to vasprintf: %s.  Bailing out\n", strerror(errno));
        return 1;
    }
    return retval;
}

/*
 * plog();
 */
void plog(const char *format, ...) 
{
    va_list vlist;
    char *fmt = NULL;

    va_start(vlist, format);
    safe_vasprintf(&fmt, format, vlist);
    va_end(vlist);


    time_t timep;
    struct tm *ptm = NULL;
    time(&timep);
    ptm = localtime(&timep);
//    printf("%s[plog]%s", asctime(ptm), fmt);

   	printf("[%04d-%02d-%02d %s] %s\n", 
				ptm->tm_year + 1900, 
				ptm->tm_mon + 1,
				ptm->tm_mday, 
				__TIME__,
				fmt);

    free(fmt);
}

void pbuffer(int isSendToSerial, char *buffer, int count)
{
	if(isSendToSerial == 0)
		printf("Send: \n");
	else
		printf("Recv: \n");
	if(count > 10)
		count = 10;
	int i=0;
	while(i < count)
	{
		printf("%p, ", buffer[i]);
		i++;
	}
	printf("\n");

}

/*
 * pinfo();
 */
void pinfo(const char *format, ...) 
{
    va_list vlist;
    char *fmt = NULL;


    va_start(vlist, format);
    safe_vasprintf(&fmt, format, vlist);
    va_end(vlist);
    if (!fmt) {
        return;
    }

    printf("%s", fmt);

    free(fmt);
}

/*
 * pflog();
 */
void pflog(const char *format, ...) 
{

    pthread_mutex_lock(&fileMutex);

    FILE *fp = NULL;
    va_list vlist;
    char *fmt = NULL;

    // Open debug info output file.
    if (!(fp = fopen("log.txt", "a+"))) {
        pthread_mutex_unlock(&fileMutex);
        return;
    }

    va_start(vlist, format);
    safe_vasprintf(&fmt, format, vlist);
    va_end(vlist);
    if (!fmt) {
        pthread_mutex_unlock(&fileMutex);
        return;
    }

    time_t timep;
    struct tm *ptm = NULL;
    time(&timep);
    ptm = localtime(&timep);
    fprintf(fp, "[%04d-%02d-%02d-%02d-%02d-%02d] %s", 
            ptm->tm_year + 1900, 
            ptm->tm_mon + 1,
            ptm->tm_mday, 
            ptm->tm_hour, 
            ptm->tm_min, 
            ptm->tm_sec, 
            fmt);

    free(fmt);
    fsync(fileno(fp));
    fclose(fp);

    pthread_mutex_unlock(&fileMutex);
}

/*
 * pfinfo();
 */
void pfinfo(const char *format, ...) 
{
    pthread_mutex_lock(&fileMutex);

    FILE *fp = NULL;
    va_list vlist;
    char *fmt = NULL;

    // Open debug info output file.
    if (!(fp = fopen("log.txt", "a+"))) {
        pthread_mutex_unlock(&fileMutex);
        return;
    }

    va_start(vlist, format);
    safe_vasprintf(&fmt, format, vlist);
    va_end(vlist);
    if (!fmt) {
        pthread_mutex_unlock(&fileMutex);
        return;
    }

    fprintf(fp, "%s", fmt);

    free(fmt);
    fsync(fileno(fp));
    fclose(fp);

    pthread_mutex_unlock(&fileMutex);
}


