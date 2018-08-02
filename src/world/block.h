#ifndef block_h
#define block_h

class Block
{
	typedef unsigned char **Grid;

  public:
	Block(int width, int height);

  protected:
	int block_num;								   // the number of this block
	int x, y;									   // x and y position of the block (in blocks not pixels)
	int width, height;							   // height and width of the block (without borders)
	int width_byte, height_byte;				   // height and width of the block in compressed in bytes
	int max_width, max_height;					   // the maximum width and height between all blocks
	int max_width_byte, max_height_byte;		   // the maximum width and height between all blocks compressed into bytes
	int starting_x, starting_y;					   // upper left corner x and y position (pixels)
	bool first_row, first_col, last_row, last_col; // booleans indicating specific block positions

  protected:
	char *write_grid; // Holds the data needed for one write

  public:
	// Delegate methods
	void write(char *grid, int bytes_per_row);
	void load_for_write(MPI_Request *request);
	void send_block();
	void recv_block();

  public:
	void print();
};

#endif