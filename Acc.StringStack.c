#include "Acc.StringStack.h"

/* local functions */
static void CopyToStackNode(StackItem item, StackNode * pn);
static void CopyToStackItem(StackNode * pn, StackItem * pi);

void InitializeStack(Stack * pq)
{
    pq->top = pq->bottom = NULL;
    pq->items = 0;
}
bool StackIsFull(const Stack * pq)
{
    return pq->items == MAXStack;
}
bool StackIsEmpty(const Stack * pq)
{
    return pq->items == 0;
}
int StackStackItemCount(const Stack * pq)
{
    return pq->items;
}

bool Push(StackItem item, Stack * pq)
{
    StackNode * pnew;
    if (StackIsFull(pq))
        return false;

    pnew = (StackNode *) malloc( sizeof(StackNode));
    if (pnew == NULL)
    {
        printe("Unable to allocate memory!\n");
        return false;
    }
    CopyToStackNode(item, pnew);     /*item to pnew*/

    pnew->prev = NULL;

    if (StackIsEmpty(pq))
        pq->bottom = pnew;           /* item goes to front     */
    else
        pnew->prev = pq->top;            /* link at end of Stack   */


    pq->top = pnew;                /* record location of end */
    pq->items++;                    /* one more item in Stack */
    return true;
}

bool Pop(StackItem * pitem, Stack * pq)
{
    StackNode * pt;
    if (StackIsEmpty(pq))
        return false;

    CopyToStackItem(pq->top, pitem); //bot
    pt = pq->top;
    pq->top = pq->top->prev;
    free(pt);
    pq->items--;
    if (pq->items == 0)
        pq->bottom = NULL;

    return true;
}
/* empty the Stack                */
void EmptyTheStack(Stack * pq)
{
    StackItem dummy;
    while (!StackIsEmpty(pq))
        Pop(&dummy, pq);
}
/* Local functions                 */
static void CopyToStackNode(StackItem item, StackNode * pn)
{
   strncpy(pn->item, item, MAX_LINE);
}
static void CopyToStackItem(StackNode * pn, StackItem * pi)
{
    strncpy(*pi, pn->item, MAX_LINE);
}
