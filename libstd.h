#ifndef LIBSTD_H_INCLUDED
#define LIBSTD_H_INCLUDED

#define MAX_LINE 2000
#define MAX_YEAR 9999
#define MIN_YEAR 1900

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*atof: converts the string s to double*/
double atof(const char s[]);
double atofp(const char s[], int precision);
/*atoul: converts the string s to unsigned int*/
unsigned long atoul(const char s[]);

/*reads line into s (maximum lim chars)
 and returns its lenght*/
int getline(char s[],int lim);

/*reads only letters, no digits*/
int getsline(char s[],int lim);

int getsword(char word[], const char src[],size_t startfrom, size_t lim);

/*Shows text centerd in boxlike structure*/
void textbox(const char text[], int width);

struct dt {
    int year;
    short month;
    short day;
};

typedef struct dt date;//

/*compares to date structures*/
int datecmp(date d1, date d2);

/*Returns 0 if the date structures are equal else 1*/
int dateequal(date d1, date d2);

/*Chacks if year is leap year*/
/*Returns 0 if true          */
int leapyear(int year);

/*parses s to date structure*/
int parsedate(const char *s, date *dt);

/*converts n to chars in s*/
char *itoa2(int n, char s[]);

/*converts unsigned n to chars in s*/
char *uitoa(unsigned int n, char s[]);

char *ultoa(unsigned long int n, char s[]);

/*reverces the string s*/
void reverse(char *s);

/*duplicates s returning pointer to alloceted new mem*/
char *strdupl(const char *s);

/* duplicates no more then len chars.
 * It also allocate min len chars.
 */
char *strdup_fixed(const char *s, size_t len);

/* duplicates s returning pointer to alloceted new mem
 * as well as the lenght of the string s is stored in len
 */
char *strdupl_len(const char *s, size_t *len);

/*chacks for newline char
 * if no new line zero is returned else the pos of the \n
 */
int hasnl(const char *s);

/*prints s in the middle of width*/
void printmid(const char *s, int width);

/*version with unspecified output*/
void fprintmid(FILE *f, const char *s, int width);

/*prints sequence of chars*/
void println(const char c,int count);

/*version with unspecified output*/
void fprintln(FILE *f, const char c,int count);

/* prints s from the right to the left in the
 * specified bounds(width)
 */
void printright(const char *s, int width);

/*clears all chars in s to \0*/
void clearstr(char s[]);

/*clears all chars in s to num*/
/* slower from clearstr but more secire
   you may first you clearstrn to ensure
   all the data is nulled and then use only clearstr*/
void clearstrn(char s[],size_t num);

/*clears all chars in s to custom char*/
void fillstr(char s[], char c);

/*clears all chars in s to custom char*/
void fillstrn(char s[], char c, size_t chars);

/*copy file s1 to file s2*/
int copyfile(char *s1, char *s2);

/*check for other than digits signs*/
int onlydigits(char *s);

int start_with_digits(char *s);

/*removs all the c chars from the beginning and
 the end of s*/
char *trim(char c,char *s);

/*removs all the c chars from the beginning*/
char *trim_start(char c, char *s);

/*get current date and time*/
date* date_now(date *Date);

#endif // LIBSTD_H_INCLUDED
