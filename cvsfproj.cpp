#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#define MAXSIZE 100
#define FILESIZE 1024
#define READ 4
#define WRITE 2
#define REGULAR 1
#define SPECIAL 2
#define CURRENT 1
#define START 0
#define END 2

//-----------------------------------------------------------------------------------------

//inode cha structure

typedef struct inode //like iit table
{
	char filename[50];
	int inodenumber;
	int filesize;
	int fileactualsize;
	int filetype;
	int linkcount;
	int referencecount;
	int permission;
	char*data;
	struct inode*next;
}INODE,*PINODE,**PPINODE;


typedef struct filetable
{
	int readoffset;
	int writeoffset;
	int count;
	int mode;
	PINODE ptr;
}ftobj,*pfiletable;


struct UFDT
{
	pfiletable ufdt[MAXSIZE];
}ufdtobj;

struct superblock
{
	int totalinode;
	int freeinode;
}superobj;


///////////////////////////////////////////////////////////////////////////////////////////


PINODE head=NULL;//global pointer
//-------------------------------------------------------------------------------------------

void createDILB()//create link list of inodes
{
	PINODE temp=head;
	int i=1;
	while(i<=MAXSIZE)//create 100 inode lisklist
	{
	PINODE newn=(PINODE)malloc(sizeof(INODE));
	newn->inodenumber=i;
	newn->filesize=FILESIZE;
	newn->fileactualsize=0;
	newn->filetype=0;
	newn->linkcount=0;
	newn->referencecount=0;
	newn->data=NULL;
	newn->next=NULL;
	
	if(temp==NULL)//first inode
	{
		head=newn;
		temp=head;
	}
	else
	{
		temp->next=newn;
		temp=temp->next;
	}
	i++;
	}
	printf("DILB successfully created !!!\n");
}

void createsuperblock()
{
	superobj.totalinode=MAXSIZE;
	superobj.freeinode=MAXSIZE;
	printf("super block  created  !!!\n");
}

void createUFDT()
{
	int i=0;
	for(i=0;i<MAXSIZE;i++)
	{
		ufdtobj.ufdt[i]=NULL;
	}
	printf("ufdt array created successfully !!!\n");
}
/////////////////////////////////////////////////////////////////////////////////////////////

//                            ALL FUNCTIONS
// checkfile function
bool checkfile(char*name)
{
	PINODE temp=head;
	
	while(temp!=NULL)
	{
		if(temp->filetype!=0)
		{
			if(strcmp(temp->filename,name)==0)
			{
				break;
			}
		}
		temp=temp->next;
	}
	if(temp==NULL)
	{
		return false;
	}
	else
	{
		return true;
	}
}

//create_file function
int create_file(char*name,int per)
{
	bool bret=false;
	if((name==NULL)||(per>READ+WRITE)||(per<WRITE))
	{
		return -1;
	}
	bret=checkfile(name);
	
	if(bret==true)
	{
		printf("file already present\n");
		return -1;
	}
	
	if(superobj.freeinode==0)
	{
		printf("there is no inode to create file\n");
		return -1;
	}
	//search in UFDT array for empty entry
	
	int i=0;
	for(i=0;i<MAXSIZE;i++)
	{
		if(ufdtobj.ufdt[i]==NULL)
		{
			break;
		}
	}
	
	if(i==MAXSIZE)
	{
		printf("unable to get entry in UFDT \n");
		return -1;
	}
	
	//allocate memory for file table
	
	ufdtobj.ufdt[i]=(pfiletable)malloc(sizeof(filetable));
	//initializing file table
	
	ufdtobj.ufdt[i]->readoffset=0;
	ufdtobj.ufdt[i]->writeoffset=0;
	ufdtobj.ufdt[i]->count=1;
	ufdtobj.ufdt[i]->mode=per;
	
	//search empty inode
	
	PINODE temp=head;
	while(temp!=NULL)
	{
		if(temp->filetype==0)
		{
			break;
		}
		temp=temp->next;
	}
	
	//initializing inode
	
	ufdtobj.ufdt[i]->ptr=temp;
	strcpy(ufdtobj.ufdt[i]->ptr->filename,name);
	ufdtobj.ufdt[i]->ptr->filesize=FILESIZE;
	ufdtobj.ufdt[i]->ptr->fileactualsize=0;
	ufdtobj.ufdt[i]->ptr->filetype=REGULAR;
	ufdtobj.ufdt[i]->ptr->permission=per;
	ufdtobj.ufdt[i]->ptr->linkcount=1;
	ufdtobj.ufdt[i]->ptr->referencecount=1;
	ufdtobj.ufdt[i]->ptr->data=(char*)malloc(sizeof(FILESIZE));//allocation of memory for data
	superobj.freeinode--;
	return i;
}

//ls funtion

void ls_file()
{
	PINODE temp=head;
	
	while(temp!=NULL)
	{
		if(temp->filetype!=0)
		{
		printf("		%s	%d\n",temp->filename,temp->inodenumber);
		}
		temp=temp->next;
	}
	
}

//get fd by name function
int getFDfromname(char *name)
{
	int i=0;
	while(i<100)//coz ufdt 0 paun suru hotat
	{
		if(ufdtobj.ufdt[i]!=NULL)
		{
			if(strcmp(ufdtobj.ufdt[i]->ptr->filename,name)==0)
			{
				break;
			}
		}
		i++;
	}
	if(i==100)//coz ufdt array 0 value to 99 travel karto
	return -1;
	else
	return i;	
}
//to get inode from name
PINODE getinode(char*name)
{
	PINODE temp=head;
	int i=0;
	if(name==NULL)
	{
		return NULL;
	}
	while(temp!=NULL)
	{
		if(strcmp(name,temp->filename)==0)
		{
			break;
		}
		temp=temp->next;
	}
	return temp;
}

//open funtion

int openfile(char*name,int mode)
{
	int i=0;
	PINODE temp1=NULL;
	if((name==NULL)||(mode<=0))
	{
		return -1;
		
	}
	temp1=getinode(name);
	if(temp1==NULL)
	{
		return -2;
	}
	if(temp1->permission<mode)
	{
		return -3;
	}
	
	while(i<MAXSIZE)//find empty ufdt[i]
	{
		if(ufdtobj.ufdt[i]==NULL)
		{
			break;
		}
		i++;
	}
	ufdtobj.ufdt[i]=(pfiletable)malloc(sizeof(ftobj));//allocate memory to store address to file table contents
	
	if(ufdtobj.ufdt[i]==NULL)
	{
		return -1;
	}
	
	ufdtobj.ufdt[i]->count=1;
	ufdtobj.ufdt[i]->mode=mode;
	if(mode==READ+WRITE)
	{
		ufdtobj.ufdt[i]->readoffset=0;
		ufdtobj.ufdt[i]->writeoffset=0;
	}
	else if(mode==WRITE)
	{
		ufdtobj.ufdt[i]->writeoffset=0;
	}
	else if(mode==READ)
	{
		ufdtobj.ufdt[i]->readoffset=0;
	}	
	ufdtobj.ufdt[i]->ptr=temp1;//ufdt[i] to filetable ptr pointiing to temp
	(ufdtobj.ufdt[i]->ptr->referencecount)++;
	return i;
}

//close file by name

int closefilebyname(char*name)
{
	int i;
	i=getFDfromname(name);
	ufdtobj.ufdt[i]->readoffset=0;
	ufdtobj.ufdt[i]->writeoffset=0;
	(ufdtobj.ufdt[i]->ptr->referencecount)--;
	return 1;
}

//close all file 

void closeallfiles()
{
	int i=0;
	while(i<MAXSIZE)
	{
		if(ufdtobj.ufdt[i]!=NULL)
		{
			ufdtobj.ufdt[i]->readoffset=0;
			ufdtobj.ufdt[i]->writeoffset=0;
			(ufdtobj.ufdt[i]->ptr->referencecount)--;
	
		}
		i++;
	}
}


//stat file function
int statfile(char *name)
{
	PINODE temp=head;
	bool bret=false;
	if(name==NULL)
	{
		return -1;
	}
	bret=checkfile(name);
	if(bret==false)
	{
		return -2;
	}
	while(temp!=NULL)
	{
		if(strcmp(temp->filename,name)==0)
		{
			break;
		}
		temp=temp->next;
	}
	printf("-----------------------------------------------------------------------\n");
	printf("Statistical infomation of file\n");
	printf("File name: %s\n",temp->filename);
	printf("File inode numb: %d\n",temp->inodenumber);
	printf("File size:%d\n",temp->filesize);
	printf("File type:%d\n",temp->filetype);
	printf("File actual size : %d\n",temp->fileactualsize);
	printf("File Link count: %d\n",temp->linkcount);
	printf("Reference count:%d\n",temp->referencecount);
	if(temp->permission==1)
	{
		printf("File permission: Read only\n");
	}
	else if(temp->permission==2)
	{
		printf("File permission: Write only\n");
	}
	else if(temp->permission==3)
	{
		printf("File permission: Read and Write only\n");
	}
	printf("-----------------------------------------------------------------------\n");
}


//fstat file function
int fstatfile(int fd)
{
	PINODE temp=head;
	int i=0;
	if(fd<0)
	{
		return -1;
	}
	if(ufdtobj.ufdt[fd]==NULL)
	{
		return -2;
	}
	temp=ufdtobj.ufdt[fd]->ptr;
	printf("-----------------------------------------------------------------------\n");
	printf("Statistical infomation of file\n");
	printf("File name: %s\n",temp->filename);
	printf("File inode numb: %d\n",temp->inodenumber);
	printf("File size:%d\n",temp->filesize);
	printf("File type:%d\n",temp->filetype);
	printf("File actual size : %d\n",temp->fileactualsize);
	printf("File Link count: %d\n",temp->linkcount);
	printf("Reference count:%d\n",temp->referencecount);
	if(temp->permission==1)
	{
		printf("File permission: Read only\n");
	}
	else if(temp->permission==2)
	{
		printf("File permission: Write only\n");
	}
	else if(temp->permission==3)
	{
		printf("File permission: Read and Write only\n");
	}
	printf("-----------------------------------------------------------------------\n");
}
//rm function
int rmfile(char *name)
{
	int fd=0;
	fd=getFDfromname(name);
	if(fd==-1)
	{
		return -1;
	}
	(ufdtobj.ufdt[fd]->ptr->linkcount)--;
	if((ufdtobj.ufdt[fd]->ptr->linkcount)==0)
	{
		ufdtobj.ufdt[fd]->ptr->filetype=0;
		free(ufdtobj.ufdt[fd]->ptr->data);
	}
	ufdtobj.ufdt[fd]=NULL;
	(superobj.freeinode)++;	
}

//write function

int writefile(int fd,char*arr,int isize)
{
	
	if(((ufdtobj.ufdt[fd]->mode)!=WRITE)&&((ufdtobj.ufdt[fd]->mode)!=(WRITE+READ)))
	{
		return -1;
	}
	
	if(((ufdtobj.ufdt[fd]->ptr->permission)!=WRITE)&&((ufdtobj.ufdt[fd]->ptr->permission)!=(WRITE+READ)))
	{
		return -1;
	}
	
	if(ufdtobj.ufdt[fd]->writeoffset==FILESIZE)
	{
		return -2;
	}
	
	if(ufdtobj.ufdt[fd]->ptr->filetype!=REGULAR)
	{
		return -3;
	}
	
	strncpy(((ufdtobj.ufdt[fd]->ptr->data)+(ufdtobj.ufdt[fd]->writeoffset)),arr,isize);
	//		strncpy(destination,source,size)								
	(ufdtobj.ufdt[fd]->ptr->fileactualsize)=(ufdtobj.ufdt[fd]->ptr->fileactualsize)+isize;
	return isize;
}

//read function

int readfile(int fd,char*arr,int isize)
{	
	//printf(" ufdtobj.ufdt[fd]->readoffset before read %d\n",ufdtobj.ufdt[fd]->readoffset);
	int readsize=0;
	
	if(ufdtobj.ufdt[fd]==NULL)
	{
		return -1;
	}
	if((ufdtobj.ufdt[fd]->mode!=READ)&&(ufdtobj.ufdt[fd]->mode!=READ+WRITE))
	{
		return -2;
	}
	if((ufdtobj.ufdt[fd]->ptr->permission!=READ)&&(ufdtobj.ufdt[fd]->ptr->permission!=READ+WRITE))
	{
		return -2;
	}
	if(ufdtobj.ufdt[fd]->readoffset==ufdtobj.ufdt[fd]->ptr->fileactualsize)
	{
		return -3;
	}
	if(ufdtobj.ufdt[fd]->ptr->filetype!=REGULAR)
	{
		return -4;
	}
	readsize=((ufdtobj.ufdt[fd]->ptr->filesize)-(ufdtobj.ufdt[fd]->readoffset));
	//printf("given size: %d\n",isize);
	//printf("read size: %d\n",readsize);
	if(readsize<isize)
	{
		//ufdtobj.ufdt[fd]->readoffset=0;//to read content everytime from offset 1
		strncpy(arr,((ufdtobj.ufdt[fd]->ptr->data)+(ufdtobj.ufdt[fd]->readoffset)),isize);
		ufdtobj.ufdt[fd]->readoffset=ufdtobj.ufdt[fd]->readoffset+readsize;
	}
	else
	{
		//ufdtobj.ufdt[fd]->readoffset=0;//to read content everytime from offset 1
		//printf("read offset %d %s\n",ufdtobj.ufdt[fd]->readoffset,ufdtobj.ufdt[fd]->ptr->data);
		strncpy(arr,((ufdtobj.ufdt[fd]->ptr->data)+(ufdtobj.ufdt[fd]->readoffset)),isize);
		ufdtobj.ufdt[fd]->readoffset=ufdtobj.ufdt[fd]->readoffset+isize;
	}
	//printf("given size: %d\n",isize);
	printf(" ufdtobj.ufdt[fd]->readoffset after read %d\n",ufdtobj.ufdt[fd]->readoffset);
	return isize;
}

// lseek function

int lseekfile(int fd,int size,int from)
{
	if((fd<0)||(from>2))
	{
		return -1;
	}
	if(ufdtobj.ufdt[fd]==NULL)
	{
		return -1;
	}
	if((ufdtobj.ufdt[fd]->mode==READ)||(ufdtobj.ufdt[fd]->mode==READ+WRITE))
	{
		/*if(from==CURRENT)
		{	printf(" READ from %d\n",from);
			if(((ufdtobj.ufdt[fd]->readoffset)+size)>ufdtobj.ufdt[fd]->ptr->fileactualsize)
			{
				return -1;
			}
		}		
		else*/if(from==START)
		{	
			printf(" READ from %d\n",from);
			((ufdtobj.ufdt[fd])->readoffset)=0;
			printf(" READ from 0 readoffset %d\n",((ufdtobj.ufdt[fd])->readoffset));
			if(size>(ufdtobj.ufdt[fd]->ptr->fileactualsize))
			{
				return -1;
			}
			if(size>0)
			{
				
				return -1;
			}
			(ufdtobj.ufdt[fd]->readoffset)=0;
			printf(" ufdtobj.ufdt[fd]->readoffset lseek %d\n",(ufdtobj.ufdt[fd])->readoffset);
		}
		else if(from==END)
		{
			if(((ufdtobj.ufdt[fd]->ptr->fileactualsize)+size)>FILESIZE)
			{
				return -1;
			}
			if(((ufdtobj.ufdt[fd]->readoffset)+size)<0)
			{
				return -1;
			}
			(ufdtobj.ufdt[fd]->readoffset)=(ufdtobj.ufdt[fd]->ptr->fileactualsize)+size;
		}
	printf(" READ from 0 readoffset last pf %d\n",((ufdtobj.ufdt[fd])->readoffset));
	}
	else if(ufdtobj.ufdt[fd]->mode==WRITE)
	{
		if(from==CURRENT)
		{
			if(((ufdtobj.ufdt[fd]->writeoffset)+size)<0)
			{
				return -1;
			}
			if(((ufdtobj.ufdt[fd]->writeoffset)+size)<0)
			{
				return -1;
			}
			if(((ufdtobj.ufdt[fd]->writeoffset)+size)>(ufdtobj.ufdt[fd]->ptr->fileactualsize))
			{
				(ufdtobj.ufdt[fd]->ptr->fileactualsize)=(ufdtobj.ufdt[fd]->writeoffset)+size;
				(ufdtobj.ufdt[fd]->writeoffset)=(ufdtobj.ufdt[fd]->writeoffset)+size;
			}
		}
		else if(from==START)
		{
			if(size>FILESIZE)
			{
				return -1;
			}
			if(size<0)
			{
				return -1;
			}
			if(size>(ufdtobj.ufdt[fd]->ptr->fileactualsize))
			{
				(ufdtobj.ufdt[fd]->ptr->fileactualsize)=size;
				(ufdtobj.ufdt[fd]->writeoffset)=size;
			}
		}
		else if(from==END)
		{
			if(((ufdtobj.ufdt[fd]->ptr->fileactualsize)+size)>FILESIZE)
			{
				return -1;
			}
			if(((ufdtobj.ufdt[fd]->writeoffset)+size)<0)
			{
				return -1;
			}
			(ufdtobj.ufdt[fd]->writeoffset)=(ufdtobj.ufdt[fd]->ptr->fileactualsize)+size;
		}
	}
			
}

///////////////////////////////////////////////////////////////////////////////////////////////

void set_environment()
{
	createDILB();
	createsuperblock();
	createUFDT();
	printf("Environment set successfully!!!\n");
} 

///////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	//variable diclaration
	
	char str[80];
	char command[4][80];
	int count=0;
	int ret=0;
	int fd=0;
	char arr[FILESIZE];
	printf("Cosiomized Virtual File System\n");
	
	set_environment();
	
	while(1)
	{
	printf("Milind VFS :> ");
	fgets(str,80,stdin);
	fflush(stdin);
		
	count=sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);	
	
	if(count==1)
	{
		/*if(strcmp(command[0],"help")==0)
		{
			Displayhelp();
		}
		else*/if(strcmp(command[0],"ls")==0)
		{
			ls_file();
		}		
		else if(strcmp(command[0],"closeall")==0)
		{
			closeallfiles();
			printf("all files close successfully !!!\n");
			continue;
		}
		else if(strcmp(command[0],"clear")==0)
		{
			system("cls");
			continue;
		}
		else if(strcmp(command[0],"exit")==0)
		{
			printf("Terminating Cosiomized Virtual File System\n");
			break;
		}
		else 
		{
			printf("\n Error : Command Not Found\n");
			continue;
		}
		
	}
	
	else if(count==2)
	{
		/*if(strcmp(command[0],"man") == 0)   // man open
        {
            ManPage(command[1]);
        }
		else*/ if(strcmp(command[0],"stat") == 0)
		{
			ret = statfile(command[1]);
				if(ret == -1)
					printf("ERROR : Incorrect parameters\n");
				if(ret == -2)
					printf("ERROR : There is no such file\n");
					continue;
		}
		else if(strcmp(command[0],"fstat") == 0)
		{
			ret = fstatfile(atoi(command[1]));
			if(ret == -1)
				printf("ERROR : Incorrect parameters\n");
			if(ret == -2)
			printf("ERROR : There is no such file\n");
			continue;
		}
		else if(strcmp(command[0],"close") == 0)
		{
			ret=closefilebyname(command[1]);
			if(ret==-1)
			{
				printf("there is no such file !!!\n");
			}
			if(ret==1)
			{
				printf("file closed successfully !!!\n");
			}
			continue;
			
		}
		
		else if(strcmp(command[0],"rm") == 0)
		{
			int ret=0;
			ret=rmfile(command[1]);
			if(ret==-1)
			{
				printf("error :unable to remove file.\n");
			}
			else
			{
				printf("File deleted successfully\n");
			}
		}
		
		else if(strcmp(command[0],"write") == 0)
		{
			fd=getFDfromname(command[1]);
			if(fd==-1)
			{
				printf("error : incorect parameter !!!\n");
				continue;
			}
			printf("Enter the data\n");
			scanf("%[^'\n']s",arr);
			ret=strlen(arr);//to check weather array is empty
			if(ret==0)
			{
				printf("error : Incorrect parameter !!!\n");
				continue;
			}
			ret=writefile(fd,arr,ret);
			if(ret==-1)
			{
				printf("permission denied \n");
				
			}
			if(ret==-2)
			{
				printf("there is no sufficient memory to write \n");
			}
			if(ret==-3)
			{
				printf("It is not a regular file \n");
			}
			if(ret>0)
			{
				printf("data successfully written \n");
				continue;
			}
			
		}
		else if(strcmp(command[0],"truncate") == 0)
		{
		}
        
        {
            printf("Command not found !!\n");
        }
        
	}

	else if(count==3)
	{
		if(strcmp(command[0],"creat")==0)
		{
			int ret=0;
			ret=create_file(command[1],atoi(command[2]));
			if(ret>=0)
			{
					printf("File successfully created with fd %d\n",ret);
			}
			if(ret==-1)
			{
					printf("Error incorrect parameters\n");
			}
			if(ret==-2)
			{
					printf("There are no inodes available\n");
			}
			if(ret==-3)
			{
					printf("File already exist\n");
			}
			if(ret==-4)
			{
					printf("Memory allocation failure\n");
			}
			continue;
			
		}
		else if(strcmp(command[0],"open")==0)
		{
			int ret=0;
			ret=openfile(command[1],atoi(command[2]));
			if(ret>0)
			{
				printf("file successfully opened !!!\n");
				
			}
			if(ret==-1)
			{
				printf("error : unable to open file !!!\n");
			}
			continue;
			
		}
		else if(strcmp(command[0],"read")==0)
		{
			char*ptr=NULL;
			fd=getFDfromname(command[1]);
			
			if(fd==-1)
			{
				printf("Error : incorect parameter \n");	
			}
			ptr=(char*)malloc(sizeof(atoi(command[2])+1));
			if(ptr==NULL)
			{
				printf("Error : memory allocation failure !!!\n");
			}
			ret=readfile(fd,ptr,atoi(command[2]));
			if(ret==-1)
			{
				printf("Error : File does not exist\n");
			}
			if(ret==-2)
			{
				printf("Error : permission denied\n");
			}
			if(ret==-3)
			{
				printf("Error : reached at the end of the file\n");
			}
			if(ret==-4)
			{
				printf("ERROR : File not regular \n");
			}
			if(ret==0)
			{
				printf("Erroe : File empty\n");
			}
			if(ret>0)
			{
				printf("\n");
				write(2,ptr,ret);
			}
			continue;
			
		}
		
    }
	
	else if(count==4)
	{	int ret=0;
		if(strcmp(command[0],"lseek")==0)
		{
			fd=getFDfromname(command[1]);
			if(fd==-1)
			{
				printf("Error : incorrexct parameter !!! \n");
			}
			continue;
			ret=lseekfile(fd,atoi(command[2]),atoi(command[3]));
			printf("***%d\n***",ret);
			if(ret==-1)
			{
				printf("Error : unable to perform lseek \n");
			}
			
			
		}
		
		
	}
	else 
	{
		printf("Bad command or file name\n");
	}
	
	
}
}