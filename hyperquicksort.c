/*
   Hyperquick sort
   No of processor must be power of two
   Author :- Suraj Desmukh
*/
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<mpi.h>

void quicksort(float *array, int firstIndex, int lastIndex)
{
    int pivotIndex, temp, index1, index2;

    if(firstIndex < lastIndex)
    {
        pivotIndex = firstIndex;
        index1 = firstIndex;
        index2 = lastIndex;
        while(index1 < index2)
        {
            while(array[index1] <= array[pivotIndex] && index1 < lastIndex)
            {
                index1++;
            }
            while(array[index2]>array[pivotIndex])
            {
                index2--;
            }
            if(index1<index2)
            {
                temp = array[index1];
                array[index1] = array[index2];
                array[index2] = temp;
            }
        }
        temp = array[pivotIndex];
        array[pivotIndex] = array[index2];
        array[index2] = temp;
        quicksort(array, firstIndex, index2-1);
        quicksort(array, index2+1, lastIndex);
    }
}

void list(float *array,float median,int c,int *lc,int *hc,int rank)  // c= count of nos in array
{
   int i=0,count=0,l=0,h=0;
   for(i=0;i<c;i++)
   {
      if(array[i]<=median)
       {  
          l++; 
       }
       else break;  
   }
   h=c-l;
   *lc=l;
   *hc=h;
    //printf("\nrank=%d,low=%d,high=%d\n",rank,l,h);
}

int main(int argc,char *argv[])
{
   int rank,size,i=0,j=0,lw=0,rlw=0,hi=0,rhi=0;
   MPI_Init(&argc,&argv);
   MPI_Comm_size(MPI_COMM_WORLD,&size);
   MPI_Comm_rank(MPI_COMM_WORLD,&rank);
   int count=0,*sendcount,*displacement;
   char c;
   float *data,n,*rec_data,median,*low,*high;
   FILE *fp;
   sendcount = (int*)calloc(sizeof(float),size);
   displacement = (int*)calloc(sizeof(float),size);
   if(rank==0)
   {
    if((size & (~size+1))!=size) { printf("No of proc must be power of two.\nCode Terminated\n"); exit(0); }
    fp=fopen("vector.txt","r");
    while(fscanf(fp,"%f",&n)!=-1)
    { 
      c=fgetc(fp);
      count++;
    }
    fseek( fp, 0, SEEK_SET );
    data = (float*)calloc(sizeof(float),count);
    for(i=0;i<count;i++){
      fscanf(fp,"%f",&data[i]);
//      printf("\n%f\n",data[i]); 
    }
    i=0; 
    while(1)
    {
       for(j=0;j<size;j++)
       {
          sendcount[j] = sendcount[j] + 1; 
          i++; 
          if(i==count) break;
       } 
       if(i==count) break;
    }
    for(i=1;i<size;i++)
    {
      displacement[i] = displacement[i-1] + sendcount[i-1];
    } 
   }
   MPI_Bcast(sendcount,size,MPI_INT,0,MPI_COMM_WORLD); 
   rec_data = (float*)calloc(sizeof(float),sendcount[rank]);
   MPI_Scatterv(data,sendcount,displacement,MPI_FLOAT,rec_data,sendcount[rank],MPI_FLOAT,0,MPI_COMM_WORLD); 
   quicksort(rec_data,0,sendcount[rank]-1);
  /* for(i=0;i<sendcount[rank];i++)
      printf("\nrank=%d data[%d]=%f\n",rank,i,rec_data[i]);*/
   if(rank==0) 
   {
      i = (sendcount[0] - 1)/2;   // i = median index 
      median = rec_data[i]; 
      //printf("median=%f\n",median);
   }
   MPI_Bcast(&median,1,MPI_FLOAT,0,MPI_COMM_WORLD);
   list(rec_data,median,sendcount[rank],&lw,&hi,rank);
   j=lw;
   low  = (float*)calloc(sizeof(float),lw);
   high = (float*)calloc(sizeof(float),hi);
   for(i=0;i<lw;i++)
     low[i] = rec_data[i];
   for(i=0;i<hi;i++)
   {
     high[i] = rec_data[j];
     j++;  
   } 
   /*for(i=0;i<lw;i++)
    printf("rank=%d low[%d]=%f\n",rank,i,low[i]);
   for(i=0;i<hi;i++)
    printf("rank=%d high[%d]=%f\n",rank,i,high[i]);*/
   for(i=0;i<(size/2);i++)
   {
      if(rank==i)
      { 
        MPI_Send(&hi,1,MPI_INT,rank + (size/2),0,MPI_COMM_WORLD);
        MPI_Send(high,hi,MPI_FLOAT,rank + (size/2),0,MPI_COMM_WORLD); 
        MPI_Recv(&rlw,1,MPI_INT,rank + (size/2),0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); 
        low = (float*)realloc(low,(lw+rlw)*sizeof(float));
        MPI_Recv(low+lw,rlw,MPI_FLOAT,rank + (size/2),0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);   
        lw=lw+rlw;     
      } 
      else{ 
       if(rank==(i+(size/2)))
       {  
          MPI_Recv(&rhi,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); 
          high = (float*)realloc(high,(hi+rhi)*sizeof(float));
          MPI_Recv(high+hi,rhi,MPI_FLOAT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
          MPI_Send(&lw,1,MPI_INT,i,0,MPI_COMM_WORLD);
          MPI_Send(low,lw,MPI_FLOAT,i,0,MPI_COMM_WORLD);    
          hi=hi+rhi;        
       }
      } 
   }
   for(i=1;i<(size/2);i++)
   {
      if(rank==0)
        {
           MPI_Recv(&rlw,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); 
           low = (float*)realloc(low,(lw+rlw)*sizeof(float));
           MPI_Recv(low+lw,rlw,MPI_FLOAT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);    
           lw = lw+rlw;         
        }
      else if(rank<(size/2))
        {
            MPI_Send(&lw,1,MPI_INT,0,0,MPI_COMM_WORLD);
            MPI_Send(low,lw,MPI_FLOAT,0,0,MPI_COMM_WORLD);
        }
      if(rank==(size/2))
        {
           MPI_Recv(&rhi,1,MPI_INT,i+(size/2),0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); 
           high = (float*)realloc(high,(hi+rhi)*sizeof(float));
           MPI_Recv(high+hi,rhi,MPI_FLOAT,i+(size/2),0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);    
           hi = hi+rhi;     
        }
      else if(rank>(size/2))
        {
            MPI_Send(&hi,1,MPI_INT,(size/2),0,MPI_COMM_WORLD);
            MPI_Send(high,hi,MPI_FLOAT,(size/2),0,MPI_COMM_WORLD);
        }  
   }
   if(rank==0)  
     {  
       quicksort(low,0,lw-1);
       printf("Sorted list:\n");
       for(i=0;i<lw;i++) printf("%.2f\n",low[i]);
       MPI_Recv(&hi,1,MPI_INT,(size/2),0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
       high = (float*)realloc(high,hi*sizeof(float));
       MPI_Recv(high,hi,MPI_FLOAT,(size/2),0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
       for(i=0;i<hi;i++) printf("%.2f\n",high[i]);
     }
   else if(rank==(size/2))
     {
       quicksort(high,0,hi-1);
       MPI_Send(&hi,1,MPI_INT,0,0,MPI_COMM_WORLD); 
       MPI_Send(high,hi,MPI_FLOAT,0,0,MPI_COMM_WORLD);  
     }
   MPI_Finalize();
   return 0;
}
