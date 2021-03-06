#define _CRT_SECURE_NO_WARNINGS
#define NO_SOLUTION 2
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "game_logic.h"
#include "user_interface.h"
#include "aux_main.h"
#include "move_list.h"
#include "file_manipulation.h"
#include "solver.h"
#include "stack.h"

/*
		This module implements the sudoku's user interface methods.
		Anything related to parsing of standart input commands, and calling the relavent operation by it.
*/




struct abc_t conversion[] = {
	{ 0, "solve" },
{ 1, "edit" },
{ 2, "mark_errors" },
{ 3, "print_board" },
{ 4, "set" },
{ 5, "validate" },
{ 6, "generate" },
{ 7, "undo" },
{ 8, "redo" },
{ 9, "save" },
{ 10, "hint" },
{ 11, "num_solutions" },
{ 12, "autofill" },
{ 13, "reset" },
{ 14, "exit" },
{ 15, "error_command" }
};


/* Private function declaration */

/*
*	The Function prints a message accordingly to its arguments regarding a recent redo change in the board.
*	The row\column arguments are the board locations (not the ones we want to print)
*
*	@row: the cell's row where the change was made (in the board. 0 <--> board_len-1 )
*	@column: the cell's column where the change was made (in the board. 0 <--> board_len-1 )
*	@prev_val: the value that was previously in the cell that has been updated
*	@updated_val: the value that is being updated to.
*/
void redo_print(int row, int column, int prev_val, int updated_val);

/*
*	The Function prints a message accordingly to its arguments regarding a recent undo change in the board.
*	The row\column arguments are the board locations (not the ones we want to print)
*
*	@row: the cell's row where the change was made (in the board. 0 <--> board_len-1 )
*	@column: the cell's column where the change was made (in the board. 0 <--> board_len-1 )
*	@prev_val: the value that was previously in the cell that has been updated
*	@updated_val: the value that is being updated to.
*/
void undo_print(int row, int column, int prev_val, int updated_val);

/*
*	This function is used by print_board.
*	The Function prints a separator row accordingly to the current
*	 sudoku.block_col_length, sudoku.block_row_length global arguments.
*
*/
void separator_row();

/*
*	The Function checks if there is only one valid value for cell [row_index][col_index].
*
*	@row_index: the board's row of the cell
*	@col_index: the board's column of the cell
*
*	@returns: the only one valid value (if exists) or 0 , if no value exists, or more then one value exists for this cell.
*/
int one_possible_value(int row_index, int col_index);

/*
*	The function checks if the board is empty (all 0's) or not
*
*   @returns: true(1) when the board is empty
*			 false(0) otherwise.
*/
int is_board_empty();

/*
*	The Function uses the "move_list" module's functions 'add_new_node' and 'add_val_to_current_node' to add
*	a new node to the move_list. Should be used in "generate", "autofill"
*	If the functions receives a temporary matrice with the relavent changes, It updates the sudoku.board and prints
*	relevant info for the "autofill" command.
*	If the argument given is "NULL", the sudoku.board is already initialized and only a node is needed to be added to the list.
*
*	@temp_matrice_values: a pointer to the temporary matrice with the values that has been changed.
*						 (every cell which has a value other then 0 has been changed).
*
*   @returns: EXIT_SUCCESS(0) on successfuly adding the node.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int update_board_and_list(int **temp_matrice_values);

/*
*	An helper function for "generate". does the filling of num_of_cells_to_fill random cells with random values
*	then uses ILP to solve the board, and then removes all but 'num_of_cells_to_keep' cells in the board.
*	adds a new Node to the move_list when successful.
*
*	@num_of_cells_to_fill: number of cells to fill before solving with ILP
*	@num_of_cells_to_clear: number of cells to clear from the solvable board after ILP.
*
*	@returns: a pointer to the new board on success.
*	    	 on any error returns EXIT_FAILURE(1) and prints the error.
*			 when ILP fails or a cell has no valid values, return NO_SOLUTION(2).
*/
int generate_a_puzzle(int num_of_cells_to_fill, int num_of_cells_to_keep);

/*
*	The function checks if the game board is finished and correct, and if it is, the sudoku parameters initilaized
*/
void board_finished_check();

/*
*   Starts a puzzle in Solve mode, loaded from a file with the name "filepath".
*   "filepath" can be a full or relative path to the file.
*	We assume the file contains valid data and is correctly formatted.
*	Available from Solve,Edit,Init.
*
*   @filepath: a full or relative path to the file being opened.
*
*   @returns: EXIT_SUCCESS(0) on successful load of the file and board.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int Solve(char* filepath);

/*
*   Starts a puzzle in Edit mode, loaded from a file with the name "filepath".
*   "filepath" can be a full or relative path to the file.
*	If no paramater is passed. the program initiates with an empty 9x9 board.
*	We assume the file contains valid data and is correctly formatted.
*	Available from Solve,Edit,Init.
*
*   @filepath: a full or relative path to the file being opened.
*
*   @returns: EXIT_SUCCESS(0) on adding a new node.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int Edit(char* filepath);

/*
*	The Function receives an integer and if it's 0\1 changes the game's mark_errors field accordingly.
*
*	 @value: the integer which decides if errors should be marked or not
*
*/
void mark_errors(int value);

/*
*	The Function prints the board to the console accordingly to the standard.
*
*	where board has values of 0, it prints a blank space, as the cell is empty.
*	when mark_errors is ON, print asterisk before the number.
*	fixed cells are printed with a dot before the number.
*
*/
void print_board();

/*	Sets the value of cell <col_index,row_index> to value
*	only available in Edit and Solve modes.
*	This command prints the game board
*	In solve Mode, checks if all board's cells are filled, validates and prints an according message
*
*	The function updates the linked list with the cells that were changed.
*
*	@col_index: the cell's column where the change is made (as the user inputted them. 1 <--> board_len )
*	@row_index: the cell's row where the change is made (as the user inputted them. 1 <--> board_len )
*	@value: the value that that will be put in the cell.
*
*   @returns: EXIT_SUCCESS(0) on succeeding in the right output.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int set(int col_index, int row_index, int value);

/*
*	Validates the current board using ILP, ensuring it is solvable.
*	available in Edit and Solve modes.
*
*	the function uses the Gurobi ILP solver in the solver.c function in order to validate and get a solution.
*
*   @returns: true(1) when there is a valid solution to the board
*	        when board is erronous or there isn't a valid solution, returns false(0)
*/
int validate();

/*
*	Generates a puzzle by randomly filling X cells with random legal values,
*	running ILP to solve the resulting board, and then clearing all but Y random
*	cells.
*
*	 only available in Edit mode
*
*	@num_of_cells_to_fill: number of cells to fill before solving with ILP
*	@num_of_cells_to_clear: number of cells to clear from the solvable board after ILP.
*
*   returns: EXIT_SUCCESS(0) on adding a new node.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int generate(int num_of_cells_to_fill, int num_of_cells_to_keep);

/*
*	Undo's a move done by the user (when possible) and updates the board accordingly.
*	only available in Edit and Solve modes.
*	Uses the undo_list function in order to traverse the moves_list.
*
*   @returns: EXIT_SUCCESS(0) on adding a new node.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int undo();

/*
*	Redo's a move done by the user (when possible) and updates the board accordingly.
*	only available in Edit and Solve modes.
*	Uses the redo_list function in order to traverse the moves_list.
*
*   @returns: EXIT_SUCCESS(0) on adding a new node.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int redo();

/*
*   Saves a current board to a file with the name "filepath".
*   "filepath" can be a full or relative path to the file.
*	Available from Solve,Edit modes.
*
*	In edit mode, the board is validated and saved only if valid.
*	All cells are saved as 'fixed'.
*
*	In solve mode, the board can be saved with mistakes.
*	Only cells that were fixed at loading the board are saved as fixed.
*
*   @filepath: a full or relative path to the file being opened.
*
*   @returns: EXIT_SUCCESS(0) on adding a new node.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int Save(char* filepath);

/*
*	The Function prints the number of solutions for the current board.
*	Available in Solve\Edit modes.
*
*	The function uses an exhaustive backtracking implemented in stack.h
*	the cell in the solution given.
*
*   @returns: EXIT_SUCCESS(0) on success.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int num_solutions();

/*
*	The Function prints a hint to the user regarding the cell <column,row>.
*	Available in Solve Mode only.
*
*	The function uses the ILP (validate() ) to solve the board, and prints the value of
*	the cell in the solution given.
*
*	@col_index: the cell's column for the hint (as the user inputted them. 1 <--> board_len )
*	@row_index: the cell's row for the hint (as the user inputted them. 1 <--> board_len )
*
*   @returns: EXIT_SUCCESS(0) on successfully finishing the function.
			EXIT_FAILURE(1) when something goes wrong. printing the error and terminating.


*/
int hint(int col_index, int row_index);

/*
*	The Function fills the board's cells which has only a single legal value.
*	Available in Solve mode only.
*
*	The function updates the linked list with the cells that were changed.
*
*  @returns: EXIT_SUCCESS(0) on adding a new node.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int autofill();

/*
*	The Function reverts the board to its original loaded state.
*	Available in Edit/Solve modes.
*
*	The function goes over the entire move_list - undo's all moves and deletes all nodes.
*
*   @returns: EXIT_SUCCESS(0) on SUCCESSFULLY restarting the game.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int reset();

/*
*	The Function free's all memory resources that are open and terminates the program.
*
*   @returns: EXIT_SUCCESS(0) on exiting gracefully.
*/
int Exit();

/*
* The Function recives the command from the user and interprets it to a function that handles the command.
*
* @param buffer - the user's command. (its contents may be erased after calling this function)
*
*   @returns: EXIT_SUCCESS(0) on SUCCESSFULLY restarting the game.
*	         on any error returns EXIT_FAILURE(1) and prints the error.
*/
int user_command(char* buffer);

/*
*	The Function converts a string to one of the possbilties in the enum sudokuCommands.
*
*	 @str: the given string which will be converted.
*
*	@returns: the matching sudokuCommands(enum).
*/
sudokuCommands str2enum(const char *str);

/* Public functions */

int get_command_and_parse() {

	char command[MAX_COMMAND_SIZE];
	char* fgets_ret; /* for EOF checking */
	char c;
					 
	/* Get Commands and Play*/
	do {

		printf("Enter your command:\n");
		fgets_ret = fgets(command, MAX_COMMAND_SIZE, stdin);

		if (feof(stdin)) { /* EOF reached. exit. */
			printf("Exiting...\n");
			return EXIT_SUCCESS;
		}

		if (fgets_ret == NULL && ferror(stdin)) { /* fgets ancountered some error */
			perror("Error: fgets has failed.");
			return EXIT_FAILURE;
		}

		if (command[strlen(command) - 1] == '\n') {
			user_command(command);
		}
		else
		{
			printf("ERROR: invalid command. max length 256.\n");
			/* clear stdin */
			while ((c = getchar()) != '\n' && c != EOF);
		}

	} while (fgets_ret != NULL);

	return EXIT_SUCCESS;

}


/* Private functions implementations */


void redo_print(int row, int column, int prev_val, int updated_val) {
	if (updated_val == 0) {
		if (prev_val == 0) {
			printf("Redo %d,%d: from %c to %c\n",
				column + 1, row + 1, '_', '_');
		}
		else {
			printf("Redo %d,%d: from %d to %c\n",
				column + 1, row + 1, prev_val, '_');
		}
	}
	else {
		if (prev_val == 0) { /* updated not 0 , prev = 0 */
			printf("Redo %d,%d: from %c to %d\n",
				column + 1, row + 1, '_', updated_val);
		}
		else {
			printf("Redo %d,%d: from %d to %d\n",
				column + 1, row + 1, prev_val, updated_val);
		}
	}
}

void undo_print(int row, int column, int prev_val, int updated_val) {
	if (updated_val == 0) {
		if (prev_val == 0) {
			printf("Undo %d,%d: from %c to %c\n",
				column + 1, row + 1, '_', '_');
		}
		else {
			printf("Undo %d,%d: from %c to %d\n",
				column + 1, row + 1, '_', prev_val);
		}
	}
	else {
		if (prev_val == 0) {
			printf("Undo %d,%d: from %d to %c\n",
				column + 1, row + 1, updated_val, '_');
		}
		else {
			printf("Undo %d,%d: from %d to %d\n",
				column + 1, row + 1, updated_val, prev_val);
		}
	}
}

void separator_row() {
	int i;
	for (i = 0; i <= 4 * sudoku.block_col_length*sudoku.block_row_length + sudoku.block_row_length; i++)
		printf("-");
	printf("\n");
}

int one_possible_value(int row_index, int col_index) {
	int i, count, board_length, value;
	count = 0;
	value = 0;
	board_length = sudoku.block_col_length*sudoku.block_row_length;
	for (i = 1; i <= board_length; i++) {
		if (valid_value(row_index, col_index, i)) {
			count++;
			value = i;
			if (count > 1) {
				return false;
			}
		}
	}
	return value;
}

int is_board_empty() {
	int i, j, board_size;

	board_size = sudoku.block_col_length * sudoku.block_row_length;

	for (i = 0; i < board_size; i++) {
		for (j = 0; j < board_size; j++) {
			if (sudoku.board[i][j].value != 0) {
				return false;
			}

		}
	}
	return true;
}

int update_board_and_list(int **temp_matrice_values) {
	int add_node_flag, col_index, board_length, row_index, updated_val;

	board_length = sudoku.block_row_length * sudoku.block_col_length;
	add_node_flag = true;
	
	for (row_index = 0; row_index < board_length; row_index++) {
		for (col_index = 0; col_index < board_length; col_index++) {
			if (temp_matrice_values) { /* Autofill function. Copy value from the temp matrix to the board */
				if (temp_matrice_values[row_index][col_index] != 0) {
					updated_val = temp_matrice_values[row_index][col_index];
					if (add_node_flag == true) { /* In case this is the first value which is being changed in the board */
						if (add_new_node(row_index, col_index, ZERO, updated_val) == EXIT_FAILURE) {
							printf("adding new node to list failed. Exiting.\n");
							return EXIT_FAILURE;
						}
						add_node_flag = false;
					}
					else {
						if (add_val_to_current_node(row_index, col_index, ZERO, updated_val) == EXIT_FAILURE) {
							printf("add new value to existing node failed. Exiting.\n");
							return EXIT_FAILURE;
						}
					}
					/* Update the value in the board, and print a message regarding */
					sudoku.board[row_index][col_index].value = updated_val;
					printf("Cell <%d,%d> set to %d\n", col_index + 1, row_index + 1, updated_val);
					update_num_of_filled_cells(ZERO, updated_val);
				}
			}
			else { /* Generate function. the values are already filled on the board */
				if (sudoku.board[row_index][col_index].value != 0) {
					updated_val = sudoku.board[row_index][col_index].value;
					if (add_node_flag == true) { /* In case this is the first value which is being changed in the board */
						if (add_new_node(row_index, col_index, ZERO, updated_val) == EXIT_FAILURE) {
							printf("adding new node to list failed. Exiting.\n");
							return EXIT_FAILURE;
						}
						add_node_flag = false;
					}
					else {
						if (add_val_to_current_node(row_index, col_index, ZERO, updated_val) == EXIT_FAILURE) {
							printf("add new value to existing node failed. Exiting.\n");
							return EXIT_FAILURE;
						}
					}
					printf("Cell <%d,%d> set to %d\n", col_index + 1, row_index + 1, updated_val);
					update_num_of_filled_cells(ZERO, updated_val);
				}
			}

			
		}
	}

	return EXIT_SUCCESS;
}

int generate_a_puzzle(int num_of_cells_to_fill, int num_of_cells_to_keep) {
	int i, rand_row, rand_col, board_len, num_of_filled, rand_index;
	int *optional_values, num_of_options, num_of_cells_in_board, fill_values_not_solution;

	board_len = sudoku.block_col_length * sudoku.block_row_length;
	num_of_filled = 0;
	fill_values_not_solution = true;
	optional_values = calloc(board_len, sizeof(int));
	if (!optional_values) {
		printf("calloc failed. Exiting.\n");
		exit(EXIT_FAILURE);
	}


	/* generate random cells untill 'num_of_cells_to_fill' cells has been filled */
	while (num_of_filled < num_of_cells_to_fill) {
		rand_row = (rand() % (board_len)); /* random numbers between 0 ~ board_len-1 */
		rand_col = (rand() % (board_len)); /* random numbers between 0 ~ board_len-1 */


		num_of_options = 0; /* num of values in the optional_values array */
		if (sudoku.board[rand_row][rand_col].value == 0) {  /* if random cell hasn't been initialized yet */

			/* create an array with possible values for the cell. 1 ~ board_len */
			for (i = 1; i <= board_len; i++) {
				if (valid_value(rand_row, rand_col, i)) {
					optional_values[num_of_options] = i;
					num_of_options++;
				}
			}

			if (num_of_options > 0) { /* cell has some valid values */
				rand_index = (rand() % (num_of_options)); /* random numbers between 0 ~ num_of_options-1 */
				sudoku.board[rand_row][rand_col].value = optional_values[rand_index];
				num_of_filled++;

			}
			else {
				free(optional_values);
				reset_sudoku_board_values();
				return NO_SOLUTION;
			}
		}
	}

	/*Solve the matrice using ILP */
		if (is_there_a_solution(NULL, fill_values_not_solution) == false) {
		reset_sudoku_board_values();
		free(optional_values);
		return NO_SOLUTION;
	}


	/* remove all but "num_of_cells_to_keep" cells in the board */
	num_of_cells_in_board = board_len * board_len;
	while (num_of_cells_in_board > num_of_cells_to_keep) {
		rand_row = (rand() % (board_len)); /* random numbers between 0 ~ board_len-1 */
		rand_col = (rand() % (board_len)); /* random numbers between 0 ~ board_len-1 */
	
		/* if random cell hasn't been deleted yet */
		if (sudoku.board[rand_row][rand_col].value != 0) {
			sudoku.board[rand_row][rand_col].value = 0;
			num_of_cells_in_board--;
		}
	}

	/* Copy the temp_matrice to the sudoku.board and add a new node to the list */
	if (update_board_and_list(NULL) == EXIT_FAILURE) {
		reset_sudoku_board_values();
		free(optional_values);
		return EXIT_FAILURE;
	}

	free(optional_values);
	return EXIT_SUCCESS;

}

void board_finished_check() {
	int num_of_cells_in_board, board_len;

	board_len = sudoku.block_col_length * sudoku.block_row_length;
	num_of_cells_in_board = board_len * board_len;

	if (sudoku.num_of_filled_cells == num_of_cells_in_board && sudoku.game_mode == solve) {
		if (is_board_erronous()) {
			printf("Puzzle solution erroneous\n");
		}
		else {
			printf("Puzzle solved successfully\n");

			/* Change the game mode */
			sudoku.game_mode = init;
			sudoku.num_of_filled_cells = 0;

			/* Reset basic game utilities */
			delete_list_full();
			free_board();
			sudoku.board = NULL;
		}
	}
}

/*
*			Private main functions for user commands:
*/

int Solve(char* filepath) {
	FILE* fd;
	int num_of_filled_cells;
	int block_rows, block_cols;


	/* Open the file*/
	fd = fopen(filepath, "r");
	if (!fd) {
		printf("Error: File doesn't exist or cannot be opened\n"); /* case b */
		return EXIT_SUCCESS;
	}

	/* Change the game mode */
	sudoku.game_mode = solve;
	num_of_filled_cells = 0;

	/* Reset basic game utilities (case d) */
	delete_list_full();
	free_board();
	sudoku.board = NULL;

	/* Read from the file and initialize the board and sudoku's block_col/row lengths */
	if (read_from_file(fd, &block_rows, &block_cols, &num_of_filled_cells) == EXIT_FAILURE) {
		free_board();
		printf("Error: reading from file has failed. Exiting\n"); /* case b */
		exit(EXIT_FAILURE);
	}

	
	/* Set basic sudoku utilities */
	sudoku.block_row_length = block_rows;
	sudoku.block_col_length = block_cols;
	sudoku.num_of_filled_cells = num_of_filled_cells;

	update_board_errors();
	print_board();

	fclose(fd);

	return EXIT_SUCCESS;
}

int Edit(char* filepath) {
	FILE* fd;
	int block_rows, block_cols, num_of_filled_cells;

	if (filepath != NULL) {
		/* Open the file*/
		fd = fopen(filepath, "r");
		if (!fd) {
			printf("Error: File cannot be opened\n"); /* case b */
			return EXIT_SUCCESS;
		}

		/* Change the game mode */
		sudoku.game_mode = edit;
		num_of_filled_cells = 0;

		/* Reset basic game utilities (case f) */
		delete_list_full();
		free_board();
		sudoku.board = NULL;


		/* Read from the file and initialize the board and sudoku's block_col/row lengths */
		if (read_from_file(fd, &block_rows, &block_cols, &num_of_filled_cells) == EXIT_FAILURE) {
			printf("Error: reading from file has failed. Exiting\n"); /* case b */
			exit(EXIT_FAILURE);
		}

		/* Set basic sudoku utilities */
		sudoku.block_row_length = block_rows;
		sudoku.block_col_length = block_cols;
		sudoku.num_of_filled_cells = num_of_filled_cells;

		fclose(fd);
	}
	else {

		/* Change the game mode */
		sudoku.game_mode = edit;
		num_of_filled_cells = 0;

		/* Reset basic game utilities (case f) */
		delete_list_full();
		free_board();
		sudoku.board = NULL;


		if (initialize_new_board(DEFAULT_BLOCK_LEN, DEFAULT_BLOCK_LEN) == EXIT_FAILURE) {
			printf("Error: Initializing a new board has failed. Exiting\n"); /* case b */
			exit(EXIT_FAILURE);
			
		}

		sudoku.block_row_length = DEFAULT_BLOCK_LEN;
		sudoku.block_col_length = DEFAULT_BLOCK_LEN;
		sudoku.num_of_filled_cells = num_of_filled_cells;
	}


	update_board_errors();
	print_board();


	return EXIT_SUCCESS;
}

void mark_errors(int value) {
	if (value != 0 && value != 1) {
		printf("Error: the value should be 0 or 1\n");
	}
	else {
		sudoku.mark_errors = value;
	}
}

void print_board(){
	/* variables declarations */
	int i, j, board_length;
	board_length = sudoku.block_col_length * sudoku.block_row_length;

	/* Print 4N+n+1 dashes for the start*/
	separator_row();

	/* Go over the columns */
	for (i = 0; i < board_length; i++) {
		printf("|"); /* Opening pipe */

					 /* Go over Columns*/
		for (j = 0; j < board_length; j++) {

			if (sudoku.board[i][j].value == 0) /* blank */
			{
				printf("    ");
			}
			else if (sudoku.board[i][j].is_fixed) {				/* If fixed number */

				printf(" %2d.", sudoku.board[i][j].value); /* DOT for fixed number*/
			}
			else if (!sudoku.board[i][j].is_fixed) { /* Non-fixed number that the user inputed */
				printf(" %2d", sudoku.board[i][j].value);
				if ( (sudoku.game_mode == edit || sudoku.mark_errors) && sudoku.board[i][j].error) /* check if we need to mark an error */
					printf("*");
				else printf(" ");
			}

			/* after every m numbers , print a pipe*/
			if (j != board_length - 1) {
				if (j % sudoku.block_col_length == sudoku.block_col_length - 1)
					printf("|");
			}
			else printf("|");

		}
		printf("\n"); /*  Next line*/

					  /*Print dashes every 3 lines*/
		if (i % sudoku.block_row_length == sudoku.block_row_length - 1) {
			separator_row();
		}
	}
}

int set(int col_index, int row_index, int value) { 

	int prev_val, updated_val, board_len;
	int row_index_board, col_index_board;

	board_len = sudoku.block_row_length * sudoku.block_col_length;
	updated_val = value;

	/* check if the values are legal */
	if (row_index == FAILURE || col_index == FAILURE || value == FAILURE ||
		value < 0 || value > board_len ||
		row_index < 1 || row_index > board_len ||
		col_index < 1 || col_index > board_len) {
		printf("Error: value not in range 0-%d\n", board_len);
		return EXIT_SUCCESS;
	}

	row_index_board = row_index - 1;
	col_index_board = col_index - 1;


	/* check if (i,j) is a fixed cell (case e) */
	if (sudoku.board[row_index_board][col_index_board].is_fixed) { /* it is fixed.*/
		printf("Error: cell is fixed\n");
		return EXIT_SUCCESS;
	}

	prev_val = sudoku.board[row_index_board][col_index_board].value;


	/* Update the value in the board itself and the number of filled cells */
	sudoku.board[row_index_board][col_index_board].value = value;
	update_num_of_filled_cells(prev_val, updated_val);

	/* Update the move_list. (case f) */
	if (add_new_node(row_index_board, col_index_board, prev_val, updated_val) == EXIT_FAILURE) {
		printf("Error: adding a new node has failed. Exiting\n"); /* case b */
		exit(EXIT_FAILURE);
	}

	/* update errors for all relevant cells */
	update_errors(row_index_board, col_index_board);

	print_board();

	board_finished_check();

	return EXIT_SUCCESS;
}

int validate() {
	int fill_values_not_solution;

	fill_values_not_solution = false;

	if (is_board_erronous()) {
		printf("Error: board contains erroneous values\n");
		return false;
	}

	if (is_there_a_solution(NULL, fill_values_not_solution) == true) {
		printf("Validation passed: board is solvable\n");

		return true;
	}
	else {
		printf("Validation failed: board is unsolvable\n");
		return false;
	}

}

int generate(int num_of_cells_to_fill, int num_of_cells_to_keep) {
	int DIM, num_of_tries, ret_val, generate_success_flag;

	DIM = sudoku.block_col_length * sudoku.block_row_length;
	num_of_tries = 1000;

	if (num_of_cells_to_fill == FAILURE || num_of_cells_to_keep == FAILURE ||
		num_of_cells_to_fill > DIM*DIM ||
		num_of_cells_to_fill < 0 ||
		num_of_cells_to_keep > DIM*DIM || 
		num_of_cells_to_keep < 0) {
		printf("Error: value not in range 0-%d\n", DIM*DIM); /* case d */
		return EXIT_SUCCESS;
	}

	if ( ! is_board_empty()) {
		printf("Error: board is not empty\n");
		return EXIT_SUCCESS;
	}

	generate_success_flag = false;
	while (num_of_tries > 0 && (!generate_success_flag) ) {

		/* helper function to generate a board */
		ret_val = generate_a_puzzle(num_of_cells_to_fill, num_of_cells_to_keep); 

		if (ret_val == EXIT_SUCCESS) {
			generate_success_flag = true;
		}
		else if (ret_val == NO_SOLUTION) {
			num_of_tries--;
		}
		else if (ret_val == EXIT_FAILURE) {
			printf("Error: Generate has failed. Exiting\n");
			exit(EXIT_FAILURE);
		}

		if (num_of_tries == 0) {
			printf("Error: puzzle generator failed\n");
			return EXIT_SUCCESS;
		}
	}

	update_board_errors();
	print_board();
	return EXIT_SUCCESS;
	
}

int undo() {

	int num_of_values, i, row, col, prev, updated;
	node_vals* values_array;
	int reset_flag;

	reset_flag = false;

	values_array = undo_list(&num_of_values, reset_flag);

	if (values_array) {
		/* Update the board to the values in the previous turn */
		for (i = 0; i < num_of_values; i++) {
			row = values_array[i].row;
			col = values_array[i].column;
			prev = values_array[i].prev_val;
			updated = values_array[i].updated_val;

			update_num_of_filled_cells(updated, prev);
			sudoku.board[row][col].value = prev;
			update_errors(row, col);  /* update board's errors in that cell's neighbors */
		}

		/* Print the board and then the changed cells (case e) */
		print_board();
		for (i = 0; i < num_of_values; i++) {
			row = values_array[i].row;
			col = values_array[i].column;
			prev = values_array[i].prev_val;
			updated = values_array[i].updated_val;

			undo_print(row, col, prev, updated);
		}
	}

	return EXIT_SUCCESS;
}

int redo() {
	int num_of_values, i, row, col, prev, updated;
	node_vals* values_array;

	values_array = redo_list(&num_of_values);

	if (values_array) {
		/* Update the board to the values in future turn */
		for (i = 0; i < num_of_values; i++) {
			row = values_array[i].row;
			col = values_array[i].column;
			prev = values_array[i].prev_val;
			updated = values_array[i].updated_val;

			update_num_of_filled_cells(prev, updated);
			sudoku.board[row][col].value = updated;
			update_errors(row, col); /* update board's errors in that cell's neighbors */
		}

		print_board();
		/* Print the changed cells (case e) */
		for (i = 0; i < num_of_values; i++) {
			row = values_array[i].row;
			col = values_array[i].column;
			prev = values_array[i].prev_val;
			updated = values_array[i].updated_val;

			redo_print(row, col, prev, updated);
		}
	}

	return EXIT_SUCCESS;
}

int Save(char* filepath) {
	FILE* fd;

	if (sudoku.game_mode == edit) {
		if ( is_board_erronous() ) {
			printf("Error: board contains erroneous values\n");
			return EXIT_SUCCESS;
		}

		if (is_there_a_solution(NULL, false) == false) {
			printf("Error: board validation failed\n");
			return EXIT_SUCCESS;
		}
	}

	/* Open the file*/
	fd = fopen(filepath, "w");
	if (!fd) {
		printf("Error: File cannot be created or modified\n");
		return EXIT_SUCCESS;
	}

	if (save_to_file(fd) == EXIT_FAILURE) {
		printf("Saving to the file has failed.\n");
		return EXIT_FAILURE;
	}

	printf("Saved to: %s\n", filepath);
	fclose(fd);
	return EXIT_SUCCESS;
}

int hint(int col_index, int row_index) {

	int board_len, row_index_board, col_index_board, fill_values_not_solution;

	fill_values_not_solution = false;
	board_len = sudoku.block_row_length * sudoku.block_col_length;

	/* check if the values are legal */
	if (row_index == FAILURE || col_index == FAILURE ||
		row_index < 1 || row_index > board_len ||
		col_index < 1 || col_index > board_len) {
		printf("Error: value not in range 1-%d\n", board_len);
		return EXIT_SUCCESS;
	}

	if (is_board_erronous()) {
		printf("Error: board contains erroneous values\n");
		return EXIT_SUCCESS;
	}
	row_index_board = row_index - 1;
	col_index_board = col_index - 1;

	if (sudoku.board[row_index_board][col_index_board].is_fixed) { /* it is fixed.*/
		printf("Error: cell is fixed\n");
		return EXIT_SUCCESS;
	}

	if (sudoku.board[row_index_board][col_index_board].value != 0) { /* it has a value.*/
		printf("Error: cell already contains a value\n");
		return EXIT_SUCCESS;
	}

	if (is_there_a_solution(NULL, fill_values_not_solution) == true) {
		printf("Hint: set cell to %d\n", sudoku.board[row_index_board][col_index_board].solution);
		return EXIT_SUCCESS;
	}
	else {
		printf("Error: board is unsolvable\n");
		return EXIT_SUCCESS;
	}
}

int num_solutions() {
	return numberOfSolutions();
}

int autofill() {
	int row_index, col_index, board_length, value, **temp_matrice_values;
	board_length = sudoku.block_col_length*sudoku.block_row_length;

	temp_matrice_values = NULL;

	/* Validate the board's errors, and print this if it's not valid: (case c)*/
	if (is_board_erronous()) {
		printf("Error: board contains erroneous values\n");
		return EXIT_SUCCESS;
	}

	/* Initialize a temp matrix */
	temp_matrice_values = initialize_integer_board();
	if (!temp_matrice_values) {
		printf("Error: Initializing a new board has failed. Exiting\n"); /* case b */
		exit(EXIT_FAILURE);
	}

	for (col_index = 0; col_index < board_length; col_index++) {
		for (row_index = 0; row_index < board_length; row_index++) {
			if (sudoku.board[row_index][col_index].value == 0) { /* check if the cell is unfilled (value=0) */
				value = one_possible_value(row_index, col_index);  /* checks that there is only 1 valid value */
				temp_matrice_values[row_index][col_index] = value;
			}
		}
	}

	/* Copy the temp_matrice to the sudoku.board and add a new node to the list 
	   Also prints the changes to the std-output */
	if (update_board_and_list(temp_matrice_values) == EXIT_FAILURE) {
		printf("Error: Updating the board has failed. Exiting\n"); /* case b */
		exit(EXIT_FAILURE);
	}

	free_int_matrix(temp_matrice_values, sudoku.block_col_length, sudoku.block_row_length);

	print_board(); /* case g */

	board_finished_check(); /* set command (case i) */

	return EXIT_SUCCESS;
}

int reset() {

	int num_of_values, i, j, row, col, prev, board_len;
	node_vals* values_array;
	int reset_flag;

	board_len = sudoku.block_col_length * sudoku.block_row_length ;

	reset_flag = true;
	do { /* Initiate undo on the list until the head */
		values_array = undo_list(&num_of_values, reset_flag);

		if (values_array) {
			/* Update the board to the values in the previous turn */
			for (i = 0; i < num_of_values; i++) {
				row = values_array[i].row;
				col = values_array[i].column;
				prev = values_array[i].prev_val;
				/*updated = values_array[i].updated_val;*/

				sudoku.board[row][col].value = prev; /* Update the board accordingly */
			}
		}
	} while (values_array);

	/* Update the sudoku.num_of_filled_cells variable */
	sudoku.num_of_filled_cells = ZERO;
	for (i = 0; i < board_len; i++) {
		for (j = 0; j < num_of_values; j++) {
			if (sudoku.board[i][j].value != 0) {
				(sudoku.num_of_filled_cells)++;
			}
		}
	}
	/* Delete the list except the head */
	delete_list_from_the_current_node();

	printf("Board reset\n");

	return EXIT_SUCCESS;
}

int Exit() {
	free_board();
	delete_list_on_exit();

	printf("Exiting...\n");

	/* Terminate the program */
	exit(EXIT_SUCCESS);

}

sudokuCommands str2enum(const char *str)
{
	unsigned int j;
	for (j = 0; j < sizeof(conversion) / sizeof(conversion[0]); ++j)
		if (!strcmp(str, conversion[j].str))
			return conversion[j].val;
	return error_command;
}

int user_command(char* buffer) {
	sudokuCommands sudoku_command;
	char *xchar, *ychar, *zchar, *command;
	int xchar_asInt, ychar_asInt, zchar_asInt;
	command = strtok(buffer, " \t\r\n");
	xchar = strtok(NULL, " \t\r\n");
	ychar = strtok(NULL, " \t\r\n");
	zchar = strtok(NULL, " \t\r\n");
	if (command == NULL) {
		return EXIT_SUCCESS; 	/* checks for empty line */
	}
	sudoku_command = str2enum(command);
	xchar_asInt = str_to_num(xchar);
	ychar_asInt = str_to_num(ychar);
	zchar_asInt = str_to_num(zchar);
	switch (sudoku_command)
	{
	case solve_command:
		if (!xchar) {
			printf("ERROR: invalid command\n"); /* case b */
		}
		else {
			return Solve(xchar);
		}
		break;
	case edit_command:
		return Edit(xchar);
		break;
	case mark_errors_command:
		if (sudoku.game_mode != solve || (!xchar)) {
			printf("ERROR: invalid command\n"); /* case b */
		}
		else {
			mark_errors(xchar_asInt);
		}
		break;
	case print_board_command:
		if (sudoku.game_mode == init) {
			printf("ERROR: invalid command\n"); /* case b */
		}
		else {
			print_board();
		}
		break;
	case set_command:
		if (sudoku.game_mode == init || !xchar || !ychar || !zchar) {
			printf("ERROR: invalid command\n"); /* case b */
		}
		else{
			set(xchar_asInt, ychar_asInt, zchar_asInt);
		}
		break;
	case validate_command:
		if (sudoku.game_mode == init)
			printf("ERROR: invalid command\n"); /* case b */
		else {
			validate();
		}
		break;
	case generate_command:
		if (sudoku.game_mode != edit || !xchar || !ychar)
			printf("ERROR: invalid command\n"); /* case b */
		else {
			generate(xchar_asInt, ychar_asInt);
		}
		break;
	case undo_command:
		if (sudoku.game_mode == init)
			printf("ERROR: invalid command\n"); /* case b */
		else {
			undo();
		}
		break;
	case redo_command:
		if (sudoku.game_mode == init)
			printf("ERROR: invalid command\n"); /* case b */
		else {
			redo();
		}
		break;
	case save_command:
		if (sudoku.game_mode == init || !xchar) {
			printf("ERROR: invalid command\n"); /* case b */
		}
		else {
			Save(xchar);
		}
		break;
	case hint_command:
		if (sudoku.game_mode != solve || !xchar || !ychar)
			printf("ERROR: invalid command\n"); /* case b */
		else {
			hint(xchar_asInt, ychar_asInt);
		}
		break;
	case num_solutions_command:
		if (sudoku.game_mode == init)
			printf("ERROR: invalid command\n"); /* case b */
		else {
			num_solutions();
		}
		break;
	case autofill_command:
		if (sudoku.game_mode != solve)
			printf("ERROR: invalid command\n"); /* case b */
		else {
			autofill();
		}
		break;
	case reset_command:
		if (sudoku.game_mode == init) {
			printf("ERROR: invalid command\n");
		}
		else {
			reset();
		}
		break;
	case exit_command:
		Exit();
		break;
	default:
		printf("ERROR: invalid command\n");
		break;
	}
	return EXIT_SUCCESS;
}
