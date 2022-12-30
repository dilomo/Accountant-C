#ifndef ACC_STRINGSTACK_H_INCLUDED
#define ACC_STRINGSTACK_H_INCLUDED

#include "Acc.Main.h"


typedef char StackItem[MAX_LINE];

#define MAXStack 1000

typedef struct snode
{
    StackItem item;
    struct snode * prev;
} StackNode;

typedef struct Stack
{
    StackNode * bottom;  /* pointer to front of Stack  */
    StackNode * top;   /* pointer to rear of Stack   */
    int items;     /* number of items in Stack   */
} Stack;

typedef Stack StringStack;

/* operation:        initialize the Stack                       */
/* precondition:     pq points to a Stack                       */
/* postcondition:    Stack is initialized to being empty        */
void InitializeStack(Stack * pq);
/* operation:        check if Stack is full                     */
/* precondition:     pq points to previously initialized Stack  */
/* postcondition:   returns True if Stack is full, else False  */
bool StackIsFull(const Stack * pq);
/* operation:        check if queue is empty                    */
/* precondition:     pq points to previously initialized Stack  */
/* postcondition:    returns True if Stack is empty, else False */
bool StackIsEmpty(const Stack *pq);
/* operation:        determine number of items in Stack         */
/* precondition:     pq points to previously initialized Stack  */
/* postcondition:    returns number of items in Stack           */
int StackItemCount(const Stack * pq);
/* operation:        add item to rear of Stack                  */
/* precondition:     pq points to previously initialized Stack */
/*                   item is to be placed at rear of Stack      */
/* postcondition:    if Stack is not empty, item is placed at   */
/*                   rear of Stack and function returns         */
/*                   True; otherwise, Stack is unchanged and    */
/*                   function returns False                     */
bool Push(StackItem item, Stack * pq);
/* operation:        remove item from front of Stack            */
/* precondition:     pq points to previously initialized Stack  */
/* postcondition:    if Stack is not empty, item at head of     */
/*                   Stack is copied to *pitem and deleted from */
/*                   Stack, and function returns True; if the   */
/*                   operation empties the Stack, the Stack is  */
/*                   reset to empty. If the Stack is empty to   */
/*                   begin with, Stack is unchanged and the     */
/*                   function returns False                     */
bool Pop(StackItem *pitem, Stack * pq);
/* operation:        empty the Stack                            */
/* precondition:     pq points to previously initialized Stack  */
/* postconditions:   the Stack is empty                         */
void EmptyTheStack(Stack * pq);

#endif // ACC_STRINGSTACK_H_INCLUDED
