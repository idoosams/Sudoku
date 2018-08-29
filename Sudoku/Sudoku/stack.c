/*
* The module implements stack operations.
*/
#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "aux_main.h"
#include "user_interface.h"
#include "game_logic.h"

struct node_stack *top_node = NULL;


/* to insert elements in stack*/
void push(int row_index, int col_index, int value)
{
	struct node_stack *temp;
	temp = (struct node_stack*)malloc(sizeof(struct node_stack));

	temp->col_index = col_index;
	temp->row_index = row_index;
	temp->value = value;
	temp->link = top_node;
	top_node = temp;

}

/* to delete elements from stack */
void pop(int *row_index, int *col_index, int *value)
{
	struct node_stack *temp;
	if (top_node == NULL)
		printf("**Stack is empty**\n");
	else
	{
		*row_index = top_node->row_index;
		*col_index = top_node->col_index;
		*value = top_node->value;
		temp = top_node;
		top_node = top_node->link;
		free(temp);
	}
}

/* to check if stack is empty*/
int empty()
{
	if (top_node == NULL)
		return true;
	else
		return false;
}




/* to empty and destroy the stack*/
void destroy()
{
	struct node_stack *temp;
	int *row_index, *col_index, *value;
	temp = top_node;
	while (temp!= NULL)
	{
		pop(row_index, col_index, value);
		temp = temp->link;
	}
	printf("stack destroyed\n");
}


int numberOfSolutions()  {
	/* variables declarations */
	int row_index, col_index, count, value, board_length, **temp_matrice_values;
	int flag;
	count = 0;
	board_length = sudoku.block_col_length * sudoku.block_row_length;

	if (is_board_erronous()) {
		printf("Error: board contains erroneous values\n");
		return 0;
	}

	temp_matrice_values = initialize_integer_board(sudoku.block_col_length, sudoku.block_row_length);

	/* Save the board's values in a temporary board */
	for (col_index = 0; col_index < board_length; col_index++) {
		for (row_index = 0; row_index < board_length; row_index++) {
			temp_matrice_values[row_index][col_index] = sudoku.board[row_index][col_index].value;
		}
	}

	col_index = row_index = 0;
	value = flag = 1;
	while(flag && row_index < board_length) {
		col_index = col_index == board_length ? 0 : col_index;
		while (flag && col_index < board_length){
			if (sudoku.board[row_index][col_index].is_fixed == false) {
				if (sudoku.board[row_index][col_index].value == 0) {
					while(flag && value <= board_length) {
						if (set_reset_save_the_value(&value, row_index, col_index)) {
							if (row_index == board_length - 1 && col_index == board_length - 1) {
								count++;
								sudoku.board[row_index][col_index].value = 0;
								pop(&row_index, &col_index, &value);
								printf("POP: row_index = %d, col_index = %d, value = %d \n", row_index, col_index, value);
								print_board();
							}
							else break;
						}
						else {
							while (flag && value >= board_length) {
								if (!empty()) {
									sudoku.board[row_index][col_index].value = 0;
									pop(&row_index, &col_index, &value);
									printf("POP :row_index = %d, col_index = %d, value = %d \n", row_index, col_index, value);
									print_board();
								}
								else {
									flag = 0;
								}
							}
						}
						value++;
					}
				}
			}
			col_index++;
		}
		row_index++;
	}

	/* Copy the original values back to the sudoku.board */
	for (col_index = 0; col_index < board_length; col_index++) {
		for (row_index = 0; row_index < board_length; row_index++) {
			sudoku.board[row_index][col_index].value = temp_matrice_values[row_index][col_index];
		}
	}

	free_int_matrix(temp_matrice_values, sudoku.block_col_length, sudoku.block_row_length);

	if (count != 0) {
		printf("Number of solutions: %d\n", count);
		if (count == 1)
			printf("This is a good board!\n");
		else printf("The puzzle has more than 1 solution, try to edit it further\n");
	}
	else printf("The puzzle has 0 solutions.");

	return count;
}

int set_reset_save_the_value(int *value, int row_index, int col_index) {
	if (valid_value(row_index, col_index, *value)) {
		push(row_index, col_index, *value);
		sudoku.board[row_index][col_index].value = *value;
		print_board();
		*value = 1;
		return true;
	}
	return false;
}