#ifndef __BLOCKS_H__
#define __BLOCKS_H__

typedef unsigned char  uc;
typedef   signed char  sc;
typedef unsigned short us;

// GLOBALS
uc POS_ARRAY[4][8];

uc Check_For_Contact(uc matrix, uc pos, sc poc, uc block)
{
	// Neg. poc means part of the block is on the lower matrix
	// Matrix + 1 > 3 means that we're on the lowest matrix
	if (poc <= 0 && matrix + 1 <= 3)
	{
		if (POS_ARRAY[matrix + 1][pos] & (0x01 << (7 + poc)))
		{
//			Set_POS_ARRAY(matrix + 1, pos, block);
		}
		return 0;
	}

	uc par_pos = POS_ARRAY[matrix][pos] & 0x01;
	uc blk_pos = block & 0x02;

	blk_pos = blk_pos >> 1;

	PORTB &= 0x07;
	PORTB  = PORTB | (par_pos << 7) | (blk_pos << 6);
	
	if (par_pos == blk_pos && par_pos != 0)
	{
//		PORTB &= 0x07;
//		PORTB |= 0xF0 << 4;
//		Set_POS_ARRAY(matrix, pos, block);
		return 1;
	}
	return 0;
}

void Set_POS_ARRAY(uc matrix, uc pos, uc block)
{
	POS_ARRAY[matrix][pos] |= block;
}

void Draw_O_Block(uc matrix, uc left, uc right, sc from_bottom, uc orientation)
{
	uc blk;
	if (matrix != BOTTOM_MATRIX && from_bottom == -1)
	{
		blk = 0x01; // Half of the block is on the lower matrix
	}
	else if (from_bottom == 7)
	{
		blk = 0x01 << from_bottom; // Half of the block is on the upper matrix
	}
	else
	{
		blk = 0x03 << from_bottom; // Regular block
	}

	for (char j = right; j <= left; j++)
	{
		for(char k = 0; k < 4; k++)
		{
			if (k == matrix)
			{
				MAX7219_SendByte(j);
				MAX7219_SendByte(blk);
			}
			else
			{
				MAX7219_SendByte(0);
				MAX7219_SendByte(0x00);
			}
		}
		Load_Word();
	}
}

enum Block_Orientation { Vertical_Up, Horizontal_Up, Vertical_Down, Horizontal_Down};

uc Assign_L_Block(uc orientation, uc h_pos)
{
	uc L = 0x00;
	switch(orientation)
	{
		case Vertical_Up:
			L = h_pos == 0 ? 0x01 : 0x07;
			break;
		case Horizontal_Up:
			L = h_pos == 0 ? 0x03 : 0x01;
			break;
		case Vertical_Down:
			L = h_pos == 0 ? 0x07 : 0x04;
			break;
		case Horizontal_Down:
			L = h_pos == 2 ? 0x03 : 0x02;
			break;
		default: break;
	}
	return L;
}

void Draw_L_Block(uc matrix, uc left, uc right, sc from_bottom, uc orientation)
{
//	for(char pos = right; pos <= left; pos++)
	for(char pos = 1; pos <= 8; pos++)
	{
		for(char curr_matrix = 0; curr_matrix < 4; curr_matrix++)
		{
			if (curr_matrix == matrix)
			{
				uc l_blk = Assign_L_Block(orientation, pos - right);

				l_blk = from_bottom < 0 ? l_blk >> abs(from_bottom) : l_blk << from_bottom;
			
				if (pos >= right && pos <= left)
				{
					if (Check_For_Contact(curr_matrix, pos - 1, from_bottom, l_blk) == 1)
					{
//						PORTB |= from_bottom < 0 ? 0xF0 : PORTB;
					}
					MAX7219_SendByte(pos);
					MAX7219_SendByte(POS_ARRAY[curr_matrix][pos-1] | l_blk);
				}
				else
				{
					MAX7219_SendByte(pos);
					MAX7219_SendByte(POS_ARRAY[curr_matrix][pos-1]);
				}
//				MAX7219_SendByte(pos);
//				MAX7219_SendByte(l_blk);

			}
			else
			{
				MAX7219_SendByte(pos);
				MAX7219_SendByte(POS_ARRAY[curr_matrix][pos-1]);
//				MAX7219_SendByte(0);
//				MAX7219_SendByte(0x00);
			}
		}
		Load_Word();
	}
}

uc Assign_I_Block(uc orientation)
{
	uc bar = 0x00;
	switch(orientation)
	{
		case Vertical_Up:
		case Vertical_Down:
			bar = 0x0F;
			break;
		case Horizontal_Down:
		case Horizontal_Up:
			bar = 0x01;
			break;
		default: break;
	}
	return bar;
}

void Draw_I_Block(uc matrix, uc left, uc right, sc from_bottom, uc orientation)
{
	for(char pos = right; pos <= left; pos++)
	{
		for(char k = 0; k < 4; k++)
		{
			if (k == matrix)
			{
				MAX7219_SendByte(pos);
				MAX7219_SendByte( Assign_I_Block(orientation) << from_bottom );
			}
			else
			{
				MAX7219_SendByte(0);
				MAX7219_SendByte(0x00);
			}
		}
		Load_Word();
	}
}
#endif