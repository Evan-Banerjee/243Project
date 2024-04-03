#include <stdlib.h>

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

typedef struct{
	int xCoordinate;
	int yCoordinate;
} twoDPoint;

typedef struct{
	twoDPoint p1;
	twoDPoint p2;
} line;

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)
	

    /* set front pixel buffer to Buffer 1 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer1; // first store the address in the  back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer

    /* set back pixel buffer to Buffer 2 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer
	
	line lines[1000];
	int lineIndex = 0;
	add_line(lines, &lineIndex, 10, 10, 100, 100);

    while (1)
    {
        /* Erase any boxes and lines that were drawn in the last iteration */
        drawLineFromLineObject(lines[0]);

        // code for drawing the boxes and lines (not shown)
        // code for updating the locations of boxes (not shown)

        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
		
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}

// code for subroutines (not shown)

void add_line(line lines[1000], int* lineIndex, int x1, int y1, int x2, int y2){
	twoDPoint testStart;
	twoDPoint testEnd;
	
	testStart.xCoordinate = x1;
	testStart.yCoordinate = y1;
	
	testEnd.xCoordinate = x2;
	testEnd.yCoordinate = x2;
	
	line testLine;
	
	testLine.p1 = testStart;
	testLine.p2 = testEnd;
	
	lines[*lineIndex] = testLine;
	(*lineIndex)++;
}

void drawLineFromLineObject(line inputLine){
	draw_line(inputLine.p1.xCoordinate, inputLine.p1.yCoordinate, 
				 inputLine.p2.xCoordinate, inputLine.p2.yCoordinate, 0x1f);
}

void plot_pixel(int x, int y, short int line_color)
{
    volatile short int *one_pixel_address;
        
        one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);
        
        *one_pixel_address = line_color;
}

void draw_line(int startX, int startY, int endX, int endY, int colour){
	int is_steep =  abs(endY - startY) > abs(endX - startX);
	if(is_steep){
		swap(&startX, &startY);
		swap(&endX, &endY);
	}
	if(startX > endX){
		swap(&startX, &endX);
		swap(&startY, &endY);
	}
	
	int deltaX = endX - startX;
	int deltaY = abs(endY - startY);
	int error = -deltaX / 2;
	
	int y = startY;
	
	int yStep = -1;
	if(startY < endY) yStep = 1;
	
	for(int x = startX; x <= endX; x++){
		if(is_steep){
			plot_pixel(y, x, colour);
		}else{
			plot_pixel(x, y, colour);
		}
		
		error = error + deltaY;
		if(error > 0){
			y = y + yStep;
			error = error - deltaX;
		}
	}
	return;
}

void swap(int* a, int* b){
	int temp = *b;
	*b = *a;
	*a = temp;
	return;
}

void wait_for_vsync(){
	volatile int * pixel_ctrl_ptr = (int *) 0xff203020; // base address
	int status;
	*pixel_ctrl_ptr = 1; // start the synchronization process
	// write 1 into front buffer address register
	
	status = *(pixel_ctrl_ptr + 3); // read the status register
	
	while ((status & 0x01) != 0){ // polling loop waiting for S bit to go to 0
		status = *(pixel_ctrl_ptr + 3);
	} // loop/function exits when status bit goes to 0
}


void clear_screen(){
	for(int i = 0; i < 319; i++){
		for(int j = 0; j < 239; j++){
			plot_pixel(i, j, 0);
		}
	}
	return;
}

void draw_lines_and_boxes(int boxesX[8], int boxesY[8], int colour, int colour2){
	
	draw_line(boxesX[7], boxesY[7], boxesX[0], boxesY[0], colour);
	
	for(int i = 0; i < 7; i++){
		draw_line(boxesX[i], boxesY[i], boxesX[i+1], boxesY[i+1], colour);
	}
		
}



