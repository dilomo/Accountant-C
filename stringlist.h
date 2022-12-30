#ifndef STRINGLIST_H_INCLUDED
#define STRINGLIST_H_INCLUDED

#define DEFAULT_COUNT 100
#define RESIZE_TRESHOLD 70

/*sets the default behaviour*/
void InitStringList();

/* Returns:     true if operation is OK else false
 * Parameters:  a string to add*/
bool AddString(char * str);

/* Returns:     true if operation is OK else false
 * Parameters:  the index to remove from the list*/
bool RemoveString(long int index);

/* Returns:     the index of the item matched else -1
 * Parameters:  a string to search for*/
long int Contains(char * str);

/* Returns:     the index of the item matched else -1
 * Parameters:  a string to search for*/
long int ContainsExact(char * str);

/*Returns:  the count of string in the list*/
long int Count();

/*frees allocated memory*/
void ClearStrings();


//long int TraverseStrings((*func)());

#endif // ARRAYLIST_H_INCLUDED
