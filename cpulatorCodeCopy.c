#include <stdlib.h>
#include <math.h>

volatile int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

typedef struct{
	double xCoordinate;
	double yCoordinate;
} twoDPoint;

typedef struct {
    double xCoordinate;
    double yCoordinate;
    double zCoordinate;
} threeDPoint;

typedef struct{
	twoDPoint p1;
	twoDPoint p2;
	twoDPoint p3;
	int c;
} twoDTriangle;

typedef struct{
	threeDPoint p1;
	threeDPoint p2;
	threeDPoint p3;
	int c;
} threeDTriangle;

typedef struct{
	double m[4][4];
} fourByFourMatrix;

twoDTriangle proj_ThreeToTwoTriangle(fourByFourMatrix projMatrix, threeDTriangle projTriangle);
twoDPoint proj_ThreeToTwoPoint(fourByFourMatrix projMatrix, threeDPoint projPoint);
threeDTriangle rotate_triangle(threeDTriangle inputTri, fourByFourMatrix matRot);
void draw_all_triangles(twoDTriangle triangles[1000]);
void add_triangle(twoDTriangle triangles[1000], int* triangleIndex, double x1, double y1, 
				  double x2, double y2, double x3, double y3, int colour);
void add_3dTriangle(threeDTriangle threeD_Triangles[1000], int* threeDTriangleIndex,
 double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double Colour);
void draw_triangle_from_triangle_object(twoDTriangle inputTriangle);
void plot_pixel(int x, int y, short int line_color);
void draw_line(int startX, int startY, int endX, int endY, int colour);
void swap(int* a, int* b);
void wait_for_vsync();
void clear_screen();
void copy_triangles(twoDTriangle trianglesBase[1000], twoDTriangle trianglesCopy[1000]);
void multiply_matrix(threeDPoint i, threeDPoint *o, fourByFourMatrix m);
void update_x_rotate_matrix(fourByFourMatrix *matRotX, double fTheta);
void update_z_rotate_matrix(fourByFourMatrix *matRotZ, double fTheta);
threeDTriangle rotate_triangle(threeDTriangle inputTri, fourByFourMatrix matRot);

double screenWidth = 320;
double screenHeight = 240;

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    volatile int * PS2_ptr = (int *)0xFF200100;
	volatile int * LED_ptr = (int *)0xFF200000;
	volatile int * SLIDER = (int*)0xFF200040;
	volatile int * BUTTONS = (int*)0xFF200050;
	
	int PS2_data, RVALID;
	char byte1 = 0, byte2 = 0, byte3 = 0;
	
	*(PS2_ptr) = 0xFF;
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
	
	twoDTriangle triangles[1000];
	twoDTriangle oldTriangles[1000];
	threeDTriangle threeD_Triangles[1000];
	threeDTriangle rotatedTriangles[1000];
	for(int i = 0; i < 1000; i++){
		twoDPoint p1;
		p1.xCoordinate = -1;
		p1.yCoordinate = -1;
		
		twoDPoint p2;
		p2.xCoordinate = -1;
		p2.yCoordinate = -1;
		
		twoDPoint p3;
		p3.xCoordinate = -1;
		p3.yCoordinate = -1;
		
		twoDTriangle blankTriangle;
		blankTriangle.p1 = p1;
		blankTriangle.p2 = p2;
		blankTriangle.p3 = p3;
		blankTriangle.c = 0;
		
		triangles[i] = blankTriangle; //this makes it so that, while iterating through the lines array, if you reach a triangle 
							  //with -1 as a coordinate point, then you've reached the end of the array
	}
	for (int i = 0; i<1000; i++) {
		threeDPoint p1;
		p1.xCoordinate = -1;
		p1.yCoordinate = -1;
		p1.zCoordinate = -1;

		threeDPoint p2;
		p2.xCoordinate = -1;
		p2.yCoordinate = -1;
		p2.zCoordinate = -1;

		threeDPoint p3;
		p3.xCoordinate = -1;
		p3.yCoordinate = -1;
		p3.zCoordinate = -1;

		threeDTriangle blank_3D;
		blank_3D.p1 = p1;
		blank_3D.p2 = p2;
		blank_3D.p3 = p3;
		blank_3D.c = 0;

		threeD_Triangles[i] = blank_3D;
	}
		
	int triangleIndex = 0;
	int threeDTriangleIndex = 0;
	
	copy_triangles(triangles, oldTriangles);

	double fNear = 0.1;
	double fFar = 1000.0;
	double fAspectRatio = 240.0/320.0;
	double fFov = 90.0;
	double fFovRad = 1.0f / tan((fFov * 0.5) * (3.14159/180));

	//printf("fFovRad is %f\n", fFovRad);
	//printf("FFFFFFFFFFFFFFFFFFFFFFFFFFFF\n");


	fourByFourMatrix matProj;

	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++) matProj.m[i][j] = 0;
	}

	matProj.m[0][0] = fAspectRatio * fFovRad;
	matProj.m[1][1] = fFovRad;
	matProj.m[2][2] = fFar / (fFar - fNear);
	matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matProj.m[2][3] = 1.0;
	matProj.m[3][3] = 0.0;

	// //printf("at the beginning of the program:");
	// for(int k = 0; k < 4; k++){
	// 	for(int j = 0; j < 4; j++){
	// 		//printf("m.m[%d][%d] is %f\n", k, j, matProj.m[k][j]);
	// 	}
	// }
	//end projection matrix code

	//start rotate matrix code
	// Rotation Z
	fourByFourMatrix matRotX;
	fourByFourMatrix matRotZ;

	update_x_rotate_matrix(&matRotX, 0);
	update_z_rotate_matrix(&matRotZ, 0);

	double fTheta = 0.0;

	
	double cubeArray[12][3][3] = {
	//SOUTH
    {{0.0, 0.0, 0.0},    {0.0, 1.0, 0.0},    {1.0, 1.0, 0.0}},
    {{0.0, 0.0, 0.0},    {1.0, 1.0, 0.0},    {1.0, 0.0, 0.0}},

    // EAST
    {{1.0, 0.0, 0.0},    {1.0, 1.0, 0.0},    {1.0, 1.0, 1.0}},
    {{1.0, 0.0, 0.0},    {1.0, 1.0, 1.0},    {1.0, 0.0, 1.0}},

    // NORTH
    {{1.0, 0.0, 1.0},    {1.0, 1.0, 1.0},    {0.0, 1.0, 1.0}},
    {{1.0, 0.0, 1.0},    {0.0, 1.0, 1.0},    {0.0, 0.0, 1.0}},

    // WEST
    {{0.0, 0.0, 1.0},    {0.0, 1.0, 1.0},    {0.0, 1.0, 0.0}},
    {{0.0, 0.0, 1.0},    {0.0, 1.0, 0.0},    {0.0, 0.0, 0.0}},

    // TOP
    {{0.0, 1.0, 0.0},    {0.0, 1.0, 1.0},    {1.0, 1.0, 1.0}},
    {{0.0, 1.0, 0.0},    {1.0, 1.0, 1.0},    {1.0, 1.0, 0.0}},

    // BOTTOM
    {{1.0, 0.0, 1.0},    {0.0, 0.0, 1.0},    {0.0, 0.0, 0.0}},
    {{1.0, 0.0, 1.0},    {0.0, 0.0, 0.0},    {1.0, 0.0, 0.0}}
};



	for(int i = 0; i < 12; i++){
		add_3dTriangle(threeD_Triangles, &threeDTriangleIndex,
		cubeArray[i][0][0], cubeArray[i][0][1], cubeArray[i][0][2], 
		cubeArray[i][1][0], cubeArray[i][1][1], cubeArray[i][2][2], 
		cubeArray[i][2][0], cubeArray[i][2][1], cubeArray[i][3][2], 
		0x1F);

		// threeD_Triangles[i].p1.zCoordinate += 2;
		// threeD_Triangles[i].p2.zCoordinate += 2;
		// threeD_Triangles[i].p3.zCoordinate += 2;
	}

	// add_3dTriangle(threeD_Triangles, &threeDTriangleIndex, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0x1F);
	// twoDTriangle testTri = proj_ThreeToTwoTriangle(matProj, threeD_Triangles[0]);

	// double point1X = testTri.p2.xCoordinate;
	// double point1Y = testTri.p2.yCoordinate;
	
	//*(LED_ptr) = point1X && 0x3FF;

	//add_triangle(triangles, &triangleIndex, 100.0, 0.0, 100.0, 100.0, 200.0, 50.0, 0x1F);

	// add_3dTriangle(threeD_Triangles, &threeDTriangleIndex, 
	// 			0.0, 0.0, 0.0,    0.0, 1.0, 0.0,    1.0, 1.0, 0.0, 0x1F);

	// threeD_Triangles[0].p1.zCoordinate += 3.0;
	// threeD_Triangles[0].p2.zCoordinate += 3.0;
	// threeD_Triangles[0].p3.zCoordinate += 3.0;


	//printThreeDTriangle(threeD_Triangles[0]);

	//printf("\ntesting out projecting the triangle\n");
	twoDTriangle testProj = proj_ThreeToTwoTriangle(matProj, threeD_Triangles[0]);
	printTwoDTriangle(testProj);

	// //printf("\napplying matrix transform to point 1\n");
	// //printf("before projecting point:\n");
	// for(int k = 0; k < 4; k++){
	// 	for(int j = 0; j < 4; j++){
	// 		//printf("m.m[%d][%d] is %f\n", k, j, matProj.m[k][j]);
	// 	}
	// }

	// twoDPoint test2DPoint = proj_ThreeToTwoPoint(matProj, threeD_Triangles[0].p1);
	// //printf("xCoordinate for p1 is %f,\nyCoordinate for p1 is %f\n\n", test2DPoint.xCoordinate, test2DPoint.yCoordinate);

	// //printf("\napplying matrix transform to point 2\n");
	// test2DPoint = proj_ThreeToTwoPoint(matProj, threeD_Triangles[0].p2);
	// //printf("xCoordinate for p2 is %f,\nyCoordinate for p2 is %f\n\n", test2DPoint.xCoordinate, test2DPoint.yCoordinate);

	// //printf("\napplying matrix transform to point 3\n");
	// test2DPoint = proj_ThreeToTwoPoint(matProj, threeD_Triangles[0].p3);
	// //printf("xCoordinate for p3 is %f,\nyCoordinate for p3 is %f\n\n", test2DPoint.xCoordinate, test2DPoint.yCoordinate);

	// twoDTriangle projTri = proj_ThreeToTwoTriangle(matProj, threeD_Triangles[0]);
	// printTwoDTriangle(projTri);

	for(int i = 0; i < 12; i++){
		//printf("doing a rotation. value of theta is %f\n", fTheta);
		rotatedTriangles[i] = rotate_triangle(threeD_Triangles[i], matRotX);
	}

	double fThetaZ = 0.0;

    while (1)
    {
        /* Erase any boxes and lines that were drawn in the last iteration */
		printf("restarting loop\n");
		printf("triangle 0 coordinates:");
		printTwoDTriangle(triangles[0]);
        draw_all_triangles(triangles);
		//draw_triangle_from_triangle_object(testProj);

        // code for drawing the boxes and lines (not shown)
        // code for updating the locations of boxes (not shown)

        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
		
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
		
		draw_all_triangles(oldTriangles); //erase old image
		
		copy_triangles(triangles, oldTriangles);
		
		int iterator = 0;
		// while(triangles[iterator].p1.xCoordinate != -1){
		// 	triangles[iterator].p1.xCoordinate++;
		// 	triangles[iterator].p2.xCoordinate++;
		// 	triangles[iterator].p3.xCoordinate++;
			
		// 	if(triangles[iterator].p1.xCoordinate > 319) triangles[iterator].p1.xCoordinate = 0;
		// 	if(triangles[iterator].p2.xCoordinate > 319) triangles[iterator].p2.xCoordinate = 0;
		// 	if(triangles[iterator].p3.xCoordinate > 319) triangles[iterator].p3.xCoordinate = 0;
		// 	iterator++; //test
		// }

		PS2_data = *(PS2_ptr);
		RVALID = PS2_data & 0x8000;
		if (RVALID) {
			byte1 = byte2;
			byte2 = byte3;
			byte3 = PS2_data & 0xFF;
			
			if ((byte2 == (char)0xAA) && (byte3 == (char)0x00))
				*(PS2_ptr) = 0xF4;
			
			if((byte2 != (char)0xF0) && byte3 == (char)0x1D){
				printf("w key is pressed\n");
				fTheta += 0.1;
				update_x_rotate_matrix(&matRotX, fTheta);
			}

			if((byte2 != (char)0xF0) && byte3 == (char)0x1B){
				printf("s key is pressed\n");
				fThetaZ += 0.1;
				update_z_rotate_matrix(&matRotZ, fThetaZ);
			}
		}
		if((*(SLIDER) & 0Xf) != 0){
			printf("switch 1 is pressed\n");
			fTheta += 0.1;
			update_x_rotate_matrix(&matRotX, fTheta);
		}
		if((*(BUTTONS) & 0Xf) != 0){
			printf("switch 2 is pressed\n");
			fThetaZ += 0.1;
			update_z_rotate_matrix(&matRotZ, fThetaZ);
		}

		for(int i = 0; i < 12; i++){
			//printf("doing a rotation. value of theta is %f\n", fTheta);
			rotatedTriangles[i] = rotate_triangle(threeD_Triangles[i], matRotX);
			rotatedTriangles[i] = rotate_triangle(rotatedTriangles[i], matRotZ);
		}

		// for(int i = 0; i < 12; i++){
		// 	//printf("doing a rotation. value of theta is %f\n", fTheta);
		// 	rotatedTriangles[i] = rotate_triangle(threeD_Triangles[i], matRotX);
		// }
		// fTheta += 0.1;
		// update_x_rotate_matrix(&matRotX, fTheta);

		for(int i = 0; i < 12; i++){
			triangles[i] = proj_ThreeToTwoTriangle(matProj, rotatedTriangles[i]);
		}
    }
}

// code for subroutines (not shown)

void draw_all_triangles(twoDTriangle triangles[1000]){
	int iterator = 0;
	while(triangles[iterator].p1.xCoordinate != -1){
		draw_triangle_from_triangle_object(triangles[iterator]);
		iterator++;
	}
}

void add_triangle(twoDTriangle triangles[1000], int* triangleIndex, double x1, double y1, 
				  double x2, double y2, double x3, double y3, int colour){
	twoDPoint triStart;
	twoDPoint triMiddle;
	twoDPoint triEnd;


	triStart.xCoordinate = x1;
	triStart.yCoordinate = y1;
	
	triMiddle.xCoordinate = x2;
	triMiddle.yCoordinate = y2;
	
	triEnd.xCoordinate = x3;
	triEnd.yCoordinate = y3;

	
	twoDTriangle tempTriangle;
	
	tempTriangle.p1 = triStart;
	tempTriangle.p2 = triMiddle;
	tempTriangle.p3 = triEnd;

	
	tempTriangle.c = colour;
	
	triangles[*triangleIndex] = tempTriangle;
	(*triangleIndex)++;
}

void add_3dTriangle(threeDTriangle threeD_Triangles[1000], int* threeDTriangleIndex,
 double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double Colour){
	threeDPoint triStart;
	threeDPoint triMid;
	threeDPoint triEnd;

	triStart.xCoordinate = x1;
	triStart.yCoordinate = y1;
	triStart.zCoordinate = z1;

	triMid.xCoordinate = x2;
	triMid.yCoordinate = y2;
	triMid.zCoordinate = z2;

	triEnd.xCoordinate = x3;
	triEnd.yCoordinate = y3;
	triEnd.zCoordinate = z3;

	threeDTriangle tempTriangle;

	tempTriangle.p1 = triStart;
	tempTriangle.p2 = triMid;
	tempTriangle.p3 = triEnd;

	tempTriangle.c = Colour;

	threeD_Triangles[*threeDTriangleIndex] = tempTriangle;
	(*threeDTriangleIndex)++;
}

void draw_triangle_from_triangle_object(twoDTriangle inputTriangle){
	draw_line((int)inputTriangle.p1.xCoordinate, (int)inputTriangle.p1.yCoordinate, 
				(int)inputTriangle.p2.xCoordinate, (int)inputTriangle.p2.yCoordinate, 
			  	(int)inputTriangle.c);

	draw_line((int)inputTriangle.p2.xCoordinate, (int)inputTriangle.p2.yCoordinate, 
				(int)inputTriangle.p3.xCoordinate, (int)inputTriangle.p3.yCoordinate, 
			  	inputTriangle.c);
	draw_line((int)inputTriangle.p1.xCoordinate, (int)inputTriangle.p1.yCoordinate, 
				(int)inputTriangle.p3.xCoordinate, (int)inputTriangle.p3.yCoordinate, 
			  	inputTriangle.c);
}

void plot_pixel(int x, int y, short int line_color)
{
    volatile short int *one_pixel_address;
        
        one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);
        
        if(x < 320 && x >= 0 && y < 240 && y >= 0) *one_pixel_address = line_color;
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


void copy_triangles(twoDTriangle trianglesBase[1000], twoDTriangle trianglesCopy[1000]){
	for(int i = 0; i < 1000; i++){
		
		if(trianglesCopy[i].p1.xCoordinate == -10000 && trianglesBase[i].p1.xCoordinate == -10000) return;
		
		trianglesCopy[i].p1.xCoordinate = trianglesBase[i].p1.xCoordinate;
		trianglesCopy[i].p2.xCoordinate = trianglesBase[i].p2.xCoordinate;
		trianglesCopy[i].p3.xCoordinate = trianglesBase[i].p3.xCoordinate;
		
		trianglesCopy[i].p1.yCoordinate = trianglesBase[i].p1.yCoordinate;
		trianglesCopy[i].p2.yCoordinate = trianglesBase[i].p2.yCoordinate;
		trianglesCopy[i].p3.yCoordinate = trianglesBase[i].p3.yCoordinate;
		
		trianglesCopy[i].c = 0;
	}
}

void multiply_matrix(threeDPoint i, threeDPoint *o, fourByFourMatrix m){

	// //printf("printing out multiplication matrix in projection function\n");
	// for(int k = 0; k < 4; k++){
	// 	for(int j = 0; j < 4; j++){
	// 		//printf("m.m[%d][%d] is %f\n", k, j, m.m[k][j]);
	// 	}
	// }

	o->xCoordinate = i.xCoordinate * m.m[0][0] + i.yCoordinate * m.m[1][0] + i.zCoordinate * m.m[2][0] + m.m[3][0];
	o->yCoordinate = i.xCoordinate * m.m[0][1] + i.yCoordinate * m.m[1][1] + i.zCoordinate * m.m[2][1] + m.m[3][1];
	o->zCoordinate = i.xCoordinate * m.m[0][2] + i.yCoordinate * m.m[1][2] + i.zCoordinate * m.m[2][2] + m.m[3][2];
	double w = i.xCoordinate * m.m[0][3] + i.yCoordinate * m.m[1][3] + i.zCoordinate * m.m[2][3] + m.m[3][3];

	if (w != 0.0)
	{
		o->xCoordinate /= w; o->yCoordinate /= w; o->zCoordinate /= w;
		//printf("w ended up being nonzero. it's %f\n", w);
	}
	//printf("o.xCoordinate is %f while o.yCoordinate is %f\n", o->xCoordinate, o->yCoordinate);
}

void update_x_rotate_matrix(fourByFourMatrix *matRotX, double fTheta){
	matRotX->m[0][0] = 1;
	matRotX->m[1][1] = cos(fTheta * 0.5);
	matRotX->m[1][2] = sin(fTheta * 0.5);
	matRotX->m[2][1] = -sin(fTheta * 0.5);
	matRotX->m[2][2] = cos(fTheta * 0.5);
	matRotX->m[3][3] = 1;
}

void update_z_rotate_matrix(fourByFourMatrix *matRotZ, double fTheta){
	matRotZ->m[0][0] = cosf(fTheta);
	matRotZ->m[0][1] = sinf(fTheta);
	matRotZ->m[1][0] = -sinf(fTheta);
	matRotZ->m[1][1] = cosf(fTheta);
	matRotZ->m[2][2] = 1;
	matRotZ->m[3][3] = 1;
}

twoDPoint proj_ThreeToTwoPoint(fourByFourMatrix projMatrix, threeDPoint projPoint) {
	//printf("beginning the process of projecting a 3d point to a 2d point\n");
	//printf("3d point coordinates are (%f, %f, %f)\n", projPoint.xCoordinate, projPoint.yCoordinate, projPoint.zCoordinate);

	threeDPoint fourDProjectedPoint;
	twoDPoint twoDProjectedPoint;
	multiply_matrix(projPoint, &fourDProjectedPoint, projMatrix);
	twoDProjectedPoint.xCoordinate = fourDProjectedPoint.xCoordinate;
	twoDProjectedPoint.yCoordinate = fourDProjectedPoint.yCoordinate;

	//printf("projected coordinates are (%f, %f)\n", twoDProjectedPoint.xCoordinate, twoDProjectedPoint.yCoordinate);

	return twoDProjectedPoint;
}

twoDTriangle proj_ThreeToTwoTriangle(fourByFourMatrix projMatrix, threeDTriangle projTriangle) {
	twoDTriangle twoDProjectedTriangle;

	twoDProjectedTriangle.c = 0x7E1;

	projTriangle.p1.zCoordinate += 2;
	projTriangle.p2.zCoordinate += 2;
	projTriangle.p3.zCoordinate += 2;

	twoDProjectedTriangle.p1 = proj_ThreeToTwoPoint(projMatrix, projTriangle.p1);
	twoDProjectedTriangle.p2 = proj_ThreeToTwoPoint(projMatrix, projTriangle.p2);
	twoDProjectedTriangle.p3 = proj_ThreeToTwoPoint(projMatrix, projTriangle.p3);
	twoDProjectedTriangle.c = 0x3E0;


	twoDProjectedTriangle.p1.xCoordinate += 1;
	twoDProjectedTriangle.p1.yCoordinate += 1;

	twoDProjectedTriangle.p2.xCoordinate += 1;
	twoDProjectedTriangle.p2.yCoordinate += 1;

	twoDProjectedTriangle.p3.xCoordinate += 1;
	twoDProjectedTriangle.p3.yCoordinate += 1;


	twoDProjectedTriangle.p1.xCoordinate *= 0.5*screenWidth;
	twoDProjectedTriangle.p1.yCoordinate *= 0.5*screenHeight;

	twoDProjectedTriangle.p2.xCoordinate *= 0.5*screenWidth;
	twoDProjectedTriangle.p2.yCoordinate *= 0.5*screenHeight;

	twoDProjectedTriangle.p3.xCoordinate *= 0.5*screenWidth;
	twoDProjectedTriangle.p3.yCoordinate *= 0.5*screenHeight;

	return twoDProjectedTriangle;
}

void printTwoDTriangle(twoDTriangle tri){
	printf("first point is (%f, %f)\n", tri.p1.xCoordinate, tri.p1.yCoordinate);
	printf("second point is (%f, %f)\n", tri.p2.xCoordinate, tri.p2.yCoordinate);
	printf("third point is (%f, %f)\n", tri.p3.xCoordinate, tri.p3.yCoordinate);
}

void printThreeDTriangle(threeDTriangle tri){
	printf("first point is (%f, %f, %f)\n", tri.p1.xCoordinate, tri.p1.yCoordinate, tri.p1.zCoordinate);
	printf("second point is (%f, %f, %f)\n", tri.p2.xCoordinate, tri.p2.yCoordinate, tri.p1.zCoordinate);
	printf("third point is (%f, %f, %f)\n", tri.p3.xCoordinate, tri.p3.yCoordinate, tri.p1.zCoordinate);
}

threeDTriangle rotate_triangle(threeDTriangle inputTri, fourByFourMatrix matRot){
    threeDTriangle newTriangle;

    multiply_matrix(inputTri.p1, &newTriangle.p1, matRot);
    multiply_matrix(inputTri.p2, &newTriangle.p2, matRot);
    multiply_matrix(inputTri.p3, &newTriangle.p3, matRot);

    return newTriangle;
}
