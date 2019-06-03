#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

#include "timer.h"
//#include "io.c"
#include "MAX7219.c"

#include "consts.h"
#include "utils.c" // Init_LED_Matrices, Set_Brightness, Load_word, ...
#include "blocks.c" // Block drawing funcs

#define INFO_INDEX 0

typedef unsigned  char uc;
typedef   signed  char sc;
typedef unsigned short us;

uc two_to_the(uc exponent)
{
	if (exponent > 8) return 0;
	
	uc res = 1;
	
	for (uc i = 0; i < exponent; i++)
	{
		res *= 2;
	}
	return res;
}

void MakeRandom()
{
	uc matrix = 0; uc col = 0;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b01111111;
	POS_ARRAY[matrix][col++] = 0b00011111;
	POS_ARRAY[matrix][col++] = 0b00011111;
	POS_ARRAY[matrix][col++] = 0b00001111;
	POS_ARRAY[matrix][col++] = 0b00000111;
	POS_ARRAY[matrix][col++] = 0b00111111;
	POS_ARRAY[matrix][col++] = 0b00001111;
	POS_ARRAY[matrix][col++] = 0b00000011;	
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111101;
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111101;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11011111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11110111;
	POS_ARRAY[matrix][col++] = 0b11111111;
}

void MakeWall()
{
	uc matrix = 0; uc col = 0;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
}

void Drop_A_Block(uc (*fct) (uc, uc, uc, sc, uc, uc), uc *cur_matrix, uc *is_done, uc *left, uc *right, uc *orientation, sc *from_bottom)
{
	uc contact = 0;
	
	if (!(*is_done))
	{	
		// *** MAY ABSTRACT AWAY ***
		if (~PINA & 0x08 && *left < 8) // Shift check
		{
			// When horizontal, there should be an extra space left for the left/right most light
			++(*left);
			++(*right);
		}
		else if (~PINA & 0x08 && *left == 8) // resets block on the rhs
		{
			if (*orientation == Horizontal_Down || *orientation == Horizontal_Up)
				*left = 3;
			else
				*left = 2;
			*right = 1;
		}
		// *** MAY ABSTRACT AWAY ***
		if (~PINA & 0x04) // Orientation check
		{
			*orientation = (*orientation + 1) % 4;
			
			if ((*orientation == Horizontal_Down || *orientation == Horizontal_Up) && (*left - *right) == 1)
			{
				// Maintains left side in bound
				if (*left > 7)
				--(*right);
				else
				++(*left);
			}
			else if ((*left - *right) == 2)
			--(*left);
		}
		Clear_All();
		
		// from_bottom < 0 if part of the block is on the lower matrix
		if (*from_bottom < 0 && *cur_matrix < BOTTOM_MATRIX)
		{
			// Not placing this fct call first erases the upper matrix's part of block
			contact = (*fct)(*cur_matrix + 1, *left, *right, 8 + *from_bottom, *orientation, 0); 
		}
		contact = (*fct)(*cur_matrix, *left, *right, *from_bottom, *orientation, contact);
		--(*from_bottom);
		
		if(*from_bottom < -2 && *cur_matrix < 3 && !contact)
		{
			*from_bottom = 5;
			++(*cur_matrix);
		}
		else if ((*from_bottom < 0 && *cur_matrix >= 3) || contact)
		{
			*is_done = 0x01;
		}
	}
}


void DrawBlockMatrix()
{
	for (uc col = 0; col < 8; col++)
	{
		for (uc matrix = 0; matrix < 4; matrix++)
		{
			MAX7219_SendByte(col+1);
			MAX7219_SendByte(POS_ARRAY[matrix][col]);
		}
		Load_Word();
	}
}

// This function should collapse the specified row
void Row_Collapser(uc curr_matrix, uc row)
{
	for (uc col = 0; col < 8; col++)
	{
		if (row == 0)
		{
			POS_ARRAY[curr_matrix][col] >>= 1;
		}
		else
		{
			uc above_row  = POS_ARRAY[curr_matrix][col] & (0xFF << (row + 1)); // Saves everything above solid row
			POS_ARRAY[curr_matrix][col] &= (0xFF >> (7 - (row - 1))); // Will mask off everything above solid row
			POS_ARRAY[curr_matrix][col] |= (above_row >> 1); // Shifts everything below solid row by one
		}
	}
}

Collapsed_Row_Merger(uc re[4][9])
{
/*
	// Maintains sum of collapsed rows per matrix
	uc sum_of_collapsed_rows[4];
	sum_of_collapsed_rows[3] = re[3][0];
	sum_of_collapsed_rows[2] = sum_of_collapsed_rows[3] + re[2][0];
	sum_of_collapsed_rows[1] = sum_of_collapsed_rows[2] + re[1][0];
	sum_of_collapsed_rows[0] = sum_of_collapsed_rows[1] + re[0][0];
*/

	// matrix < 3 ensures that (matrix  + 1) will never be greater than 3
	for (uc matrix = 3; matrix > 0; --matrix)
	{
		for (uc col = 0; col < 8; ++col)
		{
			for (uc tmp_mtrx = matrix - 1; tmp_mtrx < 3; ++tmp_mtrx)
			{
				PORTB &= 0x07;
				PORTB |= tmp_mtrx << 4;
				//*********************
				
				uc tmp_col = POS_ARRAY[tmp_mtrx][col];
				
				POS_ARRAY[tmp_mtrx][col] = tmp_col >> (re[tmp_mtrx + 1][INFO_INDEX]);
				POS_ARRAY[tmp_mtrx + 1][col] |= (tmp_col << (7 - (re[tmp_mtrx + 1][INFO_INDEX] - 1)));
				
				//******************************
				while(!TimerFlag); TimerFlag = 0;
				DrawBlockMatrix();
			}
		}
	}
}

void Solid_Row_Eliminator()
{
		/*
		 * re[0:4][ x ] = number of matrix starting from the top
		 * re[0:4][ 0 ] = number of solid rows per matrix
		 * re[0:4][1-n] = solid row numbers
		 */  
		uc re[4][9];
		uc elimination_sum = 0;
		for (uc curr_matrix = 0; curr_matrix < 4; curr_matrix++)
		{
			Solid_Row_Detector(curr_matrix, re[curr_matrix]);
					
			for (uc row = re[curr_matrix][INFO_INDEX]; row >= 1; --row)
			{
				Row_Collapser(curr_matrix, re[curr_matrix][row]);
				
				while(!TimerFlag); TimerFlag = 0;
				DrawBlockMatrix();
			}
		}
		Collapsed_Row_Merger(re);
//		DrawBlockMatrix();
		
}

int main(void)
{
	DDRA = 0x03; PORTA = 0xFC;
	DDRB = 0xFF; PORTB = 0x00;

	TimerSet(150);
	TimerOn();

	Init_LED_Matrices(4);
	Set_Brightness(0, 4);
	Clear_All();

	uc cur_matrix = 0;
	sc from_bottom = 7;
	uc is_done = 0;
	
	uc cur_matrix2 = 0;
	sc from_bottom2 = 7;
	uc is_done2 = 0;
	
	uc right = 4, left = 5;
	uc right2 = 4, left2 = 5;
	
	enum Block_Orientation orientation = Vertical_Up;
	enum Block_Orientation orientation2 = Vertical_Up;
	
	uc (*fct) (uc, uc, uc, sc, uc, uc);
	
	fct = &Draw_L_Block;

	MakeRandom();
	DrawBlockMatrix();

	Solid_Row_Eliminator();

	while (1) 
    {
	
//		Show_Orientations("l block");	
//		Drop_A_Block(fct, &cur_matrix, &is_done, &left, &right, &orientation, &from_bottom);
	/*
		while(!is_done)
		{
			Clear_All();
			DrawBlockMatrix();
			Drop_A_Block(fct, &cur_matrix, &is_done, &left, &right, &orientation, &from_bottom);
			
			while(!TimerFlag);
			TimerFlag = 0;
		}
		is_done = 0;
		cur_matrix = 0;
		left = left2;
		right = right2;
		orientation = orientation2;
		from_bottom = from_bottom2;
*/
	
//		TestForSolids();
    }
}