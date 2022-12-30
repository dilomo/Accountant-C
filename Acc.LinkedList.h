#ifndef GEN_H_INCLUDED
#define GEN_H_INCLUDED

#include <stdio.h>
#include "libstd.h"

#define MAX_LST_LEN 5000000

enum rec_ty {income=0, outcome, both};

/*defins boolean data type*/
enum boolean {
    false=0,
    true=1
};

    typedef enum boolean bool;//!111111111
    typedef enum rec_ty rectype;//

struct rec_t{
    unsigned long ID;//should be automatically set by the app
    double value;
    char * type;
    char * descr;
    date date;
};
    typedef struct rec_t recdata;

struct rec_m{
   recdata data;
   struct rec_m *next;
} ;

    typedef struct rec_m REC;
    typedef unsigned long Index;

/*call before first use of the list*/
void InitialzieList(void);

/*adds record r of type t to the list*/
bool addtolist(recdata r, rectype t);
/*does not change the ID if r if it doesn't conflicts with other records*/
bool addtolist_withID(recdata r, rectype t);
/*disconnects r from the list and frees the mem used*/
bool rmfromlist(REC *r, rectype t);
/*clean up the memory*/
void clearlist(rectype r);

/*calculates the count of the elements of type r*/
Index count(rectype r);
/*calculates the count of the elements of type r which pass the conditions in *func*/
Index count_filtered(rectype t, bool (*func)(recdata r));

/*gives new IDs to all the records of type t*/
bool refreshIndex(rectype t);

/*Finds the last index that is in use */
Index lastIndex(rectype t);

/*Returns pointer to the last/first added record*/
REC * getlast(rectype t);
REC * getfirst(rectype t);

/*Finds the biggest value stored in the list of type t*/
double biggestValue(rectype t);
double biggestValue_filtered(rectype t, bool (*func)(recdata r));

bool listisempty(rectype r);
bool listisfull(rectype r);

REC * findrec(recdata r,rectype t);
REC * findreci(recdata r,rectype t); //ignore case
REC * findrec_ID(Index ID,rectype t);
REC * findrec_Value(double v,rectype t);
REC * findrec_Date(date d,rectype t);
/*returns: the pinter to the rec that is num_records ahead*/
REC * jump(REC **start, int num_records, rectype t);

int traverse(void(*func)(recdata rp),rectype t);
int traversen(void(*func)(recdata rp),rectype t, int max, bool firstpage);
/*the function returns bool because it is needed to display correct info in ShowList*/
int traverse_filetered(bool(*func)(recdata r),rectype t);
/*used for theSortArray*/
int traverse_filetered_raw(bool(*func)(REC * r),rectype t);
int traversen_filtered(bool(*func)(recdata rp),rectype t, int max, bool firstpage);

/*comparison*/
int reccmp(recdata r1, recdata r2);
int reccmpi(recdata r1, recdata r2);
bool recequal(recdata r1, recdata r2);

/*print error message*/
void printe(char * fmt, ...);

#endif // GEN_H_INCLUDED
