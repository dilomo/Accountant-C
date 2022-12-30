#ifndef ACC_SORT_H_INCLUDED
#define ACC_SORT_H_INCLUDED

#include "Acc.Main.h"

union sort_union
{
    unsigned long ID;
    double value;
    date date;
//    char type[MAX_LINE];
    char* type;
};

enum sort_type
{
    SORT_BY_NONE = 210, SORT_BY_ID , SORT_BY_DATE, SORT_BY_VALUE, SORT_BY_TYPE /*, DESCR*/
};

enum sort_direction
{
    NODIR, ASC = 4588, DESC = 8854 /*ascending and descending*/
};

struct sort_element
{
    enum sort_type    type;
    union sort_union  data;
    REC *            rec_ptr;
};

/*types definition*/
typedef enum sort_direction  SortDirection;
typedef enum sort_type       SortType;
typedef struct sort_element  SortElement;
typedef struct sort_element* SortArray;

/*function interface*/
bool CreateSortArray(SortArray* s, unsigned long aNumOfItems);
void DisposeSortArray(const SortArray s);
bool ResizeSortArray(SortArray* s,unsigned long oldNumOfItems, unsigned long newNumOfItems);

int Traverse_SortArray(const SortArray arr, unsigned long num, void (*func)(SortElement sl));
int Traverse_SortArray_Default(const SortArray arr, unsigned long num);
int Traversen_SortArray(const SortArray arr, unsigned long num, void (*func)(SortElement sl), unsigned long start,  unsigned long count);
int Traversen_SortArray_Default(const SortArray arr, unsigned long num, unsigned long start,  unsigned long count);
/*comp shoud be aware of SortDirection enum*/
int QuckSort(SortArray arr, unsigned long num,  int (*comp)(const SortElement* a, const SortElement* b));
int QuckSort_Default(SortArray arr, unsigned long num);

int PrintSortElement(SortElement sl);

/*properties*/
void            set_sortDirect(SortDirection sd);
SortDirection   get_sortDirect(void);

void            set_printFunc(void *);
void *          get_printFunc(void);
#endif // ACC_SORT_H_INCLUDED
