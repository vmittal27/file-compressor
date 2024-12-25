#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stack.h"

stack *stack_new() {
    stack *st = malloc(sizeof(stack));
    st->size = 0;
    st->head = NULL;

    return st;
}

void stack_push(stack *stack, int data) {
    if (stack != NULL) {
        
        node *n = malloc(sizeof(node)); 
        n->data = data;
        n->next = stack->head; 

        stack->head = n; 
        stack->size++;
    }
}

int stack_pop(stack *stack) {
    if (stack != NULL && stack->head != NULL) {

        // store data to return
        int data = stack->head->data;
        node *node = stack->head;

        // move pointers to remove current head
        stack->head = stack->head->next;
        stack->size--;
        free(node);
    
        return data; 
    }

    return -1;
}

void stack_free(stack *stack) {
    if (stack != NULL) {
        while (stack->head) { // free linked list
            node *n = stack->head;
            stack->head = stack->head->next;
            free(n);
        }
    }

    free(stack); 
}




