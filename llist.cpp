#include <iostream>
#include <cstdlib>
#include "nheader.h"

using namespace std;

// **p is a pointer to a pointer...
// **p gets the node
// *p get the pointer to the node
// p is where the node should go (the next node?)
llist * list_add(llist **p, void *add)
{
  // p isn't null, and neither is add.
  if (p == NULL || add == NULL) { return NULL; }
  
  // Allocate new space for the next node.
  // Make sure it succeeded.
  llist *n = (llist *)malloc(sizeof(llist));
  if (n == NULL) { return NULL; }

  n->next = *p; // new pointer points to what used to previous one.
  *p = n; // Input pointer address updated to new head.
  n->x = add;

  return *p; // return new head pointer.

}

void * list_merge(llist **p, llist *q)
{

  if (p != NULL) {
    if (q != NULL) {
      if (*p != NULL) {
	llist *n;
	n=*p;
	while (n->next != NULL) {n=*(&(n->next));}
	n->next=q;
      }
      else {
	*p=q;
      }
    }
  }

}

llist * list_append(llist **p, void *add)
{
  // p isn't null
  if (p==NULL) {return NULL;}

  if (*p != NULL) {return list_append(&(*p)->next,add);}
  else {return list_add(p,add);}

}

void list_delete(llist **p)
{
  if (p != NULL && *p != NULL) {
    while (*p != NULL) {

      list_remove(p);
    
    }
  }
}

void list_remove(llist **p) // remove item
{
  // pointer to pointer to first node exists,
  // as does the pointer to the first node.
  if (p != NULL && *p != NULL) {

    // copy pointer to node to be removed.
    llist *n = *p;

    // pointer to node to be removed is replaced with pointer to next node.
    *p = (*p)->next;

    // memory for node to be removed is deallocated.
    // The list item dies with it.
    free(n);

  }
}

// Search for stored data.
// n is pointer to pointer to node from which
// to begin search.
llist **list_search(llist **n, void *find)
{
  // check that n points somewhere.
  if (n == NULL)
    return NULL;

  // While the pointer to the next node goes somewhere.
  while (*n != NULL) {

    // hey whoa you found it. Return the pointer to the pointer to the node.
    if ((*n)->x == find)
      {
	return n;
      }
    // get the pointer to the pointer to the next node.
    n = &(*n)->next;

  }

  //found nothing
  return NULL;

}

void *list_item(llist **p)
{
  if (p == NULL || *p == NULL) {return NULL;}
  return (*p)->x;
}
