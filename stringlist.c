#include "stringlist.h"
#include "libstd.h"
#include <string.h>

static void resize(int newsize);
static long int mSize;
static long int mCount;
static long int mCurIndex;

static char* stringlist;

long int Count()
{
    return mCount;
}

long int ContainsExact(char * str)
{

}

long int Contains(char * str)
{

}


bool RemoveString(long int index)
{

}


bool AddString(char * str)
{
    if (mCurIndex >= mSize) {
        resize( mCurIndex + RESIZE_TRESHOLD);
    } else
    stringlist[mCurIndex*MAX_LINE] = strdup_fixed(str, MAX_LINE);

    mCount++;
    mCurIndex++;
}

void InitStringList()
{
    mSize = DEFAULT_COUNT;
    mCount = 0;
    mCurIndex = 0;
    /*allocate memory for the whole array of strings*/
    stringlist = (char*) malloc(MAX_LINE * DEFAULT_COUNT);
}


void ClearStrings()
{
    free(stringlist);
}

void resize(int newsize)
{

}

