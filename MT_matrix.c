#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include<fcntl.h>
#include <errno.h>
#include <sys/ioctl.h> // for open
#include <unistd.h> // for close

//////////////Global Variables//////////////////////////

int matrix1[10000][10000];
int matrix2[10000][10000];
int matrixOut[10000][10000];
int t_num=0;
int row1,row2,col1,col2;
struct rowCol{
    int row;
    int col;
};
char dims[1000];
char fileName[1000],outputFile[1000];

/////////////////Read Files//////////////////////////////////////////////////


void matrixElements(FILE *file, int matrix[10000][10000], int row, int col ) {//Read each element in the matrix
    for (int i = 0; i < row; i++){
        for (int j = 0; j < col; j++){
            fscanf(file, "%d", &matrix[i][j]);
        }
    }
}


void readFile( char* fName, int flag ){
    int *row, *col, *matrix;
    if( flag==0 ){//First matrix.
        row = &row1;
        col = &col1;
        matrix = matrix1;
    }else{//Second Matrix
        row = &row2;
        col = &col2;
        matrix = matrix2;
    }

    FILE *f = fopen( fName, "r" );//Read the file
    if (f==NULL){
        printf("File doesn't exist!!\n");
        exit(-1);
    }

    fscanf(f, "%[^\n]",dims);
    if(  sscanf(dims, "%d %d", row,col) != 2 ){//Check if the first line that contain the dims exists.
        exit(-1);
    }

    matrixElements(f,matrix,*row,*col);//Read the elements of this matrix.

    fclose(f);

}


//////////////////Save Output Matrices//////////////////////////////////////


void save2(){

    //Saving the output matrix using 1 thread per row in the required file.
    strcpy(outputFile, "");
    // strcat(outputFile, fileName);
    strcat(outputFile, "result.txt");

    FILE *fil =fopen(outputFile, "w");

    // fprintf(fil,"Method: A thread per row\n");
    fprintf(fil,"%d %d\n",row1,col2);
    for(int i=0;i<row1;i++){
        for(int j=0;j<col2;j++){
            fprintf(fil,"%d ",matrixOut[i][j]);
        }
        fprintf(fil,"\n");
    }
}



/////////////////Matrix Multiplication Different Cases/////////////////////
#define IOCTL_MISCDEV_SET 0x00
#define IOCTL_MISCDEV_GET 0x04
struct miscdev_data {
    int val;
    char data[64];
}; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//Second Method: 1 Thread per row.
void* matrixMultCase2(void* rowInd){
    pid_t id = gettid(); // Get pthread ID
    int k=(int)rowInd;
    int ret,fd;
    char buf[128]={0};
    struct miscdev_data data;
    while (k<row1)
    {   //printf("thread: %d , k: %d\n",(int)rowInd,k);
        for(int i=0;i<col2;i++){
            matrixOut[k][i]=0;
            for(int j=0;j<row2;j++){
                matrixOut[k][i] += matrix1[k][j]*matrix2[j][i];
            }
        } 
        k+=t_num;
    }
    fd = open("/proc/thread_info", O_RDWR);
    if(fd < 0)
        perror("open");

    // printf("%d\n",id);
    
    
    sprintf(buf, "%d",id);
    pthread_mutex_lock(&mutex);
    ret = write(fd, buf, 128);
    if(ret < 0) {
        perror("write");
    } 

    memset(&data, 0, sizeof(data));
    // printf("before: %d\n",id);
    
    ret = ioctl(fd, IOCTL_MISCDEV_SET, &id);
    if(ret < 0) {
        perror("ioctl set");
    }
    char test[]="";
    ret = ioctl(fd, IOCTL_MISCDEV_GET, &id);
    if(ret < 0) {
        perror("ioctl get");
    }
    // printf("after: %d\n",id);
    // printf("Get data: miscdata_data.val = %d, miscdata_data.data = %s\n", data.val, data.data);
    ret = read(fd, buf, 300);
    if(ret < 0) {
        perror("read");
    }    
    char exetime[50],context[50];
    int s = 0;
    int j = 0;
    for(int i = 0; i < 300 && buf[i]!=NULL; i++ ){
        if(s==0){
            if(buf[i] ==' '){
                s=1;
                j=0;
                continue;
            }
            exetime[j] = buf[i];
            j++;
        }
        else{
            context[j] = buf[i];
            j++;           
        }
        
    }
    printf("\tThreadID:%d time:%s(ms) context switch times:%s\n",id,exetime,context);
    if(fd > 0)
        close(fd);   
    pthread_mutex_unlock(&mutex);
    //close(fd);
    pthread_exit(NULL);//Terminate calling thread
}


//Creating threads for the matrix.

void mat2(){

    int tid;
    pthread_t thread[t_num];

    for(int i=0;i<t_num;i++) {
        tid = pthread_create(&thread[i], NULL, matrixMultCase2, (void *) i);
        if (tid != 0) {//Error Happened.
            printf("ERROR!!\n");
            exit(-1);
        }
    }

    for(int i=0;i<t_num;i++) {
        pthread_join(thread[i], NULL);//Waiting for each thread to terminate.
    }
}



////////////////Printing Matrix in the console//////////////////////


void printMatrix(int matrixOutput[10000][10000],int rowOut,int colOut){//Printing the Output Matrix.

    printf("row=%d col=%d\n",rowOut,colOut);
    for(int i=0;i<rowOut;i++){
        for(int j=0;j<colOut;j++){
            printf("%d ",matrixOutput[i][j]);
        }
        printf("\n");
    }
}


void runtime2(){//Run the second method: 1 thread per row.

    struct timeval stop, start;

    gettimeofday(&start, NULL); //start checking time
    mat2();
    gettimeofday(&stop, NULL); //end checking time

    save2();//Saving the output matrix.

    // printMatrix(matrixOut,row1,col2);//Printing the output matrix in the console.
    // printf("tnum: %d\n",t_num);
    // printf("Number of Threads Created: %d Threads\n",row1);
    // printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    // printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
}




/////////////////main Function////////////////////


int main(int argc, char *argv[]) {
        pid_t id = getpid();
        printf("PID:%d\n",id);   
    if(argc==4){//entered specified files names for matrices 1, 2 and the output matrix.
        
        strcpy(fileName, "");
        strcat(fileName, "c");
        t_num = atoi(argv[1]);

        strcpy(fileName, "");
        strcat(fileName, argv[2]);
        readFile(fileName, 0);

        strcpy(fileName, "");
        strcat(fileName, argv[3]);
        readFile(fileName, 1);



    }else{

        printf("Invalid Args!!\n");
        exit(-1);
    }


    if(col1!=row2){//The 2 matrices entered can't be multiplied because they have invalid dimensions.
        printf("Invalid Dimensions!!\n");
        exit(-1);
    }


    // printf("\n--------------->SECOND METHOD: 1 THREAD PER ROW<---------------\n");
    runtime2();
    // printf("\n//////////////////////////////////////////////////////\n");


    return 0;
}