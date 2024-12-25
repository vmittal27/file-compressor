/*
Linked-list based stack implementation in C. 
*/
#ifndef STACK
#define STACK

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// DATA DEFINITIONS

struct node {
    int data;
    struct node *next;
}; 

typedef struct node node; 

struct stack {
    node *head;
    int size; 
}; 

typedef struct stack stack;

// METHODS

// constructs a new empty stack
stack *stack_new();

// pushes data to the top of the stack
void stack_push(stack *stack, int data);

// removes and returns the data held at the top of the stack
// returns -1 if the stack was empty
int stack_pop(stack *stack);


// frees a stack
void stack_free(stack *stack);

#endif