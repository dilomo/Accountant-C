#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "libstd.h"


/*A library of usefull functions that I use in Accountant*/


static int daytab[2][13] = {
                               {
                                   0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
                               },
                               {
                                   0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
                               }
                           };

/*gets string from stdin with \n at the end*/
int getline(char s[],int lim) {
    int c, i;
    c = '\0';
    for ( i=0; i< lim -1 && (c=getchar()) != EOF && c!='\n'; i++)
        s[i] = c;
    if (c =='\n')
        s[i++] = c;

    s[i] = '\0'; /* remove /n */

    return i;
}

int getsline(char s[],int lim) {
    int c, i;
    c = '\0';
    for ( i=0; i< lim -1 && (c=getchar())!=EOF && c!='\n' && isalpha(c); i++)
        s[i] = c;
    if (c =='\n' || isalpha(c))
        s[i++] = c;

    s[i] = '\0';

    return i;
}

int getsword(char word[], const char src[],size_t startfrom, size_t lim) {
    clearstr(word);
    int c, i, j;
    j = startfrom;
    c = '\0'; //compiler wanted :(

    for (i=0 ; i< lim -1 && (c=src[j])!='\0' && c!='\n' && (isalnum(c) || ispunct(c)); i++, j++)
        word[i] = c;
    if (c =='\n' || isalpha(c))
        word[i++] = c;

    word[i] = '\0';

    return i;

}

/*Special atof() accomodated to Bulgarian currency rules*/
double atof(const char s[]) {
    double val, power;
    int i, sign;
    int zerodigit = 0;

    for (i=0; isspace(s[i]); i++)//skips empty spaces
        ;

    sign = (s[i]=='-')? -1 : 1; //gets sign if any
    if (s[i] =='+' || s[i] =='-')
        i++;

    for (val = 0.0; isdigit(s[i]); i++)//gets the whole part
    {
        zerodigit = 1;
        val = 10.0 * val + (s[i] - '0');
    }

    if (s[i] == '.' || s[i] == ',')
        i++;

    for (power = 1.0; isdigit(s[i]); i++)//дробна част
    {
        val = 10.0 * val +(s[i] - '0');
        power *= 10;
    }
    if (zerodigit)
        return sign * val / power;
    else
        return -1;
}

/*Special atof() accomodated to Bulgarian currency rules
 *used to set precision of the taken value
 */
double atofp(const char s[], int p) {
    double val, power;
    int i, sign, plim;

    for (i=0; isspace(s[i]); i++)//skips empty spaces
        ;

    sign = (s[i]=='-')? -1 : 1; //gets sign if any
    if (s[i] =='+' || s[i] =='-')
        i++;

    for (val = 0.0; isdigit(s[i]); i++)//gets the whole part
    {
        val = 10.0 * val + (s[i] - '0');
    }

    if (s[i] == '.' || s[i] == ',')
        i++;

    for (power = 1.0, plim = 0; isdigit(s[i]); i++, plim++)//дробна част
    {
        if (plim >= p)
            break;

        val = 10.0 * val +(s[i] - '0');
        power *= 10.0;
    }
    if ((sign * val / power) >= 0)
        return sign * val / power;
    else
        return -1;
}

unsigned long atoul(const char s[]) {
    unsigned long long val;
    int i;
    int zerodigit = 0;

    for (i=0; isspace(s[i]); i++)//skips empty spaces
        ;

    for (val = 0 ; isdigit(s[i]); i++) {
        zerodigit = 1;
        val = 10 * val + (s[i] - '0');
    }
    return val;
}

/*prints box-like structure to the stdout*/
void textbox(const char text[], int width) {
    println('*',width);
    printmid(text,width);
    println('*',width);
}

/*Cpmpares d1 to d2 date structures*/
int datecmp(date d1, date d2) {
    if (d1.year > d2.year)
        return 1;
    else if (d1.year < d2.year)
        return -1;

    if (d1.month > d2.month)
        return 1;
    else if (d1.month < d2.month)
        return -1;

    if (d1.day > d2.day)
        return 1;
    else if (d1.day < d2.day)
        return -1;

    return 0;
}

int dateequal(date d1, date d2) {
    if (d1.year != d2.year)
        return 0;
    if (d1.month != d2.month)
        return 0;
    if (d1.day != d2.day)
        return 0;

    return 1;
}

int leapyear(int year) {
    return ((year%4 == 0) && (year%100 != 0 || year%400 == 0));
}

//format: year/month/day or day.month.year
//separators: / \ . : -
int parsedate(const char *s, date *dt) {
    int i,val;

    for (i=0; isspace(s[i]); i++)//skips empty spaces
        ;

    for (val = 0; isdigit(s[i]); i++)//gets the first part (year or day)
        val = 10 * val + (s[i] - '0');

    if (val > 31) //US format- year first
    {
        if (val >= MIN_YEAR && val <= MAX_YEAR)
            dt->year = val;
        else
            return 0;

        if (s[i] == '/' || s[i] == '\\' || s[i] == '.'|| s[i] == '-' || s[i] == ':')//separators
            i++;
        else
            return 0;

        for (val = 0; isdigit(s[i]); i++)//gets the month
            val = 10 * val + (s[i] - '0');

        if (val > 0 && val < 13)//checks month
            dt->month=val;
        else
            return 0;

        if (s[i] == '/' || s[i] == '\\' || s[i] == '.'|| s[i] == '-' || s[i] == ':')
            i++;
        else
            return 0;

        for (val = 0; isdigit(s[i]); i++)//gets the day
            val = 10 * val + (s[i] - '0');

        //cheks number of days for (leap and nonleap year)
        if (val > 0 && val <= daytab[leapyear(dt->year)][dt->month])
            dt->day = val;
        else
            return 0;

    } else//Bulgarian and EU format
    {
        //cheks day at bottom
        int day = val;

        if (s[i] == '/' || s[i] == '\\' || s[i] == '.'|| s[i] == '-' || s[i] == ':')//separator US
            i++;
        else
            return 0;

        for (val = 0; isdigit(s[i]); i++)//gets the month
            val = 10 * val + (s[i] - '0');

        if (val > 0 && val < 13)//checks month
            dt->month=val;
        else
            return 0;

        if (s[i] == '/' || s[i] == '\\' || s[i] == '.'|| s[i] == '-' || s[i] == ':')//separator US
            i++;
        else
            return 0;

        for (val = 0; isdigit(s[i]); i++)//gets the year
            val = 10 * val + (s[i] - '0');

        if (val >= MIN_YEAR && val <= MAX_YEAR)
            dt->year = val;
        else
            return 0;

        //cheks number of days for (leap and nonleap year)
        if (day > 0 && day <= daytab[leapyear(dt->year)][dt->month])
            dt->day = day;
        else
            return 0;
    }

    return 1;
}

/*converts an integer into string*/
char * itoa2(int n, char s[]) {

    int i, sign;
    if ((sign=n)<0)
        n = -n;
    i=0;

    do {
        s[i++] = n % 10 + '0';
    } while ((n/=10)>0);

    if (sign <0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);

    return s;
}
/*converts unsigned integer into string*/
char * uitoa(unsigned int n, char s[]) {
    unsigned int i = 0;

    do {
        s[i++] = n % 10 + '0';
    } while ((n/=10) > 0);

    s[i] = '\0';
    reverse(s);
    return s;
}

char * ultoa(unsigned long int n, char s[]) {
    unsigned long int i = 0;

    do {
        s[i++] = n % 10 + '0';
    } while ((n/=10) > 0);

    s[i] = '\0';
    reverse(s);
    return s;
}

void reverse(char s[])// string only
{
    int c, i, j;

    for (i=0, j = strlen(s)-1;i<j;i++,j--) {
        c=s[i];
        s[i]=s[j];
        s[j]=c;
    }
}

/*makes copy of s and returns the pointer to it*/
char *strdupl(const char *s) {
    if (s == NULL)
        return NULL;

    char *p;

    p=(char*) malloc(sizeof(char) *(strlen(s)+1));
    if (p != NULL) {
        strcpy(p,s);
    }

    return p;
}

char *strdup_fixed(const char *s, size_t len) {
    if (s == NULL)
        return NULL;

    char *p;

    /* I add one byte buffer because when using malloc
     * for other data it intrfirece with the string
     * and the result is not very pleasant
     */
    p=(char*) malloc(sizeof(char) * (len+1));
    if (p != NULL) {
        clearstr(p);
        strncpy(p , s, len);
    }

    return p;
}

char *strdupl_len(const char *s, size_t *len) {
    if (s == NULL)
        return NULL;

    char *p;
    *len= strlen(s);

    p=(char*) malloc(sizeof(char) *((*len)+1));
    if (p != NULL)
        strcpy(p,s);

    return p;
}

int hasnl(const char *s) {
    char c;
    int i = 0;
    while ((c = *s++) != '\0') {
        if (c == '\n')
            return i;
        i++;
    }

    return 0;
}

void printmid(const char *s, int width) {
    int i;
    int mid = (width/2)-(strlen(s)/2);

    for (i = 0; i <=mid; i++)
        printf(" ");

    printf(s);
    printf("\n");
}

void fprintmid(FILE *f, const char *s, int width) {
    int i;
    int mid = (width/2)-(strlen(s)/2);

    for (i = 0; i <=mid; i++)
        fprintf(f, " ");

    fprintf(f, s);
    fprintf(f, "\n");
}

void println(const char c,int count) {
    int i;
    for (i =0; i< count; i++)
        printf("%c", c);
    printf("\n");
}

void fprintln(FILE *f, const char c,int count) {
    int i;
    for (i =0; i<count; i++)
        fprintf(f,"%c", c);
    fprintf(f, "\n");
}

void printright(const char *s, int width) {
    int i;
    int empty = width - strlen(s);

    for (i = 0; i < empty; i++)
        printf(" ");

    printf(s);
    printf("\n");
}

void clearstr(char s[]) {
    int i = 0;
    while (s[i] != '\0')
        s[i++] = '\0';
}

void clearstrn(char s[], size_t num) {
    int i = 0;
    while (i <  num)
        s[i++] = '\0';
}

void fillstr(char s[], char c)
{
    int i = 0;
    while (s[i] != '\0')
        s[i++] = c;
}

void fillstrn(char s[], char c, size_t chars)
{
    int i = 0;
    while (s[i] != '\0' && i < chars)
        s[i++] = c;
}

int copyfile(char *s1, char *s2) {
    FILE *in = fopen(s1,"r");
    FILE *out = fopen(s2, "w");
    char ch;

    if ((in == NULL) || (out == NULL))
        return 1;

    while ((ch = getc(in)) != EOF)
        putc(ch, out);

    // clean up
    if (fclose(in) != 0 || fclose(out) != 0) {
        fprintf(stderr,"Error in closing files\n");
        return 1;
    }

    return 0;
}

int onlydigits(char *s) {
    int c, i;
    i = 0;
    while ((c = s[i++]) != '\0') {
        if (c != '.' && c != ',' && !isdigit(c) && !isspace(c))
            return 0;
    }

    return 1;
}

int start_with_digits(char *s) {
    int c, i;
    i = 0;
    while ((c = s[i++]) != '\0') {
        if (c != '.' && c != ',' && !isdigit(c) && !isspace(c))
            return 0;
        if (isdigit(c))
            break;
    }

    return 1;
}

char *trim(char c, char *s) {
    int i, start = 0, len = strlen(s), end = len-1;
    char c1;

    /*remove c from the begining of array*/
    while ((c1 = s[start]) != '\0' && c1 == c)
        start+=1;
    /*remove from the end and*/
    while ((c1 = s[end]) != '\0' && c1 == c && end != start)
        end-=1;//printf("remove %c from the end\n of %s",c1,s);//s[end] = '\0';

//    --start;
//    ++end;

    /*if nothing is changed return original*/
    if (start <= 0 && end >= len-1){
//        printf(" no change %s in trim;", s);
        return s;

    }

    /*form result */
    for (i = 0; i < end; i++) {
        s[i] = s[start++];
    }
//    printf(" %s in trim;", s);
    s[i] = '\0';
    return s;
}

char *trim_start(char c, char *s) {
    int i, start = 0, len = strlen(s);
    char c1;

    /*remove c from the begining of array*/
    while ((c1 = s[start]) != '\0' && c1 == c)
        start+=1;

    /*if nothing is changed return original*/
    if (start <= 0){
        return s;
    }

    /*form result */
    for (i = 0; i < len; i++) {
        s[i] = s[start++];
    }
    s[i] = '\0';
    return s;
}

date* date_now(date *Date)
{
    time_t *ctm = (time_t *) malloc(sizeof(time_t));
    time(ctm);
    struct tm *now = localtime(ctm);

    Date->year = now->tm_year+1900;
    Date->month = (short) now->tm_mon+1;
    Date->day =(short) now->tm_mday;

    free(ctm);
    return Date;
}
